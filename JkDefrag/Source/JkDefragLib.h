/*

The JkDefragDll library.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

For the full text of the license see the "License lgpl.txt" file.

Jeroen C. Kessels
Internet Engineer
http://www.kessels.com/

*/


/* Include guard */

#ifndef JKDEFRAGLIB
#define JKDEFRAGLIB

#define NO     0
#define YES    1

#define VIRTUALFRAGMENT 18446744073709551615          /* _UI64_MAX - 1 */

/* The three running states. */
#define RUNNING  0
#define STOPPING 1
#define STOPPED  2

/* List in memory of the fragments of a file. */

struct FragmentListStruct
{
	ULONG64 Lcn;                            /* Logical cluster number, location on disk. */
	ULONG64 NextVcn;                        /* Virtual cluster number of next fragment. */
	struct FragmentListStruct *Next;
};

/* List in memory of all the files on disk, sorted by LCN (Logical Cluster Number). */

struct ItemStruct
{
	struct ItemStruct *Parent;              /* Parent item. */
	struct ItemStruct *Smaller;             /* Next smaller item. */
	struct ItemStruct *Bigger;              /* Next bigger item. */

	WCHAR     *LongFilename;                /* Long filename. */
	WCHAR     *LongPath;                    /* Full path on disk, long filenames. */
	WCHAR     *ShortFilename;               /* Short filename (8.3 DOS). */
	WCHAR     *ShortPath;                   /* Full path on disk, short filenames. */

	ULONG64   Bytes;                        /* Total number of bytes. */
	ULONG64   Clusters;                     /* Total number of clusters. */
	ULONG64   CreationTime;                 /* 1 second = 10000000 */
	ULONG64   MftChangeTime;
	ULONG64   LastAccessTime;

	struct FragmentListStruct *Fragments;   /* List of fragments. */

	ULONG64   ParentInode;                  /* The Inode number of the parent directory. */

	struct ItemStruct *ParentDirectory;

	BOOL      Directory;                    /* YES: it's a directory. */
	BOOL      Unmovable;                    /* YES: file can't/couldn't be moved. */
	BOOL      Exclude;                      /* YES: file is not to be defragged/optimized. */
	BOOL      SpaceHog;                     /* YES: file to be moved to end of disk. */
};

enum DiskType
{
	UnknownType = 0,
	NTFS        = 1,
	FAT12       = 12,
	FAT16       = 16,
	FAT32       = 32
};

/* Information about a disk volume. */

struct DiskStruct
{
	HANDLE    VolumeHandle;

	WCHAR     *MountPoint;          /* Example: "c:" */
	WCHAR     *MountPointSlash;     /* Example: "c:\" */
	WCHAR     VolumeName[52];       /* Example: "\\?\Volume{08439462-3004-11da-bbca-806d6172696f}" */
	WCHAR     VolumeNameSlash[52];  /* Example: "\\?\Volume{08439462-3004-11da-bbca-806d6172696f}\" */

	DiskType  Type;

	ULONG64   MftLockedClusters;    /* Number of clusters at begin of MFT that cannot be moved. */
};

/* List of clusters used by the MFT. */

struct ExcludesStruct
{
	ULONG64 Start;
	ULONG64 End;
};

/* The big data struct that holds all the defragger's variables for a single thread. */

struct DefragDataStruct
{
	int Phase;                             /* The current Phase (1...3). */
	int Zone;                              /* The current Zone (0..2) for Phase 3. */
	int *Running;                          /* If not RUNNING then stop defragging. */
//	int *RedrawScreen;                     /* 0:no, 1:request, 2: busy. */
	BOOL UseLastAccessTime;                /* If TRUE then use LastAccessTime for SpaceHogs. */
	int CannotMoveDirs;                    /* If bigger than 20 then do not move dirs. */

	WCHAR *IncludeMask;                    /* Example: "c:\t1\*" */
	struct DiskStruct Disk;

	double FreeSpace;                      /* Percentage of total disk size 0..100. */

	/* Tree in memory with information about all the files. */

	struct ItemStruct *ItemTree;
	int BalanceCount;
	WCHAR **Excludes;                      /* Array with exclude masks. */
	BOOL UseDefaultSpaceHogs;              /* TRUE: use the built-in SpaceHogs. */
	WCHAR **SpaceHogs;                     /* Array with SpaceHog masks. */
	ULONG64 Zones[4];                      /* Begin (LCN) of the zones. */
	
	struct ExcludesStruct MftExcludes[3];  /* List of clusters reserved for the MFT. */

	/* Counters filled before Phase 1. */

	ULONG64 TotalClusters;                 /* Size of the volume, in clusters. */
	ULONG64 BytesPerCluster;               /* Number of bytes per cluster. */

	/* Counters updated before/after every Phase. */

	ULONG64 CountFreeClusters;             /* Number of free clusters. */
	ULONG64 CountGaps;                     /* Number of gaps. */
	ULONG64 BiggestGap;                    /* Size of biggest gap, in clusters. */
	ULONG64 CountGapsLess16;               /* Number of gaps smaller than 16 clusters. */
	ULONG64 CountClustersLess16;           /* Number of clusters in gaps that are smaller than 16 clusters. */

	/* Counters updated after every Phase, but not before Phase 1 (analyze). */

	ULONG64 CountDirectories;              /* Number of analysed subdirectories. */
	ULONG64 CountAllFiles;                 /* Number of analysed files. */
	ULONG64 CountFragmentedItems;          /* Number of fragmented files. */
	ULONG64 CountAllBytes;                 /* Bytes in analysed files. */
	ULONG64 CountFragmentedBytes;          /* Bytes in fragmented files. */
	ULONG64 CountAllClusters;              /* Clusters in analysed files. */
	ULONG64 CountFragmentedClusters;       /* Clusters in fragmented files. */
	double AverageDistance;                /* Between end and begin of files. */

	/* Counters used to calculate the percentage of work done. */

	ULONG64 PhaseTodo;                     /* Number of items to do in this Phase. */
	ULONG64 PhaseDone;                     /* Number of items already done in this Phase. */

	/* Variables used to throttle the speed. */

	int Speed;                            /* Speed as a percentage 1..100. */
	LONG64 StartTime;
	LONG64 RunningTime;
	LONG64 LastCheckpoint;

	/* The array with error messages. */
	WCHAR **DebugMsg;
};


class JKDefragLib
{
public:
	JKDefragLib();
	~JKDefragLib();

	static JKDefragLib *getInstance();

/* Run the defragger/optimizer.

The parameters:

Path:
	The name of a disk, mountpoint, directory, or file. It may contain
	wildcards '*' and '?'. If Path is empty or NULL then defrag all the
	mounted, writable, fixed disks on the computer. Some examples:

	c:
	c:\xyz
	c:\xyz\*.txt
	\\?\Volume{08439462-3004-11da-bbca-806d6172696f}

Mode:
	0 = Analyze only, do not defragment and do not optimize.
	1 = Analyze and fixup, do not optimize.
	2 = Analyze, fixup, and fast optimization (default).
	3 = Deprecated. Analyze, fixup, and full optimization.
	4 = Analyze and force together.
	5 = Analyze and move to end of disk.
	6 = Analyze and sort files by name.
	7 = Analyze and sort files by size (smallest first).
	8 = Analyze and sort files by last access (newest first).
	9 = Analyze and sort files by last change (oldest first).
	10 = Analyze and sort files by creation time (oldest first).

Speed:
	Percentage 0...100 of the normal speed. The defragger will slow down
	by inserting sleep periods so that the wall time is 100% and the
	actual processing time is this percentage. Specify 100 (or zero) to
	run at maximum speed.

FreeSpace:
	Percentage 0...100 of the total volume space that must be kept
	free after the MFT and directories.

Excludes:
	Array of strings. Each string contains a mask, last string must be
	NULL. If an item (disk, file, directory) matches one of the strings
	in this array then it will be ignored (skipped). Specify NULL to
	disable this feature.

SpaceHogs:
	Array of strings. Each string contains a mask, last string must be
	NULL. If an item (file, directory) matches one of the strings in
	this array then it will be marked as a space hog and moved to the end
	of the disk. A build-in list of spacehogs will be added to this list,
	except if one of the strings in the array is "DisableDefaults".

Running:
	Pointer to an integer. It is used by the StopJkDefrag() subroutine
	to stop the defragger. If the pointer is NULL then this feature is
	disabled.

RedrawScreen:
	Pointer to an integer. It can be used by other threads to signal the
	defragger that it must redraw the screen, for example when the window
	is resized. If the pointer is NULL then this feature is disabled.

ShowStatus:
	Callback subroutine. Is called just before the defragger starts a
	new Phase, and when it finishes a volume. Specify NULL if the callback
	is not needed.

ShowMove:
	Callback subroutine. Is called whenever an item (file, directory) is
	moved on disk. Specify NULL if the callback is not needed.

ShowAnalyze:
	Callback subroutine. Is called for every file during analysis.
	This subroutine is called one last time with Item=NULL when analysis
	has finished. Specify NULL if the callback is not needed.

ShowDebug:
	Callback subroutine. Is called for every message to show. Specify NULL
	if the callback is not needed.

DrawCluster:
	Callback subroutine. Is called to paint a fragment on the screen in
	a color. There are 7 colors, see the .h file. Specify NULL if the
	callback is not needed.

ClearScreen:
	Callback subroutine. Is called when the defragger wants to clear the
	diskmap on the screen. Specify NULL if the callback is not needed.

DebugMsg:
	Array of textmessages, used when the defragger wants to show a debug
	message. Specify NULL to use the internal default array of english text
	messages.
*/

__declspec(dllexport) void              RunJkDefrag(WCHAR *Path, int Mode, int Speed, double FreeSpace, WCHAR **Excludes, WCHAR **SpaceHogs, int *Running,
													/*int *RedrawScreen, */WCHAR **DebugMsg);

/*

Stop the defragger. Wait for a maximum of TimeOut milliseconds for the
defragger to stop. If TimeOut is zero then wait indefinitely. If TimeOut is
negative then immediately return without waiting.
Note: The "Running" variable must be the same as what was given to the
RunJkDefrag() subroutine.

*/
__declspec(dllexport) void              StopJkDefrag(int *Running, int TimeOut);

/* Other exported functions that might be useful in programs that use JkDefrag. */

__declspec(dllexport) char              *stristr(char *Haystack, char *Needle);
__declspec(dllexport) WCHAR             *stristrW(WCHAR *Haystack, WCHAR *Needle);

__declspec(dllexport) void               SystemErrorStr(DWORD ErrorCode, WCHAR *Out, size_t Width);
__declspec(dllexport) void               ShowHex(struct DefragDataStruct *Data, BYTE *Buffer, ULONG64 Count);

__declspec(dllexport) int                MatchMask(WCHAR *String, WCHAR *Mask);

__declspec(dllexport) WCHAR            **AddArrayString(WCHAR **Array, WCHAR *NewString);
__declspec(dllexport) WCHAR             *GetShortPath(struct DefragDataStruct *Data, struct ItemStruct *Item);
__declspec(dllexport) WCHAR             *GetLongPath(struct DefragDataStruct *Data, struct ItemStruct *Item);

__declspec(dllexport) void               SlowDown(struct DefragDataStruct *Data);

__declspec(dllexport) ULONG64            GetItemLcn(struct ItemStruct *Item);

__declspec(dllexport) struct ItemStruct *TreeSmallest(struct ItemStruct *Top);
__declspec(dllexport) struct ItemStruct *TreeBiggest(struct ItemStruct *Top);
__declspec(dllexport) struct ItemStruct *TreeFirst(struct ItemStruct *Top, int Direction);
__declspec(dllexport) struct ItemStruct *TreePrev(struct ItemStruct *Here);
__declspec(dllexport) struct ItemStruct *TreeNext(struct ItemStruct *Here);
__declspec(dllexport) struct ItemStruct *TreeNextPrev(struct ItemStruct *Here, int Direction);

__declspec(dllexport) void               TreeInsert(struct DefragDataStruct *Data, struct ItemStruct *New);
__declspec(dllexport) void               TreeDetach(struct DefragDataStruct *Data, struct ItemStruct *Item);
__declspec(dllexport) void               DeleteItemTree(struct ItemStruct *Top);
__declspec(dllexport) int                FragmentCount(struct ItemStruct *Item);
__declspec(dllexport) int                IsFragmented(struct ItemStruct *Item, ULONG64 Offset, ULONG64 Size);
__declspec(dllexport) void               ColorizeItem(struct DefragDataStruct *Data,struct ItemStruct *Item, ULONG64 BusyOffset, ULONG64 BusySize, int UnDraw);
/*
__declspec(dllexport) void               ShowDiskmap(struct DefragDataStruct *Data);
*/
__declspec(dllexport) void               CallShowStatus(struct DefragDataStruct *Data, int Phase, int Zone);

private:
	WCHAR LowerCase(WCHAR c);

	void AppendToShortPath(struct ItemStruct *Item, WCHAR *Path, size_t Length);
	void AppendToLongPath(struct ItemStruct *Item, WCHAR *Path, size_t Length);
	ULONG64 FindFragmentBegin(struct ItemStruct *Item, ULONG64 Lcn);

	struct ItemStruct *FindItemAtLcn(struct DefragDataStruct *Data, ULONG64 Lcn);
	HANDLE OpenItemHandle(struct DefragDataStruct *Data, struct ItemStruct *Item);
	int GetFragments(struct DefragDataStruct *Data, struct ItemStruct *Item, HANDLE FileHandle);

	int FindGap(struct DefragDataStruct *Data,
		ULONG64 MinimumLcn,          /* Gap must be at or above this LCN. */
		ULONG64 MaximumLcn,          /* Gap must be below this LCN. */
		ULONG64 MinimumSize,         /* Gap must be at least this big. */
		int MustFit,                 /* YES: gap must be at least MinimumSize. */
		int FindHighestGap,          /* YES: return the last gap that fits. */
		ULONG64 *BeginLcn,           /* Result, LCN of begin of cluster. */
		ULONG64 *EndLcn,             /* Result, LCN of end of cluster. */
		BOOL IgnoreMftExcludes);

	void CalculateZones(struct DefragDataStruct *Data);

	DWORD MoveItem1(struct DefragDataStruct *Data,
		HANDLE FileHandle,
		struct ItemStruct *Item,
		ULONG64 NewLcn,                   /* Where to move to. */
		ULONG64 Offset,                   /* Number of first cluster to be moved. */
		ULONG64 Size);                    /* Number of clusters to be moved. */

	DWORD MoveItem2(struct DefragDataStruct *Data,
		HANDLE FileHandle,
		struct ItemStruct *Item,
		ULONG64 NewLcn,                /* Where to move to. */
		ULONG64 Offset,                /* Number of first cluster to be moved. */
		ULONG64 Size);                 /* Number of clusters to be moved. */

	int MoveItem3(struct DefragDataStruct *Data,
		struct ItemStruct *Item,
		HANDLE FileHandle,
		ULONG64 NewLcn,          /* Where to move to. */
		ULONG64 Offset,          /* Number of first cluster to be moved. */
		ULONG64 Size,            /* Number of clusters to be moved. */
		int Strategy);            /* 0: move in one part, 1: move individual fragments. */

	int MoveItem4(struct DefragDataStruct *Data,
		struct ItemStruct *Item,
		HANDLE FileHandle,
		ULONG64 NewLcn,                                   /* Where to move to. */
		ULONG64 Offset,                /* Number of first cluster to be moved. */
		ULONG64 Size,                       /* Number of clusters to be moved. */
		int Direction);                          /* 0: move up, 1: move down. */

	int MoveItem(struct DefragDataStruct *Data,
		struct ItemStruct *Item,
		ULONG64 NewLcn,                                   /* Where to move to. */
		ULONG64 Offset,                /* Number of first cluster to be moved. */
		ULONG64 Size,                       /* Number of clusters to be moved. */
		int Direction);                          /* 0: move up, 1: move down. */

	struct ItemStruct *FindHighestItem(struct DefragDataStruct *Data,
		ULONG64 ClusterStart,
		ULONG64 ClusterEnd,
		int Direction,
		int Zone);

	struct ItemStruct *FindBestItem(struct DefragDataStruct *Data,
		ULONG64 ClusterStart,
		ULONG64 ClusterEnd,
		int Direction,
		int Zone);

	void CompareItems(struct DefragDataStruct *Data, struct ItemStruct *Item);

	int  CompareItems(struct ItemStruct *Item1, struct ItemStruct *Item2, int SortField);

	void ScanDir(struct DefragDataStruct *Data, WCHAR *Mask, struct ItemStruct *ParentDirectory);

	void AnalyzeVolume(struct DefragDataStruct *Data);

	void Fixup(struct DefragDataStruct *Data);

	void Defragment(struct DefragDataStruct *Data);

	void ForcedFill(struct DefragDataStruct *Data);

	void Vacate(struct DefragDataStruct *Data, ULONG64 Lcn, ULONG64 Clusters, BOOL IgnoreMftExcludes);

	void MoveMftToBeginOfDisk(struct DefragDataStruct *Data);
	
	void OptimizeVolume(struct DefragDataStruct *Data);

	void OptimizeSort(struct DefragDataStruct *Data, int SortField);

	void OptimizeUp(struct DefragDataStruct *Data);

	void DefragOnePath(struct DefragDataStruct *Data, WCHAR *Path, int Mode);

	void DefragMountpoints(struct DefragDataStruct *Data, WCHAR *MountPoint, int Mode);
/*
	JKScanFat   *m_jkScanFat;
	JKScanNtfs  *m_jkScanNtfs;
*/

	// static member that is an instance of itself
	static JKDefragLib *m_jkDefragLib;
};

#endif    /* Include guard */