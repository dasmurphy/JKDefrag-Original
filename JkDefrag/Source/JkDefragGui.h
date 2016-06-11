#ifndef __JKDEFRAGGUI_H__
#define __JKDEFRAGGUI_H__

const COLORREF Colors[9] = {
	RGB(150,150,150),     /* 0 COLOREMPTY         Empty diskspace. */
	RGB(200,200,200),     /* 1 COLORALLOCATED     Used diskspace / system files. */
	RGB(0,150,0),         /* 2 COLORUNFRAGMENTED  Unfragmented files. */
	RGB(128,0,0),         /* 3 COLORUNMOVABLE     Unmovable files. */
	RGB(200,100,60),      /* 4 COLORFRAGMENTED    Fragmented files. */
	RGB(0,0,255),         /* 5 COLORBUSY          Busy color. */
	RGB(255,0,255),       /* 6 COLORMFT           MFT reserved zones. */
	RGB(0,150,150),       /* 7 COLORSPACEHOG      Spacehogs. */
	RGB(255,255,255)      /* 8 background      */
};

struct clusterSquareStruct {
	bool dirty;
	byte color;
} ;

class JKDefragGui
{
public:

	// Constructor and destructor
	JKDefragGui();
	~JKDefragGui();

	// Get instance of the class
	static JKDefragGui *getInstance();

	void ClearScreen(WCHAR *Format, ...);
	void DrawCluster(struct DefragDataStruct *Data, ULONG64 ClusterStart, ULONG64 ClusterEnd, int Color);

	void FillSquares( int clusterStartSquareNum, int clusterEndSquareNum );
	void ShowDebug(int Level, struct ItemStruct *Item, WCHAR *Format, ...);
	void ShowStatus(struct DefragDataStruct *Data);
	void ShowAnalyze(struct DefragDataStruct *Data, struct ItemStruct *Item);
	void ShowMove(struct ItemStruct *Item, ULONG64 Clusters, ULONG64 FromLcn, ULONG64 ToLcn, ULONG64 FromVcn);
	void ShowDiskmap(struct DefragDataStruct *Data);

	int Initialize(HINSTANCE hInstance, int nCmdShow, JKDefragLog *jkLog, int debugLevel);
	void setDisplayData(HDC hdc);

	WPARAM DoModal();

	void OnPaint(HDC hdc);
	void PaintImage(HDC hdc);

	static LRESULT CALLBACK ProcessMessagefn(HWND WindowHandle,	UINT Message, WPARAM wParam, LPARAM lParam);

private:

	HWND m_hWnd;
	WNDCLASSEX m_wndClass;
	MSG Message;
	UINT_PTR m_sizeTimer;

	WCHAR Messages[6][50000];        /* The texts displayed on the screen. */
	int m_debugLevel;

	ULONG64 ProgressStartTime;       /* The time at percentage zero. */
	ULONG64 ProgressTime;            /* When ProgressDone/ProgressTodo were last updated. */
	ULONG64 ProgressTodo;            /* Number of clusters to do. */
	ULONG64 ProgressDone;            /* Number of clusters already done. */

	JKDefragStruct *jkStruct;

	/* graphics data */

	int m_topHeight;
	int m_squareSize;

	// Offsets of drawing area of disk
	int m_offsetX;
	int m_offsetY;

	// Calculated offset of drawing area of disk
	int m_realOffsetX;
	int m_realOffsetY;

	// Size of drawing area of disk
	Rect m_diskAreaSize;

	// Number of squares in horizontal direction of disk area
	int m_numDiskSquaresX;

	// Number of squares in horizontal direction of disk area
	int m_numDiskSquaresY;

	// Total number of squares in disk area
	int m_numDiskSquares;

	// Color of each square in disk area and status if it is "dirty"
	struct clusterSquareStruct m_clusterSquares[1000000];

	// Color of each disk cluster
	byte *clusterInfo;

	// Number of disk clusters
	UINT64 m_numClusters;

    /* 0:no, 1:request, 2: busy. */
//	int RedrawScreen;

	// Current window size
	Rect m_clientWindowSize;

	// Mutex to make the display single-threaded.
	HANDLE m_displayMutex;

	// Handle to graphics device context
	HDC m_hDC;

	// Bitmap used for double buffering
	Bitmap *m_bmp;

	// pointer to logger
	JKDefragLog *m_jkLog;

	// pointer to library
	JKDefragLib *m_jkLib;

	// static member that is an instance of itself
	static JKDefragGui *m_jkDefragGui;
};


#endif