/* pnp_wait - wait for PnP installation activities to complete.
 * Author: Roman Kagan <rkagan@virtuozzo.com>
 * Copyright (C) 2016 Parallels IP Holdings GmbH.
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

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <windows.h>
#include <cfgmgr32.h>

#define RET_SUCCESS	0
#define RET_TIMEOUT	1
#define RET_ERROR	2

static const char *waitres2str(int res)
{
	switch (res) {
	case WAIT_OBJECT_0:
		return "done";
	case WAIT_TIMEOUT:
		return "timed out";
	default:
		return "error";
	}
}

static int waitres2retcode(int res)
{
	switch (res) {
	case WAIT_OBJECT_0:
		return RET_SUCCESS;
	case WAIT_TIMEOUT:
		return RET_TIMEOUT;
	default:
		return RET_ERROR;
	}
}

void usage(FILE *fp, const char *cmd)
{
	fprintf(fp,
		"Usage: %s [-h|--help] [TIMEOUT]\n"
		"Wait for PnP activities to complete\n"
		"\n"
		"  -h,--help   show this message\n"
		"  TIMEOUT     timeout in ms (default: wait forever)\n"
		"\n"
		"exit code:\n"
		"  %d          %s\n"
		"  %d          %s\n"
		"  %d          %s\n",
		cmd,
		waitres2retcode(WAIT_OBJECT_0), waitres2str(WAIT_OBJECT_0),
		waitres2retcode(WAIT_TIMEOUT), waitres2str(WAIT_TIMEOUT),
		waitres2retcode(WAIT_FAILED), waitres2str(WAIT_FAILED));
}

int main(int argc, char **argv)
{
	int ret, i;
	time_t t;
	unsigned tmo = INFINITE;
	const char *prog = argv[0];

	for (i = 1; i < argc; i++) {
		const char *s = argv[i];
		char *e;

		if (!strcmp(s, "-h") || !strcmp(s, "--help")) {
			usage(stdout, prog);
			return RET_SUCCESS;
		}

		tmo = strtoul(s, &e, 0);
		if (*e == '\0' && e != s)
			continue;

		fprintf(stderr, "failed to parse parameter: \"%s\"\n", s);
		usage(stderr, prog);
		return RET_ERROR;
	}

	t = time(NULL);
	printf("start waiting for PnP to complete @ %s", ctime(&t));

	ret = CMP_WaitNoPendingInstallEvents(tmo);

	t = time(NULL);
	printf("%s waiting for PnP to complete @ %s", waitres2str(ret),
	       ctime(&t));
	return waitres2retcode(ret);
}
