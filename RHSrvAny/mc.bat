rem RHSrvAny - Turn any Windows program into a Windows service.
rem Written by Yuval Kashtan.
rem Copyright (C) 2010 Red Hat Inc.
rem
rem This program is free software; you can redistribute it and/or modify
rem it under the terms of the GNU General Public License as published by
rem the Free Software Foundation; either version 2 of the License, or
rem (at your option) any later version.
rem
rem This program is distributed in the hope that it will be useful,
rem but WITHOUT ANY WARRANTY; without even the implied warranty of
rem MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
rem GNU General Public License for more details.
rem
rem You should have received a copy of the GNU General Public License
rem along with this program; if not, write to the Free Software
rem Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

mc.exe -U RHSrvAny.mc
rc.exe -r RHSrvAny.rc
link /dll /noentry /MACHINE:x86 /out:RHSrvAny.dll RHSrvAny.res