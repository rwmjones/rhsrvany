/* RHSrvAny - Turn any Windows program into a Windows service.
 * Written by Yuval Kashtan.
 * Copyright (C) 2010,2013 Red Hat Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#else
/* Assume we're not using autoconf, eg. for Visual C++. */
#define HAVE_STRSAFE 1
#define HAVE_STRINGCCHPRINTF 1
#endif /* HAVE_CONFIG_H */

#include <windows.h>

#include <stdio.h>
#include <stdlib.h>

#ifdef HAVE_STRSAFE
#include <strsafe.h>
#endif

#include <tchar.h>

#include "RHSrvAny.h"

static TCHAR *svcname = TEXT("RHSrvAny");

static SERVICE_STATUS gSvcStatus;
static HANDLE ghSvcStopEvent = NULL;
static SERVICE_STATUS_HANDLE gSvcStatusHandle;

static int SvcInstall (void);
static int SvcUninstall (void);

VOID WINAPI SvcCtrlHandler (DWORD);
VOID WINAPI SvcMain (DWORD, LPTSTR *);

static VOID SvcReportEvent (LPTSTR);
static VOID SvcInit (DWORD, LPTSTR *);
static VOID ReportSvcStatus (DWORD, DWORD, DWORD);

static int
argument_error (const char *msg)
{
    printf("%s\n", msg);

    return EXIT_FAILURE;
}

int
main (int argc, char **a_argv)
{
    /* For compatibility with MinGW, see:
    http://demosten-eng.blogspot.com/2008/08/mingw-and-unicode-support.html */
    TCHAR **argv;
    argv = CommandLineToArgvW (GetCommandLineW (), &argc);

    size_t i;
    for (i = 1; i < argc; i++) {
        TCHAR *arg = argv[i];

        if (arg[0] == _T('-')) {
            if (lstrcmpi(arg + 1, _T("s")) == 0) {
                if (i == argc) {
                    return argument_error("Option -s requires an argument");
                }

                svcname = argv[++i];
            }

            else {
                return argument_error("Unknown option");
            }
        }

        /* Stop parsing arguments when we hit something which isn't an option */
        else {
            break;
        }
    }

    if (lstrcmpi(argv[i], TEXT("install")) == 0) {
        return SvcInstall();
    } else if (lstrcmpi(argv[i], TEXT("uninstall")) == 0) {
        return SvcUninstall();
    }

    SERVICE_TABLE_ENTRY DispatchTable[] = {
        {
            svcname,
            (LPSERVICE_MAIN_FUNCTION) SvcMain
        },
        { NULL, NULL }
    };

    if (!StartServiceCtrlDispatcher( DispatchTable ))
    {
        SvcReportEvent(TEXT("StartServiceCtrlDispatcher"));
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

static int
SvcUninstall() {
    SC_HANDLE schSCManager = NULL;
    SC_HANDLE schService = NULL;

    schSCManager = OpenSCManager(
        NULL,
        NULL,
        SC_MANAGER_ALL_ACCESS
    );

    if (NULL == schSCManager) {
        printf("OpenSCManager failed (%d)\n", (int) GetLastError());
        return EXIT_FAILURE;
    }

    schService = OpenService(
        schSCManager,
        svcname,
        SERVICE_ALL_ACCESS
    );

    if (schService == NULL) {
        DWORD err = GetLastError();
        switch (err) {
        case ERROR_ACCESS_DENIED:
            printf("You do not have permission to uninstall this service\n");
            break;

        case ERROR_SERVICE_DOES_NOT_EXIST:
            printf("The service does not exist\n");
            break;

        default:
            printf("OpenService failed (%d)\n", (int) err);
        }

        goto error;
    }

    if (DeleteService(schService) == 0) {
        printf("DeleteService failed (%d)\n", (int) GetLastError());
        goto error;
    } else {
        printf("Service uninstalled successfully\n");
    }

    CloseServiceHandle (schService);
    CloseServiceHandle (schSCManager);

    return EXIT_SUCCESS;

error:
    if (schService) CloseServiceHandle (schService);
    if (schSCManager) CloseServiceHandle (schSCManager);

    return EXIT_FAILURE;
}

static int
SvcInstall() {
    SC_HANDLE schSCManager;
    SC_HANDLE schService;
    TCHAR szPath[MAX_PATH];
    wchar_t imagePath[MAX_PATH];

    schSCManager = OpenSCManager(
        NULL,
        NULL,
        SC_MANAGER_ALL_ACCESS
    );

    if (NULL == schSCManager) {
        printf("OpenSCManager failed (%d)\n", (int) GetLastError());
        return EXIT_FAILURE;
    }

    /* Get the full path of the current executable in szPath */
    if (GetModuleFileName(NULL, szPath, MAX_PATH) == 0) {
        printf("GetModuleFileName failed (%d)\n", (int) GetLastError());
        return EXIT_FAILURE;
    }

    /* Construct ImagePath, which is actually a command line in this instance */
    if (snwprintf(imagePath, MAX_PATH, L"%s -s %s", szPath, svcname) >= MAX_PATH)
    {
        printf("ImagePath exceeded %d characters\n", MAX_PATH);
        return EXIT_FAILURE;
    }

    schService = CreateService (
        schSCManager,
        svcname,
        svcname,
        SERVICE_ALL_ACCESS,
        SERVICE_WIN32_OWN_PROCESS,
        SERVICE_AUTO_START,
        SERVICE_ERROR_NORMAL,
        imagePath,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
    );

    if (schService == NULL) {
        DWORD err = GetLastError();
        switch (err) {
        case ERROR_SERVICE_EXISTS:
            printf("A service with this name already exists\n");
            break;

        default:
            printf("CreateService failed (%d)\n", (int) err);
        }

        CloseServiceHandle (schSCManager);
        return EXIT_FAILURE;
    }
    else {
        printf("Service installed successfully\n");
    }

    CloseServiceHandle (schService);
    CloseServiceHandle (schSCManager);

    return EXIT_SUCCESS;
}

VOID
WINAPI
SvcMain (
    DWORD dwArgc,
    LPTSTR *lpszArgv
) {
    gSvcStatusHandle = RegisterServiceCtrlHandler (
        svcname,
        SvcCtrlHandler
    );

    if (!gSvcStatusHandle) {
        SvcReportEvent(TEXT("RegisterServiceCtrlHandler"));
        return;
    }

    gSvcStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    gSvcStatus.dwServiceSpecificExitCode = 0;

    ReportSvcStatus (
        SERVICE_START_PENDING,
        NO_ERROR,
        3000
    );

    SvcInit (
        dwArgc,
        lpszArgv
    );
}

static BOOL
RegistryRead (
    HKEY hHive,
    wchar_t *szKeyPath,
    wchar_t *szValue,
    wchar_t *szData,
    DWORD *nSize
) {
    HKEY hKey;
    long lSuccess;

    lSuccess = RegOpenKey (
        hHive,
        szKeyPath,
        &hKey
    );

    if (lSuccess == ERROR_SUCCESS) {
        lSuccess = RegQueryValueEx (
            hKey,
            szValue,
            NULL,
            NULL,
            (LPBYTE) szData,
            nSize
        );

        if (lSuccess == ERROR_SUCCESS) {
            return (TRUE);
        }
    }

    return (FALSE);
}

static VOID
SvcInit (
    DWORD dwArgc,
    LPTSTR *lpszArgv
) {
    DWORD nSize;
    BOOL fSuccess;
    STARTUPINFO si;
    wchar_t szPWD[1024];
    PROCESS_INFORMATION pi;
    wchar_t szCmdLine[1024];
    wchar_t szRegistryPath[1024];

    // TO_DO: Declare and set any required variables.
    //   Be sure to periodically call ReportSvcStatus() with
    //   SERVICE_START_PENDING. If initialization fails, call
    //   ReportSvcStatus with SERVICE_STOPPED.

    // Create an event. The control handler function, SvcCtrlHandler,
    // signals this event when it receives the stop control code.

    ghSvcStopEvent = CreateEvent (
        NULL,
        TRUE,
        FALSE,
        NULL
    );

    if (ghSvcStopEvent == NULL) {
        ReportSvcStatus (
            SERVICE_STOPPED,
            NO_ERROR,
            0
        );
        return;
    }

    // Report running status when initialization is complete.
    ReportSvcStatus (
        SERVICE_RUNNING,
        NO_ERROR,
        0
    );

    // TO_DO: Perform work until service stops.
    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );
    nSize=1024;

    snwprintf (
        szRegistryPath,
        sizeof szRegistryPath,
        L"SYSTEM\\CurrentControlSet\\services\\%s\\Parameters",
        svcname
    );

    fSuccess = RegistryRead (
        HKEY_LOCAL_MACHINE,
        szRegistryPath,
        L"CommandLine",
        szCmdLine,
        &nSize
    );

    if (fSuccess) {
        fSuccess = RegistryRead (
            HKEY_LOCAL_MACHINE,
            szRegistryPath,
            L"PWD",
            szPWD,
            &nSize
        );
    }

    if (fSuccess) {
        fSuccess = CreateProcess (
            NULL,
            szCmdLine,
            NULL,
            NULL,
            FALSE,
            (
                CREATE_NO_WINDOW
            ),
            NULL,
            szPWD,
            &si,
            &pi
        );
    }

    // treat errors

    while(1)
    {
        // Check whether to stop the service.
        WaitForSingleObject (
            ghSvcStopEvent,
            INFINITE
        );

        ReportSvcStatus (
            SERVICE_STOPPED,
            NO_ERROR,
            0
        );

        return;
    }
}

static VOID
ReportSvcStatus (
    DWORD dwCurrentState,
    DWORD dwWin32ExitCode,
    DWORD dwWaitHint
) {
    static DWORD dwCheckPoint = 1;

    // Fill in the SERVICE_STATUS structure.
    gSvcStatus.dwCurrentState = dwCurrentState;
    gSvcStatus.dwWin32ExitCode = dwWin32ExitCode;
    gSvcStatus.dwWaitHint = dwWaitHint;

    if (dwCurrentState == SERVICE_START_PENDING) {
        gSvcStatus.dwControlsAccepted = 0;
    }
    else {
        gSvcStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
    }

    if (
        (dwCurrentState == SERVICE_RUNNING) ||
        (dwCurrentState == SERVICE_STOPPED)
    ) {
        gSvcStatus.dwCheckPoint = 0;
    } else {
        gSvcStatus.dwCheckPoint = dwCheckPoint++;
    }

    // Report the status of the service to the SCM.
    SetServiceStatus (
        gSvcStatusHandle,
        &gSvcStatus
    );
}

VOID
WINAPI
SvcCtrlHandler (
    DWORD dwCtrl
) {
   switch(dwCtrl) {
      case SERVICE_CONTROL_STOP:
         ReportSvcStatus(SERVICE_STOP_PENDING, NO_ERROR, 0);

         /* Signal the service to stop. */
         SetEvent(ghSvcStopEvent);

         return;

      case SERVICE_CONTROL_INTERROGATE:
         /* Fall through to send current status. */
         break;

      default:
         break;
   }

   ReportSvcStatus(gSvcStatus.dwCurrentState, NO_ERROR, 0);
}

/* Logs messages to the event log */
static VOID
SvcReportEvent (
    LPTSTR szFunction
) {
    TCHAR Buffer[80];
    HANDLE hEventSource;
    LPCTSTR lpszStrings[2];

    hEventSource = RegisterEventSource (
        NULL,
        svcname
    );

    if (
        NULL != hEventSource
    ) {
#ifdef HAVE_STRINGCCHPRINTF
        StringCchPrintf
#else
        snwprintf
#endif
                  (
            Buffer,
            80,
            TEXT("%s failed with %d"),
            szFunction,
            GetLastError()
        );

        lpszStrings[0] = svcname;
        lpszStrings[1] = Buffer;

        ReportEvent (
            hEventSource,
            EVENTLOG_ERROR_TYPE,
            0,
            SVC_ERROR,
            NULL,
            2,
            0,
            lpszStrings,
            NULL
        );

        DeregisterEventSource (hEventSource);
    }
}
