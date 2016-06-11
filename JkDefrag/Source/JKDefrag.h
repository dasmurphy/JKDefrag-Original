
#ifndef JKDEFRAG
#define JKDEFRAG

class JKDefrag
{
public:
	JKDefrag();
	~JKDefrag();

	// Get instance of the class
	static JKDefrag *getInstance();
	static void releaseInstance();

	WPARAM startProgram(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);

	static DWORD WINAPI DefragThread(LPVOID);

#ifdef _DEBUG

	static LONG __stdcall CrashReport(EXCEPTION_POINTERS *ExceptionInfo);

#endif

	int AlreadyRunning(void);

protected:
private:
	int Running;           /* If not RUNNING then stop defragging. */
	int IamRunning;

	/* Debug level.
	0: Fatal errors.
	1: Warning messages.
	2: General progress messages.
	3: Detailed progress messages.
	4: Detailed file information.
	5: Detailed gap-filling messages.
	6: Detailed gap-finding messages.
	*/
	int Debug;

	JKDefragGui *m_jkGui;
	JKDefragLib *m_jkLib;
	JKDefragLog *m_jkLog;
	JKDefragStruct *m_jkStruct;

	// static member that is an instance of itself
	static JKDefrag *m_jkDefrag;
};

#endif