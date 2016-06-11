#include "StdAfx.h"

/*
#include "JKDefragStruct.h"
#include "JKDefragLog.h"
#include "JkDefragLib.h"
#include "JkDefragGui.h"
*/

JKDefragGui *JKDefragGui::m_jkDefragGui = 0;

JKDefragGui::JKDefragGui()
{
	m_jkLib = JKDefragLib::getInstance();

	m_bmp = NULL;

	jkStruct = new JKDefragStruct();

	m_squareSize = 12;
//	m_clusterSquares = NULL;
	m_numDiskSquares = 0;

	m_offsetX = 26;
	m_offsetY = 16;

	clusterInfo = NULL;
	m_numClusters = 1;

	ProgressStartTime = 0;
	ProgressTime = 0;
	ProgressDone = 0;

	int i = 0;

	for (i = 0; i < 6; i++) *Messages[i] = '\0';

//	RedrawScreen = 0;
}

JKDefragGui::~JKDefragGui()
{
	delete jkStruct;
	delete m_jkDefragGui;

/*
	if (m_jkDefragGui->m_clusterSquares != NULL)
	{
		delete[] m_jkDefragGui->m_clusterSquares;
	}
*/

	if (m_bmp != NULL)
	{
		delete m_bmp;
	}
}

JKDefragGui *JKDefragGui::getInstance()
{
	if (m_jkDefragGui == NULL)
	{
		m_jkDefragGui = new JKDefragGui();
	}

	return m_jkDefragGui;
}

int JKDefragGui::Initialize(HINSTANCE hInstance, int nCmdShow, JKDefragLog *jkLog, int debugLevel)
{
	ULONG_PTR gdiplusToken;

	GdiplusStartupInput gdiplusStartupInput;

	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	m_jkLog = jkLog;
	m_debugLevel = debugLevel;

	m_displayMutex = CreateMutex(NULL,FALSE,"JKDefrag");

	m_wndClass.cbClsExtra    = 0;
	m_wndClass.cbWndExtra    = 0;
	m_wndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	m_wndClass.hCursor       = LoadCursor(NULL,IDC_ARROW);
	m_wndClass.hIcon         = LoadIcon(NULL,MAKEINTRESOURCE(1));
	m_wndClass.hInstance     = hInstance;
	m_wndClass.lpfnWndProc   = (WNDPROC)JKDefragGui::ProcessMessagefn;
	m_wndClass.lpszClassName = "MyClass";
	m_wndClass.lpszMenuName  = NULL;
	m_wndClass.style         = CS_HREDRAW | CS_VREDRAW;
	m_wndClass.cbSize        = sizeof(WNDCLASSEX);
	m_wndClass.hIconSm       = LoadIcon(hInstance,MAKEINTRESOURCE(1));

	CHAR koko[100];

	LoadString(hInstance,2,koko, 99);

	if (RegisterClassEx(&m_wndClass) == 0)
	{
		MessageBoxW(NULL,L"Cannot register class",jkStruct->VERSIONTEXT,MB_ICONEXCLAMATION | MB_OK);
		return(0);
	}

	m_hWnd = CreateWindowW(L"MyClass",jkStruct->VERSIONTEXT,WS_TILEDWINDOW,
		CW_USEDEFAULT,0,1024,768,NULL,NULL,hInstance,NULL);

	if (m_hWnd == NULL)
	{
		MessageBoxW(NULL,L"Cannot create window",jkStruct->VERSIONTEXT,MB_ICONEXCLAMATION | MB_OK);
		return(0);
	}

	/* Show the window in the state that Windows has specified, minimized or maximized. */
	ShowWindow(m_hWnd,nCmdShow);
	UpdateWindow(m_hWnd);

	SetTimer(m_hWnd,1,300,NULL);

//	InvalidateRect(m_hWnd,NULL,FALSE);

	return 1;
}

WPARAM JKDefragGui::DoModal()
{
	int GetMessageResult;

	/* The main message thread. */
	while (TRUE)
	{
		GetMessageResult = GetMessage(&Message,NULL,0,0);

		if (GetMessageResult == 0) break;
		if (GetMessageResult == -1) break;
		if (Message.message == WM_QUIT) break;

		TranslateMessage(&Message);
		DispatchMessage(&Message);
	}

	return Message.wParam;;
}

void JKDefragGui::setDisplayData(HDC hdc)
{
	Graphics graphics(hdc);

	Rect clientWindowSize;

	Status status = graphics.GetVisibleClipBounds(&clientWindowSize);

	m_clientWindowSize = clientWindowSize;

/*
	if (m_clusterSquares != NULL)
	{
		delete[] m_clusterSquares;
	}
*/

	m_topHeight = 33;

	if (m_debugLevel > 1)
	{
		m_topHeight = 49;
	}

	m_diskAreaSize.Width  = clientWindowSize.Width  - m_offsetX * 2;
	m_diskAreaSize.Height = clientWindowSize.Height - m_topHeight - m_offsetY * 2;

	m_numDiskSquaresX = (int)(m_diskAreaSize.Width  / m_squareSize);
	m_numDiskSquaresY = (int)(m_diskAreaSize.Height / m_squareSize);

	m_numDiskSquares  = m_numDiskSquaresX * m_numDiskSquaresY;

//	m_clusterSquares = new clusterSquareStruct[m_numDiskSquares];

	for (int ii = 0; ii < m_numDiskSquares; ii++)
	{
		m_clusterSquares[ii].color = 0;
		m_clusterSquares[ii].dirty = true;
	}

	m_realOffsetX = (int)((m_clientWindowSize.Width - m_numDiskSquaresX * m_squareSize) * 0.5);
	m_realOffsetY = (int)((m_clientWindowSize.Height - m_topHeight - m_numDiskSquaresY * m_squareSize) * 0.5);

	if (m_bmp != NULL)
	{
		delete m_bmp;

		m_bmp = NULL;
	}

	m_bmp = new Bitmap(m_clientWindowSize.Width, m_clientWindowSize.Height);

//	Color bottomPartColor;
//	bottomPartColor.SetFromCOLORREF(RGB(255,255,255));

//	SolidBrush bottomPartBrush(bottomPartColor);

//	Rect drawArea(0, 0, m_clientWindowSize.Width, m_clientWindowSize.Height);
//	graphics.FillRectangle(&bottomPartBrush, drawArea);

	/* Ask defragger to completely redraw the screen. */
//	RedrawScreen = 0;
}

/* Callback: clear the screen. */
void JKDefragGui::ClearScreen(WCHAR *Format, ...)
{
	va_list VarArgs;

	int i;

	/* If there is no message then return. */
	if (Format == NULL) return;

	/* Clear all the messages. */
	for (i = 0; i < 6; i++) *Messages[i] = '\0';

	/* Save the message in Messages 0. */
	va_start(VarArgs,Format);
	vswprintf_s(Messages[0],50000,Format,VarArgs);

	/* If there is no logfile then return. */
	if (m_jkLog != NULL)
	{
		m_jkLog->LogMessage(Format, VarArgs);
	}

	va_end(VarArgs);

	PaintImage(m_hDC);
//	InvalidateRect(m_hWnd,NULL,FALSE);
}

/* Callback: whenever an item (file, directory) is moved on disk. */
void JKDefragGui::ShowMove(struct ItemStruct *Item,
						   ULONG64 Clusters,
						   ULONG64 FromLcn,
						   ULONG64 ToLcn,
						   ULONG64 FromVcn)
{
	/* Save the message in Messages 3. */
	if (Clusters == 1)
	{
		swprintf_s(Messages[3],50000,L"Moving 1 cluster from %I64d to %I64d.",FromLcn,ToLcn);
	}
	else
	{
		swprintf_s(Messages[3],50000,L"Moving %I64d clusters from %I64d to %I64d.",
			Clusters,FromLcn,ToLcn);
	}

	/* Save the name of the file in Messages 4. */
	if ((Item != NULL) && (Item->LongPath != NULL))
	{
		swprintf_s(Messages[4],50000,L"%s",Item->LongPath);
	}
	else
	{
		*(Messages[4]) = '\0';
	}

	/* If debug mode then write a message to the logfile. */
	if (m_debugLevel < 3) return;

	if (FromVcn > 0)
	{
		if (Clusters == 1)
		{
			m_jkLog->LogMessage(L"%s\n  Moving 1 cluster from %I64d to %I64d, VCN=%I64d.",
				Item->LongPath,FromLcn,ToLcn,FromVcn);
		}
		else
		{
			m_jkLog->LogMessage(L"%s\n  Moving %I64d clusters from %I64d to %I64d, VCN=%I64d.",
				Item->LongPath,Clusters,FromLcn,ToLcn,FromVcn);
		}
	}
	else
	{
		if (Clusters == 1)
		{
			m_jkLog->LogMessage(L"%s\n  Moving 1 cluster from %I64d to %I64d.",
				Item->LongPath,FromLcn,ToLcn);
		}
		else
		{
			m_jkLog->LogMessage(L"%s\n  Moving %I64d clusters from %I64d to %I64d.",
				Item->LongPath,Clusters,FromLcn,ToLcn);
		}
	}
	PaintImage(m_hDC);

//	InvalidateRect(m_hWnd,NULL,FALSE);
}


/* Callback: for every file during analysis.
This subroutine is called one last time with Item=NULL when analysis has
finished. */
void JKDefragGui::ShowAnalyze(struct DefragDataStruct *Data, struct ItemStruct *Item)
{
	if ((Data != NULL) && (Data->CountAllFiles != 0))
	{
		swprintf_s(Messages[3],50000,L"Files %I64d, Directories %I64d, Clusters %I64d",
			Data->CountAllFiles,Data->CountDirectories,Data->CountAllClusters);
	}
	else
	{
		swprintf_s(Messages[3],50000,L"Applying Exclude and SpaceHogs masks....");
	}

	/* Save the name of the file in Messages 4. */
	if ((Item != NULL) && (Item->LongPath != NULL))
	{
		swprintf_s(Messages[4],50000,L"%s",Item->LongPath);
	}
	else
	{
		*(Messages[4]) = '\0';
	}
	PaintImage(m_hDC);

//	InvalidateRect(m_hWnd,NULL,FALSE);
}

/* Callback: show a debug message. */

void  JKDefragGui::ShowDebug(int Level, struct ItemStruct *Item, WCHAR *Format, ...)
{
	va_list VarArgs;

	if (m_debugLevel < Level) return;

	/* Save the name of the file in Messages 4. */
	if ((Item != NULL) && (Item->LongPath != NULL))
	{
		swprintf_s(Messages[4],50000,L"%s",Item->LongPath);
	}

	/* If there is no message then return. */
	if (Format == NULL) return;

	/* Save the debug message in Messages 5. */
	va_start(VarArgs,Format);
	vswprintf_s(Messages[5],50000,Format,VarArgs);
	m_jkLog->LogMessage(Format, VarArgs);
	va_end(VarArgs);
	PaintImage(m_hDC);

//	InvalidateRect(m_hWnd,NULL,FALSE);
}

/* Callback: paint a cluster on the screen in a color. */
void JKDefragGui::DrawCluster(struct DefragDataStruct *Data,
							  ULONG64 ClusterStart,
							  ULONG64 ClusterEnd,
							  int Color)
{
	struct __timeb64 Now;

	Rect windowSize = m_clientWindowSize;

	/* Save the PhaseTodo and PhaseDone counters for later use by the progress counter. */
	if (Data->PhaseTodo != 0)
	{
		_ftime64_s(&Now);

		ProgressTime = Now.time * 1000 + Now.millitm;
		ProgressDone = Data->PhaseDone;
		ProgressTodo = Data->PhaseTodo;
	}

	/* Sanity check. */
	if (Data->TotalClusters == 0) return;
	if (m_hDC == NULL) return;
	if (ClusterStart == ClusterEnd) return;
//	if (ClusterStart > Data->TotalClusters) ClusterStart = 0;

	WaitForSingleObject(m_displayMutex,100);

	m_displayMutex = CreateMutex(NULL,FALSE,"JKDefrag");

	if (m_numClusters != Data->TotalClusters || clusterInfo == NULL)
	{
		if (clusterInfo != NULL)
		{
			free(clusterInfo);

			clusterInfo = NULL;
		}

		m_numClusters = Data->TotalClusters;

		clusterInfo = (byte *)malloc((size_t)m_numClusters);

		for(int ii = 0; ii <= m_numClusters; ii++)
		{
			clusterInfo[ii] = JKDefragStruct::COLOREMPTY;
		}

//		RedrawScreen = 0;

		return;
	} 


	for(ULONG64 ii = ClusterStart; ii <= ClusterEnd; ii++)
	{
		clusterInfo[ii] = Color;
	}

	float clusterPerSquare = (float)(m_numClusters / m_numDiskSquares);
	int clusterStartSquareNum = (int)((ULONG64)ClusterStart / (ULONG64)clusterPerSquare);
	int clusterEndSquareNum = (int)((ULONG64)ClusterEnd / (ULONG64)clusterPerSquare);

	FillSquares(clusterStartSquareNum, clusterEndSquareNum);

	ReleaseMutex(m_displayMutex);
	PaintImage(m_hDC);

//	InvalidateRect(m_hWnd,NULL,FALSE);
}

/* Callback: just before the defragger starts a new Phase, and when it finishes. */
void JKDefragGui::ShowStatus(struct DefragDataStruct *Data)
{
	struct ItemStruct *Item;

	int Fragments;

	ULONG64 TotalFragments;
	ULONG64 TotalBytes;
	ULONG64 TotalClusters;

	struct ItemStruct *LargestItems[25];

	int LastLargest;

	struct __timeb64 Now;

	int i;
	int j;

	/* Reset the progress counter. */
	_ftime64_s(&Now);

	ProgressStartTime = Now.time * 1000 + Now.millitm;
	ProgressTime = ProgressStartTime;
	ProgressDone = 0;
	ProgressTodo = 0;

	/* Reset all the messages. */
	for (i = 0; i < 6; i++) *(Messages[i]) = '\0';

	/* Update Message 0 and 1. */
	if (Data != NULL)
	{
		swprintf_s(Messages[0],50000,L"%s",Data->Disk.MountPoint);

		switch(Data->Phase)
		{
		case 1: wcscpy_s(Messages[1],50000,L"Phase 1: Analyze"); break;
		case 2: wcscpy_s(Messages[1],50000,L"Phase 2: Defragment"); break;
		case 3: wcscpy_s(Messages[1],50000,L"Phase 3: ForcedFill"); break;
		case 4: swprintf_s(Messages[1],50000,L"Zone %u: Sort",Data->Zone + 1); break;
		case 5: swprintf_s(Messages[1],50000,L"Zone %u: Fast Optimize",Data->Zone + 1); break;
		case 6: wcscpy_s(Messages[1],50000,L"Phase 3: Move Up"); break;
		case 7:
			wcscpy_s(Messages[1],50000,L"Finished.");
			swprintf_s(Messages[4],50000,L"Logfile: %s",m_jkLog->GetLogFilename());
			break;
		case 8: wcscpy_s(Messages[1],50000,L"Phase 3: Fixup"); break;
		}

		m_jkLog->LogMessage(Messages[1]);
	}

	/* Write some statistics to the logfile. */
	if ((Data != NULL) && (Data->Phase == 7))
	{
		m_jkLog->LogMessage(L"- Total disk space: %I64d bytes (%.04f gigabytes), %I64d clusters",
			Data->BytesPerCluster * Data->TotalClusters,
			(double)(Data->BytesPerCluster * Data->TotalClusters) / (1024 * 1024 * 1024),
			Data->TotalClusters);

		m_jkLog->LogMessage(L"- Bytes per cluster: %I64d bytes",Data->BytesPerCluster);

		m_jkLog->LogMessage(L"- Number of files: %I64d",Data->CountAllFiles);
		m_jkLog->LogMessage(L"- Number of directories: %I64d",Data->CountDirectories);
		m_jkLog->LogMessage(L"- Total size of analyzed items: %I64d bytes (%.04f gigabytes), %I64d clusters",
			Data->CountAllClusters * Data->BytesPerCluster,
			(double)(Data->CountAllClusters * Data->BytesPerCluster) / (1024 * 1024 * 1024),
			Data->CountAllClusters);

		if (Data->CountAllFiles + Data->CountDirectories > 0)
		{
			m_jkLog->LogMessage(L"- Number of fragmented items: %I64d (%.04f%% of all items)",
				Data->CountFragmentedItems,
				(double)(Data->CountFragmentedItems * 100) / (Data->CountAllFiles + Data->CountDirectories));
		}
		else
		{
			m_jkLog->LogMessage(L"- Number of fragmented items: %I64d",Data->CountFragmentedItems);
		}

		if ((Data->CountAllClusters > 0) && (Data->TotalClusters > 0))
		{
			m_jkLog->LogMessage(L"- Total size of fragmented items: %I64d bytes, %I64d clusters, %.04f%% of all items, %.04f%% of disk",
				Data->CountFragmentedClusters * Data->BytesPerCluster,
				Data->CountFragmentedClusters,
				(double)(Data->CountFragmentedClusters * 100) / Data->CountAllClusters,
				(double)(Data->CountFragmentedClusters * 100) / Data->TotalClusters);
		}
		else
		{
			m_jkLog->LogMessage(L"- Total size of fragmented items: %I64d bytes, %I64d clusters",
				Data->CountFragmentedClusters * Data->BytesPerCluster,
				Data->CountFragmentedClusters);
		}

		if (Data->TotalClusters > 0)
		{
			m_jkLog->LogMessage(L"- Free disk space: %I64d bytes, %I64d clusters, %.04f%% of disk",
				Data->CountFreeClusters * Data->BytesPerCluster,
				Data->CountFreeClusters,
				(double)(Data->CountFreeClusters * 100) / Data->TotalClusters);
		}
		else
		{
			m_jkLog->LogMessage(L"- Free disk space: %I64d bytes, %I64d clusters",
				Data->CountFreeClusters * Data->BytesPerCluster,
				Data->CountFreeClusters);
		}

		m_jkLog->LogMessage(L"- Number of gaps: %I64d",Data->CountGaps);

		if (Data->CountGaps > 0)
		{
			m_jkLog->LogMessage(L"- Number of small gaps: %I64d (%.04f%% of all gaps)",
				Data->CountGapsLess16,
				(double)(Data->CountGapsLess16 * 100) / Data->CountGaps);
		}
		else
		{
			m_jkLog->LogMessage(L"- Number of small gaps: %I64d",
				Data->CountGapsLess16);
		}

		if (Data->CountFreeClusters > 0)
		{
			m_jkLog->LogMessage(L"- Size of small gaps: %I64d bytes, %I64d clusters, %.04f%% of free disk space",
				Data->CountClustersLess16 * Data->BytesPerCluster,
				Data->CountClustersLess16,
				(double)(Data->CountClustersLess16 * 100) / Data->CountFreeClusters);
		}
		else
		{
			m_jkLog->LogMessage(L"- Size of small gaps: %I64d bytes, %I64d clusters",
				Data->CountClustersLess16 * Data->BytesPerCluster,
				Data->CountClustersLess16);
		}

		if (Data->CountGaps > 0)
		{
			m_jkLog->LogMessage(L"- Number of big gaps: %I64d (%.04f%% of all gaps)",
				Data->CountGaps - Data->CountGapsLess16,
				(double)((Data->CountGaps - Data->CountGapsLess16) * 100) / Data->CountGaps);
		}
		else
		{
			m_jkLog->LogMessage(L"- Number of big gaps: %I64d",
				Data->CountGaps - Data->CountGapsLess16);
		}

		if (Data->CountFreeClusters > 0)
		{
			m_jkLog->LogMessage(L"- Size of big gaps: %I64d bytes, %I64d clusters, %.04f%% of free disk space",
				(Data->CountFreeClusters - Data->CountClustersLess16) * Data->BytesPerCluster,
				Data->CountFreeClusters - Data->CountClustersLess16,
				(double)((Data->CountFreeClusters - Data->CountClustersLess16) * 100) / Data->CountFreeClusters);
		}
		else
		{
			m_jkLog->LogMessage(L"- Size of big gaps: %I64d bytes, %I64d clusters",
				(Data->CountFreeClusters - Data->CountClustersLess16) * Data->BytesPerCluster,
				Data->CountFreeClusters - Data->CountClustersLess16);
		}

		if (Data->CountGaps > 0)
		{
			m_jkLog->LogMessage(L"- Average gap size: %.04f clusters",
				(double)(Data->CountFreeClusters) / Data->CountGaps);
		}

		if (Data->CountFreeClusters > 0)
		{
			m_jkLog->LogMessage(L"- Biggest gap: %I64d bytes, %I64d clusters, %.04f%% of free disk space",
				Data->BiggestGap * Data->BytesPerCluster,
				Data->BiggestGap,
				(double)(Data->BiggestGap * 100) / Data->CountFreeClusters);
		}
		else
		{
			m_jkLog->LogMessage(L"- Biggest gap: %I64d bytes, %I64d clusters",
				Data->BiggestGap * Data->BytesPerCluster,
				Data->BiggestGap);
		}

		if (Data->TotalClusters > 0)
		{
			m_jkLog->LogMessage(L"- Average end-begin distance: %.0f clusters, %.4f%% of volume size",
				Data->AverageDistance,100.0 * Data->AverageDistance / Data->TotalClusters);
		}
		else
		{
			m_jkLog->LogMessage(L"- Average end-begin distance: %.0f clusters",Data->AverageDistance);
		}

		for (Item = m_jkLib->TreeSmallest(Data->ItemTree); Item != NULL; Item = m_jkLib->TreeNext(Item))
		{
			if (Item->Unmovable != YES) continue;
			if (Item->Exclude == YES) continue;
			if ((Item->Directory == YES) && (Data->CannotMoveDirs > 20)) continue;
			break;
		}

		if (Item != NULL)
		{
			m_jkLog->LogMessage(L"These items could not be moved:");
			m_jkLog->LogMessage(L"  Fragments       Bytes  Clusters Name");

			TotalFragments = 0;
			TotalBytes = 0;
			TotalClusters = 0;

			for (Item = m_jkLib->TreeSmallest(Data->ItemTree); Item != NULL; Item = m_jkLib->TreeNext(Item))
			{
				if (Item->Unmovable != YES) continue;
				if (Item->Exclude == YES) continue;
				if ((Item->Directory == YES) && (Data->CannotMoveDirs > 20)) continue;
				if ((Item->LongFilename != NULL) &&
					((_wcsicmp(Item->LongFilename,L"$BadClus") == 0) ||
					(_wcsicmp(Item->LongFilename,L"$BadClus:$Bad:$DATA") == 0))) continue;

				Fragments = m_jkLib->FragmentCount(Item);

				if (Item->LongPath == NULL)
				{
					m_jkLog->LogMessage(L"  %9lu %11I64u %9I64u [at cluster %I64u]",Fragments,Item->Bytes,Item->Clusters,
						m_jkLib->GetItemLcn(Item));
				}
				else
				{
					m_jkLog->LogMessage(L"  %9lu %11I64u %9I64u %s",Fragments,Item->Bytes,Item->Clusters,Item->LongPath);
				}

				TotalFragments = TotalFragments + Fragments;
				TotalBytes = TotalBytes + Item->Bytes;
				TotalClusters = TotalClusters + Item->Clusters;
			}

			m_jkLog->LogMessage(L"  --------- ----------- --------- -----");
			m_jkLog->LogMessage(L"  %9I64u %11I64u %9I64u Total",TotalFragments,TotalBytes,TotalClusters);
		}

		for (Item = m_jkLib->TreeSmallest(Data->ItemTree); Item != NULL; Item = m_jkLib->TreeNext(Item))
		{
			if (Item->Exclude == YES) continue;
			if ((Item->Directory == YES) && (Data->CannotMoveDirs > 20)) continue;

			Fragments = m_jkLib->FragmentCount(Item);

			if (Fragments <= 1) continue;

			break;
		}

		if (Item != NULL)
		{
			m_jkLog->LogMessage(L"These items are still fragmented:");
			m_jkLog->LogMessage(L"  Fragments       Bytes  Clusters Name");

			TotalFragments = 0;
			TotalBytes = 0;
			TotalClusters = 0;

			for (Item = m_jkLib->TreeSmallest(Data->ItemTree); Item != NULL; Item = m_jkLib->TreeNext(Item))
			{
				if (Item->Exclude == YES) continue;
				if ((Item->Directory == YES) && (Data->CannotMoveDirs > 20)) continue;

				Fragments = m_jkLib->FragmentCount(Item);

				if (Fragments <= 1) continue;

				if (Item->LongPath == NULL)
				{
					m_jkLog->LogMessage(L"  %9lu %11I64u %9I64u [at cluster %I64u]",Fragments,Item->Bytes,Item->Clusters,
						m_jkLib->GetItemLcn(Item));
				}
				else
				{
					m_jkLog->LogMessage(L"  %9lu %11I64u %9I64u %s",Fragments,Item->Bytes,Item->Clusters,Item->LongPath);
				}

				TotalFragments = TotalFragments + Fragments;
				TotalBytes = TotalBytes + Item->Bytes;
				TotalClusters = TotalClusters + Item->Clusters;
			}

			m_jkLog->LogMessage(L"  --------- ----------- --------- -----");
			m_jkLog->LogMessage(L"  %9I64u %11I64u %9I64u Total",TotalFragments,TotalBytes,TotalClusters);
		}

		LastLargest = 0;

		for (Item = m_jkLib->TreeSmallest(Data->ItemTree); Item != NULL; Item = m_jkLib->TreeNext(Item))
		{
			if ((Item->LongFilename != NULL) &&
				((_wcsicmp(Item->LongFilename,L"$BadClus") == 0) ||
				(_wcsicmp(Item->LongFilename,L"$BadClus:$Bad:$DATA") == 0)))
			{
				continue;
			}

			for (i = LastLargest - 1; i >= 0; i--)
			{
				if (Item->Clusters < LargestItems[i]->Clusters) break;

				if ((Item->Clusters == LargestItems[i]->Clusters) &&
					(Item->Bytes < LargestItems[i]->Bytes)) break;

				if ((Item->Clusters == LargestItems[i]->Clusters) &&
					(Item->Bytes == LargestItems[i]->Bytes) &&
					(Item->LongPath != NULL) &&
					(LargestItems[i]->LongPath != NULL) &&
					(_wcsicmp(Item->LongPath,LargestItems[i]->LongPath) > 0)) break;
			}

			if (i < 24)
			{
				if (LastLargest < 25) LastLargest++;

				for (j = LastLargest - 1; j > i + 1; j--)
				{
					LargestItems[j] = LargestItems[j-1];
				}

				LargestItems[i + 1] = Item;
			}
		}

		if (LastLargest > 0)
		{
			m_jkLog->LogMessage(L"The 25 largest items on disk:");
			m_jkLog->LogMessage(L"  Fragments       Bytes  Clusters Name");

			for (i = 0; i < LastLargest; i++)
			{
				if (LargestItems[i]->LongPath == NULL)
				{
					m_jkLog->LogMessage(L"  %9u %11I64u %9I64u [at cluster %I64u]",m_jkLib->FragmentCount(LargestItems[i]),
						LargestItems[i]->Bytes,LargestItems[i]->Clusters,m_jkLib->GetItemLcn(LargestItems[i]));
				}
				else
				{
					m_jkLog->LogMessage(L"  %9u %11I64u %9I64u %s",m_jkLib->FragmentCount(LargestItems[i]),
						LargestItems[i]->Bytes,LargestItems[i]->Clusters,LargestItems[i]->LongPath);
				}
			}
		}
	}
}


void JKDefragGui::PaintImage(HDC hdc)
{
	Graphics *graphics = Graphics::FromImage(m_bmp);

	double Done;

	Rect windowSize = m_clientWindowSize;
	Rect drawArea;

	float squareSizeUnit = (float)(1 / (float)m_squareSize);

	/* Reset the display idle timer (screen saver) and system idle timer (power saver). */
	SetThreadExecutionState(ES_SYSTEM_REQUIRED | ES_DISPLAY_REQUIRED);

	if (ProgressTodo > 0)
	{
		Done = (double)((double)ProgressDone / (double)ProgressTodo);

		if (Done > 1) Done = 1;

		swprintf_s(Messages[2],50000,L"%.4f%%",100 * Done);
	}

	Color backColor1;
	backColor1.SetFromCOLORREF(RGB(0, 0, 255));

	Color backColor2;
	backColor2.SetFromCOLORREF(RGB(255, 0, 0));

	LinearGradientBrush bgBrush(windowSize,Color::DarkBlue,Color::LightBlue,LinearGradientModeForwardDiagonal);

	drawArea = windowSize;

	drawArea.Height = m_topHeight + 1;

	Color busyColor;
	busyColor.SetFromCOLORREF(Colors[JKDefragStruct::COLORBUSY]);

	SolidBrush busyBrush(busyColor);

/*
	graphics->FillRectangle(&busyBrush, drawArea);
*/
	graphics->FillRectangle(&bgBrush, drawArea);

	SolidBrush brush(Color::White);

	FontFamily fontFamily(L"Tahoma");
	Font       font(&fontFamily,12,FontStyleRegular, UnitPixel);
	WCHAR      *text;
	PointF     pointF(2.0f, 0.0f);

	text = Messages[0];
	graphics->DrawString(text, -1, &font, pointF, &brush);

	pointF = PointF(40.0f, 0.0f);
	text = Messages[1];
	graphics->DrawString(text, -1, &font, pointF, &brush);

	pointF = PointF(200.0f, 0.0f);
	text = Messages[2];
	graphics->DrawString(text, -1, &font, pointF, &brush);

	pointF = PointF(280.0f, 0.0f);
	text = Messages[3];
	graphics->DrawString(text, -1, &font, pointF, &brush);

	pointF = PointF(2.0f, 17.0f);
	text = Messages[4];
	graphics->DrawString(text, -1, &font, pointF, &brush);

	if (m_debugLevel > 1)
	{
		pointF = PointF(2.0f, 33.0f);
		text = Messages[5];
		graphics->DrawString(text, -1, &font, pointF, &brush);
	}


	int xx1 = m_realOffsetX - 1;
	int yy1 = m_realOffsetY + m_topHeight - 1;

	int xx2 = xx1 + m_numDiskSquaresX * m_squareSize + 1;
	int yy2 = yy1 + m_numDiskSquaresY * m_squareSize + 1;

/*
	Color bottomPartColor;
	bottomPartColor.SetFromCOLORREF(Colors[JKDefragStruct::COLORBUSY]);

	SolidBrush bottomPartBrush(bottomPartColor);
*/

	drawArea = Rect(0, m_topHeight + 1, m_clientWindowSize.Width, yy1 - m_topHeight - 2);
/*
	graphics->FillRectangle(&bottomPartBrush, drawArea);
*/
	graphics->FillRectangle(&bgBrush, drawArea);

	drawArea = Rect(0, yy2 + 2, m_clientWindowSize.Width, m_clientWindowSize.Height - yy2 - 2);
/*
	graphics->FillRectangle(&bottomPartBrush, drawArea);
*/
	graphics->FillRectangle(&bgBrush, drawArea);

	drawArea = Rect(0, yy1 - 1, xx1 - 1, yy2 - yy1 + 3);
/*
	graphics->FillRectangle(&bottomPartBrush, drawArea);
*/
	graphics->FillRectangle(&bgBrush, drawArea);

	drawArea = Rect(xx2, yy1 - 1, m_clientWindowSize.Width - xx2, yy2 - yy1 + 3);
/*
	graphics->FillRectangle(&bottomPartBrush, drawArea);
*/
	graphics->FillRectangle(&bgBrush, drawArea);

	Pen pen1(Color(0,0,0));
	Pen pen2(Color(255,255,255));

	graphics->DrawLine(&pen1, xx1, yy2, xx1, yy1);
	graphics->DrawLine(&pen1, xx1, yy1, xx2, yy1);
	graphics->DrawLine(&pen1, xx2, yy1, xx2, yy2);
	graphics->DrawLine(&pen1, xx2, yy2, xx1, yy2);

	graphics->DrawLine(&pen2, xx1 - 1, yy2 + 1, xx1 - 1, yy1 - 1);
	graphics->DrawLine(&pen2, xx1 - 1, yy1 - 1, xx2 + 1, yy1 - 1);
	graphics->DrawLine(&pen2, xx2 + 1, yy1 - 1, xx2 + 1, yy2 + 1);
	graphics->DrawLine(&pen2, xx2 + 1, yy2 + 1, xx1 - 1, yy2 + 1);

	COLORREF colEmpty = Colors[JKDefragStruct::COLOREMPTY];
	Color colorEmpty;
	colorEmpty.SetFromCOLORREF(colEmpty);

	Pen pen(Color(210,210,210));
	Pen penEmpty(colorEmpty);

	for (int jj = 0; jj < m_numDiskSquares; jj++)
	{
		if (m_clusterSquares[jj].dirty == false)
		{
			continue;
		}

		m_clusterSquares[jj].dirty = false;

		int x1 = jj % m_numDiskSquaresX;
		int y1 = jj / m_numDiskSquaresX;

		int xx1 = m_realOffsetX + x1 * m_squareSize;
		int yy1 = m_realOffsetY + y1 * m_squareSize + m_topHeight;

		byte clusterEmpty        = (m_clusterSquares[jj].color & (1 << 7)) >> 7;
		byte clusterAllocated    = (m_clusterSquares[jj].color & (1 << 6)) >> 6;
		byte clusterUnfragmented = (m_clusterSquares[jj].color & (1 << 5)) >> 5;
		byte clusterUnmovable    = (m_clusterSquares[jj].color & (1 << 4)) >> 4;
		byte clusterFragmented   = (m_clusterSquares[jj].color & (1 << 3)) >> 3;
		byte clusterBusy         = (m_clusterSquares[jj].color & (1 << 2)) >> 2;
		byte clusterMft          = (m_clusterSquares[jj].color & (1 << 1)) >> 1;
		byte clusterSpacehog     = (m_clusterSquares[jj].color & 1);

		COLORREF col = Colors[JKDefragStruct::COLOREMPTY];

		int emptyCluster = true;

		if (clusterBusy == 1)
		{
			col = Colors[JKDefragStruct::COLORBUSY];
			emptyCluster = false;
		}
		else if (clusterUnmovable == 1)
		{
			col = Colors[JKDefragStruct::COLORUNMOVABLE];
			emptyCluster = false;
		}
		else if (clusterFragmented == 1)
		{
			col = Colors[JKDefragStruct::COLORFRAGMENTED];
			emptyCluster = false;
		}
		else if (clusterMft == 1)
		{
			col = Colors[JKDefragStruct::COLORMFT];
			emptyCluster = false;
		}
		else if (clusterUnfragmented == 1)
		{
			col = Colors[JKDefragStruct::COLORUNFRAGMENTED];
			emptyCluster = false;
		}
		else if (clusterSpacehog == 1)
		{
			col = Colors[JKDefragStruct::COLORSPACEHOG];
			emptyCluster = false;
		}

		Color C1;
		Color C2;

		C1.SetFromCOLORREF(col);

		int RR = GetRValue(col) + 200;
		RR = (RR > 255) ? 255 : RR;

		int GG = GetGValue(col) + 200;
		GG = (GG > 255) ? 255 : GG;

		int BB = GetBValue(col) + 100;
		BB = (BB > 255) ? 255 : BB;

		C2.SetFromCOLORREF(RGB((byte)RR, (byte)GG, (byte)BB));

		if (emptyCluster)
		{
			Rect drawArea2(xx1, yy1, m_squareSize - 0, m_squareSize - 0);

			LinearGradientBrush BB2(drawArea2,C1,C2,LinearGradientModeVertical);
			graphics->FillRectangle(&BB2, drawArea2);

			int lineX1 = drawArea2.X;
			int lineY1 = drawArea2.Y;
			int lineX2 = drawArea2.X + m_squareSize - 1;
			int lineY2 = drawArea2.Y;
			int lineX3 = drawArea2.X;
			int lineY3 = drawArea2.Y + m_squareSize - 1;
			int lineX4 = drawArea2.X + m_squareSize - 1;
			int lineY4 = drawArea2.Y + m_squareSize - 1;

			graphics->DrawLine(&penEmpty, lineX1, lineY1, lineX2, lineY2);
			graphics->DrawLine(&pen, lineX3, lineY3, lineX4, lineY4);
		}
		else
		{
			Rect drawArea2(xx1, yy1, m_squareSize - 0, m_squareSize - 0);

			LinearGradientBrush BB1(drawArea2,C2,C1,LinearGradientModeForwardDiagonal);

			graphics->FillRectangle(&BB1, drawArea2);

			int lineX1 = drawArea2.X;
			int lineY1 = drawArea2.Y + m_squareSize - 1;
			int lineX2 = drawArea2.X + m_squareSize - 1;
			int lineY2 = drawArea2.Y;
			int lineX3 = drawArea2.X + m_squareSize - 1;
			int lineY3 = drawArea2.Y + m_squareSize - 1;

			graphics->DrawLine(&pen, lineX1, lineY1, lineX3, lineY3);
			graphics->DrawLine(&pen, lineX2, lineY2, lineX3, lineY3);
		}

	}

	delete graphics;

}

void JKDefragGui::OnPaint(HDC hdc)
{
/*
	Bitmap bmp(m_clientWindowSize.Width, m_clientWindowSize.Height);

	Graphics *graphics2 = Graphics::FromImage(&bmp);
*/

	Graphics graphics(hdc);

/*
	graphics2->DrawImage(m_bmp,0,0);
*/

/*
	Color busyColor(128,128,128,128);

	SolidBrush busyBrush(busyColor);

	Rect rr = Rect(100, 100, 400, 100);

	graphics2->FillRectangle(&busyBrush, rr);

	SolidBrush brush(Color::White);

	FontFamily fontFamily(L"Tahoma");
	Font       font(&fontFamily,12,FontStyleRegular, UnitPixel);
	PointF     pointF(132.0f, 120.0f);

	WCHAR      *text;

	text = Messages[2];

	graphics2->DrawString(text, -1, &font, pointF, &brush);
*/

	graphics.DrawImage(m_bmp,0,0);

/*
	delete graphics2;
*/

	return;
}

/* Message handler. */
LRESULT CALLBACK JKDefragGui::ProcessMessagefn(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	switch(Message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	case WM_TIMER:

/*
		if (wParam == 333)
		{
			PAINTSTRUCT ps;

			WaitForSingleObject(m_jkDefragGui->m_displayMutex,100);

			m_jkDefragGui->m_displayMutex = CreateMutex(NULL,FALSE,"JKDefrag");

			m_jkDefragGui->m_hDC = BeginPaint(hWnd, &ps);

			m_jkDefragGui->setDisplayData(m_jkDefragGui->m_hDC);

			m_jkDefragGui->FillSquares( 0, m_jkDefragGui->m_numDiskSquares);

			m_jkDefragGui->PaintImage(m_jkDefragGui->m_hDC);

			EndPaint(hWnd, &ps);

			ReleaseMutex(m_jkDefragGui->m_displayMutex);

			KillTimer(m_jkDefragGui->m_hWnd, m_jkDefragGui->m_sizeTimer);
		}
*/


		InvalidateRect(hWnd,NULL,FALSE);

		return 0;

	case WM_PAINT:
		{
			/* Grab the display mutex, to make sure that we are the only thread changing the window. */
			WaitForSingleObject(m_jkDefragGui->m_displayMutex,100);

			m_jkDefragGui->m_displayMutex = CreateMutex(NULL,FALSE,"JKDefrag");

			PAINTSTRUCT ps;

			m_jkDefragGui->m_hDC = BeginPaint(hWnd, &ps);

			m_jkDefragGui->OnPaint(m_jkDefragGui->m_hDC);

			EndPaint(hWnd, &ps);

			ReleaseMutex(m_jkDefragGui->m_displayMutex);
		}

		return 0;


	case WM_ERASEBKGND:
		{
//			m_jkDefragGui->RedrawScreen = 0;
			InvalidateRect(m_jkDefragGui->m_hWnd,NULL,FALSE);
		}

		return 0;
/*
	case WM_WINDOWPOSCHANGED:
		{
			m_jkDefragGui->RedrawScreen = 0;
			InvalidateRect(m_jkDefragGui->m_hWnd,NULL,FALSE);
		}

		return 0;
*/

	case WM_SIZE:
		{

/*
			m_jkDefragGui->m_sizeTimer = SetTimer(m_jkDefragGui->m_hWnd,333,500,NULL);
*/



			PAINTSTRUCT ps;

			WaitForSingleObject(m_jkDefragGui->m_displayMutex,100);

			m_jkDefragGui->m_displayMutex = CreateMutex(NULL,FALSE,"JKDefrag");

			m_jkDefragGui->m_hDC = BeginPaint(hWnd, &ps);

			m_jkDefragGui->setDisplayData(m_jkDefragGui->m_hDC);

			m_jkDefragGui->FillSquares( 0, m_jkDefragGui->m_numDiskSquares);

			m_jkDefragGui->PaintImage(m_jkDefragGui->m_hDC);

			EndPaint(hWnd, &ps);

			ReleaseMutex(m_jkDefragGui->m_displayMutex);

		}

		return 0;
	}

	return(DefWindowProc(hWnd,Message,wParam,lParam));
}

void JKDefragGui::FillSquares( int clusterStartSquareNum, int clusterEndSquareNum )
{
	float clusterPerSquare = (float)(m_numClusters / m_numDiskSquares);

	for(int ii = clusterStartSquareNum; ii <= clusterEndSquareNum; ii++)
	{
		byte currentColor = JKDefragStruct::COLOREMPTY;

		byte clusterEmpty = 0;
		byte clusterAllocated = 0;
		byte clusterUnfragmented = 0;
		byte clusterUnmovable= 0;
		byte clusterFragmented = 0;
		byte clusterBusy = 0;
		byte clusterMft = 0;
		byte clusterSpacehog = 0;

		for(int kk = (int)(ii * clusterPerSquare); kk < m_numClusters && kk < (int)((ii + 1) * clusterPerSquare); kk++)
		{
			switch (clusterInfo[kk])
			{
			case JKDefragStruct::COLOREMPTY:
				clusterEmpty = 1;
				break;
			case JKDefragStruct::COLORALLOCATED:
				clusterAllocated = 1;
				break;
			case JKDefragStruct::COLORUNFRAGMENTED:
				clusterUnfragmented = 1;
				break;
			case JKDefragStruct::COLORUNMOVABLE:
				clusterUnmovable = 1;
				break;
			case JKDefragStruct::COLORFRAGMENTED:
				clusterFragmented = 1;
				break;
			case JKDefragStruct::COLORBUSY:
				clusterBusy = 1;
				break;
			case JKDefragStruct::COLORMFT:
				clusterMft = 1;
				break;
			case JKDefragStruct::COLORSPACEHOG:
				clusterSpacehog = 1;
				break;
			}
		}

		if (ii < m_numDiskSquares)
		{
			m_clusterSquares[ii].dirty = true;
			m_clusterSquares[ii].color = //maxColor;
				clusterEmpty << 7 |
				clusterAllocated << 6 |
				clusterUnfragmented << 5 |
				clusterUnmovable << 4 |
				clusterFragmented << 3 |
				clusterBusy << 2 |
				clusterMft << 1 |
				clusterSpacehog;
		}
	}
}

/*

Show a map on the screen of all the clusters on disk. The map shows
which clusters are free and which are in use.
The Data->RedrawScreen flag controls redrawing of the screen. It is set
to "2" (busy) when the subroutine starts. If another thread changes it to
"1" (request) while the subroutine is busy then it will immediately exit
without completing the redraw. When redrawing is completely finished the
flag is set to "0" (no).

*/
void JKDefragGui::ShowDiskmap(struct DefragDataStruct *Data)
{
	struct ItemStruct *Item;

	STARTING_LCN_INPUT_BUFFER BitmapParam;

	struct
	{
		ULONG64 StartingLcn;
		ULONG64 BitmapSize;

		BYTE Buffer[65536];               /* Most efficient if binary multiple. */
	} BitmapData;

	ULONG64 Lcn;
	ULONG64 ClusterStart;

	DWORD ErrorCode;

	int Index;
	int IndexMax;

	BYTE Mask;

	int InUse;
	int PrevInUse;

	DWORD w;

	int i;

//	*Data->RedrawScreen = 2;                       /* Set the flag to "busy". */

	/* Exit if the library is not processing a disk yet. */
	if (Data->Disk.VolumeHandle == NULL)
	{
//		*Data->RedrawScreen = 0;                       /* Set the flag to "no". */
		return;
	}

	/* Clear screen. */
	ClearScreen(NULL);

	/* Show the map of all the clusters in use. */
	Lcn = 0;
	ClusterStart = 0;
	PrevInUse = 1;

	do
	{
		if (*Data->Running != RUNNING) break;
//		if (*Data->RedrawScreen != 2) break;
		if (Data->Disk.VolumeHandle == INVALID_HANDLE_VALUE) break;

		/* Fetch a block of cluster data. */
		BitmapParam.StartingLcn.QuadPart = Lcn;

		ErrorCode = DeviceIoControl(Data->Disk.VolumeHandle,FSCTL_GET_VOLUME_BITMAP,
			&BitmapParam,sizeof(BitmapParam),&BitmapData,sizeof(BitmapData),&w,NULL);

		if (ErrorCode != 0)
		{
			ErrorCode = NO_ERROR;
		}
		else
		{
			ErrorCode = GetLastError();
		}

		if ((ErrorCode != NO_ERROR) && (ErrorCode != ERROR_MORE_DATA)) break;

		/* Sanity check. */
		if (Lcn >= BitmapData.StartingLcn + BitmapData.BitmapSize) break;

		/* Analyze the clusterdata. We resume where the previous block left off. */
		Lcn = BitmapData.StartingLcn;
		Index = 0;
		Mask = 1;

		IndexMax = sizeof(BitmapData.Buffer);

		if (BitmapData.BitmapSize / 8 < IndexMax) IndexMax = (int)(BitmapData.BitmapSize / 8);

		while ((Index < IndexMax) && (*Data->Running == RUNNING))
		{
			InUse = (BitmapData.Buffer[Index] & Mask);

			/* If at the beginning of the disk then copy the InUse value as our
			starting value. */
			if (Lcn == 0) PrevInUse = InUse;

			/* At the beginning and end of an Exclude draw the cluster. */
			if ((Lcn == Data->MftExcludes[0].Start) || (Lcn == Data->MftExcludes[0].End) ||
				(Lcn == Data->MftExcludes[1].Start) || (Lcn == Data->MftExcludes[1].End) ||
				(Lcn == Data->MftExcludes[2].Start) || (Lcn == Data->MftExcludes[2].End))
			{
				if ((Lcn == Data->MftExcludes[0].End) ||
					(Lcn == Data->MftExcludes[1].End) ||
					(Lcn == Data->MftExcludes[2].End))
				{
					DrawCluster(Data,ClusterStart,Lcn,JKDefragStruct::COLORUNMOVABLE);
				}
				else
				if (PrevInUse == 0)
				{
					DrawCluster(Data,ClusterStart,Lcn,JKDefragStruct::COLOREMPTY);
				}
				else
				{
					DrawCluster(Data,ClusterStart,Lcn,JKDefragStruct::COLORALLOCATED);
				}

				InUse = 1;
				PrevInUse = 1;
				ClusterStart = Lcn;
			}

			if ((PrevInUse == 0) && (InUse != 0))          /* Free */
			{
				DrawCluster(Data,ClusterStart,Lcn,JKDefragStruct::COLOREMPTY);

				ClusterStart = Lcn;
			}

			if ((PrevInUse != 0) && (InUse == 0))          /* In use */
			{
				DrawCluster(Data,ClusterStart,Lcn,JKDefragStruct::COLORALLOCATED);

				ClusterStart = Lcn;
			}

			PrevInUse = InUse;

			if (Mask == 128)
			{
				Mask = 1;
				Index = Index + 1;
			}
			else
			{
				Mask = Mask << 1;
			}

			Lcn = Lcn + 1;
		}
	} while ((ErrorCode == ERROR_MORE_DATA) && (Lcn < BitmapData.StartingLcn + BitmapData.BitmapSize));

	if ((Lcn > 0)/* && (*Data->RedrawScreen == 2)*/)
	{
		if (PrevInUse == 0)          /* Free */
		{
			DrawCluster(Data,ClusterStart,Lcn,JKDefragStruct::COLOREMPTY);
		}

		if (PrevInUse != 0)          /* In use */
		{
			DrawCluster(Data,ClusterStart,Lcn,JKDefragStruct::COLORALLOCATED);
		}
	}

	/* Show the MFT zones. */
	for (i = 0; i < 3; i++)
	{
//		if (*Data->RedrawScreen != 2) break;
		if (Data->MftExcludes[i].Start <= 0) continue;

		DrawCluster(Data,Data->MftExcludes[i].Start, Data->MftExcludes[i].End, JKDefragStruct::COLORMFT);
	}

	/* Colorize all the files on the screen.
	Note: the "$BadClus" file on NTFS disks maps the entire disk, so we have to
	ignore it. */
	for (Item = m_jkLib->TreeSmallest(Data->ItemTree); Item != NULL; Item = m_jkLib->TreeNext(Item))
	{
		if (*Data->Running != RUNNING) break;
//		if (*Data->RedrawScreen != 2) break;

		if ((Item->LongFilename != NULL) &&
			((_wcsicmp(Item->LongFilename,L"$BadClus") == 0) ||
			(_wcsicmp(Item->LongFilename,L"$BadClus:$Bad:$DATA") == 0))) continue;

		m_jkLib->ColorizeItem(Data,Item,0,0,NO);
	}

	/* Set the flag to "no". */
//	if (*Data->RedrawScreen == 2) *Data->RedrawScreen = 0;
}
