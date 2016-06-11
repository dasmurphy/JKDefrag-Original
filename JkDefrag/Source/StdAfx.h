
#ifndef __STDAFX_H__
#define __STDAFX_H__

#define _WIN32_WINNT 0x0500

#include <windows.h>
#include <stdio.h>
#include <wchar.h>
#include <time.h>
#include <sys/timeb.h>
#include <gdiplus.h>

using namespace Gdiplus;

#include <tlhelp32.h>                  /* CreateToolhelp32Snapshot() */

#ifdef _DEBUG
#include <dbghelp.h>                   /* SetUnhandledExceptionFilter() */
#endif

#include "JKDefragStruct.h"
#include "JkDefragLib.h"
#include "JKDefragLog.h"
#include "ScanFat.h"
#include "ScanNtfs.h"
#include "JkDefragGui.h"

#endif // __STDAFX_H__
