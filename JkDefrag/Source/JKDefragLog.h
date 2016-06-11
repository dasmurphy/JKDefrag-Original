#ifndef __JKDEFRAGLOG_H__
#define __JKDEFRAGLOG_H__

class JKDefragLog
{
public:
	JKDefragLog();
	void LogMessage(WCHAR *Format, ...);
	void SetLogFilename(WCHAR *fileName);

	WCHAR MyName[MAX_PATH];
	WCHAR MyShortName[MAX_PATH];

	WCHAR *GetLogFilename();

protected:
private:
	WCHAR LogFile[MAX_PATH];

	JKDefragLib *m_jkLib;
};

#endif