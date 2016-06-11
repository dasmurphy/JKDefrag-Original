/*
JkDefrag  --  Defragment and optimize all harddisks.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

For the full text of the license see the "License gpl.txt" file.

Jeroen C. Kessels
Internet Engineer
http://www.kessels.com/
*/

#include "StdAfx.h"

#include "JKDefrag.h"

JKDefrag *JKDefrag::m_jkDefrag = 0;

JKDefrag::JKDefrag()
{
	Running = STOPPED;

	Debug = 1;

	m_jkGui = JKDefragGui::getInstance();
	m_jkLib = JKDefragLib::getInstance();

	m_jkLog = new JKDefragLog();
	m_jkStruct = new JKDefragStruct();
}

JKDefrag::~JKDefrag()
{
	delete m_jkLog;
	delete m_jkStruct;

	delete m_jkDefrag;
}

JKDefrag *JKDefrag::getInstance()
{
	if (m_jkDefrag == NULL)
	{
		m_jkDefrag = new JKDefrag();
	}

	return m_jkDefrag;
}

void JKDefrag::releaseInstance()
{
	if (m_jkDefrag != NULL)
	{
		delete m_jkDefrag;
	}
}

WPARAM JKDefrag::startProgram(HINSTANCE hInstance,
							  HINSTANCE hPrevInstance,
							  LPSTR lpCmdLine,
							  int nCmdShow)
{
	IamRunning = RUNNING;

	/* Test if another instance is already running. */
	if (AlreadyRunning() == YES) return(0);

#ifdef _DEBUG
	/* Setup crash report handler. */
	SetUnhandledExceptionFilter(&JKDefrag::CrashReport);
#endif

	m_jkGui->Initialize(hInstance,nCmdShow, m_jkLog, Debug);

	/* Start up the defragmentation and timer threads. */
	if (CreateThread(NULL, 0, &JKDefrag::DefragThread, NULL, 0, NULL) == NULL) return(0);

	WPARAM wParam = m_jkGui->DoModal();

	/* If the defragger is still running then ask & wait for it to stop. */
	IamRunning = STOPPED;

	m_jkLib->StopJkDefrag(&Running,0);

	return wParam;
}


#ifdef _DEBUG

/*

Write a crash report to the log.
To test the crash handler add something like this:
char *p1;
p1 = 0;
*p1 = 0;

*/

LONG __stdcall JKDefrag::CrashReport(EXCEPTION_POINTERS *ExceptionInfo)
{
	IMAGEHLP_LINE64 SourceLine;
	DWORD LineDisplacement;
	STACKFRAME64 StackFrame;
	DWORD ImageType;
	BOOL Result;
	int FrameNumber;
	char s1[BUFSIZ];
	WCHAR s2[BUFSIZ];

	JKDefragLog *jkLog = m_jkDefrag->m_jkLog;
	JKDefragLib *jkLib = m_jkDefrag->m_jkLib;

	/* Exit if we're running inside a debugger. */
	//  if (IsDebuggerPresent() == TRUE) return(EXCEPTION_EXECUTE_HANDLER);

	jkLog->LogMessage(L"I have crashed!");
	jkLog->LogMessage(L"  Command line: %s",GetCommandLineW());

	/* Show the type of exception. */
	switch(ExceptionInfo->ExceptionRecord->ExceptionCode) {
	case EXCEPTION_ACCESS_VIOLATION         : strcpy_s(s1,BUFSIZ,"ACCESS_VIOLATION (the memory could not be read or written)"); break;
	case EXCEPTION_DATATYPE_MISALIGNMENT    : strcpy_s(s1,BUFSIZ,"DATATYPE_MISALIGNMENT (a datatype misalignment error was detected in a load or store instruction)"); break;
	case EXCEPTION_BREAKPOINT               : strcpy_s(s1,BUFSIZ,"BREAKPOINT"); break;
	case EXCEPTION_SINGLE_STEP              : strcpy_s(s1,BUFSIZ,"SINGLE_STEP"); break;
	case EXCEPTION_ARRAY_BOUNDS_EXCEEDED    : strcpy_s(s1,BUFSIZ,"ARRAY_BOUNDS_EXCEEDED"); break;
	case EXCEPTION_FLT_DENORMAL_OPERAND     : strcpy_s(s1,BUFSIZ,"FLT_DENORMAL_OPERAND"); break;
	case EXCEPTION_FLT_DIVIDE_BY_ZERO       : strcpy_s(s1,BUFSIZ,"FLT_DIVIDE_BY_ZERO"); break;
	case EXCEPTION_FLT_INEXACT_RESULT       : strcpy_s(s1,BUFSIZ,"FLT_INEXACT_RESULT"); break;
	case EXCEPTION_FLT_INVALID_OPERATION    : strcpy_s(s1,BUFSIZ,"FLT_INVALID_OPERATION"); break;
	case EXCEPTION_FLT_OVERFLOW             : strcpy_s(s1,BUFSIZ,"FLT_OVERFLOW"); break;
	case EXCEPTION_FLT_STACK_CHECK          : strcpy_s(s1,BUFSIZ,"FLT_STACK_CHECK"); break;
	case EXCEPTION_FLT_UNDERFLOW            : strcpy_s(s1,BUFSIZ,"FLT_UNDERFLOW"); break;
	case EXCEPTION_INT_DIVIDE_BY_ZERO       : strcpy_s(s1,BUFSIZ,"INT_DIVIDE_BY_ZERO"); break;
	case EXCEPTION_INT_OVERFLOW             : strcpy_s(s1,BUFSIZ,"INT_OVERFLOW"); break;
	case EXCEPTION_PRIV_INSTRUCTION         : strcpy_s(s1,BUFSIZ,"PRIV_INSTRUCTION"); break;
	case EXCEPTION_IN_PAGE_ERROR            : strcpy_s(s1,BUFSIZ,"IN_PAGE_ERROR"); break;
	case EXCEPTION_ILLEGAL_INSTRUCTION      : strcpy_s(s1,BUFSIZ,"ILLEGAL_INSTRUCTION"); break;
	case EXCEPTION_NONCONTINUABLE_EXCEPTION : strcpy_s(s1,BUFSIZ,"NONCONTINUABLE_EXCEPTION"); break;
	case EXCEPTION_STACK_OVERFLOW           : strcpy_s(s1,BUFSIZ,"STACK_OVERFLOW"); break;
	case EXCEPTION_INVALID_DISPOSITION      : strcpy_s(s1,BUFSIZ,"INVALID_DISPOSITION"); break;
	case EXCEPTION_GUARD_PAGE               : strcpy_s(s1,BUFSIZ,"GUARD_PAGE"); break;
	case EXCEPTION_INVALID_HANDLE           : strcpy_s(s1,BUFSIZ,"INVALID_HANDLE"); break;
	case CONTROL_C_EXIT                     : strcpy_s(s1,BUFSIZ,"STATUS_CONTROL_C_EXIT"); break;
	case DBG_TERMINATE_THREAD               : strcpy_s(s1,BUFSIZ,"DBG_TERMINATE_THREAD (Debugger terminated thread)"); break;
	case DBG_TERMINATE_PROCESS              : strcpy_s(s1,BUFSIZ,"DBG_TERMINATE_PROCESS (Debugger terminated process)"); break;
	case DBG_CONTROL_C                      : strcpy_s(s1,BUFSIZ,"DBG_CONTROL_C (Debugger got control C)"); break;
	case DBG_CONTROL_BREAK                  : strcpy_s(s1,BUFSIZ,"DBG_CONTROL_BREAK (Debugger received control break)"); break;
	case DBG_COMMAND_EXCEPTION              : strcpy_s(s1,BUFSIZ,"DBG_COMMAND_EXCEPTION (Debugger command communication exception)"); break;
	default                                 : strcpy_s(s1,BUFSIZ,"(unknown exception)");
	}

	jkLog->LogMessage(L"  Exception: %S",s1);

	/* Try to show the linenumber of the sourcefile. */
	SymSetOptions(SymGetOptions() || SYMOPT_LOAD_LINES);
	Result = SymInitialize(GetCurrentProcess(),NULL,TRUE);

	if (Result == FALSE)
	{
		jkLib->SystemErrorStr(GetLastError(),s2,BUFSIZ);

		jkLog->LogMessage(L"  Failed to initialize SymInitialize(): %s",s2);

		return(EXCEPTION_EXECUTE_HANDLER);
	}

	ZeroMemory(&StackFrame,sizeof(StackFrame));

#ifdef _M_IX86
	ImageType = IMAGE_FILE_MACHINE_I386;
	StackFrame.AddrPC.Offset = ExceptionInfo->ContextRecord->Eip;
	StackFrame.AddrPC.Mode = AddrModeFlat;
	StackFrame.AddrFrame.Offset = ExceptionInfo->ContextRecord->Ebp;
	StackFrame.AddrFrame.Mode = AddrModeFlat;
	StackFrame.AddrStack.Offset = ExceptionInfo->ContextRecord->Esp;
	StackFrame.AddrStack.Mode = AddrModeFlat;
#elif _M_X64
	ImageType = IMAGE_FILE_MACHINE_AMD64;
	StackFrame.AddrPC.Offset = ExceptionInfo->ContextRecord->Rip;
	StackFrame.AddrPC.Mode = AddrModeFlat;
	StackFrame.AddrFrame.Offset = ExceptionInfo->ContextRecord->Rsp;
	StackFrame.AddrFrame.Mode = AddrModeFlat;
	StackFrame.AddrStack.Offset = ExceptionInfo->ContextRecord->Rsp;
	StackFrame.AddrStack.Mode = AddrModeFlat;
#elif _M_IA64
	ImageType = IMAGE_FILE_MACHINE_IA64;
	StackFrame.AddrPC.Offset = ExceptionInfo->ContextRecord->StIIP;
	StackFrame.AddrPC.Mode = AddrModeFlat;
	StackFrame.AddrFrame.Offset = ExceptionInfo->ContextRecord->IntSp;
	StackFrame.AddrFrame.Mode = AddrModeFlat;
	StackFrame.AddrBStore.Offset = ExceptionInfo->ContextRecord->RsBSP;
	StackFrame.AddrBStore.Mode = AddrModeFlat;
	StackFrame.AddrStack.Offset = ExceptionInfo->ContextRecord->IntSp;
	StackFrame.AddrStack.Mode = AddrModeFlat;
#endif
	for (FrameNumber = 1; ; FrameNumber++) {
		Result = StackWalk64(ImageType,GetCurrentProcess(),GetCurrentThread(),&StackFrame,
			ExceptionInfo->ContextRecord,NULL,SymFunctionTableAccess64,SymGetModuleBase64,NULL);
		if (Result == FALSE) break;
		if (StackFrame.AddrPC.Offset == StackFrame.AddrReturn.Offset) break;
		if (StackFrame.AddrPC.Offset != 0) {
			LineDisplacement = 0;
			ZeroMemory(&SourceLine,sizeof(SourceLine));
			SourceLine.SizeOfStruct = sizeof(SourceLine);
			Result = SymGetLineFromAddr64(GetCurrentProcess(),StackFrame.AddrPC.Offset,
				&LineDisplacement,&SourceLine);
			if (Result == TRUE) {
				jkLog->LogMessage(L"  %i. At line %d in '%S'",FrameNumber,SourceLine.LineNumber,SourceLine.FileName);
			} else {
				jkLog->LogMessage(L"  %i. At line (unknown) in (unknown)",FrameNumber);
				/*
				SystemErrorStr(GetLastError(),s2,BUFSIZ);
				LogMessage(L"  Error executing SymGetLineFromAddr64(): %s",s2);
				*/
			}
		}
	}

	/* Possible return values:
	EXCEPTION_CONTINUE_SEARCH    = popup a window about the error, user has to click.
	EXCEPTION_CONTINUE_EXECUTION = infinite loop
	EXCEPTION_EXECUTE_HANDLER    = stop program, do not run debugger
	*/
	return(EXCEPTION_EXECUTE_HANDLER);
}
#endif

/*

The main thread that performs all the work. Interpret the commandline
parameters and call the defragger library.

*/
DWORD WINAPI JKDefrag::DefragThread(LPVOID)
{
	int QuitOnFinish;
	int OptimizeMode;                /* 1...11 */
	int Speed;                       /* 0...100 */
	double FreeSpace;                /* 0...100 */
	WCHAR **Excludes;
	WCHAR **SpaceHogs;
	int DoAllVolumes;
	LPWSTR *argv;
	int argc;
	time_t Now;
	struct tm NowTm;
	OSVERSIONINFO OsVersion;
	int i;

	JKDefragLog *jkLog = m_jkDefrag->m_jkLog;
	JKDefragStruct *jkStruct = m_jkDefrag->m_jkStruct;
	JKDefragGui *jkGui = m_jkDefrag->m_jkGui;
	JKDefragLib *jkLib = m_jkDefrag->m_jkLib;

	/* Setup the defaults. */
	OptimizeMode = 2;
	Speed = 100;
	FreeSpace = 1;
	Excludes = NULL;
	SpaceHogs = NULL;
	QuitOnFinish = NO;

	/* Fetch the commandline. */
	argv = CommandLineToArgvW(GetCommandLineW(),&argc);

	/* Scan the commandline arguments for "-l" and setup the logfile. */
	if (argc > 1)
	{
		for (i = 1; i < argc; i++)
		{
			if (wcscmp(argv[i],L"-l") == 0)
			{
				i++;
				if (i >= argc) continue;

				jkLog->SetLogFilename(argv[i]);

				continue;
			}
			if ((wcsncmp(argv[i],L"-l",2) == 0) && (wcslen(argv[i]) >= 3))
			{
				jkLog->SetLogFilename(&argv[i][2]);

				continue;
			}
		}
	}

	/* Show some standard information in the logfile. */
	jkLog->LogMessage(jkStruct->VERSIONTEXT);
	time(&Now);

	localtime_s(&NowTm,&Now);
	jkLog->LogMessage(L"Date: %04lu/%02lu/%02lu",1900 + NowTm.tm_year,1 + NowTm.tm_mon,NowTm.tm_mday);

	ZeroMemory(&OsVersion,sizeof(OSVERSIONINFO));
	OsVersion.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

	if (GetVersionEx(&OsVersion) != 0)
	{
		jkLog->LogMessage(L"Windows version: v%lu.%lu build %lu %S",OsVersion.dwMajorVersion,
			OsVersion.dwMinorVersion,OsVersion.dwBuildNumber,OsVersion.szCSDVersion);
	}

	/* Scan the commandline again for all the other arguments. */
	if (argc > 1)
	{
		for (i = 1; i < argc; i++)
		{
			if (wcscmp(argv[i],L"-a") == 0)
			{
				i++;

				if (i >= argc)
				{
					jkGui->ShowDebug(0,NULL,L"Error: you have not specified a number after the \"-a\" commandline argument.");

					continue;
				}

				OptimizeMode = _wtol(argv[i]);

				if ((OptimizeMode < 1) || (OptimizeMode > 11))
				{
					jkGui->ShowDebug(0,NULL,L"Error: the number after the \"-a\" commandline argument is invalid.");

					OptimizeMode = 3;
				}

				OptimizeMode = OptimizeMode - 1;

				jkGui->ShowDebug(0,NULL,L"Commandline argument '-a' accepted, optimizemode = %u",OptimizeMode+1);

				continue;
			}

			if (wcsncmp(argv[i],L"-a",2) == 0)
			{
				OptimizeMode = _wtol(&argv[i][2]);

				if ((OptimizeMode < 1) || (OptimizeMode > 11))
				{
					jkGui->ShowDebug(0,NULL,L"Error: the number after the \"-a\" commandline argument is invalid.");

					OptimizeMode = 3;
				}

				OptimizeMode = OptimizeMode - 1;

				jkGui->ShowDebug(0,NULL,L"Commandline argument '-a' accepted, optimizemode = %u",OptimizeMode+1);

				continue;
			}

			if (wcscmp(argv[i],L"-s") == 0)
			{
				i++;

				if (i >= argc)
				{
					jkGui->ShowDebug(0,NULL,L"Error: you have not specified a number after the \"-s\" commandline argument.");

					continue;
				}

				Speed = _wtol(argv[i]);

				if ((Speed < 1) || (Speed > 100))
				{
					jkGui->ShowDebug(0,NULL,L"Error: the number after the \"-s\" commandline argument is invalid.");

					Speed = 100;
				}

			    jkGui->ShowDebug(0,NULL,L"Commandline argument '-s' accepted, speed = %u%%",Speed);

				continue;
			}

			if ((wcsncmp(argv[i],L"-s",2) == 0) && (wcslen(argv[i]) >= 3))
			{
				Speed = _wtol(&argv[i][2]);

				if ((Speed < 1) || (Speed > 100))
				{
					jkGui->ShowDebug(0,NULL,L"Error: the number after the \"-s\" commandline argument is invalid.");

					Speed = 100;
				}

				jkGui->ShowDebug(0,NULL,L"Commandline argument '-s' accepted, speed = %u%%",Speed);

				continue;
			}

			if (wcscmp(argv[i],L"-f") == 0)
			{
				i++;

				if (i >= argc)
				{
					jkGui->ShowDebug(0,NULL,L"Error: you have not specified a number after the \"-f\" commandline argument.");

					continue;
				}

				FreeSpace = _wtof(argv[i]);

				if ((FreeSpace < 0) || (FreeSpace > 100))
				{
					jkGui->ShowDebug(0,NULL,L"Error: the number after the \"-f\" commandline argument is invalid.");

					FreeSpace = 1;
				}

				jkGui->ShowDebug(0,NULL,L"Commandline argument '-f' accepted, freespace = %0.1f%%",FreeSpace);

				continue;
			}

			if ((wcsncmp(argv[i],L"-f",2) == 0) && (wcslen(argv[i]) >= 3))
			{
				FreeSpace = _wtof(&argv[i][2]);

				if ((FreeSpace < 0) || (FreeSpace > 100))
				{
					jkGui->ShowDebug(0,NULL,L"Error: the number after the \"-f\" command line argument is invalid.");

					FreeSpace = 1;
				}

				jkGui->ShowDebug(0,NULL,L"Command line argument '-f' accepted, free space = %0.1f%%",FreeSpace);

				continue;
			}

			if (wcscmp(argv[i],L"-d") == 0)
			{
				i++;

				if (i >= argc)
				{
					jkGui->ShowDebug(0,NULL,L"Error: you have not specified a number after the \"-d\" commandline argument.");

					continue;
				}

				m_jkDefrag->Debug = _wtol(argv[i]);

				if ((m_jkDefrag->Debug < 0) || (m_jkDefrag->Debug > 6))
				{
					jkGui->ShowDebug(0,NULL,L"Error: the number after the \"-d\" commandline argument is invalid.");

					m_jkDefrag->Debug = 1;
				}

				jkGui->ShowDebug(0,NULL,L"Commandline argument '-d' accepted, debug = %u",m_jkDefrag->Debug);

				continue;
			}

			if ((wcsncmp(argv[i],L"-d",2) == 0) && (wcslen(argv[i]) == 3) &&
				(argv[i][2] >= '0') && (argv[i][2] <= '6'))
			{
				m_jkDefrag->Debug = _wtol(&argv[i][2]);

				jkGui->ShowDebug(0,NULL,L"Commandline argument '-d' accepted, debug = %u",m_jkDefrag->Debug);

				continue;
			}

			if (wcscmp(argv[i],L"-l") == 0)
			{
				i++;

				if (i >= argc)
				{
					jkGui->ShowDebug(0,NULL,L"Error: you have not specified a filename after the \"-l\" commandline argument.");

					continue;
				}

				WCHAR *LogFile = jkLog->GetLogFilename();

				if (*LogFile != '\0')
				{
					jkGui->ShowDebug(0,NULL,L"Commandline argument '-l' accepted, logfile = %s",LogFile);
				}
				else
				{
					jkGui->ShowDebug(0,NULL,L"Commandline argument '-l' accepted, logfile turned off");
				}

				continue;
			}

			if ((wcsncmp(argv[i],L"-l",2) == 0) && (wcslen(argv[i]) >= 3))
			{
				WCHAR *LogFile = jkLog->GetLogFilename();

				if (*LogFile != '\0')
				{
					jkGui->ShowDebug(0,NULL,L"Commandline argument '-l' accepted, logfile = %s",LogFile);
				}
				else
				{
					jkGui->ShowDebug(0,NULL,L"Commandline argument '-l' accepted, logfile turned off");
				}

				continue;
			}

			if (wcscmp(argv[i],L"-e") == 0)
			{
				i++;

				if (i >= argc)
				{
					jkGui->ShowDebug(0,NULL,L"Error: you have not specified a mask after the \"-e\" commandline argument.");

					continue;
				}

				Excludes = jkLib->AddArrayString(Excludes,argv[i]);

				jkGui->ShowDebug(0,NULL,L"Commandline argument '-e' accepted, added '%s' to the excludes",argv[i]);

				continue;
			}

			if ((wcsncmp(argv[i],L"-e",2) == 0) && (wcslen(argv[i]) >= 3))
			{
				Excludes = jkLib->AddArrayString(Excludes,&argv[i][2]);

				jkGui->ShowDebug(0,NULL,L"Commandline argument '-e' accepted, added '%s' to the excludes",&argv[i][2]);

				continue;
			}

			if (wcscmp(argv[i],L"-u") == 0)
			{
				i++;

				if (i >= argc)
				{
					jkGui->ShowDebug(0,NULL,L"Error: you have not specified a mask after the \"-u\" commandline argument.");

					continue;
				}

				SpaceHogs = jkLib->AddArrayString(SpaceHogs,argv[i]);

				jkGui->ShowDebug(0,NULL,L"Commandline argument '-u' accepted, added '%s' to the spacehogs",argv[i]);

				continue;
			}

			if ((wcsncmp(argv[i],L"-u",2) == 0) && (wcslen(argv[i]) >= 3))
			{
				SpaceHogs = jkLib->AddArrayString(SpaceHogs,&argv[i][2]);

				jkGui->ShowDebug(0,NULL,L"Commandline argument '-u' accepted, added '%s' to the spacehogs",&argv[i][2]);

				continue;
			}

			if (wcscmp(argv[i],L"-q") == 0)
			{
				QuitOnFinish = YES;

				jkGui->ShowDebug(0,NULL,L"Commandline argument '-q' accepted, quitonfinish = yes");

				continue;
			}

			if (argv[i][0] == '-')
			{
				jkGui->ShowDebug(0,NULL,L"Error: commandline argument not recognised: %s",argv[i]);
			}
		}
	}

	/* Defragment all the paths that are specified on the commandline one by one. */
	DoAllVolumes = YES;

	if (argc > 1)
	{
		for (i = 1; i < argc; i++)
		{
			if (m_jkDefrag->IamRunning != RUNNING) break;

			if ((wcscmp(argv[i],L"-a") == 0) ||
				(wcscmp(argv[i],L"-e") == 0) ||
				(wcscmp(argv[i],L"-u") == 0) ||
				(wcscmp(argv[i],L"-s") == 0) ||
				(wcscmp(argv[i],L"-f") == 0) ||
				(wcscmp(argv[i],L"-d") == 0) ||
				(wcscmp(argv[i],L"-l") == 0))
			{
				i++;
				continue;
			}

			if (*argv[i] == '-') continue;
			if (*argv[i] == '\0') continue;

			jkLib->RunJkDefrag(argv[i],OptimizeMode,Speed,FreeSpace,Excludes,SpaceHogs,&m_jkDefrag->Running,
				/*&JKDefragGui::getInstance()->RedrawScreen,*/NULL);

			DoAllVolumes = NO;
		}
	}

	/* If no paths are specified on the commandline then defrag all fixed harddisks. */
	if ((DoAllVolumes == YES) && (m_jkDefrag->IamRunning == RUNNING))
	{
		jkLib->RunJkDefrag(NULL,OptimizeMode,Speed,FreeSpace,Excludes,SpaceHogs,&m_jkDefrag->Running,
			/*&JKDefragGui::getInstance()->RedrawScreen,*/NULL);
	}

	/* If the "-q" command line argument was specified then exit the program. */
	if (QuitOnFinish == YES) exit(EXIT_SUCCESS);

	/* End of this thread. */
	return(0);
}

/*

If the defragger is not yet running then return NO. If it's already
running or if there was an error getting the processlist then return
YES.

*/
int JKDefrag::AlreadyRunning(void)
{
	HANDLE Snapshot;
	PROCESSENTRY32 pe32;
	DWORD MyPid;
	char MyName[MAX_PATH];
	WCHAR s1[BUFSIZ];
	WCHAR s2[BUFSIZ];

	/* Get a process-snapshot from the kernel. */
	Snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);

	if (Snapshot == INVALID_HANDLE_VALUE)
	{
		m_jkLib->SystemErrorStr(GetLastError(),s1,BUFSIZ);

		swprintf_s(s2,BUFSIZ,L"Cannot get process snapshot: %s",s1);

		m_jkGui->ShowDebug(0,NULL,s2);

		return(YES);
	}

	pe32.dwSize = sizeof(PROCESSENTRY32);

	/* Get my own executable name. */
	MyPid = GetCurrentProcessId();
	*MyName = '\0';

	if (Process32First(Snapshot,&pe32) != FALSE)
	{
		do
		{
			if (MyPid == pe32.th32ProcessID)
			{
				strcpy_s(MyName,MAX_PATH,pe32.szExeFile);
				break;
			}
		} while (Process32Next(Snapshot,&pe32));
	}

	if (*MyName == '\0')
	{
		/* "Cannot find my own name in the process list: %s" */
		swprintf_s(s1,BUFSIZ,L"Cannot find my own name in the process list: %s",MyName);

		m_jkGui->ShowDebug(0,NULL,s1);

		return(YES);
	}

	/* Search for any other process with the same executable name as
	myself. If found then return YES. */
	Process32First(Snapshot,&pe32);

	do
	{
		if (MyPid == pe32.th32ProcessID) continue;          /* Ignore myself. */

		if ((_stricmp(pe32.szExeFile,MyName) == 0) ||
			(_stricmp(pe32.szExeFile,"jkdefrag.exe") == 0) ||
			(_stricmp(pe32.szExeFile,"jkdefragscreensaver.exe") == 0) ||
			(_stricmp(pe32.szExeFile,"jkdefragcmd.exe") == 0))
		{
			CloseHandle(Snapshot);

			swprintf_s(s1,BUFSIZ,L"I am already running: %S",pe32.szExeFile);

			m_jkGui->ShowDebug(0,NULL,s1);

			return(YES);
		}
	} while (Process32Next(Snapshot,&pe32));

	/* Return NO, not yet running. */
	CloseHandle(Snapshot);
	return(NO);
}


int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	JKDefrag *jkDefrag = JKDefrag::getInstance();

	WPARAM retValue = 0;

	if (jkDefrag != NULL)
	{
		retValue = jkDefrag->startProgram(hInstance,hPrevInstance,lpCmdLine,nCmdShow);

		JKDefrag::releaseInstance();
	}

	return((int)retValue);
}
