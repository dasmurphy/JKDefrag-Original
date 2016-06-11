#include "StdAfx.h"

JKDefragLog::JKDefragLog()
{
	WCHAR *p1;

	m_jkLib = JKDefragLib::getInstance();

	GetModuleFileNameW(NULL,MyName,MAX_PATH);
	GetShortPathNameW(MyName,MyShortName,MAX_PATH);
	GetLongPathNameW(MyShortName,MyName,MAX_PATH);

	/* Determine default path to logfile. */
	swprintf_s(LogFile,MAX_PATH,L"%s",MyName);

	p1 = m_jkLib->stristrW(LogFile,L".exe");

	if (p1 == NULL) p1 = m_jkLib->stristrW(LogFile,L".scr");

	if (p1 != NULL)
	{
		*p1 = '\0';

		wcscat_s(LogFile,MAX_PATH,L".log");
		_wunlink(LogFile);

	}
	else
	{
		*LogFile = '\0';
	}
}

void JKDefragLog::SetLogFilename(WCHAR *fileName)
{
	/* Determine default path to logfile. */
	swprintf_s(LogFile,MAX_PATH,L"%s",fileName);
	_wunlink(LogFile);
}

WCHAR *JKDefragLog::GetLogFilename()
{
	return LogFile;
}

/* Write a text to the logfile. The parameters are the same as for the "printf"
functions, a Format string and a series of parameters. */
void JKDefragLog::LogMessage(WCHAR *Format, ...)
{
	va_list VarArgs;
	FILE *Fout;
	int Result;
	time_t Now;
	struct tm NowTm;

	/* If there is no message then return. */
	if (Format == NULL) return;

	/* If there is no logfile then return. */
	if (*LogFile == '\0') return;

	/* Open the logfile. */
	Result = _wfopen_s(&Fout,LogFile,L"a, ccs=UTF-8");
	if ((Result != 0) || (Fout == NULL)) return;

	/* Write the string to the logfile. */
	time(&Now);
	Result = localtime_s(&NowTm,&Now);
	fwprintf_s(Fout,L"%02lu:%02lu:%02lu ",NowTm.tm_hour,NowTm.tm_min,NowTm.tm_sec);
	va_start(VarArgs,Format);
	vfwprintf_s(Fout,Format,VarArgs);
	va_end(VarArgs);
	fwprintf_s(Fout,L"\n");

	/* Close the logfile. */
	fflush(Fout);
	fclose(Fout);
}

