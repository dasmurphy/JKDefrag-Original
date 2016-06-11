/*

The JkDefrag library.

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

#include "StdAfx.h"

JKDefragLib *JKDefragLib::m_jkDefragLib = 0;

JKDefragLib::JKDefragLib()
{
}

JKDefragLib::~JKDefragLib()
{
	delete m_jkDefragLib;
}

JKDefragLib *JKDefragLib::getInstance()
{
	if (m_jkDefragLib == NULL)
	{
		m_jkDefragLib = new JKDefragLib();
	}

	return m_jkDefragLib;
}

/*

All the text strings used by the defragger library.
Note: The RunJkDefrag() function call has a parameter where you can specify
a different array. Do not change this default array, simply create a new
array in your program and specify it as a parameter.

*/
WCHAR *DefaultDebugMsg[] =
{
	/*  0 */   L"",
	/*  1 */   L"",
	/*  2 */   L"",
	/*  3 */   L"",
	/*  4 */   L"",
	/*  5 */   L"",
	/*  6 */   L"",
	/*  7 */   L"",
	/*  8 */   L"",
	/*  9 */   L"",
	/* 10 */   L"Getting cluster bitmap: %s",
	/* 11 */   L"Extent: Lcn=%I64u, Vcn=%I64u, NextVcn=%I64u",
	/* 12 */   L"ERROR: could not get volume bitmap: %s",
	/* 13 */   L"Gap found: LCN=%I64d, Size=%I64d",
	/* 14 */   L"Processing '%s'",
	/* 15 */   L"Could not open '%s': %s",
	/* 16 */   L"%I64d clusters at %I64d, %I64d bytes",
	/* 17 */   L"Special file attribute: Compressed",
	/* 18 */   L"Special file attribute: Encrypted",
	/* 19 */   L"Special file attribute: Offline",
	/* 20 */   L"Special file attribute: Read-only",
	/* 21 */   L"Special file attribute: Sparse-file",
	/* 22 */   L"Special file attribute: Temporary",
	/* 23 */   L"Analyzing: %s",
	/* 24 */   L"",
	/* 25 */   L"Cannot move file away because no gap is big enough: %I64d[%I64d]",
	/* 26 */   L"Don't know which file is at the end of the gap: %I64d[%I64d]",
	/* 27 */   L"Enlarging gap %I64d[%I64d] by moving %I64d[%I64d]",
	/* 28 */   L"Skipping gap, cannot fill: %I64d[%I64d]",
	/* 29 */   L"Opening volume '%s' at mountpoint '%s'",
	/* 30 */   L"",
	/* 31 */   L"Volume '%s' at mountpoint '%s' is not mounted.",
	/* 32 */   L"Cannot defragment volume '%s' at mountpoint '%s'",
	/* 33 */   L"MftStartLcn=%I64d, MftZoneStart=%I64d, MftZoneEnd=%I64d, Mft2StartLcn=%I64d, MftValidDataLength=%I64d",
	/* 34 */   L"MftExcludes[%u].Start=%I64d, MftExcludes[%u].End=%I64d",
	/* 35 */   L"",
	/* 36 */   L"Ignoring volume '%s' because it is read-only.",
	/* 37 */   L"Analyzing volume '%s'",
	/* 38 */   L"Finished.",
	/* 39 */   L"Could not get list of volumes: %s",
	/* 40 */   L"Cannot find volume name for mountpoint '%s': %s",
	/* 41 */   L"Cannot enlarge gap at %I64d[%I64d] because of unmovable data.",
	/* 42 */   L"Windows could not move the file, trying alternative method.",
	/* 43 */   L"Cannot process clustermap of '%s': %s",
	/* 44 */   L"Disk is full, cannot defragment.",
	/* 45 */   L"Alternative method failed, leaving file where it is.",
	/* 46 */   L"Extent (virtual): Vcn=%I64u, NextVcn=%I64u",
	/* 47 */   L"Ignoring volume '%s' because of exclude mask '%s'.",
	/* 48 */   L"Vacating %I64u clusters starting at LCN=%I64u",
	/* 49 */   L"Vacated %I64u clusters (until %I64u) from LCN=%I64u",
	/* 50 */   L"Finished vacating %I64u clusters, until LCN=%I64u",
	/* 51 */   L"",
	/* 52 */   L"",
	/* 53 */   L"I am fragmented.",
	/* 54 */   L"I am in MFT reserved space.",
	/* 55 */   L"I am a regular file in zone 1.",
	/* 56 */   L"I am a spacehog in zone 1 or 2.",
	/* 57 */   L"Ignoring volume '%s' because it is not a harddisk."
};

/* Search case-insensitive for a substring. */
char *JKDefragLib::stristr(char *Haystack, char *Needle)
{
	register char *p1;
	register size_t i;

	if ((Haystack == NULL) || (Needle == NULL)) return(NULL);

	p1 = Haystack;
	i = strlen(Needle);

	while (*p1 != '\0')
	{
		if (_strnicmp(p1,Needle,i) == 0) return(p1);

		p1++;
	}

	return(NULL);
}

/* Search case-insensitive for a substring. */
WCHAR *JKDefragLib::stristrW(WCHAR *Haystack, WCHAR *Needle)
{
	WCHAR *p1;
	size_t i;

	if ((Haystack == NULL) || (Needle == NULL)) return(NULL);

	p1 = Haystack;
	i = wcslen(Needle);

	while (*p1 != 0)
	{
		if (_wcsnicmp(p1,Needle,i) == 0) return(p1);

		p1++;
	}

	return(NULL);
}

/* Return a string with the error message for GetLastError(). */
void JKDefragLib::SystemErrorStr(DWORD ErrorCode, WCHAR *Out, size_t Width)
{
	WCHAR s1[BUFSIZ];
	WCHAR *p1;

	FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ARGUMENT_ARRAY,
		NULL, ErrorCode, 0, s1, BUFSIZ, NULL);

	/* Strip trailing whitespace. */
	p1 = wcschr(s1,'\0');

	while (p1 != s1)
	{
		p1--;

		if ((*p1 != ' ') && (*p1 != '\t') && (*p1 != '\n') && (*p1 != '\r')) break;

		*p1 = '\0';
	}

	/* Add error number. */
	swprintf_s(Out,Width,L"[%lu] %s",ErrorCode,s1);
}

/* Translate character to lowercase. */
WCHAR JKDefragLib::LowerCase(WCHAR c)
{
	if ((c >= 'A') && (c <= 'Z')) return((c - 'A') + 'a');

	return(c);
}

/* Dump a block of data to standard output, for debugging purposes. */
void JKDefragLib::ShowHex(struct DefragDataStruct *Data, BYTE *Buffer, ULONG64 Count)
{
	JKDefragGui *jkGui = JKDefragGui::getInstance();

	WCHAR s1[BUFSIZ];
	WCHAR s2[BUFSIZ];

	int i;
	int j;

	for (i = 0; i < Count; i = i + 16)
	{
		swprintf_s(s1,BUFSIZ,L"%4u %4X   ",i,i);

		for (j = 0; j < 16; j++)
		{
			if (j == 8) wcscat_s(s1,BUFSIZ,L" ");

			if (j + i >= Count)
			{
				wcscat_s(s1,BUFSIZ,L"   ");
			}
			else
			{
				swprintf_s(s2,BUFSIZ,L"%02X ",Buffer[i + j]);
				wcscat_s(s1,BUFSIZ,s2);
			}
		}

		wcscat_s(s1,BUFSIZ,L" ");

		for (j = 0; j < 16; j++)
		{
			if (j + i >= Count)
			{
				wcscat_s(s1,BUFSIZ,L" ");
			}
			else
			{
				if (Buffer[i + j] < 32)
				{
					wcscat_s(s1,BUFSIZ,L".");
				}
				else
				{
					swprintf_s(s2,BUFSIZ,L"%c",Buffer[i + j]);
					wcscat_s(s1,BUFSIZ,s2);
				}
			}
		}

		jkGui->ShowDebug(2,NULL,L"%s",s1);
	}
}

/*

Compare a string with a mask, case-insensitive. If it matches then return
YES, otherwise NO. The mask may contain wildcard characters '?' (any
character) '*' (any characters).

*/
int JKDefragLib::MatchMask(WCHAR *String, WCHAR *Mask)
{
	WCHAR *m;
	WCHAR *s;

	if (String == NULL) return NO;                /* Just to speed up things. */
	if (Mask == NULL) return NO;
	if (wcscmp(Mask,L"*") == 0) return YES;

	m = Mask;
	s = String;

	while ((*m != '\0') && (*s != '\0'))
	{
		if ((LowerCase(*m) != LowerCase(*s)) && (*m != '?'))
		{
			if (*m != '*') return NO;
		
			m++;

			if (*m == '\0') return YES;

			while (*s != '\0')
			{
				if (MatchMask(s,m) == YES) return YES;
				s++;
			}

			return NO;
		}

		m++;
		s++;
	}

	while (*m == '*') m++;

	if ((*s == '\0') && (*m == '\0')) return YES;

	return NO;
}

/*

Add a string to a string array. If the array is empty then initialize, the
last item in the array is NULL. If the array is not empty then append the
new string, realloc() the array.

*/
WCHAR **JKDefragLib::AddArrayString(WCHAR **Array, WCHAR *NewString)
{
	WCHAR **NewArray;
	int i;

	/* Sanity check. */
	if (NewString == NULL) return(Array);

	if (Array == NULL)
	{
		NewArray = (WCHAR **)malloc(2 * sizeof(WCHAR *));

		if (NewArray == NULL) return(NULL);

		NewArray[0] = _wcsdup(NewString);

		if (NewArray[0] == NULL) return(NULL);

		NewArray[1] = NULL;

		return(NewArray);
	}

	i = 0;

	while (Array[i] != NULL) i++;

	NewArray = (WCHAR **)realloc(Array,(i + 2) * sizeof(WCHAR *));

	if (NewArray == NULL) return(NULL);

	NewArray[i] = _wcsdup(NewString);

	if (NewArray[i] == NULL) return(NULL);

	NewArray[i+1] = NULL;

	return(NewArray);
}

/* Subfunction of GetShortPath(). */
void JKDefragLib::AppendToShortPath(struct ItemStruct *Item, WCHAR *Path, size_t Length)
{
	if (Item->ParentDirectory != NULL) AppendToShortPath(Item->ParentDirectory,Path,Length);

	wcscat_s(Path,Length,L"\\");

	if (Item->ShortFilename != NULL)
	{
		wcscat_s(Path,Length,Item->ShortFilename);
	}
	else if (Item->LongFilename != NULL)
	{
		wcscat_s(Path,Length,Item->LongFilename);
	}
}

/*

Return a string with the full path of an item, constructed from the short names.
Return NULL if error. The caller must free() the new string.

*/
WCHAR *JKDefragLib::GetShortPath(struct DefragDataStruct *Data, struct ItemStruct *Item)
{
	struct ItemStruct *TempItem;
	size_t Length;
	WCHAR *Path;

	/* Sanity check. */
	if (Item == NULL) return(NULL);

	/* Count the size of all the ShortFilename's. */
	Length = wcslen(Data->Disk.MountPoint) + 1;

	for (TempItem = Item; TempItem != NULL; TempItem = TempItem->ParentDirectory)
	{
		if (TempItem->ShortFilename != NULL)
		{
			Length = Length + wcslen(TempItem->ShortFilename) + 1;
		}
		else if (TempItem->LongFilename != NULL)
		{
			Length = Length + wcslen(TempItem->LongFilename) + 1;
		}
		else
		{
			Length = Length + 1;
		}
	}

	/* Allocate new string. */
	Path = (WCHAR *)malloc(sizeof(WCHAR) * Length);

	if (Path == NULL) return(NULL);

	wcscpy_s(Path,Length,Data->Disk.MountPoint);

	/* Append all the strings. */
	AppendToShortPath(Item,Path,Length);

	return(Path);
}

/* Subfunction of GetLongPath(). */
void JKDefragLib::AppendToLongPath(struct ItemStruct *Item, WCHAR *Path, size_t Length)
{
	if (Item->ParentDirectory != NULL) AppendToLongPath(Item->ParentDirectory,Path,Length);

	wcscat_s(Path,Length,L"\\");

	if (Item->LongFilename != NULL)
	{
		wcscat_s(Path,Length,Item->LongFilename);
	}
	else if (Item->ShortFilename != NULL)
	{
		wcscat_s(Path,Length,Item->ShortFilename);
	}
}

/*

Return a string with the full path of an item, constructed from the long names.
Return NULL if error. The caller must free() the new string.

*/
WCHAR *JKDefragLib::GetLongPath(struct DefragDataStruct *Data, struct ItemStruct *Item)
{
	struct ItemStruct *TempItem;
	size_t Length;
	WCHAR *Path;

	/* Sanity check. */
	if (Item == NULL) return(NULL);

	/* Count the size of all the LongFilename's. */
	Length = wcslen(Data->Disk.MountPoint) + 1;

	for (TempItem = Item; TempItem != NULL; TempItem = TempItem->ParentDirectory)
	{
		if (TempItem->LongFilename != NULL)
		{
			Length = Length + wcslen(TempItem->LongFilename) + 1;
		}
		else if (Item->ShortFilename != NULL)
		{
			Length = Length + wcslen(TempItem->ShortFilename) + 1;
		}
		else
		{
			Length = Length + 1;
		}
	}

	/* Allocate new string. */
	Path = (WCHAR *)malloc(sizeof(WCHAR) * Length);

	if (Path == NULL) return(NULL);

	wcscpy_s(Path,Length,Data->Disk.MountPoint);

	/* Append all the strings. */
	AppendToLongPath(Item,Path,Length);

	return(Path);
}

/* Slow the program down. */
void JKDefragLib::SlowDown(struct DefragDataStruct *Data)
{
	struct __timeb64 Time;

	LONG64 Now;
	LONG64 Delay;

	/* Sanity check. */
	if ((Data->Speed <= 0) || (Data->Speed >= 100)) return;

	/* Calculate the time we have to sleep so that the wall time is 100% and the
	actual running time is the "-s" parameter percentage. */
	_ftime64_s(&Time);

	Now = Time.time * 1000 + Time.millitm;

	if (Now > Data->LastCheckpoint)
	{
		Data->RunningTime = Data->RunningTime + Now - Data->LastCheckpoint;
	}

	if (Now < Data->StartTime) Data->StartTime = Now;    /* Should never happen. */

	/* Sleep. */
	if (Data->RunningTime > 0)
	{
		Delay = Data->RunningTime * (LONG64)100 / (LONG64)(Data->Speed) - (Now - Data->StartTime);

		if (Delay > 30000) Delay = 30000;
		if (Delay > 0) Sleep((DWORD)Delay);
	}

	/* Save the current wall time, so next time we can calculate the time spent in	the program. */
	_ftime64_s(&Time);

	Data->LastCheckpoint = Time.time * 1000 + Time.millitm;
}

/* Return the location on disk (LCN, Logical Cluster Number) of an item. */
ULONG64 JKDefragLib::GetItemLcn(struct ItemStruct *Item)
{
	struct FragmentListStruct *Fragment;

	/* Sanity check. */
	if (Item == NULL) return(0);

	Fragment = Item->Fragments;

	while ((Fragment != NULL) && (Fragment->Lcn == VIRTUALFRAGMENT))
	{
		Fragment = Fragment->Next;
	}

	if (Fragment == NULL) return(0);

	return(Fragment->Lcn);
}

/* Return pointer to the first item in the tree (the first file on the volume). */
struct ItemStruct *JKDefragLib::TreeSmallest(struct ItemStruct *Top)
{
	if (Top == NULL) return(NULL);

	while (Top->Smaller != NULL) Top = Top->Smaller;

	return(Top);
}

/* Return pointer to the last item in the tree (the last file on the volume). */
struct ItemStruct *JKDefragLib::TreeBiggest(struct ItemStruct *Top)
{
	if (Top == NULL) return(NULL);

	while (Top->Bigger != NULL) Top = Top->Bigger;

	return(Top);
}

/*

If Direction=0 then return a pointer to the first file on the volume,
if Direction=1 then the last file.

*/
struct ItemStruct *JKDefragLib::TreeFirst(struct ItemStruct *Top, int Direction)
{
	if (Direction == 0) return(TreeSmallest(Top));

	return(TreeBiggest(Top));
}

/* Return pointer to the previous item in the tree. */
struct ItemStruct *JKDefragLib::TreePrev(struct ItemStruct *Here)
{
	struct ItemStruct *Temp;

	if (Here == NULL) return(Here);

	if (Here->Smaller != NULL)
	{
		Here = Here->Smaller;

		while (Here->Bigger != NULL) Here = Here->Bigger;

		return(Here);
	}

	do
	{
		Temp = Here;
		Here = Here->Parent;
	} while ((Here != NULL) && (Here->Smaller == Temp));

	return(Here);
}

/* Return pointer to the next item in the tree. */
struct ItemStruct *JKDefragLib::TreeNext(struct ItemStruct *Here)
{
	struct ItemStruct *Temp;

	if (Here == NULL) return(NULL);

	if (Here->Bigger != NULL)
	{
		Here = Here->Bigger;

		while (Here->Smaller != NULL) Here = Here->Smaller;

		return(Here);
	}

	do
	{
		Temp = Here;
		Here = Here->Parent;
	} while ((Here != NULL) && (Here->Bigger == Temp));

	return(Here);
}

/*

If Direction=0 then return a pointer to the next file on the volume,
if Direction=1 then the previous file.

*/
struct ItemStruct *JKDefragLib::TreeNextPrev(struct ItemStruct *Here, int Direction)
{
	if (Direction == 0) return(TreeNext(Here));

	return(TreePrev(Here));
}

/* Insert a record into the tree. The tree is sorted by LCN (Logical Cluster Number). */
void JKDefragLib::TreeInsert(struct DefragDataStruct *Data, struct ItemStruct *New)
{
	struct ItemStruct *Here;
	struct ItemStruct *Ins;

	ULONG64 HereLcn;
	ULONG64 NewLcn;

	int Found;

	struct ItemStruct *A;
	struct ItemStruct *B;
	struct ItemStruct *C;

	long Count;
	long Skip;

	if (New == NULL) return;

	NewLcn = GetItemLcn(New);

	/* Locate the place where the record should be inserted. */
	Here = Data->ItemTree;
	Ins = NULL;
	Found = 1;

	while (Here != NULL)
	{
		Ins = Here;
		Found = 0;

		HereLcn = GetItemLcn(Here);

		if (HereLcn > NewLcn)
		{
			Found = 1;
			Here = Here->Smaller;
		}
		else
		{
			if (HereLcn < NewLcn) Found = -1;

			Here = Here->Bigger;
		}
	}

	/* Insert the record. */
	New->Parent = Ins;
	New->Smaller = NULL;
	New->Bigger = NULL;

	if (Ins == NULL)
	{
		Data->ItemTree = New;
	}
	else
	{
		if (Found > 0)
		{
			Ins->Smaller = New;
		}
		else
		{
			Ins->Bigger = New;
		}
	}

	/* If there have been less than 1000 inserts then return. */
	Data->BalanceCount = Data->BalanceCount + 1;

	if (Data->BalanceCount < 1000) return;

	/* Balance the tree.
	It's difficult to explain what exactly happens here. For an excellent
	tutorial see:
	http://www.stanford.edu/~blp/avl/libavl.html/Balancing-a-BST.html
	*/

	Data->BalanceCount = 0;

	/* Convert the tree into a vine. */
	A = Data->ItemTree;
	C = A;
	Count = 0;

	while (A != NULL)
	{
		/* If A has no Bigger child then move down the tree. */
		if (A->Bigger == NULL)
		{
			Count = Count + 1;
			C = A;
			A = A->Smaller;

			continue;
		}

		/* Rotate left at A. */
		B = A->Bigger;

		if (Data->ItemTree == A) Data->ItemTree = B;

		A->Bigger = B->Smaller;

		if (A->Bigger != NULL) A->Bigger->Parent = A;

		B->Parent = A->Parent;

		if (B->Parent != NULL)
		{
			if (B->Parent->Smaller == A)
			{
				B->Parent->Smaller = B;
			}
			else
			{
				A->Parent->Bigger = B;
			}
		}

		B->Smaller = A;
		A->Parent = B;

		/* Do again. */
		A = B;
	}

	/* Calculate the number of skips. */
	Skip = 1;

	while (Skip < Count + 2) Skip = (Skip << 1);

	Skip = Count + 1 - (Skip >> 1);

	/* Compress the tree. */
	while (C != NULL)
	{
		if (Skip <= 0) C = C->Parent;

		A = C;

		while (A != NULL)
		{
			B = A;
			A = A->Parent;

			if (A == NULL) break;

			/* Rotate right at A. */
			if (Data->ItemTree == A) Data->ItemTree = B;

			A->Smaller = B->Bigger;

			if (A->Smaller != NULL) A->Smaller->Parent = A;

			B->Parent = A->Parent;

			if (B->Parent != NULL)
			{
				if (B->Parent->Smaller == A)
				{
					B->Parent->Smaller = B;
				}
				else
				{
					B->Parent->Bigger = B;
				}
			}

			A->Parent = B;
			B->Bigger = A;

			/* Next item. */
			A = B->Parent;

			/* If there were skips then leave if all done. */
			Skip = Skip - 1;
			if (Skip == 0) break;
		}
	}
}

/*

Detach (unlink) a record from the tree. The record is not freed().
See: http://www.stanford.edu/~blp/avl/libavl.html/Deleting-from-a-BST.html

*/
void JKDefragLib::TreeDetach(struct DefragDataStruct *Data, struct ItemStruct *Item)
{
	struct ItemStruct *B;

	/* Sanity check. */
	if ((Data->ItemTree == NULL) || (Item == NULL)) return;

	if (Item->Bigger == NULL)
	{
		/* It is trivial to delete a node with no Bigger child. We replace
		the pointer leading to the node by it's Smaller child. In
		other words, we replace the deleted node by its Smaller child. */
		if (Item->Parent != NULL)
		{
			if (Item->Parent->Smaller == Item)
			{
				Item->Parent->Smaller = Item->Smaller;
			}
			else
			{
				Item->Parent->Bigger = Item->Smaller;
			}
		}
		else
		{
			Data->ItemTree = Item->Smaller;
		}

		if (Item->Smaller != NULL) Item->Smaller->Parent = Item->Parent;
	}
	else if (Item->Bigger->Smaller == NULL)
	{
		/* The Bigger child has no Smaller child. In this case, we move Bigger
		into the node's place, attaching the node's Smaller subtree as the
		new Smaller. */
		if (Item->Parent != NULL)
		{
			if (Item->Parent->Smaller == Item)
			{
				Item->Parent->Smaller = Item->Bigger;
			}
			else
			{
				Item->Parent->Bigger = Item->Bigger;
			}
		}
		else
		{
			Data->ItemTree = Item->Bigger;
		}

		Item->Bigger->Parent = Item->Parent;
		Item->Bigger->Smaller = Item->Smaller;

		if (Item->Smaller != NULL) Item->Smaller->Parent = Item->Bigger;
	}
	else
	{
		/* Replace the node by it's inorder successor, that is, the node with
		the smallest value greater than the node. We know it exists because
		otherwise this would be case 1 or case 2, and it cannot have a Smaller
		value because that would be the node itself. The successor can
		therefore be detached and can be used to replace the node. */

		/* Find the inorder successor. */
		B = Item->Bigger;
		while (B->Smaller != NULL) B = B->Smaller;

		/* Detach the successor. */
		if (B->Parent != NULL)
		{
			if (B->Parent->Bigger == B)
			{
				B->Parent->Bigger = B->Bigger;
			}
			else
			{
				B->Parent->Smaller = B->Bigger;
			}
		}

		if (B->Bigger != NULL) B->Bigger->Parent = B->Parent;

		/* Replace the node with the successor. */
		if (Item->Parent != NULL)
		{
			if (Item->Parent->Smaller == Item)
			{
				Item->Parent->Smaller = B;
			}
			else
			{
				Item->Parent->Bigger = B;
			}
		}
		else
		{
			Data->ItemTree = B;
		}

		B->Parent = Item->Parent;
		B->Smaller = Item->Smaller;

		if (B->Smaller != NULL) B->Smaller->Parent = B;

		B->Bigger = Item->Bigger;

		if (B->Bigger != NULL) B->Bigger->Parent = B;
	}
}

/* Delete the entire ItemTree. */
void JKDefragLib::DeleteItemTree(struct ItemStruct *Top)
{
	struct FragmentListStruct *Fragment;

	if (Top == NULL) return;
	if (Top->Smaller != NULL) DeleteItemTree(Top->Smaller);
	if (Top->Bigger != NULL) DeleteItemTree(Top->Bigger);

	if ((Top->ShortPath != NULL) &&
		((Top->LongPath == NULL) ||
		(Top->ShortPath != Top->LongPath)))
	{
		free(Top->ShortPath);

		Top->ShortPath = NULL;
	}

	if ((Top->ShortFilename != NULL) &&
		((Top->LongFilename == NULL) ||
		(Top->ShortFilename != Top->LongFilename)))
	{
		free(Top->ShortFilename);

		Top->ShortFilename = NULL;
	}

	if (Top->LongPath != NULL) free(Top->LongPath);
	if (Top->LongFilename != NULL) free(Top->LongFilename);

	while (Top->Fragments != NULL)
	{
		Fragment = Top->Fragments->Next;

		free(Top->Fragments);

		Top->Fragments = Fragment;
	}

	free(Top);
}

/*

Return the LCN of the fragment that contains a cluster at the LCN. If the
item has no fragment that occupies the LCN then return zero.

*/
ULONG64 JKDefragLib::FindFragmentBegin(struct ItemStruct *Item, ULONG64 Lcn)
{
	struct FragmentListStruct *Fragment;
	ULONG64 Vcn;

	/* Sanity check. */
	if ((Item == NULL) || (Lcn == 0)) return(0);

	/* Walk through all the fragments of the item. If a fragment is found
	that contains the LCN then return the begin of that fragment. */
	Vcn = 0;
	for (Fragment = Item->Fragments; Fragment != NULL; Fragment = Fragment->Next)
	{
		if (Fragment->Lcn != VIRTUALFRAGMENT)
		{
			if ((Lcn >= Fragment->Lcn) &&
				(Lcn < Fragment->Lcn + Fragment->NextVcn - Vcn))
			{
				return(Fragment->Lcn);
			}
		}

		Vcn = Fragment->NextVcn;
	}

	/* Not found: return zero. */
	return(0);
}

/*

Search the list for the item that occupies the cluster at the LCN. Return a
pointer to the item. If not found then return NULL.

*/
struct ItemStruct *JKDefragLib::FindItemAtLcn(struct DefragDataStruct *Data, ULONG64 Lcn)
{
	struct ItemStruct *Item;
	ULONG64 ItemLcn;

	/* Locate the item by descending the sorted tree in memory. If found then
	return the item. */
	Item = Data->ItemTree;

	while (Item != NULL)
	{
		ItemLcn = GetItemLcn(Item);

		if (ItemLcn == Lcn) return(Item);

		if (Lcn < ItemLcn)
		{
			Item = Item->Smaller;
		}
		else
		{
			Item = Item->Bigger;
		}
	}

	/* Walk through all the fragments of all the items in the sorted tree. If a
	fragment is found that occupies the LCN then return a pointer to the item. */
	for (Item = TreeSmallest(Data->ItemTree); Item != NULL; Item = TreeNext(Item))
	{
		if (FindFragmentBegin(Item,Lcn) != 0) return(Item);
	}

	/* LCN not found, return NULL. */
	return(NULL);
}

/*

Open the item as a file or as a directory. If the item could not be
opened then show an error message and return NULL.

*/
HANDLE JKDefragLib::OpenItemHandle(struct DefragDataStruct *Data, struct ItemStruct *Item)
{
	HANDLE FileHandle;

	WCHAR ErrorString[BUFSIZ];
	WCHAR *Path;

	size_t Length;

	Length = wcslen(Item->LongPath) + 5;

	Path = (WCHAR *)malloc(sizeof(WCHAR) * Length);

	swprintf_s(Path,Length,L"\\\\?\\%s",Item->LongPath);

	if (Item->Directory == NO)
	{
		FileHandle = CreateFileW(Path,FILE_READ_ATTRIBUTES,
			FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
			NULL,OPEN_EXISTING,FILE_FLAG_NO_BUFFERING,NULL);
	}
	else
	{
		FileHandle = CreateFileW(Path,GENERIC_READ,
			FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
			NULL,OPEN_EXISTING,FILE_FLAG_BACKUP_SEMANTICS,NULL);
	}

	free(Path);

	if (FileHandle != INVALID_HANDLE_VALUE) return(FileHandle);

	/* Show error message: "Could not open '%s': %s" */
	SystemErrorStr(GetLastError(),ErrorString,BUFSIZ);

	JKDefragGui *jkGui = JKDefragGui::getInstance();

	jkGui->ShowDebug(4,NULL,Data->DebugMsg[15],Item->LongPath,ErrorString);

	return(NULL);
}

/*

Analyze an item (file, directory) and update it's Clusters and Fragments
in memory. If there was an error then return NO, otherwise return YES.
Note: Very small files are stored by Windows in the MFT and have no
clusters (zero) and no fragments (NULL).

*/
int JKDefragLib::GetFragments(struct DefragDataStruct *Data, struct ItemStruct *Item, HANDLE FileHandle)
{
	STARTING_VCN_INPUT_BUFFER RetrieveParam;

	struct
	{
		DWORD ExtentCount;
		ULONG64 StartingVcn;
		struct
		{
			ULONG64 NextVcn;
			ULONG64 Lcn;
		} Extents[1000];
	} ExtentData;

	BY_HANDLE_FILE_INFORMATION FileInformation;
	ULONG64 Vcn;

	struct FragmentListStruct *NewFragment;
	struct FragmentListStruct *LastFragment;

	DWORD ErrorCode;

	WCHAR ErrorString[BUFSIZ];

	int MaxLoop;

	ULARGE_INTEGER u;

	DWORD i;
	DWORD w;

	JKDefragGui *jkGui = JKDefragGui::getInstance();

	/* Initialize. If the item has an old list of fragments then delete it. */
	Item->Clusters = 0;

	while (Item->Fragments != NULL)
	{
		LastFragment = Item->Fragments->Next;

		free(Item->Fragments);

		Item->Fragments = LastFragment;
	}

	/* Fetch the date/times of the file. */
	if ((Item->CreationTime == 0) &&
		(Item->LastAccessTime == 0) &&
		(Item->MftChangeTime == 0) &&
		(GetFileInformationByHandle(FileHandle,&FileInformation) != 0))
	{
			u.LowPart = FileInformation.ftCreationTime.dwLowDateTime;
			u.HighPart = FileInformation.ftCreationTime.dwHighDateTime;

			Item->CreationTime = u.QuadPart;

			u.LowPart = FileInformation.ftLastAccessTime.dwLowDateTime;
			u.HighPart = FileInformation.ftLastAccessTime.dwHighDateTime;

			Item->LastAccessTime = u.QuadPart;

			u.LowPart = FileInformation.ftLastWriteTime.dwLowDateTime;
			u.HighPart = FileInformation.ftLastWriteTime.dwHighDateTime;

			Item->MftChangeTime = u.QuadPart;
	}

	/* Show debug message: "Getting cluster bitmap: %s" */
	jkGui->ShowDebug(4,NULL,Data->DebugMsg[10],Item->LongPath);

	/* Ask Windows for the clustermap of the item and save it in memory.
	The buffer that is used to ask Windows for the clustermap has a
	fixed size, so we may have to loop a couple of times. */
	Vcn = 0;
	MaxLoop = 1000;
	LastFragment = NULL;

	do
	{
		/* I strongly suspect that the FSCTL_GET_RETRIEVAL_POINTERS system call
		can sometimes return an empty bitmap and ERROR_MORE_DATA. That's not
		very nice of Microsoft, because it causes an infinite loop. I've
		therefore added a loop counter that will limit the loop to 1000
		iterations. This means the defragger cannot handle files with more
		than 100000 fragments, though. */
		if (MaxLoop <= 0)
		{
			jkGui->ShowDebug(2,NULL,L"FSCTL_GET_RETRIEVAL_POINTERS error: Infinite loop");

			return(NO);
		}

		MaxLoop = MaxLoop - 1;

		/* Ask Windows for the (next segment of the) clustermap of this file. If error
		then leave the loop. */
		RetrieveParam.StartingVcn.QuadPart = Vcn;

		ErrorCode = DeviceIoControl(FileHandle,FSCTL_GET_RETRIEVAL_POINTERS,
			&RetrieveParam,sizeof(RetrieveParam),&ExtentData,sizeof(ExtentData),&w,NULL);

		if (ErrorCode != 0)
		{
			ErrorCode = NO_ERROR;
		}
		else
		{
			ErrorCode = GetLastError();
		}

		if ((ErrorCode != NO_ERROR) && (ErrorCode != ERROR_MORE_DATA)) break;

		/* Walk through the clustermap, count the total number of clusters, and
		save all fragments in memory. */
		for (i = 0; i < ExtentData.ExtentCount; i++)
		{
			/* Show debug message. */
			if (ExtentData.Extents[i].Lcn != VIRTUALFRAGMENT)
			{
				/* "Extent: Lcn=%I64u, Vcn=%I64u, NextVcn=%I64u" */
				jkGui->ShowDebug(4,NULL,Data->DebugMsg[11],ExtentData.Extents[i].Lcn,Vcn,
					ExtentData.Extents[i].NextVcn);
			}
			else
			{
				/* "Extent (virtual): Vcn=%I64u, NextVcn=%I64u" */
				jkGui->ShowDebug(4,NULL,Data->DebugMsg[46],Vcn,ExtentData.Extents[i].NextVcn);
			}

			/* Add the size of the fragment to the total number of clusters.
			There are two kinds of fragments: real and virtual. The latter do not
			occupy clusters on disk, but are information used by compressed
			and sparse files. */
			if (ExtentData.Extents[i].Lcn != VIRTUALFRAGMENT)
			{
				Item->Clusters = Item->Clusters + ExtentData.Extents[i].NextVcn - Vcn;
			}

			/* Add the fragment to the Fragments. */
			NewFragment = (struct FragmentListStruct *)malloc(sizeof(struct FragmentListStruct));

			if (NewFragment != NULL)
			{
				NewFragment->Lcn = ExtentData.Extents[i].Lcn;
				NewFragment->NextVcn = ExtentData.Extents[i].NextVcn;
				NewFragment->Next = NULL;

				if (Item->Fragments == NULL)
				{
					Item->Fragments = NewFragment;
				}
				else
				{
					if (LastFragment != NULL) LastFragment->Next = NewFragment;
				}

				LastFragment = NewFragment;
			}

			/* The Vcn of the next fragment is the NextVcn field in this record. */
			Vcn = ExtentData.Extents[i].NextVcn;
		}

		/* Loop until we have processed the entire clustermap of the file. */
	} while (ErrorCode == ERROR_MORE_DATA);

	/* If there was an error while reading the clustermap then return NO. */
	if ((ErrorCode != NO_ERROR) && (ErrorCode != ERROR_HANDLE_EOF))
	{
		/* Show debug message: "Cannot process clustermap of '%s': %s" */
		SystemErrorStr(ErrorCode,ErrorString,BUFSIZ);

		jkGui->ShowDebug(3,NULL,Data->DebugMsg[43],Item->LongPath,ErrorString);

		return(NO);
	}

	return(YES);
}

/* Return the number of fragments in the item. */
int JKDefragLib::FragmentCount(struct ItemStruct *Item)
{
	struct FragmentListStruct *Fragment;

	int Fragments;

	ULONG64 Vcn;
	ULONG64 NextLcn;

	Fragments = 0;
	Vcn = 0;
	NextLcn = 0;

	for (Fragment = Item->Fragments; Fragment != NULL; Fragment = Fragment->Next)
	{
		if (Fragment->Lcn != VIRTUALFRAGMENT)
		{
			if ((NextLcn != 0) && (Fragment->Lcn != NextLcn)) Fragments++;

			NextLcn = Fragment->Lcn + Fragment->NextVcn - Vcn;
		}

		Vcn = Fragment->NextVcn;
	}

	if (NextLcn != 0) Fragments++;

	return(Fragments);
}

/*

Return YES if the block in the item starting at Offset with Size clusters
is fragmented, otherwise return NO.
Note: this function does not ask Windows for a fresh list of fragments,
it only looks at cached information in memory.

*/
int JKDefragLib::IsFragmented(struct ItemStruct *Item, ULONG64 Offset, ULONG64 Size)
{
	struct FragmentListStruct *Fragment;

	ULONG64 FragmentBegin;
	ULONG64 FragmentEnd;
	ULONG64 Vcn;
	ULONG64 NextLcn;

	/* Walk through all fragments. If a fragment is found where either the
	begin or the end of the fragment is inside the block then the file is
	fragmented and return YES. */
	FragmentBegin = 0;
	FragmentEnd = 0;
	Vcn = 0;
	NextLcn = 0;
	Fragment = Item->Fragments;

	while (Fragment != NULL)
	{
		/* Virtual fragments do not occupy space on disk and do not count as fragments. */
		if (Fragment->Lcn != VIRTUALFRAGMENT)
		{
			/* Treat aligned fragments as a single fragment. Windows will frequently
			split files in fragments even though they are perfectly aligned on disk,
			especially system files and very large files. The defragger treats these
			files as unfragmented. */
			if ((NextLcn != 0) && (Fragment->Lcn != NextLcn))
			{
				/* If the fragment is above the block then return NO, the block is
				not fragmented and we don't have to scan any further. */
				if (FragmentBegin >= Offset + Size) return(NO);

				/* If the first cluster of the fragment is above the first cluster of
				the block, or the last cluster of the fragment is before the last
				cluster of the block, then the block is fragmented, return YES. */
				if ((FragmentBegin > Offset) ||
					((FragmentEnd - 1 >= Offset) &&
					(FragmentEnd - 1 < Offset + Size - 1)))
				{
					return(YES);
				}

				FragmentBegin = FragmentEnd;
			}

			FragmentEnd = FragmentEnd + Fragment->NextVcn - Vcn;
			NextLcn = Fragment->Lcn + Fragment->NextVcn - Vcn;
		}

		/* Next fragment. */
		Vcn = Fragment->NextVcn;
		Fragment = Fragment->Next;
	}

	/* Handle the last fragment. */
	if (FragmentBegin >= Offset + Size) return(NO);

	if ((FragmentBegin > Offset) ||
		((FragmentEnd - 1 >= Offset) &&
		(FragmentEnd - 1 < Offset + Size - 1)))
	{
		return(YES);
	}

	/* Return NO, the item is not fragmented inside the block. */
	return(NO);
}

/*

Colorize an item (file, directory) on the screen in the proper color
(fragmented, unfragmented, unmovable, empty). If specified then highlight
part of the item. If Undraw=YES then remove the item from the screen.
Note: the offset and size of the highlight block is in absolute clusters,
not virtual clusters.

*/
void JKDefragLib::ColorizeItem(struct DefragDataStruct *Data,
				struct ItemStruct *Item,
				ULONG64 BusyOffset,              /* Number of first cluster to be highlighted. */
				ULONG64 BusySize,                /* Number of clusters to be highlighted. */
				int UnDraw)                      /* YES to undraw the file from the screen. */
{
	struct FragmentListStruct *Fragment;

	ULONG64 Vcn;
	ULONG64 RealVcn;

	ULONG64 SegmentBegin;
	ULONG64 SegmentEnd;

	int Fragmented;                 /* YES: file is fragmented. */
	int Color;
	int i;

	JKDefragGui *jkGui = JKDefragGui::getInstance();

	/* Determine if the item is fragmented. */
	Fragmented = IsFragmented(Item,0,Item->Clusters);

	/* Walk through all the fragments of the file. */
	Vcn = 0;
	RealVcn = 0;

	Fragment = Item->Fragments;

	while (Fragment != NULL)
	{
		/*
		Ignore virtual fragments. They do not occupy space on disk and do not require colorization.
		*/
		if (Fragment->Lcn == VIRTUALFRAGMENT)
		{
			Vcn = Fragment->NextVcn;
			Fragment = Fragment->Next;

			continue;
		}

		/*
		Walk through all the segments of the file. A segment is usually
		the same as a fragment, but if a fragment spans across a boundary
		then we must determine the color of the left and right parts
		individually. So we pretend the fragment is divided into segments
		at the various possible boundaries.
		*/
		SegmentBegin = RealVcn;

		while (SegmentBegin < RealVcn + Fragment->NextVcn - Vcn)
		{
			SegmentEnd = RealVcn + Fragment->NextVcn - Vcn;

			/* Determine the color with which to draw this segment. */
			if (UnDraw == NO)
			{
				Color = JKDefragStruct::COLORUNFRAGMENTED;

				if (Item->SpaceHog == YES) Color = JKDefragStruct::COLORSPACEHOG;
				if (Fragmented == YES) Color = JKDefragStruct::COLORFRAGMENTED;
				if (Item->Unmovable == YES) Color = JKDefragStruct::COLORUNMOVABLE;
				if (Item->Exclude == YES) Color = JKDefragStruct::COLORUNMOVABLE;

				if ((Vcn + SegmentBegin - RealVcn < BusyOffset) &&
					(Vcn + SegmentEnd - RealVcn > BusyOffset))
				{
					SegmentEnd = RealVcn + BusyOffset - Vcn;
				}

				if ((Vcn + SegmentBegin - RealVcn >= BusyOffset) &&
					(Vcn + SegmentBegin - RealVcn < BusyOffset + BusySize))
				{
					if (Vcn + SegmentEnd - RealVcn > BusyOffset + BusySize)
					{
						SegmentEnd = RealVcn + BusyOffset + BusySize - Vcn;
					}

					Color = JKDefragStruct::COLORBUSY;
				}
			}
			else
			{
				Color = JKDefragStruct::COLOREMPTY;

				for (i = 0; i < 3; i++)
				{
					if ((Fragment->Lcn + SegmentBegin - RealVcn < Data->MftExcludes[i].Start) &&
						(Fragment->Lcn + SegmentEnd - RealVcn > Data->MftExcludes[i].Start))
					{
						SegmentEnd = RealVcn + Data->MftExcludes[i].Start - Fragment->Lcn;
					}

					if ((Fragment->Lcn + SegmentBegin - RealVcn >= Data->MftExcludes[i].Start) &&
						(Fragment->Lcn + SegmentBegin - RealVcn < Data->MftExcludes[i].End))
					{
						if (Fragment->Lcn + SegmentEnd - RealVcn > Data->MftExcludes[i].End)
						{
							SegmentEnd = RealVcn + Data->MftExcludes[i].End - Fragment->Lcn;
						}

						Color = JKDefragStruct::COLORMFT;
					}
				}
			}

			/* Colorize the segment. */
			jkGui->DrawCluster(Data,Fragment->Lcn + SegmentBegin - RealVcn, Fragment->Lcn + SegmentEnd - RealVcn,Color);

			/* Next segment. */
			SegmentBegin = SegmentEnd;
		}

		/* Next fragment. */
		RealVcn = RealVcn + Fragment->NextVcn - Vcn;

		Vcn = Fragment->NextVcn;
		Fragment = Fragment->Next;
	}
}

/*

Show a map on the screen of all the clusters on disk. The map shows
which clusters are free and which are in use.
The Data->RedrawScreen flag controls redrawing of the screen. It is set
to "2" (busy) when the subroutine starts. If another thread changes it to
"1" (request) while the subroutine is busy then it will immediately exit
without completing the redraw. When redrawing is completely finished the
flag is set to "0" (no). */
/*
void ShowDiskmap2(struct DefragDataStruct *Data) {
	struct ItemStruct *Item;
	STARTING_LCN_INPUT_BUFFER BitmapParam;
	struct {
		ULONG64 StartingLcn;
		ULONG64 BitmapSize;
		BYTE Buffer[65536];               / * Most efficient if binary multiple. * /
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

	*Data->RedrawScreen = 2;                       / * Set the flag to "busy". * /

	/ * Exit if the library is not processing a disk yet. * /
	if (Data->Disk.VolumeHandle == NULL) {
		*Data->RedrawScreen = 0;                       / * Set the flag to "no". * /
		return;
	}

	/ * Clear screen. * /
	m_jkGui->ClearScreen(NULL);

	/ * Show the map of all the clusters in use. * /
	Lcn = 0;
	ClusterStart = 0;
	PrevInUse = 1;
	do {
		if (*Data->Running != RUNNING) break;
		if (*Data->RedrawScreen != 2) break;
		if (Data->Disk.VolumeHandle == INVALID_HANDLE_VALUE) break;

		/ * Fetch a block of cluster data. * /
		BitmapParam.StartingLcn.QuadPart = Lcn;
		ErrorCode = DeviceIoControl(Data->Disk.VolumeHandle,FSCTL_GET_VOLUME_BITMAP,
			&BitmapParam,sizeof(BitmapParam),&BitmapData,sizeof(BitmapData),&w,NULL);
		if (ErrorCode != 0) {
			ErrorCode = NO_ERROR;
		} else {
			ErrorCode = GetLastError();
		}
		if ((ErrorCode != NO_ERROR) && (ErrorCode != ERROR_MORE_DATA)) break;

		/ * Sanity check. * /
		if (Lcn >= BitmapData.StartingLcn + BitmapData.BitmapSize) break;

		/ * Analyze the clusterdata. We resume where the previous block left off. * /
		Lcn = BitmapData.StartingLcn;
		Index = 0;
		Mask = 1;
		IndexMax = sizeof(BitmapData.Buffer);
		if (BitmapData.BitmapSize / 8 < IndexMax) IndexMax = (int)(BitmapData.BitmapSize / 8);
		while ((Index < IndexMax) && (*Data->Running == RUNNING)) {
			InUse = (BitmapData.Buffer[Index] & Mask);
			/ * If at the beginning of the disk then copy the InUse value as our
			starting value. * /
			if (Lcn == 0) PrevInUse = InUse;
			/ * At the beginning and end of an Exclude draw the cluster. * /
			if ((Lcn == Data->MftExcludes[0].Start) || (Lcn == Data->MftExcludes[0].End) ||
				(Lcn == Data->MftExcludes[1].Start) || (Lcn == Data->MftExcludes[1].End) ||
				(Lcn == Data->MftExcludes[2].Start) || (Lcn == Data->MftExcludes[2].End)) {
					if ((Lcn == Data->MftExcludes[0].End) ||
						(Lcn == Data->MftExcludes[1].End) ||
						(Lcn == Data->MftExcludes[2].End)) {
							m_jkGui->DrawCluster(Data,ClusterStart,Lcn,JKDefragStruct::COLORUNMOVABLE);
					} else if (PrevInUse == 0) {
						m_jkGui->DrawCluster(Data,ClusterStart,Lcn,JKDefragStruct::COLOREMPTY);
					} else {
						m_jkGui->DrawCluster(Data,ClusterStart,Lcn,JKDefragStruct::COLORALLOCATED);
					}
					InUse = 1;
					PrevInUse = 1;
					ClusterStart = Lcn;
			}
			if ((PrevInUse == 0) && (InUse != 0)) {          / * Free * /
				m_jkGui->DrawCluster(Data,ClusterStart,Lcn,JKDefragStruct::COLOREMPTY);
				ClusterStart = Lcn;
			}
			if ((PrevInUse != 0) && (InUse == 0)) {          / * In use * /
				m_jkGui->DrawCluster(Data,ClusterStart,Lcn,JKDefragStruct::COLORALLOCATED);
				ClusterStart = Lcn;
			}
			PrevInUse = InUse;
			if (Mask == 128) {
				Mask = 1;
				Index = Index + 1;
			} else {
				Mask = Mask << 1;
			}
			Lcn = Lcn + 1;
		}

	} while ((ErrorCode == ERROR_MORE_DATA) &&
		(Lcn < BitmapData.StartingLcn + BitmapData.BitmapSize));

	if ((Lcn > 0) && (*Data->RedrawScreen == 2)) {
		if (PrevInUse == 0) {          / * Free * /
			m_jkGui->DrawCluster(Data,ClusterStart,Lcn,JKDefragStruct::COLOREMPTY);
		}
		if (PrevInUse != 0) {          / * In use * /
			m_jkGui->DrawCluster(Data,ClusterStart,Lcn,JKDefragStruct::COLORALLOCATED);
		}
	}

	/ * Show the MFT zones. * /
	for (i = 0; i < 3; i++) {
		if (*Data->RedrawScreen != 2) break;
		if (Data->MftExcludes[i].Start <= 0) continue;
		m_jkGui->DrawCluster(Data,Data->MftExcludes[i].Start,Data->MftExcludes[i].End,JKDefragStruct::COLORMFT);
	}

	/ * Colorize all the files on the screen.
	Note: the "$BadClus" file on NTFS disks maps the entire disk, so we have to
	ignore it. * /
	for (Item = TreeSmallest(Data->ItemTree); Item != NULL; Item = TreeNext(Item)) {
		if (*Data->Running != RUNNING) break;
		if (*Data->RedrawScreen != 2) break;
		if ((Item->LongFilename != NULL) &&
			((_wcsicmp(Item->LongFilename,L"$BadClus") == 0) ||
			(_wcsicmp(Item->LongFilename,L"$BadClus:$Bad:$DATA") == 0))) continue;
		ColorizeItem(Data,Item,0,0,NO);
	}

	/ * Set the flag to "no". * /
	if (*Data->RedrawScreen == 2) *Data->RedrawScreen = 0;
}
*/

/*

Look for a gap, a block of empty clusters on the volume.
MinimumLcn: Start scanning for gaps at this location. If there is a gap
at this location then return it. Zero is the begin of the disk.
MaximumLcn: Stop scanning for gaps at this location. Zero is the end of
the disk.
MinimumSize: The gap must have at least this many contiguous free clusters.
Zero will match any gap, so will return the first gap at or above
MinimumLcn.
MustFit: if YES then only return a gap that is bigger/equal than the
MinimumSize. If NO then return a gap bigger/equal than MinimumSize,
or if no such gap is found return the largest gap on the volume (above
MinimumLcn).
FindHighestGap: if NO then return the lowest gap that is bigger/equal
than the MinimumSize. If YES then return the highest gap.
Return YES if succes, NO if no gap was found or an error occurred.
The routine asks Windows for the cluster bitmap every time. It would be
faster to cache the bitmap in memory, but that would cause more fails
because of stale information.

*/
int JKDefragLib::FindGap(struct DefragDataStruct *Data,
					ULONG64 MinimumLcn,          /* Gap must be at or above this LCN. */
					ULONG64 MaximumLcn,          /* Gap must be below this LCN. */
					ULONG64 MinimumSize,         /* Gap must be at least this big. */
					int MustFit,                 /* YES: gap must be at least MinimumSize. */
					int FindHighestGap,          /* YES: return the last gap that fits. */
					ULONG64 *BeginLcn,           /* Result, LCN of begin of cluster. */
					ULONG64 *EndLcn,             /* Result, LCN of end of cluster. */
					BOOL IgnoreMftExcludes)
{
	STARTING_LCN_INPUT_BUFFER BitmapParam;

	struct
	{
		ULONG64 StartingLcn;
		ULONG64 BitmapSize;
	
		BYTE Buffer[65536];               /* Most efficient if binary multiple. */
	} BitmapData;

	ULONG64 Lcn;
	ULONG64 ClusterStart;
	ULONG64 HighestBeginLcn;
	ULONG64 HighestEndLcn;
	ULONG64 LargestBeginLcn;
	ULONG64 LargestEndLcn;

	int Index;
	int IndexMax;

	BYTE Mask;

	int InUse;
	int PrevInUse;

	DWORD ErrorCode;

	WCHAR s1[BUFSIZ];

	DWORD w;

	JKDefragGui *jkGui = JKDefragGui::getInstance();

	/* Sanity check. */
	if (MinimumLcn >= Data->TotalClusters) return(NO);

	/* Main loop to walk through the entire clustermap. */
	Lcn = MinimumLcn;
	ClusterStart = 0;
	PrevInUse = 1;
	HighestBeginLcn = 0;
	HighestEndLcn = 0;
	LargestBeginLcn = 0;
	LargestEndLcn = 0;

	do
	{
		/* Fetch a block of cluster data. If error then return NO. */
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

		if ((ErrorCode != NO_ERROR) && (ErrorCode != ERROR_MORE_DATA))
		{
			/* Show debug message: "ERROR: could not get volume bitmap: %s" */
			SystemErrorStr(GetLastError(),s1,BUFSIZ);

			jkGui->ShowDebug(1,NULL,Data->DebugMsg[12],s1);

			return(NO);
		}

		/* Sanity check. */
		if (Lcn >= BitmapData.StartingLcn + BitmapData.BitmapSize) return(NO);
		if (MaximumLcn == 0) MaximumLcn = BitmapData.StartingLcn + BitmapData.BitmapSize;

		/* Analyze the clusterdata. We resume where the previous block left
		off. If a cluster is found that matches the criteria then return
		it's LCN (Logical Cluster Number). */
		Lcn = BitmapData.StartingLcn;
		Index = 0;
		Mask = 1;

		IndexMax = sizeof(BitmapData.Buffer);

		if (BitmapData.BitmapSize / 8 < IndexMax) IndexMax = (int)(BitmapData.BitmapSize / 8);

		while ((Index < IndexMax) && (Lcn < MaximumLcn))
		{
			if (Lcn >= MinimumLcn)
			{
				InUse = (BitmapData.Buffer[Index] & Mask);

				if (((Lcn >= Data->MftExcludes[0].Start) && (Lcn < Data->MftExcludes[0].End)) ||
					((Lcn >= Data->MftExcludes[1].Start) && (Lcn < Data->MftExcludes[1].End)) ||
					((Lcn >= Data->MftExcludes[2].Start) && (Lcn < Data->MftExcludes[2].End)))
				{
					if (IgnoreMftExcludes == FALSE) InUse = 1;
				}

				if ((PrevInUse == 0) && (InUse != 0))
				{
					/* Show debug message: "Gap found: LCN=%I64d, Size=%I64d" */
					jkGui->ShowDebug(6,NULL,Data->DebugMsg[13],ClusterStart,Lcn - ClusterStart);

					/* If the gap is bigger/equal than the mimimum size then return it,
					or remember it, depending on the FindHighestGap parameter. */
					if ((ClusterStart >= MinimumLcn) &&
						(Lcn - ClusterStart >= MinimumSize))
					{
						if (FindHighestGap == NO)
						{
							if (BeginLcn != NULL) *BeginLcn = ClusterStart;

							if (EndLcn != NULL) *EndLcn = Lcn;

							return(YES);
						}

						HighestBeginLcn = ClusterStart;
						HighestEndLcn = Lcn;
					}

					/* Remember the largest gap on the volume. */
					if ((LargestBeginLcn == 0) ||
						(LargestEndLcn - LargestBeginLcn < Lcn - ClusterStart))
					{
						LargestBeginLcn = ClusterStart;
						LargestEndLcn = Lcn;
					}
				}

				if ((PrevInUse != 0) && (InUse == 0)) ClusterStart = Lcn;

				PrevInUse = InUse;
			}

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
	} while ((ErrorCode == ERROR_MORE_DATA) &&
			(Lcn < BitmapData.StartingLcn + BitmapData.BitmapSize) &&
			(Lcn < MaximumLcn));

	/* Process the last gap. */
	if (PrevInUse == 0)
	{
		/* Show debug message: "Gap found: LCN=%I64d, Size=%I64d" */
		jkGui->ShowDebug(6,NULL,Data->DebugMsg[13],ClusterStart,Lcn - ClusterStart);

		if ((ClusterStart >= MinimumLcn) && (Lcn - ClusterStart >= MinimumSize))
		{
			if (FindHighestGap == NO)
			{
				if (BeginLcn != NULL) *BeginLcn = ClusterStart;
				if (EndLcn != NULL) *EndLcn = Lcn;

				return(YES);
			}

			HighestBeginLcn = ClusterStart;
			HighestEndLcn = Lcn;
		}

		/* Remember the largest gap on the volume. */
		if ((LargestBeginLcn == 0) ||
			(LargestEndLcn - LargestBeginLcn < Lcn - ClusterStart))
		{
			LargestBeginLcn = ClusterStart;
			LargestEndLcn = Lcn;
		}
	}

	/* If the FindHighestGap flag is YES then return the highest gap we have found. */
	if ((FindHighestGap == YES) && (HighestBeginLcn != 0))
	{
		if (BeginLcn != NULL) *BeginLcn = HighestBeginLcn;
		if (EndLcn != NULL) *EndLcn = HighestEndLcn;

		return(YES);
	}

	/* If the MustFit flag is NO then return the largest gap we have found. */
	if ((MustFit == NO) && (LargestBeginLcn != 0))
	{
		if (BeginLcn != NULL) *BeginLcn = LargestBeginLcn;
		if (EndLcn != NULL) *EndLcn = LargestEndLcn;

		return(YES);
	}

	/* No gap found, return NO. */
	return(NO);
}

/*

Calculate the begin of the 3 zones.
Unmovable files pose an interesting problem. Suppose an unmovable file is in
zone 1, then the calculation for the beginning of zone 2 must count that file.
But that changes the beginning of zone 2. Some unmovable files may now suddenly
be in another zone. So we have to recalculate, which causes another border
change, and again, and again....
Note: the program only knows if a file is unmovable after it has tried to move a
file. So we have to recalculate the beginning of the zones every time we encounter
an unmovable file.

*/
void JKDefragLib::CalculateZones(struct DefragDataStruct *Data)
{
	struct ItemStruct *Item;

	struct FragmentListStruct *Fragment;

	ULONG64 SizeOfMovableFiles[3];
	ULONG64 SizeOfUnmovableFragments[3];
	ULONG64 ZoneEnd[3];
	ULONG64 OldZoneEnd[3];
	ULONG64 Vcn;
	ULONG64 RealVcn;

	int Zone;
	int Iterate;
	int i;

	JKDefragGui *jkGui = JKDefragGui::getInstance();

	/* Calculate the number of clusters in movable items for every zone. */
	for (Zone = 0; Zone <= 2; Zone++) SizeOfMovableFiles[Zone] = 0;

	for (Item = TreeSmallest(Data->ItemTree); Item != NULL; Item = TreeNext(Item))
	{
		if (Item->Unmovable == YES) continue;
		if (Item->Exclude == YES) continue;
		if ((Item->Directory == YES) && (Data->CannotMoveDirs > 20)) continue;

		Zone = 1;

		if (Item->SpaceHog == YES) Zone = 2;
		if (Item->Directory == YES) Zone = 0;

		SizeOfMovableFiles[Zone] = SizeOfMovableFiles[Zone] + Item->Clusters;
	}

	/* Iterate until the calculation does not change anymore, max 10 times. */
	for (Zone = 0; Zone <= 2; Zone++) SizeOfUnmovableFragments[Zone] = 0;

	for (Zone = 0; Zone <= 2; Zone++) OldZoneEnd[Zone] = 0;

	for (Iterate = 1; Iterate <= 10; Iterate++)
	{
		/* Calculate the end of the zones. */
		ZoneEnd[0] = SizeOfMovableFiles[0] + SizeOfUnmovableFragments[0] +
			(ULONG64)(Data->TotalClusters * Data->FreeSpace / 100.0);

		ZoneEnd[1] = ZoneEnd[0] + SizeOfMovableFiles[1] + SizeOfUnmovableFragments[1] +
			(ULONG64)(Data->TotalClusters * Data->FreeSpace / 100.0);

		ZoneEnd[2] = ZoneEnd[1] + SizeOfMovableFiles[2] + SizeOfUnmovableFragments[2];

		/* Exit if there was no change. */
		if ((OldZoneEnd[0] == ZoneEnd[0]) &&
			(OldZoneEnd[1] == ZoneEnd[1]) &&
			(OldZoneEnd[2] == ZoneEnd[2])) break;

		for (Zone = 0; Zone <= 2; Zone++) OldZoneEnd[Zone] = ZoneEnd[Zone];

		/* Show debug info. */
		jkGui->ShowDebug(4,NULL,L"Zone calculation, iteration %u: 0 - %I64d - %I64d - %I64d",Iterate,
			ZoneEnd[0],ZoneEnd[1],ZoneEnd[2]);

		/* Reset the SizeOfUnmovableFragments array. We are going to (re)calculate these numbers
		based on the just calculates ZoneEnd's. */
		for (Zone = 0; Zone <= 2; Zone++) SizeOfUnmovableFragments[Zone] = 0;

		/* The MFT reserved areas are counted as unmovable data. */
		for (i = 0; i < 3; i++)
		{
			if (Data->MftExcludes[i].Start < ZoneEnd[0])
			{
				SizeOfUnmovableFragments[0] = SizeOfUnmovableFragments[0] + Data->MftExcludes[i].End - Data->MftExcludes[i].Start;
			}
			else if (Data->MftExcludes[i].Start < ZoneEnd[1])
			{
				SizeOfUnmovableFragments[1] = SizeOfUnmovableFragments[1] + Data->MftExcludes[i].End - Data->MftExcludes[i].Start;
			}
			else if (Data->MftExcludes[i].Start < ZoneEnd[2])
			{
				SizeOfUnmovableFragments[2] = SizeOfUnmovableFragments[2] + Data->MftExcludes[i].End - Data->MftExcludes[i].Start;
			}
		}

		/* Walk through all items and count the unmovable fragments. Ignore unmovable fragments
		in the MFT zones, we have already counted the zones. */
		for (Item = TreeSmallest(Data->ItemTree); Item != NULL; Item = TreeNext(Item))
		{
			if ((Item->Unmovable == NO) &&
				(Item->Exclude == NO) &&
				((Item->Directory == NO) || (Data->CannotMoveDirs <= 20))) continue;

			Vcn = 0;
			RealVcn = 0;

			for (Fragment = Item->Fragments; Fragment != NULL; Fragment = Fragment->Next)
			{
				if (Fragment->Lcn != VIRTUALFRAGMENT)
				{
					if (((Fragment->Lcn < Data->MftExcludes[0].Start) || (Fragment->Lcn >= Data->MftExcludes[0].End)) &&
						((Fragment->Lcn < Data->MftExcludes[1].Start) || (Fragment->Lcn >= Data->MftExcludes[1].End)) &&
						((Fragment->Lcn < Data->MftExcludes[2].Start) || (Fragment->Lcn >= Data->MftExcludes[2].End)))
					{
						if (Fragment->Lcn < ZoneEnd[0])
						{
							SizeOfUnmovableFragments[0] = SizeOfUnmovableFragments[0] + Fragment->NextVcn - Vcn;
						}
						else if (Fragment->Lcn < ZoneEnd[1])
						{
							SizeOfUnmovableFragments[1] = SizeOfUnmovableFragments[1] + Fragment->NextVcn - Vcn;
						}
						else if (Fragment->Lcn < ZoneEnd[2])
						{
							SizeOfUnmovableFragments[2] = SizeOfUnmovableFragments[2] + Fragment->NextVcn - Vcn;
						}
					}

					RealVcn = RealVcn + Fragment->NextVcn - Vcn;
				}

				Vcn = Fragment->NextVcn;
			}
		}
	}

	/* Calculated the begin of the zones. */
	Data->Zones[0] = 0;

	for (i = 1; i <= 3; i++) Data->Zones[i] = ZoneEnd[i-1];
}

/*

Subfunction for MoveItem(), see below. Move (part of) an item to a new
location on disk. Return errorcode from DeviceIoControl().
The file is moved in a single FSCTL_MOVE_FILE call. If the file has
fragments then Windows will join them up.
Note: the offset and size of the block is in absolute clusters, not
virtual clusters.

*/
DWORD JKDefragLib::MoveItem1(struct DefragDataStruct *Data,
				HANDLE FileHandle,
				struct ItemStruct *Item,
				ULONG64 NewLcn,                   /* Where to move to. */
				ULONG64 Offset,                   /* Number of first cluster to be moved. */
				ULONG64 Size)                     /* Number of clusters to be moved. */
{
	MOVE_FILE_DATA MoveParams;

	struct FragmentListStruct *Fragment;

	ULONG64 Vcn;
	ULONG64 RealVcn;
	ULONG64 Lcn;

	DWORD ErrorCode;
	DWORD w;

	JKDefragGui *jkGui = JKDefragGui::getInstance();

	/* Find the first fragment that contains clusters inside the block, so we
	can translate the absolute cluster number of the block into the virtual
	cluster number used by Windows. */
	Vcn = 0;
	RealVcn = 0;

	for (Fragment = Item->Fragments; Fragment != NULL; Fragment = Fragment->Next)
	{
		if (Fragment->Lcn != VIRTUALFRAGMENT)
		{
			if (RealVcn + Fragment->NextVcn - Vcn - 1 >= Offset) break;

			RealVcn = RealVcn + Fragment->NextVcn - Vcn;
		}

		Vcn = Fragment->NextVcn;
	}

	/* Setup the parameters for the move. */
	MoveParams.FileHandle = FileHandle;
	MoveParams.StartingLcn.QuadPart = NewLcn;
	MoveParams.StartingVcn.QuadPart = Vcn + (Offset - RealVcn);
	MoveParams.ClusterCount = (DWORD)(Size);

	if (Fragment == NULL)
	{
		Lcn = 0;
	}
	else
	{
		Lcn = Fragment->Lcn + (Offset - RealVcn);
	}

	/* Show progress message. */
	jkGui->ShowMove(Item,MoveParams.ClusterCount,Lcn,NewLcn,MoveParams.StartingVcn.QuadPart);

	/* Draw the item and the destination clusters on the screen in the BUSY	color. */
	ColorizeItem(Data,Item,MoveParams.StartingVcn.QuadPart,MoveParams.ClusterCount,NO);

	jkGui->DrawCluster(Data,NewLcn,NewLcn + Size,JKDefragStruct::COLORBUSY);

	/* Call Windows to perform the move. */
	ErrorCode = DeviceIoControl(Data->Disk.VolumeHandle,FSCTL_MOVE_FILE,&MoveParams,
			sizeof(MoveParams),NULL,0,&w,NULL);

	if (ErrorCode != 0)
	{
		ErrorCode = NO_ERROR;
	}
	else
	{
		ErrorCode = GetLastError();
	}

	/* Update the PhaseDone counter for the progress bar. */
	Data->PhaseDone = Data->PhaseDone + MoveParams.ClusterCount;

	/* Undraw the destination clusters on the screen. */
	jkGui->DrawCluster(Data,NewLcn,NewLcn + Size,JKDefragStruct::COLOREMPTY);

	return(ErrorCode);
}

/*

Subfunction for MoveItem(), see below. Move (part of) an item to a new
location on disk. Return errorcode from DeviceIoControl().
Move the item one fragment at a time, a FSCTL_MOVE_FILE call per fragment.
The fragments will be lined up on disk and the defragger will treat the
item as unfragmented.
Note: the offset and size of the block is in absolute clusters, not
virtual clusters.

*/
DWORD JKDefragLib::MoveItem2(struct DefragDataStruct *Data,
					HANDLE FileHandle,
					struct ItemStruct *Item,
					ULONG64 NewLcn,                /* Where to move to. */
					ULONG64 Offset,                /* Number of first cluster to be moved. */
					ULONG64 Size)                  /* Number of clusters to be moved. */
{
	MOVE_FILE_DATA MoveParams;

	struct FragmentListStruct *Fragment;

	ULONG64 Vcn;
	ULONG64 RealVcn;
	ULONG64 FromLcn;

	DWORD ErrorCode;
	DWORD w;

	JKDefragGui *jkGui = JKDefragGui::getInstance();

	/* Walk through the fragments of the item and move them one by one to the new location. */
	ErrorCode = NO_ERROR;
	Vcn = 0;
	RealVcn = 0;

	for (Fragment = Item->Fragments; Fragment != NULL; Fragment = Fragment->Next)
	{
		if (*Data->Running != RUNNING) break;

		if (Fragment->Lcn != VIRTUALFRAGMENT)
		{
			if (RealVcn >= Offset + Size) break;

			if (RealVcn + Fragment->NextVcn - Vcn - 1 >= Offset)
			{
				/* Setup the parameters for the move. If the block that we want to move
				begins somewhere in the middle of a fragment then we have to setup
				slightly differently than when the fragment is at or after the begin
				of the block. */
				MoveParams.FileHandle = FileHandle;

				if (RealVcn < Offset)
				{
					/* The fragment starts before the Offset and overlaps. Move the
					part of the fragment from the Offset until the end of the
					fragment or the block. */
					MoveParams.StartingLcn.QuadPart = NewLcn;
					MoveParams.StartingVcn.QuadPart = Vcn + (Offset - RealVcn);

					if (Size < (Fragment->NextVcn - Vcn) - (Offset - RealVcn))
					{
						MoveParams.ClusterCount = (DWORD)Size;
					}
					else
					{
						MoveParams.ClusterCount = (DWORD)((Fragment->NextVcn - Vcn) - (Offset - RealVcn));
					}

					FromLcn = Fragment->Lcn + (Offset - RealVcn);
				}
				else
				{
					/* The fragment starts at or after the Offset. Move the part of
					the fragment inside the block (up until Offset+Size). */
					MoveParams.StartingLcn.QuadPart = NewLcn + RealVcn - Offset;
					MoveParams.StartingVcn.QuadPart = Vcn;

					if (Fragment->NextVcn - Vcn < Offset + Size - RealVcn)
					{
						MoveParams.ClusterCount = (DWORD)(Fragment->NextVcn - Vcn);
					}
					else
					{
						MoveParams.ClusterCount = (DWORD)(Offset + Size - RealVcn);
					}
						FromLcn = Fragment->Lcn;
				}

				/* Show progress message. */
				jkGui->ShowMove(Item,MoveParams.ClusterCount,FromLcn,MoveParams.StartingLcn.QuadPart,
						MoveParams.StartingVcn.QuadPart);

				/* Draw the item and the destination clusters on the screen in the BUSY	color. */
//				if (*Data->RedrawScreen == 0) {
					ColorizeItem(Data,Item,MoveParams.StartingVcn.QuadPart,MoveParams.ClusterCount,NO);
//				} else {
//					m_jkGui->ShowDiskmap(Data);
//				}

				jkGui->DrawCluster(Data,MoveParams.StartingLcn.QuadPart,
						MoveParams.StartingLcn.QuadPart + MoveParams.ClusterCount,JKDefragStruct::COLORBUSY);

				/* Call Windows to perform the move. */
				ErrorCode = DeviceIoControl(Data->Disk.VolumeHandle,FSCTL_MOVE_FILE,&MoveParams,
						sizeof(MoveParams),NULL,0,&w,NULL);

				if (ErrorCode != 0)
				{
					ErrorCode = NO_ERROR;
				}
				else
				{
					ErrorCode = GetLastError();
				}

				/* Update the PhaseDone counter for the progress bar. */
				Data->PhaseDone = Data->PhaseDone + MoveParams.ClusterCount;

				/* Undraw the destination clusters on the screen. */
				jkGui->DrawCluster(Data,MoveParams.StartingLcn.QuadPart,
						MoveParams.StartingLcn.QuadPart + MoveParams.ClusterCount,JKDefragStruct::COLOREMPTY);

				/* If there was an error then exit. */
				if (ErrorCode != NO_ERROR) return(ErrorCode);
			}

			RealVcn = RealVcn + Fragment->NextVcn - Vcn;
		}

		/* Next fragment. */
		Vcn = Fragment->NextVcn;
	}

	return(ErrorCode);
}

/*

Subfunction for MoveItem(), see below. Move (part of) an item to a new
location on disk. Return YES if success, NO if failure.
Strategy 0: move the block in a single FSCTL_MOVE_FILE call. If the block
has fragments then Windows will join them up.
Strategy 1: move the block one fragment at a time. The fragments will be
lined up on disk and the defragger will treat them as unfragmented.
Note: the offset and size of the block is in absolute clusters, not
virtual clusters.

*/
int JKDefragLib::MoveItem3(struct DefragDataStruct *Data,
			struct ItemStruct *Item,
			HANDLE FileHandle,
			ULONG64 NewLcn,          /* Where to move to. */
			ULONG64 Offset,          /* Number of first cluster to be moved. */
			ULONG64 Size,            /* Number of clusters to be moved. */
			int Strategy)            /* 0: move in one part, 1: move individual fragments. */
{
	DWORD ErrorCode;

	WCHAR ErrorString[BUFSIZ];

	int Result;

	JKDefragGui *jkGui = JKDefragGui::getInstance();

	/* Slow the program down if so selected. */
	SlowDown(Data);

	/* Move the item, either in a single block or fragment by fragment. */
	if (Strategy == 0)
	{
		ErrorCode = MoveItem1(Data,FileHandle,Item,NewLcn,Offset,Size);
	}
	else
	{
		ErrorCode = MoveItem2(Data,FileHandle,Item,NewLcn,Offset,Size);
	}

	/* If there was an error then fetch the errormessage and save it. */
	if (ErrorCode != NO_ERROR) SystemErrorStr(ErrorCode,ErrorString,BUFSIZ);

	/* Fetch the new fragment map of the item and refresh the screen. */
	ColorizeItem(Data,Item,0,0,YES);

	TreeDetach(Data,Item);

	Result = GetFragments(Data,Item,FileHandle);

	TreeInsert(Data,Item);

//		if (*Data->RedrawScreen == 0) {
		ColorizeItem(Data,Item,0,0,NO);
//		} else {
//			m_jkGui->ShowDiskmap(Data);
//		}

	/* If Windows reported an error while moving the item then show the
	errormessage and return NO. */
	if (ErrorCode != NO_ERROR)
	{
		jkGui->ShowDebug(3,Item,ErrorString);

		return(NO);
	}

	/* If there was an error analyzing the item then return NO. */
	if (Result == NO) return(NO);

	return(YES);
}

/*

Subfunction for MoveItem(), see below. Move the item with strategy 0.
If this results in fragmentation then try again using strategy 1.
Return YES if success, NO if failed to move without fragmenting the
item.
Note: The Windows defragmentation API does not report an error if it only
moves part of the file and has fragmented the file. This can for example
happen when part of the file is locked and cannot be moved, or when (part
of) the gap was previously in use by another file but has not yet been
released by the NTFS checkpoint system.
Note: the offset and size of the block is in absolute clusters, not
virtual clusters.

*/
int JKDefragLib::MoveItem4(struct DefragDataStruct *Data,
			struct ItemStruct *Item,
			HANDLE FileHandle,
			ULONG64 NewLcn,                                   /* Where to move to. */
			ULONG64 Offset,                /* Number of first cluster to be moved. */
			ULONG64 Size,                       /* Number of clusters to be moved. */
			int Direction)                          /* 0: move up, 1: move down. */
{
	ULONG64 OldLcn;
	ULONG64 ClusterStart;
	ULONG64 ClusterEnd;

	int Result;

	JKDefragGui *jkGui = JKDefragGui::getInstance();

	/* Remember the current position on disk of the item. */
	OldLcn = GetItemLcn(Item);

	/* Move the Item to the requested LCN. If error then return NO. */
	Result = MoveItem3(Data,Item,FileHandle,NewLcn,Offset,Size,0);

	if (Result == NO) return(NO);
	if (*Data->Running != RUNNING) return(NO);

	/* If the block is not fragmented then return YES. */
	if (IsFragmented(Item,Offset,Size) == NO) return(YES);

	/* Show debug message: "Windows could not move the file, trying alternative method." */
	jkGui->ShowDebug(3,Item,Data->DebugMsg[42]);

	/* Find another gap on disk for the item. */
	if (Direction == 0)
	{
		ClusterStart = OldLcn + Item->Clusters;

		if ((ClusterStart + Item->Clusters >= NewLcn) &&
			(ClusterStart < NewLcn + Item->Clusters))
		{
			ClusterStart = NewLcn + Item->Clusters;
		}

		Result = FindGap(Data,ClusterStart,0,Size,YES,NO,&ClusterStart,&ClusterEnd,FALSE);
	}
	else
	{
		Result = FindGap(Data,Data->Zones[1],OldLcn,Size,YES,YES,&ClusterStart,&ClusterEnd,FALSE);
	}

	if (Result == NO) return(NO);

	/* Add the size of the item to the width of the progress bar, we have discovered
	that we have more work to do. */
	Data->PhaseTodo = Data->PhaseTodo + Size;

	/* Move the item to the other gap using strategy 1. */
	if (Direction == 0)
	{
		Result = MoveItem3(Data,Item,FileHandle,ClusterStart,Offset,Size,1);
	}
	else
	{
		Result = MoveItem3(Data,Item,FileHandle,ClusterEnd - Size,Offset,Size,1);
	}

	if (Result == NO) return(NO);

	/* If the block is still fragmented then return NO. */
	if (IsFragmented(Item,Offset,Size) == YES)
	{
		/* Show debug message: "Alternative method failed, leaving file where it is." */
		jkGui->ShowDebug(3,Item,Data->DebugMsg[45]);

		return(NO);
	}

	jkGui->ShowDebug(3,Item,L"");

	/* Add the size of the item to the width of the progress bar, we have more work to do. */
	Data->PhaseTodo = Data->PhaseTodo + Size;

	/* Strategy 1 has helped. Move the Item again to where we want it, but
	this time use strategy 1. */
	Result = MoveItem3(Data,Item,FileHandle,NewLcn,Offset,Size,1);

	return(Result);
}

/*

Move (part of) an item to a new location on disk. Moving the Item will
automatically defragment it. If unsuccesful then set the Unmovable
flag of the item and return NO, otherwise return YES.
Note: the item will move to a different location in the tree.
Note: the offset and size of the block is in absolute clusters, not
virtual clusters.

*/
int JKDefragLib::MoveItem(struct DefragDataStruct *Data,
			struct ItemStruct *Item,
			ULONG64 NewLcn,                                   /* Where to move to. */
			ULONG64 Offset,                /* Number of first cluster to be moved. */
			ULONG64 Size,                       /* Number of clusters to be moved. */
			int Direction)                          /* 0: move up, 1: move down. */
{
	HANDLE FileHandle;

	ULONG64 ClustersTodo;
	ULONG64 ClustersDone;

	int Result;

	/* If the Item is Unmovable, Excluded, or has zero size then we cannot move it. */
	if (Item->Unmovable == YES) return(NO);
	if (Item->Exclude == YES) return(NO);
	if (Item->Clusters == 0) return(NO);

	/* Directories cannot be moved on FAT volumes. This is a known Windows limitation
	and not a bug in JkDefrag. But JkDefrag will still try, to allow for possible
	circumstances where the Windows defragmentation API can move them after all.
	To speed up things we count the number of directories that could not be moved,
	and when it reaches 20 we ignore all directories from then on. */
	if ((Item->Directory == YES) && (Data->CannotMoveDirs > 20))
	{
		Item->Unmovable = YES;

		ColorizeItem(Data,Item,0,0,NO);

		return(NO);
	}

	/* Open a filehandle for the item and call the subfunctions (see above) to
	move the file. If success then return YES. */
	ClustersDone = 0;
	Result = YES;

	while ((ClustersDone < Size) && (*Data->Running == RUNNING))
	{
		ClustersTodo = Size - ClustersDone;

		if (Data->BytesPerCluster > 0)
		{
			if (ClustersTodo > 1073741824 / Data->BytesPerCluster)
			{
				ClustersTodo = 1073741824 / Data->BytesPerCluster;
			}
		}
		else
		{
			if (ClustersTodo > 262144) ClustersTodo = 262144;
		}

		FileHandle = OpenItemHandle(Data,Item);

		Result = NO;

		if (FileHandle == NULL) break;

		Result = MoveItem4(Data,Item,FileHandle,NewLcn+ClustersDone,Offset+ClustersDone,
				ClustersTodo,Direction);

		if (Result == NO) break;

		ClustersDone = ClustersDone + ClustersTodo;

		FlushFileBuffers(FileHandle);            /* Is this useful? Can't hurt. */
		CloseHandle(FileHandle);
	}

	if (Result == YES)
	{
		if (Item->Directory == YES) Data->CannotMoveDirs = 0;

		return(YES);
	}

	/* If error then set the Unmovable flag, colorize the item on the screen, recalculate
	the begin of the zone's, and return NO. */
	Item->Unmovable = YES;

	if (Item->Directory == YES) Data->CannotMoveDirs++;

	ColorizeItem(Data,Item,0,0,NO);
	CalculateZones(Data);

	return(NO);
}

/*

Look in the ItemTree and return the highest file above the gap that fits inside
the gap (cluster start - cluster end). Return a pointer to the item, or NULL if
no file could be found.
Direction=0      Search for files below the gap.
Direction=1      Search for files above the gap.
Zone=0           Only search the directories.
Zone=1           Only search the regular files.
Zone=2           Only search the SpaceHogs.
Zone=3           Search all items.

*/
struct ItemStruct *JKDefragLib::FindHighestItem(struct DefragDataStruct *Data,
			ULONG64 ClusterStart,
			ULONG64 ClusterEnd,
			int Direction,
			int Zone)
{
	struct ItemStruct *Item;

	ULONG64 ItemLcn;

	int FileZone;

	JKDefragGui *jkGui = JKDefragGui::getInstance();

	/* "Looking for highest-fit %I64d[%I64d]" */
	jkGui->ShowDebug(5,NULL,L"Looking for highest-fit %I64d[%I64d]",
			ClusterStart,ClusterEnd - ClusterStart);

	/* Walk backwards through all the items on disk and select the first
	file that fits inside the free block. If we find an exact match then
	immediately return it. */
	for (Item = TreeFirst(Data->ItemTree,Direction);
		Item != NULL;
		Item = TreeNextPrev(Item,Direction))
	{
		ItemLcn = GetItemLcn(Item);

		if (ItemLcn == 0) continue;

		if (Direction == 1)
		{
			if (ItemLcn < ClusterEnd) return(NULL);
		}
		else
		{
			if (ItemLcn > ClusterStart) return(NULL);
		}

		if (Item->Unmovable == YES) continue;
		if (Item->Exclude == YES) continue;

		if (Zone != 3)
		{
			FileZone = 1;

			if (Item->SpaceHog == YES) FileZone = 2;
			if (Item->Directory == YES) FileZone = 0;
			if (Zone != FileZone) continue;
		}

		if (Item->Clusters > ClusterEnd - ClusterStart) continue;

		return(Item);
	}

	return(NULL);
}

/*

Find the highest item on disk that fits inside the gap (cluster start - cluster
end), and combined with other items will perfectly fill the gap. Return NULL if
no perfect fit could be found. The subroutine will limit it's running time to 0.5
seconds.
Direction=0      Search for files below the gap.
Direction=1      Search for files above the gap.
Zone=0           Only search the directories.
Zone=1           Only search the regular files.
Zone=2           Only search the SpaceHogs.
Zone=3           Search all items.

*/
struct ItemStruct *JKDefragLib::FindBestItem(struct DefragDataStruct *Data,
			ULONG64 ClusterStart,
			ULONG64 ClusterEnd,
			int Direction,
			int Zone)
{
	struct ItemStruct *Item;
	struct ItemStruct *FirstItem;

	ULONG64 ItemLcn;
	ULONG64 GapSize;
	ULONG64 TotalItemsSize;

	int FileZone;

	struct __timeb64 Time;

	LONG64 MaxTime;

	JKDefragGui *jkGui = JKDefragGui::getInstance();

	jkGui->ShowDebug(5,NULL,L"Looking for perfect fit %I64d[%I64d]",
			ClusterStart,ClusterEnd - ClusterStart);

	/* Walk backwards through all the items on disk and select the first item that
	fits inside the free block, and combined with other items will fill the gap
	perfectly. If we find an exact match then immediately return it. */

	_ftime64_s(&Time);

	MaxTime = Time.time * 1000 + Time.millitm + 500;
	FirstItem = NULL;
	GapSize = ClusterEnd - ClusterStart;
	TotalItemsSize = 0;

	for (Item = TreeFirst(Data->ItemTree,Direction);
		Item != NULL;
		Item = TreeNextPrev(Item,Direction))
	{
		/* If we have passed the top of the gap then.... */
		ItemLcn = GetItemLcn(Item);

		if (ItemLcn == 0) continue;

		if (((Direction == 1) && (ItemLcn < ClusterEnd)) ||
			((Direction == 0) && (ItemLcn > ClusterEnd)))
		{
			/* If we did not find an item that fits inside the gap then exit. */
			if (FirstItem == NULL) break;

			/* Exit if the total size of all the items is less than the size of the gap.
			We know that we can never find a perfect fit. */
			if (TotalItemsSize < ClusterEnd - ClusterStart)
			{
				jkGui->ShowDebug(5,NULL,L"No perfect fit found, the total size of all the items above the gap is less than the size of the gap.");

				return(NULL);
			}

			/* Exit if the running time is more than 0.5 seconds. */
			_ftime64_s(&Time);

			if (Time.time * 1000 + Time.millitm > MaxTime)
			{
				jkGui->ShowDebug(5,NULL,L"No perfect fit found, out of time.");

				return(NULL);
			}

			/* Rewind and try again. The item that we have found previously fits in the
			gap, but it does not combine with other items to perfectly fill the gap. */
			Item = FirstItem;
			FirstItem = NULL;
			GapSize = ClusterEnd - ClusterStart;
			TotalItemsSize = 0;

			continue;
		}

		/* Ignore all unsuitable items. */
		if (Item->Unmovable == YES) continue;
		if (Item->Exclude == YES) continue;

		if (Zone != 3)
		{
			FileZone = 1;

			if (Item->SpaceHog == YES) FileZone = 2;
			if (Item->Directory == YES) FileZone = 0;
			if (Zone != FileZone) continue;
		}

		if (Item->Clusters < ClusterEnd - ClusterStart)
		{
			TotalItemsSize = TotalItemsSize + Item->Clusters;
		}

		if (Item->Clusters > GapSize) continue;

		/* Exit if this item perfectly fills the gap, or if we have found a combination
		with a previous item that perfectly fills the gap. */
		if (Item->Clusters == GapSize)
		{
			jkGui->ShowDebug(5,NULL,L"Perfect fit found.");

			if (FirstItem != NULL) return(FirstItem);

			return(Item);
		}

		/* We have found an item that fit's inside the gap, but does not perfectly fill
		the gap. We are now looking to fill a smaller gap. */
		GapSize = GapSize - Item->Clusters;

		/* Remember the first item that fits inside the gap. */
		if (FirstItem == NULL) FirstItem = Item;
	}

	jkGui->ShowDebug(5,NULL,L"No perfect fit found, all items above the gap are bigger than the gap.");

	return(NULL);
}

/* Update some numbers in the DefragData. */
void JKDefragLib::CallShowStatus(struct DefragDataStruct *Data, int Phase, int Zone)
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

	int Index;
	int IndexMax;

	BYTE Mask;

	int InUse;
	int PrevInUse;

	DWORD ErrorCode;

	LONG64 Count;
	LONG64 Factor;
	LONG64 Sum;

	DWORD w;

	JKDefragGui *jkGui = JKDefragGui::getInstance();

	/* Count the number of free gaps on the disk. */
	Data->CountGaps = 0;
	Data->CountFreeClusters = 0;
	Data->BiggestGap = 0;
	Data->CountGapsLess16 = 0;
	Data->CountClustersLess16 = 0;

	Lcn = 0;
	ClusterStart = 0;
	PrevInUse = 1;

	do
	{
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

		Lcn = BitmapData.StartingLcn;
		Index = 0;
		Mask = 1;

		IndexMax = sizeof(BitmapData.Buffer);

		if (BitmapData.BitmapSize / 8 < IndexMax) IndexMax = (int)(BitmapData.BitmapSize / 8);

		while (Index < IndexMax)
		{
			InUse = (BitmapData.Buffer[Index] & Mask);

			if (((Lcn >= Data->MftExcludes[0].Start) && (Lcn < Data->MftExcludes[0].End)) ||
				((Lcn >= Data->MftExcludes[1].Start) && (Lcn < Data->MftExcludes[1].End)) ||
				((Lcn >= Data->MftExcludes[2].Start) && (Lcn < Data->MftExcludes[2].End))) {
					InUse = 1;
			}

			if ((PrevInUse == 0) && (InUse != 0))
			{
				Data->CountGaps = Data->CountGaps + 1;
				Data->CountFreeClusters = Data->CountFreeClusters + Lcn - ClusterStart;
				if (Data->BiggestGap < Lcn - ClusterStart) Data->BiggestGap = Lcn - ClusterStart;

				if (Lcn - ClusterStart < 16)
				{
					Data->CountGapsLess16 = Data->CountGapsLess16 + 1;
					Data->CountClustersLess16 = Data->CountClustersLess16 + Lcn - ClusterStart;
				}
			}

			if ((PrevInUse != 0) && (InUse == 0)) ClusterStart = Lcn;

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

	if (PrevInUse == 0)
	{
		Data->CountGaps = Data->CountGaps + 1;
		Data->CountFreeClusters = Data->CountFreeClusters + Lcn - ClusterStart;

		if (Data->BiggestGap < Lcn - ClusterStart) Data->BiggestGap = Lcn - ClusterStart;

		if (Lcn - ClusterStart < 16)
		{
			Data->CountGapsLess16 = Data->CountGapsLess16 + 1;
			Data->CountClustersLess16 = Data->CountClustersLess16 + Lcn - ClusterStart;
		}
	}

	/* Walk through all files and update the counters. */
	Data->CountDirectories = 0;
	Data->CountAllFiles = 0;
	Data->CountFragmentedItems = 0;
	Data->CountAllBytes = 0;
	Data->CountFragmentedBytes = 0;
	Data->CountAllClusters = 0;
	Data->CountFragmentedClusters = 0;

	for (Item = TreeSmallest(Data->ItemTree); Item != NULL; Item = TreeNext(Item))
	{
		if ((Item->LongFilename != NULL) &&
			((_wcsicmp(Item->LongFilename,L"$BadClus") == 0) ||
			(_wcsicmp(Item->LongFilename,L"$BadClus:$Bad:$DATA") == 0)))
		{
			continue;
		}

		Data->CountAllBytes = Data->CountAllBytes + Item->Bytes;
		Data->CountAllClusters = Data->CountAllClusters + Item->Clusters;

		if (Item->Directory == YES)
		{
			Data->CountDirectories = Data->CountDirectories + 1;
		}
		else
		{
			Data->CountAllFiles = Data->CountAllFiles + 1;
		}

		if (FragmentCount(Item) > 1)
		{
			Data->CountFragmentedItems = Data->CountFragmentedItems + 1;
			Data->CountFragmentedBytes = Data->CountFragmentedBytes + Item->Bytes;
			Data->CountFragmentedClusters = Data->CountFragmentedClusters + Item->Clusters;
		}
	}

	/* Calculate the average distance between the end of any file to the begin of
	any other file. After reading a file the harddisk heads will have to move to
	the beginning of another file. The number is a measure of how fast files can
	be accessed.

	For example these 3 files:
	File 1 begin = 107
	File 1 end = 312
	File 2 begin = 595
	File 2 end = 645
	File 3 begin = 917
	File 3 end = 923

	File 1 end - File 2 begin = 283
	File 1 end - File 3 begin = 605
	File 2 end - File 1 begin = 538
	File 2 end - File 3 begin = 272
	File 3 end - File 1 begin = 816
	File 3 end - File 2 begin = 328
	--> Average distance from end to begin = 473.6666

	The formula used is:
	N = number of files
	Bn = Begin of file n
	En = End of file n
	Average = ( (1-N)*(B1+E1) + (3-N)*(B2+E2) + (5-N)*(B3+E3) + .... + (2*N-1-N)*(BN+EN) ) / ( N * (N-1) )

	For the above example:
	Average = ( (1-3)*(107+312) + (3-3)*(595+645) + 5-3)*(917+923) ) / ( 3 * (3-1) ) = 473.6666

	*/
	Count = 0;

	for (Item = TreeSmallest(Data->ItemTree); Item != NULL; Item = TreeNext(Item))
	{
		if ((Item->LongFilename != NULL) &&
			((_wcsicmp(Item->LongFilename,L"$BadClus") == 0) ||
			(_wcsicmp(Item->LongFilename,L"$BadClus:$Bad:$DATA") == 0)))
		{
				continue;
		}

		if (Item->Clusters == 0) continue;

		Count = Count + 1;
	}

	if (Count > 1)
	{
		Factor = 1 - Count;
		Sum = 0;

		for (Item = TreeSmallest(Data->ItemTree); Item != NULL; Item = TreeNext(Item))
		{
			if ((Item->LongFilename != NULL) &&
				((_wcsicmp(Item->LongFilename,L"$BadClus") == 0) ||
				(_wcsicmp(Item->LongFilename,L"$BadClus:$Bad:$DATA") == 0)))
			{
					continue;
			}

			if (Item->Clusters == 0) continue;

			Sum = Sum + Factor * (GetItemLcn(Item) * 2 + Item->Clusters);

			Factor = Factor + 2;
		}

		Data->AverageDistance = Sum / (double)(Count * (Count - 1));
	}
	else
	{
		Data->AverageDistance = 0;
	}

	Data->Phase = Phase;
	Data->Zone = Zone;
	Data->PhaseDone = 0;
	Data->PhaseTodo = 0;

	jkGui->ShowStatus(Data);
}

/* For debugging only: compare the data with the output from the
FSCTL_GET_RETRIEVAL_POINTERS function call.
Note: Reparse points will usually be flagged as different. A reparse point is
a symbolic link. The CreateFile call will resolve the symbolic link and retrieve
the info from the real item, but the MFT contains the info from the symbolic
link. */
void JKDefragLib::CompareItems(struct DefragDataStruct *Data, struct ItemStruct *Item)
{
	HANDLE FileHandle;

	ULONG64   Clusters;                         /* Total number of clusters. */

	STARTING_VCN_INPUT_BUFFER RetrieveParam;

	struct
	{
		DWORD ExtentCount;

		ULONG64 StartingVcn;

		struct
		{
			ULONG64 NextVcn;
			ULONG64 Lcn;
		} Extents[1000];
	} ExtentData;

	BY_HANDLE_FILE_INFORMATION FileInformation;

	ULONG64 Vcn;

	struct FragmentListStruct *Fragment;
	struct FragmentListStruct *LastFragment;

	DWORD ErrorCode;

	WCHAR ErrorString[BUFSIZ];

	int MaxLoop;

	ULARGE_INTEGER u;

	DWORD i;
	DWORD w;

	JKDefragGui *jkGui = JKDefragGui::getInstance();

	jkGui->ShowDebug(0,NULL,L"%I64u %s",GetItemLcn(Item),Item->LongFilename);

	if (Item->Directory == NO)
	{
		FileHandle = CreateFileW(Item->LongPath,FILE_READ_ATTRIBUTES,
			FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
			NULL,OPEN_EXISTING,FILE_FLAG_NO_BUFFERING,NULL);
	}
	else
	{
		FileHandle = CreateFileW(Item->LongPath,GENERIC_READ,
			FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
			NULL,OPEN_EXISTING,FILE_FLAG_BACKUP_SEMANTICS,NULL);
	}

	if (FileHandle == INVALID_HANDLE_VALUE)
	{
		SystemErrorStr(GetLastError(),ErrorString,BUFSIZ);

		jkGui->ShowDebug(0,NULL,L"  Could not open: %s",ErrorString);

		return;
	}

	/* Fetch the date/times of the file. */
	if (GetFileInformationByHandle(FileHandle,&FileInformation) != 0)
	{
		u.LowPart = FileInformation.ftCreationTime.dwLowDateTime;
		u.HighPart = FileInformation.ftCreationTime.dwHighDateTime;

		if (Item->CreationTime != u.QuadPart)
		{
			jkGui->ShowDebug(0,NULL,L"  Different CreationTime %I64u <> %I64u = %I64u",
				Item->CreationTime,u.QuadPart,Item->CreationTime - u.QuadPart);
		}

		u.LowPart = FileInformation.ftLastAccessTime.dwLowDateTime;
		u.HighPart = FileInformation.ftLastAccessTime.dwHighDateTime;

		if (Item->LastAccessTime != u.QuadPart)
		{
			jkGui->ShowDebug(0,NULL,L"  Different LastAccessTime %I64u <> %I64u = %I64u",
				Item->LastAccessTime,u.QuadPart,Item->LastAccessTime - u.QuadPart);
		}
	}

#ifdef jk
	Vcn = 0;
	for (Fragment = Item->Fragments; Fragment != NULL; Fragment = Fragment->Next) {
		if (Fragment->Lcn != VIRTUALFRAGMENT) {
			Data->ShowDebug(0,NULL,L"  Extent 1: Lcn=%I64u, Vcn=%I64u, NextVcn=%I64u",
				Fragment->Lcn,Vcn,Fragment->NextVcn);
		} else {
			Data->ShowDebug(0,NULL,L"  Extent 1 (virtual): Vcn=%I64u, NextVcn=%I64u",
				Vcn,Fragment->NextVcn);
		}
		Vcn = Fragment->NextVcn;
	}
#endif

	/* Ask Windows for the clustermap of the item and save it in memory.
	The buffer that is used to ask Windows for the clustermap has a
	fixed size, so we may have to loop a couple of times. */
	Fragment = Item->Fragments;
	Clusters = 0;
	Vcn = 0;
	MaxLoop = 1000;
	LastFragment = NULL;

	do {
		/* I strongly suspect that the FSCTL_GET_RETRIEVAL_POINTERS system call
		can sometimes return an empty bitmap and ERROR_MORE_DATA. That's not
		very nice of Microsoft, because it causes an infinite loop. I've
		therefore added a loop counter that will limit the loop to 1000
		iterations. This means the defragger cannot handle files with more
		than 100000 fragments, though. */
		if (MaxLoop <= 0)
		{
			jkGui->ShowDebug(0,NULL,L"  FSCTL_GET_RETRIEVAL_POINTERS error: Infinite loop");

			return;
		}

		MaxLoop = MaxLoop - 1;

		/* Ask Windows for the (next segment of the) clustermap of this file. If error
		then leave the loop. */
		RetrieveParam.StartingVcn.QuadPart = Vcn;

		ErrorCode = DeviceIoControl(FileHandle,FSCTL_GET_RETRIEVAL_POINTERS,
			&RetrieveParam,sizeof(RetrieveParam),&ExtentData,sizeof(ExtentData),&w,NULL);

		if (ErrorCode != 0)
		{
			ErrorCode = NO_ERROR;
		}
		else
		{
			ErrorCode = GetLastError();
		}

		if ((ErrorCode != NO_ERROR) && (ErrorCode != ERROR_MORE_DATA)) break;

		/* Walk through the clustermap, count the total number of clusters, and
		save all fragments in memory. */
		for (i = 0; i < ExtentData.ExtentCount; i++)
		{
			/* Show debug message. */
#ifdef jk
			if (ExtentData.Extents[i].Lcn != VIRTUALFRAGMENT) {
				Data->ShowDebug(0,NULL,L"  Extent 2: Lcn=%I64u, Vcn=%I64u, NextVcn=%I64u",
					ExtentData.Extents[i].Lcn,Vcn,ExtentData.Extents[i].NextVcn);
			} else {
				Data->ShowDebug(0,NULL,L"  Extent 2 (virtual): Vcn=%I64u, NextVcn=%I64u",
					Vcn,ExtentData.Extents[i].NextVcn);
			}
#endif

			/* Add the size of the fragment to the total number of clusters.
			There are two kinds of fragments: real and virtual. The latter do not
			occupy clusters on disk, but are information used by compressed
			and sparse files. */
			if (ExtentData.Extents[i].Lcn != VIRTUALFRAGMENT)
			{
				Clusters = Clusters + ExtentData.Extents[i].NextVcn - Vcn;
			}

			/* Compare the fragment. */
			if (Fragment == NULL)
			{
				jkGui->ShowDebug(0,NULL,L"  Extra fragment in FSCTL_GET_RETRIEVAL_POINTERS");
			}
			else
			{
				if (Fragment->Lcn != ExtentData.Extents[i].Lcn)
				{
					jkGui->ShowDebug(0,NULL,L"  Different LCN in fragment: %I64u <> %I64u",
						Fragment->Lcn,ExtentData.Extents[i].Lcn);
				}

				if (Fragment->NextVcn != ExtentData.Extents[i].NextVcn)
				{
					jkGui->ShowDebug(0,NULL,L"  Different NextVcn in fragment: %I64u <> %I64u",
						Fragment->NextVcn,ExtentData.Extents[i].NextVcn);
				}

				Fragment = Fragment->Next;
			}

			/* The Vcn of the next fragment is the NextVcn field in this record. */
			Vcn = ExtentData.Extents[i].NextVcn;
		}

		/* Loop until we have processed the entire clustermap of the file. */
	} while (ErrorCode == ERROR_MORE_DATA);

	/* If there was an error while reading the clustermap then return NO. */
	if ((ErrorCode != NO_ERROR) && (ErrorCode != ERROR_HANDLE_EOF))
	{
		SystemErrorStr(ErrorCode,ErrorString,BUFSIZ);

		jkGui->ShowDebug(0,Item,L"  Error while processing clustermap: %s",ErrorString);

		return;
	}

	if (Fragment != NULL)
	{
		jkGui->ShowDebug(0,NULL,L"  Extra fragment from MFT");
	}

	if (Item->Clusters != Clusters)
	{
		jkGui->ShowDebug(0,NULL,L"  Different cluster count: %I64u <> %I64u",
			Item->Clusters,Clusters);
	}
}

/* Scan all files in a directory and all it's subdirectories (recursive)
and store the information in a tree in memory for later use by the
optimizer. */
void JKDefragLib::ScanDir(struct DefragDataStruct *Data, WCHAR *Mask, struct ItemStruct *ParentDirectory)
{
	struct ItemStruct *Item;

	struct FragmentListStruct *Fragment;

	HANDLE FindHandle;

	WIN32_FIND_DATAW FindFileData;

	WCHAR *RootPath;
	WCHAR *TempPath;

	HANDLE FileHandle;

	ULONG64 SystemTime;

	SYSTEMTIME Time1;

	FILETIME Time2;

	ULARGE_INTEGER Time3;

	int Result;

	size_t Length;

	WCHAR *p1;

	JKDefragGui *jkGui = JKDefragGui::getInstance();

	/* Slow the program down to the percentage that was specified on the
	command line. */
	SlowDown(Data);

	/* Determine the rootpath (base path of the directory) by stripping
	everything after the last backslash in the Mask. The FindFirstFile()
	system call only processes wildcards in the last section (i.e. after
	the last backslash). */
	RootPath = _wcsdup(Mask);

	if (RootPath == NULL) return;

	p1 = wcsrchr(RootPath,'\\');

	if (p1 != NULL) *p1 = 0;

	/* Show debug message: "Analyzing: %s". */
	jkGui->ShowDebug(3,NULL,Data->DebugMsg[23],Mask);

	/* Fetch the current time in the ULONG64 format (1 second = 10000000). */
	GetSystemTime(&Time1);

	if (SystemTimeToFileTime(&Time1,&Time2) == FALSE)
	{
		SystemTime = 0;
	}
	else
	{
		Time3.LowPart = Time2.dwLowDateTime;
		Time3.HighPart = Time2.dwHighDateTime;

		SystemTime = Time3.QuadPart;
	}

	/* Walk through all the files. If nothing found then exit.
	Note: I am using FindFirstFileW() instead of _findfirst() because the latter
	will crash (exit program) on files with badly formed dates. */
	FindHandle = FindFirstFileW(Mask,&FindFileData);

	if (FindHandle == INVALID_HANDLE_VALUE)
	{
		free(RootPath);
		return;
	}

	Item = NULL;

	do 
	{
		if (*Data->Running != RUNNING) break;

		if (wcscmp(FindFileData.cFileName,L".") == 0) continue;
		if (wcscmp(FindFileData.cFileName,L"..") == 0) continue;

		/* Ignore reparse-points, a directory where a volume is mounted
		with the MOUNTVOL command. */
		if ((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0)
		{
			continue;
		}

		/* Cleanup old item. */
		if (Item != NULL)
		{
			if (Item->ShortPath != NULL) free(Item->ShortPath);
			if (Item->ShortFilename != NULL) free(Item->ShortFilename);
			if (Item->LongPath != NULL) free(Item->LongPath);
			if (Item->LongFilename != NULL) free(Item->LongFilename);

			while (Item->Fragments != NULL)
			{
				Fragment = Item->Fragments->Next;

				free(Item->Fragments);

				Item->Fragments = Fragment;
			}

			free(Item);

			Item = NULL;
		}

		/* Create new item. */
		Item = (struct ItemStruct *)malloc(sizeof(struct ItemStruct));

		if (Item == NULL) break;

		Item->ShortPath = NULL;
		Item->ShortFilename = NULL;
		Item->LongPath = NULL;
		Item->LongFilename = NULL;
		Item->Fragments = NULL;

		Length = wcslen(RootPath) + wcslen(FindFileData.cFileName) + 2;

		Item->LongPath = (WCHAR *)malloc(sizeof(WCHAR) * Length);

		if (Item->LongPath == NULL) break;

		swprintf_s(Item->LongPath,Length,L"%s\\%s",RootPath,FindFileData.cFileName);

		Item->LongFilename = _wcsdup(FindFileData.cFileName);

		if (Item->LongFilename == NULL) break;

		Length = wcslen(RootPath) + wcslen(FindFileData.cAlternateFileName) + 2;

		Item->ShortPath = (WCHAR *)malloc(sizeof(WCHAR) * Length);

		if (Item->ShortPath == NULL) break;

		swprintf_s(Item->ShortPath,Length,L"%s\\%s",RootPath,FindFileData.cAlternateFileName);

		Item->ShortFilename = _wcsdup(FindFileData.cAlternateFileName);

		if (Item->ShortFilename == NULL) break;

		Item->Bytes = FindFileData.nFileSizeHigh * ((ULONG64)MAXDWORD + 1) +
			FindFileData.nFileSizeLow;

		Item->Clusters = 0;
		Item->CreationTime = 0;
		Item->LastAccessTime = 0;
		Item->MftChangeTime = 0;
		Item->ParentDirectory = ParentDirectory;
		Item->Directory = NO;

		if ((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
		{
			Item->Directory = YES;
		}
		Item->Unmovable = NO;
		Item->Exclude = NO;
		Item->SpaceHog = NO;

		/* Analyze the item: Clusters and Fragments, and the CreationTime, LastAccessTime,
		and MftChangeTime. If the item could not be opened then ignore the item. */
		FileHandle = OpenItemHandle(Data,Item);

		if (FileHandle == NULL) continue;

		Result = GetFragments(Data,Item,FileHandle);

		CloseHandle(FileHandle);

		if (Result == NO) continue;

		/* Increment counters. */
		Data->CountAllFiles = Data->CountAllFiles + 1;
		Data->CountAllBytes = Data->CountAllBytes + Item->Bytes;
		Data->CountAllClusters = Data->CountAllClusters + Item->Clusters;

		if (IsFragmented(Item,0,Item->Clusters) == YES)
		{
			Data->CountFragmentedItems = Data->CountFragmentedItems + 1;
			Data->CountFragmentedBytes = Data->CountFragmentedBytes + Item->Bytes;
			Data->CountFragmentedClusters = Data->CountFragmentedClusters + Item->Clusters;
		}

		Data->PhaseDone = Data->PhaseDone + Item->Clusters;

		/* Show progress message. */
		jkGui->ShowAnalyze(Data,Item);

		/* If it's a directory then iterate subdirectories. */
		if (Item->Directory == YES)
		{
			Data->CountDirectories = Data->CountDirectories + 1;

			Length = wcslen(RootPath) + wcslen(FindFileData.cFileName) + 4;

			TempPath = (WCHAR *)malloc(sizeof(WCHAR) * Length);

			if (TempPath != NULL)
			{
				swprintf_s(TempPath,Length,L"%s\\%s\\*",RootPath,FindFileData.cFileName);
				ScanDir(Data,TempPath,Item);
				free(TempPath);
			}
		}

		/* Ignore the item if it has no clusters or no LCN. Very small
		files are stored in the MFT and are reported by Windows as
		having zero clusters and no fragments. */
		if ((Item->Clusters == 0) || (Item->Fragments == NULL)) continue;

		/* Draw the item on the screen. */
//		if (*Data->RedrawScreen == 0) {
			ColorizeItem(Data,Item,0,0,NO);
//		} else {
//			m_jkGui->ShowDiskmap(Data);
//		}

		/* Show debug info about the file. */
		/* Show debug message: "%I64d clusters at %I64d, %I64d bytes" */
		jkGui->ShowDebug(4,Item,Data->DebugMsg[16],Item->Clusters,GetItemLcn(Item),Item->Bytes);

		if ((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_COMPRESSED) != 0)
		{
			/* Show debug message: "Special file attribute: Compressed" */
			jkGui->ShowDebug(4,Item,Data->DebugMsg[17]);
		}
		
		if ((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_ENCRYPTED) != 0)
		{
			/* Show debug message: "Special file attribute: Encrypted" */
			jkGui->ShowDebug(4,Item,Data->DebugMsg[18]);
		}

		if ((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_OFFLINE) != 0)
		{
			/* Show debug message: "Special file attribute: Offline" */
			jkGui->ShowDebug(4,Item,Data->DebugMsg[19]);
		}

		if ((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_READONLY) != 0)
		{
			/* Show debug message: "Special file attribute: Read-only" */
			jkGui->ShowDebug(4,Item,Data->DebugMsg[20]);
		}

		if ((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_SPARSE_FILE) != 0)
		{
			/* Show debug message: "Special file attribute: Sparse-file" */
			jkGui->ShowDebug(4,Item,Data->DebugMsg[21]);
		}

		if ((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_TEMPORARY) != 0)
		{
			/* Show debug message: "Special file attribute: Temporary" */
			jkGui->ShowDebug(4,Item,Data->DebugMsg[22]);
		}

		/* Save some memory if short and long filename are the same. */
		if ((Item->LongFilename != NULL) &&
			(Item->ShortFilename != NULL) &&
			(_wcsicmp(Item->LongFilename,Item->ShortFilename) == 0))
		{
			free(Item->ShortFilename);
			Item->ShortFilename = Item->LongFilename;
		}

		if ((Item->LongFilename == NULL) && (Item->ShortFilename != NULL)) Item->LongFilename = Item->ShortFilename;
		if ((Item->LongFilename != NULL) && (Item->ShortFilename == NULL)) Item->ShortFilename = Item->LongFilename;

		if ((Item->LongPath != NULL) &&
			(Item->ShortPath != NULL) &&
			(_wcsicmp(Item->LongPath,Item->ShortPath) == 0))
		{
			free(Item->ShortPath);
			Item->ShortPath = Item->LongPath;
		}

		if ((Item->LongPath == NULL) && (Item->ShortPath != NULL)) Item->LongPath = Item->ShortPath;
		if ((Item->LongPath != NULL) && (Item->ShortPath == NULL)) Item->ShortPath = Item->LongPath;

		/* Add the item to the ItemTree in memory. */
		TreeInsert(Data,Item);
		Item = NULL;

	} while (FindNextFileW(FindHandle,&FindFileData) != 0);

	FindClose(FindHandle);

	/* Cleanup. */
	free(RootPath);

	if (Item != NULL)
	{
		if (Item->ShortPath != NULL) free(Item->ShortPath);
		if (Item->ShortFilename != NULL) free(Item->ShortFilename);
		if (Item->LongPath != NULL) free(Item->LongPath);
		if (Item->LongFilename != NULL) free(Item->LongFilename);

		while (Item->Fragments != NULL)
		{
			Fragment = Item->Fragments->Next;

			free(Item->Fragments);

			Item->Fragments = Fragment;
		}

		free(Item);
	}
}

/* Scan all files in a volume and store the information in a tree in
memory for later use by the optimizer. */
void JKDefragLib::AnalyzeVolume(struct DefragDataStruct *Data)
{
	struct ItemStruct *Item;

	BOOL Result;

	ULONG64 SystemTime;

	SYSTEMTIME Time1;

	FILETIME Time2;

	ULARGE_INTEGER Time3;

	int i;

	JKDefragGui *jkGui = JKDefragGui::getInstance();
	JKScanFat   *jkScanFat = JKScanFat::getInstance();
	JKScanNtfs  *jkScanNtfs = JKScanNtfs::getInstance();

	CallShowStatus(Data,1,-1);             /* "Phase 1: Analyze" */

	/* Fetch the current time in the ULONG64 format (1 second = 10000000). */
	GetSystemTime(&Time1);

	if (SystemTimeToFileTime(&Time1,&Time2) == FALSE)
	{
		SystemTime = 0;
	}
	else
	{
		Time3.LowPart = Time2.dwLowDateTime;
		Time3.HighPart = Time2.dwHighDateTime;

		SystemTime = Time3.QuadPart;
	}

	/* Scan NTFS disks. */
	Result = jkScanNtfs->AnalyzeNtfsVolume(Data);

	/* Scan FAT disks. */
	if ((Result == FALSE) && (*Data->Running == RUNNING)) Result = jkScanFat->AnalyzeFatVolume(Data);

	/* Scan all other filesystems. */
	if ((Result == FALSE) && (*Data->Running == RUNNING))
	{
		jkGui->ShowDebug(0,NULL,L"This is not a FAT or NTFS disk, using the slow scanner.");

		/* Setup the width of the progress bar. */
		Data->PhaseTodo = Data->TotalClusters - Data->CountFreeClusters;

		for (i = 0; i < 3; i++)
		{
			Data->PhaseTodo = Data->PhaseTodo - (Data->MftExcludes[i].End - Data->MftExcludes[i].Start);
		}

		/* Scan all the files. */
		ScanDir(Data,Data->IncludeMask,NULL);
	}

	/* Update the diskmap with the colors. */
	Data->PhaseDone = Data->PhaseTodo;
	jkGui->DrawCluster(Data,0,0,0);

	/* Setup the progress counter and the file/dir counters. */
	Data->PhaseDone = 0;
	Data->PhaseTodo = 0;

	for (Item = TreeSmallest(Data->ItemTree); Item != NULL; Item = TreeNext(Item))
	{
		Data->PhaseTodo = Data->PhaseTodo + 1;
	}

	jkGui->ShowAnalyze(NULL,NULL);

	/* Walk through all the items one by one. */
	for (Item = TreeSmallest(Data->ItemTree); Item != NULL; Item = TreeNext(Item))
	{
		if (*Data->Running != RUNNING) break;

		/* If requested then redraw the diskmap. */
//		if (*Data->RedrawScreen == 1) m_jkGui->ShowDiskmap(Data);

		/* Construct the full path's of the item. The MFT contains only the filename, plus
		a pointer to the directory. We have to construct the full paths's by joining
		all the names of the directories, and the name of the file. */
		if (Item->LongPath == NULL) Item->LongPath = GetLongPath(Data,Item);
		if (Item->ShortPath == NULL) Item->ShortPath = GetShortPath(Data,Item);

		/* Save some memory if the short and long paths are the same. */
		if ((Item->LongPath != NULL) &&
			(Item->ShortPath != NULL) &&
			(Item->LongPath != Item->ShortPath) &&
			(_wcsicmp(Item->LongPath,Item->ShortPath) == 0))
		{
			free(Item->ShortPath);
			Item->ShortPath = Item->LongPath;
		}

		if ((Item->LongPath == NULL) && (Item->ShortPath != NULL)) Item->LongPath = Item->ShortPath;
		if ((Item->LongPath != NULL) && (Item->ShortPath == NULL)) Item->ShortPath = Item->LongPath;

		/* For debugging only: compare the data with the output from the
		FSCTL_GET_RETRIEVAL_POINTERS function call. */
		/*
		CompareItems(Data,Item);
		*/

		/* Apply the Mask and set the Exclude flag of all items that do not match. */
		if ((MatchMask(Item->LongPath,Data->IncludeMask) == NO) &&
			(MatchMask(Item->ShortPath,Data->IncludeMask) == NO))
		{
			Item->Exclude = YES;

			ColorizeItem(Data,Item,0,0,NO);
		}

		/* Determine if the item is to be excluded by comparing it's name with the
		Exclude masks. */
		if ((Item->Exclude == NO) && (Data->Excludes != NULL))
		{
			for (i = 0; Data->Excludes[i] != NULL; i++)
			{
				if ((MatchMask(Item->LongPath,Data->Excludes[i]) == YES) ||
					(MatchMask(Item->ShortPath,Data->Excludes[i]) == YES))
				{
					Item->Exclude = YES;

					ColorizeItem(Data,Item,0,0,NO);

					break;
				}
			}
		}

		/* Exclude my own logfile. */
		if ((Item->Exclude == NO) &&
			(Item->LongFilename != NULL) &&
			((_wcsicmp(Item->LongFilename,L"jkdefrag.log") == 0) ||
			(_wcsicmp(Item->LongFilename,L"jkdefragcmd.log") == 0) ||
			(_wcsicmp(Item->LongFilename,L"jkdefragscreensaver.log") == 0)))
		{
			Item->Exclude = YES;

			ColorizeItem(Data,Item,0,0,NO);
		}

		/* The item is a SpaceHog if it's larger than 50 megabytes, or last access time
		is more than 30 days ago, or if it's filename matches a SpaceHog mask. */
		if ((Item->Exclude == NO) && (Item->Directory == NO))
		{
			if ((Data->UseDefaultSpaceHogs == YES) && (Item->Bytes > 50 * 1024 * 1024))
			{
				Item->SpaceHog = YES;
			}
			else if ((Data->UseDefaultSpaceHogs == YES) &&
				(Data->UseLastAccessTime == TRUE) &&
				(Item->LastAccessTime + (ULONG64)(30 * 24 * 60 * 60) * 10000000 < SystemTime))
			{
				Item->SpaceHog = YES;
			}
			else if (Data->SpaceHogs != NULL)
			{
				for (i = 0; Data->SpaceHogs[i] != NULL; i++)
				{
					if ((MatchMask(Item->LongPath,Data->SpaceHogs[i]) == YES) ||
						(MatchMask(Item->ShortPath,Data->SpaceHogs[i]) == YES))
					{
						Item->SpaceHog = YES;

						break;
					}
				}
			}

			if (Item->SpaceHog == YES) ColorizeItem(Data,Item,0,0,NO);
		}

		/* Special exception for "http://www.safeboot.com/". */
		if (MatchMask(Item->LongPath,L"*\\safeboot.fs") == YES) Item->Unmovable = YES;

		/* Special exception for Acronis OS Selector. */
		if (MatchMask(Item->LongPath,L"?:\\bootwiz.sys") == YES) Item->Unmovable = YES;
		if (MatchMask(Item->LongPath,L"*\\BOOTWIZ\\*") == YES) Item->Unmovable = YES;

		/* Special exception for DriveCrypt by "http://www.securstar.com/". */
		if (MatchMask(Item->LongPath,L"?:\\BootAuth?.sys") == YES) Item->Unmovable = YES;

		/* Special exception for Symantec GoBack. */
		if (MatchMask(Item->LongPath,L"*\\Gobackio.bin") == YES) Item->Unmovable = YES;

		/* The $BadClus file maps the entire disk and is always unmovable. */
		if ((Item->LongFilename != NULL) &&
			((_wcsicmp(Item->LongFilename,L"$BadClus") == 0) ||
			(_wcsicmp(Item->LongFilename,L"$BadClus:$Bad:$DATA") == 0)))
		{
			Item->Unmovable = YES;
		}

		/* Update the progress percentage. */
		Data->PhaseDone = Data->PhaseDone + 1;

		if (Data->PhaseDone % 10000 == 0) jkGui->DrawCluster(Data,0,0,0);
	}

	/* Force the percentage to 100%. */
	Data->PhaseDone = Data->PhaseTodo;
	jkGui->DrawCluster(Data,0,0,0);

	/* Calculate the begin of the zone's. */
	CalculateZones(Data);

	/* Call the ShowAnalyze() callback one last time. */
	jkGui->ShowAnalyze(Data,NULL);
}

/* Move items to their zone. This will:
- Defragment all fragmented files
- Move regular files out of the directory zone.
- Move SpaceHogs out of the directory- and regular zones.
- Move items out of the MFT reserved zones
*/
void JKDefragLib::Fixup(struct DefragDataStruct *Data)
{
	struct ItemStruct *Item;
	struct ItemStruct *NextItem;

	ULONG64 ItemLcn;
	ULONG64 GapBegin[3];
	ULONG64 GapEnd[3];

	int FileZone;

	WIN32_FILE_ATTRIBUTE_DATA Attributes;

	ULONG64 FileTime;

	FILETIME SystemTime1;

	ULONG64 SystemTime;
	ULONG64 LastCalcTime;

	int Result;

	ULARGE_INTEGER u;

	int MoveMe;

	JKDefragGui *jkGui = JKDefragGui::getInstance();

	CallShowStatus(Data,8,-1);               /* "Phase 3: Fixup" */

	/* Initialize: fetch the current time. */
	GetSystemTimeAsFileTime(&SystemTime1);

	u.LowPart = SystemTime1.dwLowDateTime;
	u.HighPart = SystemTime1.dwHighDateTime;

	SystemTime = u.QuadPart;

	/* Initialize the width of the progress bar: the total number of clusters
	of all the items. */
	for (Item = TreeSmallest(Data->ItemTree); Item != NULL; Item = TreeNext(Item))
	{
		if (Item->Unmovable == YES) continue;
		if (Item->Exclude == YES) continue;
		if (Item->Clusters == 0) continue;

		Data->PhaseTodo = Data->PhaseTodo + Item->Clusters;
	}

	LastCalcTime = SystemTime;

	/* Exit if nothing to do. */
	if (Data->PhaseTodo == 0) return;

	/* Walk through all files and move the files that need to be moved. */
	for (FileZone = 0; FileZone < 3; FileZone++)
	{
		GapBegin[FileZone] = 0;
		GapEnd[FileZone] = 0;
	}

	NextItem = TreeSmallest(Data->ItemTree);

	while ((NextItem != NULL) && (*Data->Running == RUNNING))
	{
		/* The loop will change the position of the item in the tree, so we have
		to determine the next item before executing the loop. */
		Item = NextItem;

		NextItem = TreeNext(Item);

		/* Ignore items that are unmovable or excluded. */
		if (Item->Unmovable == YES) continue;
		if (Item->Exclude == YES) continue;
		if (Item->Clusters == 0) continue;

		/* Ignore items that do not need to be moved. */
		FileZone = 1;

		if (Item->SpaceHog == YES) FileZone = 2;
		if (Item->Directory == YES) FileZone = 0;

		ItemLcn = GetItemLcn(Item);

		MoveMe = NO;

		if (IsFragmented(Item,0,Item->Clusters) == YES)
		{
			/* "I am fragmented." */
			jkGui->ShowDebug(4,Item,Data->DebugMsg[53]);

			MoveMe = YES;
		}

		if ((MoveMe == NO) &&
			(((ItemLcn >= Data->MftExcludes[0].Start) && (ItemLcn < Data->MftExcludes[0].End)) ||
			((ItemLcn >= Data->MftExcludes[1].Start) && (ItemLcn < Data->MftExcludes[1].End)) ||
			((ItemLcn >= Data->MftExcludes[2].Start) && (ItemLcn < Data->MftExcludes[2].End))) &&
			((Data->Disk.Type != NTFS) || (MatchMask(Item->LongPath,L"?:\\$MFT") != YES)))
		{
			/* "I am in MFT reserved space." */
			jkGui->ShowDebug(4,Item,Data->DebugMsg[54]);

			MoveMe = YES;
		}

		if ((FileZone == 1) && (ItemLcn < Data->Zones[1]) && (MoveMe == NO))
		{
			/* "I am a regular file in zone 1." */
			jkGui->ShowDebug(4,Item,Data->DebugMsg[55]);

			MoveMe = YES;
		}

		if ((FileZone == 2) && (ItemLcn < Data->Zones[2]) && (MoveMe == NO))
		{
			/* "I am a spacehog in zone 1 or 2." */
			jkGui->ShowDebug(4,Item,Data->DebugMsg[56]);

			MoveMe = YES;
		}

		if (MoveMe == NO)
		{
			Data->PhaseDone = Data->PhaseDone + Item->Clusters;

			continue;
		}

		/* Ignore files that have been modified less than 15 minutes ago. */
		if (Item->Directory == NO)
		{
			Result = GetFileAttributesExW(Item->LongPath,GetFileExInfoStandard,&Attributes);

			if (Result != 0)
			{
				u.LowPart = Attributes.ftLastWriteTime.dwLowDateTime;
				u.HighPart = Attributes.ftLastWriteTime.dwHighDateTime;

				FileTime = u.QuadPart;

				if (FileTime + 15 * 60 * (ULONG64)10000000 > SystemTime)
				{
					Data->PhaseDone = Data->PhaseDone + Item->Clusters;

					continue;
				}
			}
		}

		/* If the file does not fit in the current gap then find another gap. */
		if (Item->Clusters > GapEnd[FileZone] - GapBegin[FileZone])
		{
			Result = FindGap(Data,Data->Zones[FileZone],0,Item->Clusters,YES,NO,&GapBegin[FileZone],
				&GapEnd[FileZone],FALSE);

			if (Result == NO)
			{
				/* Show debug message: "Cannot move item away because no gap is big enough: %I64d[%lu]" */
				jkGui->ShowDebug(2,Item,Data->DebugMsg[25],GetItemLcn(Item),Item->Clusters);

				GapEnd[FileZone] = GapBegin[FileZone];         /* Force re-scan of gap. */

				Data->PhaseDone = Data->PhaseDone + Item->Clusters;

				continue;
			}
		}

		/* Move the item. */
		Result = MoveItem(Data,Item,GapBegin[FileZone],0,Item->Clusters,0);

		if (Result == YES)
		{
			GapBegin[FileZone] = GapBegin[FileZone] + Item->Clusters;
		}
		else
		{
			GapEnd[FileZone] = GapBegin[FileZone];         /* Force re-scan of gap. */
		}

		/* Get new system time. */
		GetSystemTimeAsFileTime(&SystemTime1);

		u.LowPart = SystemTime1.dwLowDateTime;
		u.HighPart = SystemTime1.dwHighDateTime;

		SystemTime = u.QuadPart;
	}
}

/* Defragment all the fragmented files. */
void JKDefragLib::Defragment(struct DefragDataStruct *Data)
{
	struct ItemStruct *Item;
	struct ItemStruct *NextItem;

	ULONG64 GapBegin;
	ULONG64 GapEnd;
	ULONG64 ClustersDone;
	ULONG64 Clusters;

	struct FragmentListStruct *Fragment;

	ULONG64 Vcn;
	ULONG64 RealVcn;

	HANDLE FileHandle;

	int FileZone;
	int Result;

	JKDefragGui *jkGui = JKDefragGui::getInstance();

	CallShowStatus(Data,2,-1);               /* "Phase 2: Defragment" */

	/* Setup the width of the progress bar: the number of clusters in all
	fragmented files. */
	for (Item = TreeSmallest(Data->ItemTree); Item != NULL; Item = TreeNext(Item))
	{
		if (Item->Unmovable == YES) continue;
		if (Item->Exclude == YES) continue;
		if (Item->Clusters == 0) continue;

		if (IsFragmented(Item,0,Item->Clusters) == NO) continue;

		Data->PhaseTodo = Data->PhaseTodo + Item->Clusters;
	}

	/* Exit if nothing to do. */
	if (Data->PhaseTodo == 0) return;

	/* Walk through all files and defrag. */
	NextItem = TreeSmallest(Data->ItemTree);

	while ((NextItem != NULL) && (*Data->Running == RUNNING))
	{
		/* The loop may change the position of the item in the tree, so we have
		to determine and remember the next item now. */
		Item = NextItem;

		NextItem = TreeNext(Item);

		/* Ignore if the Item cannot be moved, or is Excluded, or is not fragmented. */
		if (Item->Unmovable == YES) continue;
		if (Item->Exclude == YES) continue;
		if (Item->Clusters == 0) continue;

		if (IsFragmented(Item,0,Item->Clusters) == NO) continue;

		/* Find a gap that is large enough to hold the item, or the largest gap
		on the volume. If the disk is full then show a message and exit. */
		FileZone = 1;

		if (Item->SpaceHog == YES) FileZone = 2;
		if (Item->Directory == YES) FileZone = 0;

		Result = FindGap(Data,Data->Zones[FileZone],0,Item->Clusters,NO,NO,&GapBegin,&GapEnd,FALSE);

		if (Result == NO)
		{
			/* Try finding a gap again, this time including the free area. */
			Result = FindGap(Data,0,0,Item->Clusters,NO,NO,&GapBegin,&GapEnd,FALSE);

			if (Result == NO)
			{
				/* Show debug message: "Disk is full, cannot defragment." */
				jkGui->ShowDebug(2,Item,Data->DebugMsg[44]);

				return;
			}
		}

		/* If the gap is big enough to hold the entire item then move the file
		in a single go, and loop. */
		if (GapEnd - GapBegin >= Item->Clusters)
		{
			MoveItem(Data,Item,GapBegin,0,Item->Clusters,0);

			continue;
		}

		/* Open a filehandle for the item. If error then set the Unmovable flag,
		colorize the item on the screen, and loop. */
		FileHandle = OpenItemHandle(Data,Item);

		if (FileHandle == NULL)
		{
			Item->Unmovable = YES;

			ColorizeItem(Data,Item,0,0,NO);

			continue;
		}

		/* Move the file in parts, each time selecting the biggest gap
		available. */
		ClustersDone = 0;

		do
		{
			Clusters = GapEnd - GapBegin;

			if (Clusters > Item->Clusters - ClustersDone)
			{
				Clusters = Item->Clusters - ClustersDone;
			}

			/* Make sure that the gap is bigger than the first fragment of the
			block that we're about to move. If not then the result would be
			more fragments, not less. */
			Vcn = 0;
			RealVcn = 0;

			for (Fragment = Item->Fragments; Fragment != NULL; Fragment = Fragment->Next)
			{
				if (Fragment->Lcn != VIRTUALFRAGMENT)
				{
					if (RealVcn >= ClustersDone)
					{
						if (Clusters > Fragment->NextVcn - Vcn) break;

						ClustersDone = RealVcn + Fragment->NextVcn - Vcn;

						Data->PhaseDone = Data->PhaseDone + Fragment->NextVcn - Vcn;
					}

					RealVcn = RealVcn + Fragment->NextVcn - Vcn;
				}

				Vcn = Fragment->NextVcn;
			}

			if (ClustersDone >= Item->Clusters) break;

			/* Move the segment. */
			Result = MoveItem4(Data,Item,FileHandle,GapBegin,ClustersDone,Clusters,0);

			/* Next segment. */
			ClustersDone = ClustersDone + Clusters;

			/* Find a gap large enough to hold the remainder, or the largest gap
			on the volume. */
			if (ClustersDone < Item->Clusters)
			{
				Result = FindGap(Data,Data->Zones[FileZone],0,Item->Clusters - ClustersDone,
					NO,NO,&GapBegin,&GapEnd,FALSE);

				if (Result == NO) break;
			}

		} while ((ClustersDone < Item->Clusters) && (*Data->Running == RUNNING));

		/* Close the item. */
		FlushFileBuffers(FileHandle);            /* Is this useful? Can't hurt. */
		CloseHandle(FileHandle);
	}
}

/* Fill all the gaps at the beginning of the disk with fragments from the files above. */
void JKDefragLib::ForcedFill(struct DefragDataStruct *Data)
{
	ULONG64 GapBegin;
	ULONG64 GapEnd;

	struct ItemStruct *Item;
	struct FragmentListStruct *Fragment;
	struct ItemStruct *HighestItem;

	ULONG64 MaxLcn;
	ULONG64 HighestLcn;
	ULONG64 HighestVcn;
	ULONG64 HighestSize;
	ULONG64 Clusters;
	ULONG64 Vcn;
	ULONG64 RealVcn;

	int Result;

	CallShowStatus(Data,3,-1);            /* "Phase 3: ForcedFill" */

	/* Walk through all the gaps. */
	GapBegin = 0;
	MaxLcn = Data->TotalClusters;

	while (*Data->Running == RUNNING)
	{
		/* Find the next gap. If there are no more gaps then exit. */
		Result = FindGap(Data,GapBegin,0,0,YES,NO,&GapBegin,&GapEnd,FALSE);

		if (Result == NO) break;

		/* Find the item with the highest fragment on disk. */
		HighestItem = NULL;
		HighestLcn = 0;
		HighestVcn = 0;
		HighestSize = 0;

		for (Item = TreeBiggest(Data->ItemTree); Item != NULL; Item = TreePrev(Item))
		{
			if (Item->Unmovable == YES) continue;
			if (Item->Exclude == YES) continue;
			if (Item->Clusters == 0) continue;

			Vcn = 0;
			RealVcn = 0;

			for (Fragment = Item->Fragments; Fragment != NULL; Fragment = Fragment->Next)
			{
				if (Fragment->Lcn != VIRTUALFRAGMENT)
				{
					if ((Fragment->Lcn > HighestLcn) && (Fragment->Lcn < MaxLcn))
					{
						HighestItem = Item;
						HighestLcn = Fragment->Lcn;
						HighestVcn = RealVcn;
						HighestSize = Fragment->NextVcn - Vcn;
					}

					RealVcn = RealVcn + Fragment->NextVcn - Vcn;
				}

				Vcn = Fragment->NextVcn;
			}
		}

		if (HighestItem == NULL) break;

		/* If the highest fragment is before the gap then exit, we're finished. */
		if (HighestLcn <= GapBegin) break;

		/* Move as much of the item into the gap as possible. */
		Clusters = GapEnd - GapBegin;

		if (Clusters > HighestSize) Clusters = HighestSize;

		Result = MoveItem(Data,HighestItem,GapBegin,HighestVcn + HighestSize - Clusters, Clusters,0);

		GapBegin = GapBegin + Clusters;
		MaxLcn = HighestLcn + HighestSize - Clusters;
	}
}

/* Vacate an area by moving files upward. If there are unmovable files at the Lcn then
skip them. Then move files upward until the gap is bigger than Clusters, or when we
encounter an unmovable file. */
void JKDefragLib::Vacate(struct DefragDataStruct *Data, ULONG64 Lcn, ULONG64 Clusters, BOOL IgnoreMftExcludes)
{
	ULONG64 TestGapBegin;
	ULONG64 TestGapEnd;
	ULONG64 MoveGapBegin;
	ULONG64 MoveGapEnd;

	struct ItemStruct *Item;
	struct FragmentListStruct *Fragment;

	ULONG64 Vcn;
	ULONG64 RealVcn;

	struct ItemStruct *BiggerItem;

	ULONG64 BiggerBegin;
	ULONG64 BiggerEnd;
	ULONG64 BiggerRealVcn;
	ULONG64 MoveTo;
	ULONG64 DoneUntil;

	int Result;

	JKDefragGui *jkGui = JKDefragGui::getInstance();

	jkGui->ShowDebug(5,NULL,L"Vacating %I64u clusters starting at LCN=%I64u",Clusters,Lcn);

	/* Sanity check. */
	if (Lcn >= Data->TotalClusters)
	{
		jkGui->ShowDebug(1,NULL,L"Error: trying to vacate an area beyond the end of the disk.");

		return;
	}

	/* Determine the point to above which we will be moving the data. We want at least the
	end of the zone if everything was perfectly optimized, so data will not be moved
	again and again. */
	MoveTo = Lcn + Clusters;

	if (Data->Zone == 0) MoveTo = Data->Zones[1];
	if (Data->Zone == 1) MoveTo = Data->Zones[2];

	if (Data->Zone == 2)
	{
		/* Zone 2: end of disk minus all the free space. */
		MoveTo = Data->TotalClusters - Data->CountFreeClusters +
			(ULONG64)(Data->TotalClusters * 2.0 * Data->FreeSpace / 100.0);
	}

	if (MoveTo < Lcn + Clusters) MoveTo = Lcn + Clusters;

	jkGui->ShowDebug(5,NULL,L"MoveTo = %I64u",MoveTo);

	/* Loop forever. */
	MoveGapBegin = 0;
	MoveGapEnd = 0;
	DoneUntil = Lcn;

	while (*Data->Running == RUNNING)
	{
		/* Find the first movable data fragment at or above the DoneUntil Lcn. If there is nothing
		then return, we have reached the end of the disk. */
		BiggerItem = NULL;
		BiggerBegin = 0;

		for (Item = TreeSmallest(Data->ItemTree); Item != NULL; Item = TreeNext(Item))
		{
			if ((Item->Unmovable == YES) || (Item->Exclude == YES) || (Item->Clusters == 0))
			{
				continue;
			}

			Vcn = 0;
			RealVcn = 0;

			for (Fragment = Item->Fragments; Fragment != NULL; Fragment = Fragment->Next)
			{
				if (Fragment->Lcn != VIRTUALFRAGMENT)
				{
					if ((Fragment->Lcn >= DoneUntil) &&
						((BiggerBegin > Fragment->Lcn) || (BiggerItem == NULL)))
					{
						BiggerItem = Item;
						BiggerBegin = Fragment->Lcn;
						BiggerEnd = Fragment->Lcn + Fragment->NextVcn - Vcn;
						BiggerRealVcn = RealVcn;

						if (BiggerBegin == Lcn) break;
					}

					RealVcn = RealVcn + Fragment->NextVcn - Vcn;
				}

				Vcn = Fragment->NextVcn;
			}

			if ((BiggerBegin != 0) && (BiggerBegin == Lcn)) break;
		}

		if (BiggerItem == NULL)
		{
			jkGui->ShowDebug(5,NULL,L"No data found above LCN=%I64u",Lcn);

			return;
		}

		jkGui->ShowDebug(5,NULL,L"Data found at LCN=%I64u, %s",BiggerBegin,BiggerItem->LongPath);

		/* Find the first gap above the Lcn. */
		Result = FindGap(Data,Lcn,0,0,YES,NO,&TestGapBegin,&TestGapEnd,IgnoreMftExcludes);

		if (Result == NO)
		{
			jkGui->ShowDebug(5,NULL,L"No gaps found above LCN=%I64u",Lcn);

			return;
		}

		/* Exit if the end of the first gap is below the first movable item, the gap cannot
		be enlarged. */
		if (TestGapEnd < BiggerBegin)
		{
			jkGui->ShowDebug(5,NULL,L"Cannot enlarge the gap from %I64u to %I64u (%I64u clusters) any further.",
				TestGapBegin,TestGapEnd,TestGapEnd - TestGapBegin);

			return;
		}

		/* Exit if the first movable item is at the end of the gap and the gap is big enough,
		no need to enlarge any further. */
		if ((TestGapEnd == BiggerBegin) && (TestGapEnd - TestGapBegin >= Clusters))
		{
			jkGui->ShowDebug(5,NULL,L"Finished vacating, the gap from %I64u to %I64u (%I64u clusters) is now bigger than %I64u clusters.",
				TestGapBegin,TestGapEnd,TestGapEnd - TestGapBegin,Clusters);

			return;
		}

		/* Exit if we have moved the item before. We don't want a worm. */
		if (Lcn >= MoveTo)
		{
			jkGui->ShowDebug(5,NULL,L"Stopping vacate because of possible worm.");
			return;
		}

		/* Determine where we want to move the fragment to. Maybe the previously used
		gap is big enough, otherwise we have to locate another gap. */
		if (BiggerEnd - BiggerBegin >= MoveGapEnd - MoveGapBegin)
		{
			Result = NO;

			/* First try to find a gap above the MoveTo point. */
			if ((MoveTo < Data->TotalClusters) && (MoveTo >= BiggerEnd))
			{
				jkGui->ShowDebug(5,NULL,L"Finding gap above MoveTo=%I64u",MoveTo);

				Result = FindGap(Data,MoveTo,0,BiggerEnd - BiggerBegin,YES,NO,&MoveGapBegin,&MoveGapEnd,FALSE);
			}

			/* If no gap was found then try to find a gap as high on disk as possible, but
			above the item. */
			if (Result == NO)
			{
				jkGui->ShowDebug(5,NULL,L"Finding gap from end of disk above BiggerEnd=%I64u",BiggerEnd);

				Result = FindGap(Data,BiggerEnd,0,BiggerEnd - BiggerBegin,YES,YES,&MoveGapBegin,
					&MoveGapEnd,FALSE);
			}

			/* If no gap was found then exit, we cannot move the item. */
			if (Result == NO)
			{
				jkGui->ShowDebug(5,NULL,L"No gap found.");

				return;
			}
		}

		/* Move the fragment to the gap. */
		Result = MoveItem(Data,BiggerItem,MoveGapBegin,BiggerRealVcn,BiggerEnd - BiggerBegin,0);

		if (Result == YES)
		{
			if (MoveGapBegin < MoveTo) MoveTo = MoveGapBegin;

			MoveGapBegin = MoveGapBegin + BiggerEnd - BiggerBegin;
		}
		else
		{
			MoveGapEnd = MoveGapBegin;         /* Force re-scan of gap. */
		}

		/* Adjust the DoneUntil Lcn. We don't want an infinite loop. */
		DoneUntil = BiggerEnd;
	}
}

/* Compare two items.
SortField=0    Filename
SortField=1    Filesize, smallest first
SortField=2    Date/Time LastAccess, oldest first
SortField=3    Date/Time LastChange, oldest first
SortField=4    Date/Time Creation, oldest first
Return values:
-1   Item1 is smaller than Item2
0    Equal
1    Item1 is bigger than Item2
*/
int JKDefragLib::CompareItems(struct ItemStruct *Item1, struct ItemStruct *Item2, int SortField)
{
	int Result;

	/* If one of the items is NULL then the other item is bigger. */
	if (Item1 == NULL) return(-1);
	if (Item2 == NULL) return(1);

	/* Return zero if the items are exactly the same. */
	if (Item1 == Item2) return(0);

	/* Compare the SortField of the items and return 1 or -1 if they are not equal. */
	if (SortField == 0)
	{
		if ((Item1->LongPath == NULL) && (Item2->LongPath == NULL)) return(0);
		if (Item1->LongPath == NULL) return(-1);
		if (Item2->LongPath == NULL) return(1);

		Result = _wcsicmp(Item1->LongPath,Item2->LongPath);

		if (Result != 0) return(Result);
	}

	if (SortField == 1)
	{
		if (Item1->Bytes < Item2->Bytes) return(-1);
		if (Item1->Bytes > Item2->Bytes) return(1);
	}

	if (SortField == 2)
	{
		if (Item1->LastAccessTime > Item2->LastAccessTime) return(-1);
		if (Item1->LastAccessTime < Item2->LastAccessTime) return(1);
	}

	if (SortField == 3)
	{
		if (Item1->MftChangeTime < Item2->MftChangeTime) return(-1);
		if (Item1->MftChangeTime > Item2->MftChangeTime) return(1);
	}

	if (SortField == 4)
	{
		if (Item1->CreationTime < Item2->CreationTime) return(-1);
		if (Item1->CreationTime > Item2->CreationTime) return(1);
	}

	/* The SortField of the items is equal, so we must compare all the other fields
	to see if they are really equal. */
	if ((Item1->LongPath != NULL) && (Item2->LongPath != NULL))
	{
		if (Item1->LongPath == NULL) return(-1);
		if (Item2->LongPath == NULL) return(1);

		Result = _wcsicmp(Item1->LongPath,Item2->LongPath);

		if (Result != 0) return(Result);
	}

	if (Item1->Bytes < Item2->Bytes) return(-1);
	if (Item1->Bytes > Item2->Bytes) return(1);
	if (Item1->LastAccessTime < Item2->LastAccessTime) return(-1);
	if (Item1->LastAccessTime > Item2->LastAccessTime) return(1);
	if (Item1->MftChangeTime < Item2->MftChangeTime) return(-1);
	if (Item1->MftChangeTime > Item2->MftChangeTime) return(1);
	if (Item1->CreationTime < Item2->CreationTime) return(-1);
	if (Item1->CreationTime > Item2->CreationTime) return(1);

	/* As a last resort compare the location on harddisk. */
	if (GetItemLcn(Item1) < GetItemLcn(Item2)) return(-1);
	if (GetItemLcn(Item1) > GetItemLcn(Item2)) return(1);

	return(0);
}

/* Optimize the volume by moving all the files into a sorted order.
SortField=0    Filename
SortField=1    Filesize
SortField=2    Date/Time LastAccess
SortField=3    Date/Time LastChange
SortField=4    Date/Time Creation
*/
void JKDefragLib::OptimizeSort(struct DefragDataStruct *Data, int SortField)
{
	struct ItemStruct *Item;
	struct ItemStruct *PreviousItem;
	struct ItemStruct *TempItem;

	ULONG64 Lcn;
	ULONG64 VacatedUntil;
	ULONG64 PhaseTemp;
	ULONG64 GapBegin;
	ULONG64 GapEnd;
	ULONG64 Clusters;
	ULONG64 ClustersDone;
	ULONG64 MinimumVacate;

	int Result;
	int FileZone;

	JKDefragGui *jkGui = JKDefragGui::getInstance();

	/* Sanity check. */
	if (Data->ItemTree == NULL) return;

	/* Process all the zones. */
	VacatedUntil = 0;
	MinimumVacate = Data->TotalClusters / 200;

	for (Data->Zone = 0; Data->Zone < 3; Data->Zone++)
	{
		CallShowStatus(Data,4,Data->Zone);            /* "Zone N: Sort" */

		/* Start at the begin of the zone and move all the items there, one by one
		in the requested sorting order, making room as we go. */
		PreviousItem = NULL;

		Lcn = Data->Zones[Data->Zone];

		GapBegin = 0;
		GapEnd = 0;

		while (*Data->Running == RUNNING)
		{
			/* Find the next item that we want to place. */
			Item = NULL;
			PhaseTemp = 0;

			for (TempItem = TreeSmallest(Data->ItemTree); TempItem != NULL; TempItem = TreeNext(TempItem))
			{
				if (TempItem->Unmovable == YES) continue;
				if (TempItem->Exclude == YES) continue;
				if (TempItem->Clusters == 0) continue;

				FileZone = 1;

				if (TempItem->SpaceHog == YES) FileZone = 2;
				if (TempItem->Directory == YES) FileZone = 0;
				if (FileZone != Data->Zone) continue;

				if ((PreviousItem != NULL) &&
					(CompareItems(PreviousItem,TempItem,SortField) >= 0))
				{
					continue;
				}

				PhaseTemp = PhaseTemp + TempItem->Clusters;

				if ((Item != NULL) && (CompareItems(TempItem,Item,SortField) >= 0)) continue;

				Item = TempItem;
			}

			if (Item == NULL)
			{
				jkGui->ShowDebug(2,NULL,L"Finished sorting zone %u.",Data->Zone+1);

				break;
			}

			PreviousItem = Item;
			Data->PhaseTodo = Data->PhaseDone + PhaseTemp;

			/* If the item is already at the Lcn then skip. */
			if (GetItemLcn(Item) == Lcn)
			{
				Lcn = Lcn + Item->Clusters;

				continue;
			}

			/* Move the item to the Lcn. If the gap at Lcn is not big enough then fragment
			the file into whatever gaps are available. */
			ClustersDone = 0;

			while ((*Data->Running == RUNNING) &&
				(ClustersDone < Item->Clusters) &&
				(Item->Unmovable == NO))
			{
				if (ClustersDone > 0)
				{
					jkGui->ShowDebug(5,NULL,L"Item partially placed, %I64u clusters more to do",
							Item->Clusters - ClustersDone);
				}

				/* Call the Vacate() function to make a gap at Lcn big enough to hold the item.
				The Vacate() function may not be able to move whatever is now at the Lcn, so
				after calling it we have to locate the first gap after the Lcn. */
				if (GapBegin + Item->Clusters - ClustersDone + 16 > GapEnd)
				{
					Vacate(Data,Lcn,Item->Clusters - ClustersDone + MinimumVacate,FALSE);

					Result = FindGap(Data,Lcn,0,0,YES,NO,&GapBegin,&GapEnd,FALSE);

					if (Result == NO) return;              /* No gaps found, exit. */
				}

				/* If the gap is not big enough to hold the entire item then calculate how much
				of the item will fit in the gap. */
				Clusters = Item->Clusters - ClustersDone;

				if (Clusters > GapEnd - GapBegin)
				{
					Clusters = GapEnd - GapBegin;

					/* It looks like a partial move only succeeds if the number of clusters is a
					multiple of 8. */
					Clusters = Clusters - (Clusters % 8);

					if (Clusters == 0)
					{
						Lcn = GapEnd;
						continue;
					}
				}

				/* Move the item to the gap. */
				Result = MoveItem(Data,Item,GapBegin,ClustersDone,Clusters,0);

				if (Result == YES)
				{
					GapBegin = GapBegin + Clusters;
				}
				else
				{
					Result = FindGap(Data,GapBegin,0,0,YES,NO,&GapBegin,&GapEnd,FALSE);
					if (Result == NO) return;              /* No gaps found, exit. */
				}

				Lcn = GapBegin;
				ClustersDone = ClustersDone + Clusters;
			}
		}
	}
}

/*

Move the MFT to the beginning of the harddisk.
- The Microsoft defragmentation api only supports moving the MFT on Vista.
- What to do if there is unmovable data at the beginning of the disk? I have
chosen to wrap the MFT around that data. The fragments will be aligned, so
the performance loss is minimal, and still faster than placing the MFT
higher on the disk.

*/
void JKDefragLib::MoveMftToBeginOfDisk(struct DefragDataStruct *Data)
{
	struct ItemStruct *Item;

	ULONG64 Lcn;
	ULONG64 GapBegin;
	ULONG64 GapEnd;
	ULONG64 Clusters;
	ULONG64 ClustersDone;

	int Result;

	OSVERSIONINFO OsVersion;

	JKDefragGui *jkGui = JKDefragGui::getInstance();

	jkGui->ShowDebug(2,NULL,L"Moving the MFT to the beginning of the volume.");

	/* Exit if this is not an NTFS disk. */
	if (Data->Disk.Type != NTFS)
	{
		jkGui->ShowDebug(5,NULL,L"Cannot move the MFT because this is not an NTFS disk.");

		return;
	}

	/* The Microsoft defragmentation api only supports moving the MFT on Vista. */
	ZeroMemory(&OsVersion,sizeof(OSVERSIONINFO));

	OsVersion.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

	if ((GetVersionEx(&OsVersion) != 0) && (OsVersion.dwMajorVersion < 6))
	{
		jkGui->ShowDebug(5,NULL,L"Cannot move the MFT because it is not supported by this version of Windows.");

		return;
	}

	/* Locate the Item for the MFT. If not found then exit. */
	for (Item = TreeSmallest(Data->ItemTree); Item != NULL; Item = TreeNext(Item))
	{
		if (MatchMask(Item->LongPath,L"?:\\$MFT") == YES) break;
	}

	if (Item == NULL)
	{
		jkGui->ShowDebug(5,NULL,L"Cannot move the MFT because I cannot find it.");

		return;
	}

	/* Exit if the MFT is at the beginning of the volume (inside zone 0) and is not
	fragmented. */
#ifdef jk
	if ((Item->Fragments != NULL) &&
		(Item->Fragments->NextVcn == Data->Disk.MftLockedClusters) &&
		(Item->Fragments->Next != NULL) &&
		(Item->Fragments->Next->Lcn < Data->Zones[1]) &&
		(IsFragmented(Item,Data->Disk.MftLockedClusters,Item->Clusters - Data->Disk.MftLockedClusters) == NO)) {
			m_jkGui->ShowDebug(5,NULL,L"No need to move the MFT because it's already at the beginning of the volume and it's data part is not fragmented.");
			return;
	}
#endif

	Lcn = 0;
	GapBegin = 0;
	GapEnd = 0;
	ClustersDone = Data->Disk.MftLockedClusters;

	while ((*Data->Running == RUNNING) && (ClustersDone < Item->Clusters))
	{
		if (ClustersDone > Data->Disk.MftLockedClusters)
		{
			jkGui->ShowDebug(5,NULL,L"Partially placed, %I64u clusters more to do",
				Item->Clusters - ClustersDone);
		}

		/* Call the Vacate() function to make a gap at Lcn big enough to hold the MFT.
		The Vacate() function may not be able to move whatever is now at the Lcn, so
		after calling it we have to locate the first gap after the Lcn. */
		if (GapBegin + Item->Clusters - ClustersDone + 16 > GapEnd)
		{
			Vacate(Data,Lcn,Item->Clusters - ClustersDone,TRUE);

			Result = FindGap(Data,Lcn,0,0,YES,NO,&GapBegin,&GapEnd,TRUE);

			if (Result == NO) return;              /* No gaps found, exit. */
		}

		/* If the gap is not big enough to hold the entire MFT then calculate how much
		will fit in the gap. */
		Clusters = Item->Clusters - ClustersDone;

		if (Clusters > GapEnd - GapBegin)
		{
			Clusters = GapEnd - GapBegin;
			/* It looks like a partial move only succeeds if the number of clusters is a
			multiple of 8. */
			Clusters = Clusters - (Clusters % 8);

			if (Clusters == 0)
			{
				Lcn = GapEnd;

				continue;
			}
		}

		/* Move the MFT to the gap. */
		Result = MoveItem(Data,Item,GapBegin,ClustersDone,Clusters,0);

		if (Result == YES)
		{
			GapBegin = GapBegin + Clusters;
		}
		else
		{
			Result = FindGap(Data,GapBegin,0,0,YES,NO,&GapBegin,&GapEnd,TRUE);

			if (Result == NO) return;              /* No gaps found, exit. */
		}

		Lcn = GapBegin;
		ClustersDone = ClustersDone + Clusters;
	}

	/* Make the MFT unmovable. We don't want it moved again by any other subroutine. */
	Item->Unmovable = YES;

	ColorizeItem(Data,Item,0,0,NO);
	CalculateZones(Data);

	/* Note: The MftExcludes do not change by moving the MFT. */
}

/* Optimize the harddisk by filling gaps with files from above. */
void JKDefragLib::OptimizeVolume(struct DefragDataStruct *Data)
{
	int Zone;

	struct ItemStruct *Item;

	ULONG64 GapBegin;
	ULONG64 GapEnd;

	int Result;
	int Retry;
	int PerfectFit;

	ULONG64 PhaseTemp;

	int FileZone;

	JKDefragGui *jkGui = JKDefragGui::getInstance();

	/* Sanity check. */
	if (Data->ItemTree == NULL) return;

	/* Process all the zones. */
	for (Zone = 0; Zone < 3; Zone++)
	{
		CallShowStatus(Data,5,Zone);            /* "Zone N: Fast Optimize" */

		/* Walk through all the gaps. */
		GapBegin = Data->Zones[Zone];
		Retry = 0;

		while (*Data->Running == RUNNING)
		{
			/* Find the next gap. */
			Result = FindGap(Data,GapBegin,0,0,YES,NO,&GapBegin,&GapEnd,FALSE);

			if (Result == NO) break;

			/* Update the progress counter: the number of clusters in all the files
			above the gap. Exit if there are no more files. */
			PhaseTemp = 0;

			for (Item = TreeBiggest(Data->ItemTree); Item != NULL; Item = TreePrev(Item))
			{
				if (GetItemLcn(Item) < GapEnd) break;
				if (Item->Unmovable == YES) continue;
				if (Item->Exclude == YES) continue;

				FileZone = 1;

				if (Item->SpaceHog == YES) FileZone = 2;
				if (Item->Directory == YES) FileZone = 0;
				if (FileZone != Zone) continue;

				PhaseTemp = PhaseTemp + Item->Clusters;
			}

			Data->PhaseTodo = Data->PhaseDone + PhaseTemp;
			if (PhaseTemp == 0) break;

			/* Loop until the gap is filled. First look for combinations of files that perfectly
			fill the gap. If no combination can be found, or if there are less files than
			the gap is big, then fill with the highest file(s) that fit in the gap. */
			PerfectFit = YES;
			if (GapEnd - GapBegin > PhaseTemp) PerfectFit = NO;

			while ((GapBegin < GapEnd) && (Retry < 5) && (*Data->Running == RUNNING))
			{
				/* Find the Item that is the best fit for the gap. If nothing found (no files
				fit the gap) then exit the loop. */
				if (PerfectFit == YES)
				{
					Item = FindBestItem(Data,GapBegin,GapEnd,1,Zone);

					if (Item == NULL)
					{
						PerfectFit = NO;

						Item = FindHighestItem(Data,GapBegin,GapEnd,1,Zone);
					}
				}
				else
				{
					Item = FindHighestItem(Data,GapBegin,GapEnd,1,Zone);
				}

				if (Item == NULL) break;

				/* Move the item. */
				Result = MoveItem(Data,Item,GapBegin,0,Item->Clusters,0);

				if (Result == YES)
				{
					GapBegin = GapBegin + Item->Clusters;
					Retry = 0;
				}
				else
				{
					GapEnd = GapBegin;   /* Force re-scan of gap. */
					Retry = Retry + 1;
				}
			}

			/* If the gap could not be filled then skip. */
			if (GapBegin < GapEnd)
			{
				/* Show debug message: "Skipping gap, cannot fill: %I64d[%I64d]" */
				jkGui->ShowDebug(5,NULL,Data->DebugMsg[28],GapBegin,GapEnd - GapBegin);

				GapBegin = GapEnd;
				Retry = 0;
			}
		}
	}
}

/* Optimize the harddisk by moving the selected items up. */
void JKDefragLib::OptimizeUp(struct DefragDataStruct *Data)
{
	struct ItemStruct *Item;

	ULONG64 GapBegin;
	ULONG64 GapEnd;

	int Result;
	int Retry;
	int PerfectFit;

	ULONG64 PhaseTemp;

	JKDefragGui *jkGui = JKDefragGui::getInstance();

	CallShowStatus(Data,6,-1);            /* "Phase 3: Move Up" */

	/* Setup the progress counter: the total number of clusters in all files. */
	for (Item = TreeSmallest(Data->ItemTree); Item != NULL; Item = TreeNext(Item))
	{
		Data->PhaseTodo = Data->PhaseTodo + Item->Clusters;
	}

	/* Exit if nothing to do. */
	if (Data->ItemTree == NULL) return;

	/* Walk through all the gaps. */
	GapEnd = Data->TotalClusters;
	Retry = 0;

	while (*Data->Running == RUNNING)
	{
		/* Find the previous gap. */
		Result = FindGap(Data,Data->Zones[1],GapEnd,0,YES,YES,&GapBegin,&GapEnd,FALSE);

		if (Result == NO) break;

		/* Update the progress counter: the number of clusters in all the files
		below the gap. */
		PhaseTemp = 0;

		for (Item = TreeSmallest(Data->ItemTree); Item != NULL; Item = TreeNext(Item))
		{
			if (Item->Unmovable == YES) continue;
			if (Item->Exclude == YES) continue;
			if (GetItemLcn(Item) >= GapEnd) break;

			PhaseTemp = PhaseTemp + Item->Clusters;
		}

		Data->PhaseTodo = Data->PhaseDone + PhaseTemp;
		if (PhaseTemp == 0) break;

		/* Loop until the gap is filled. First look for combinations of files that perfectly
		fill the gap. If no combination can be found, or if there are less files than
		the gap is big, then fill with the highest file(s) that fit in the gap. */
		PerfectFit = YES;
		if (GapEnd - GapBegin > PhaseTemp) PerfectFit = NO;

		while ((GapBegin < GapEnd) && (Retry < 5) && (*Data->Running == RUNNING))
		{
			/* Find the Item that is the best fit for the gap. If nothing found (no files
			fit the gap) then exit the loop. */
			if (PerfectFit == YES)
			{
				Item = FindBestItem(Data,GapBegin,GapEnd,0,3);

				if (Item == NULL)
				{
					PerfectFit = NO;
					Item = FindHighestItem(Data,GapBegin,GapEnd,0,3);
				}
			}
			else
			{
				Item = FindHighestItem(Data,GapBegin,GapEnd,0,3);
			}

			if (Item == NULL) break;

			/* Move the item. */
			Result = MoveItem(Data,Item,GapEnd - Item->Clusters,0,Item->Clusters,1);

			if (Result == YES)
			{
				GapEnd = GapEnd - Item->Clusters;
				Retry = 0;
			}
			else
			{
				GapBegin = GapEnd;   /* Force re-scan of gap. */
				Retry = Retry + 1;
			}
		}

		/* If the gap could not be filled then skip. */
		if (GapBegin < GapEnd)
		{
			/* Show debug message: "Skipping gap, cannot fill: %I64d[%I64d]" */
			jkGui->ShowDebug(5,NULL,Data->DebugMsg[28],GapBegin,GapEnd - GapBegin);

			GapEnd = GapBegin;
			Retry = 0;
		}
	}
}

/* Run the defragmenter. Input is the name of a disk, mountpoint, directory, or file,
and may contain wildcards '*' and '?'. */
void JKDefragLib::DefragOnePath(struct DefragDataStruct *Data, WCHAR *Path, int Mode)
{
	HANDLE ProcessTokenHandle;

	LUID TakeOwnershipValue;

	TOKEN_PRIVILEGES TokenPrivileges;

	STARTING_LCN_INPUT_BUFFER BitmapParam;

	struct
	{
		ULONG64 StartingLcn;
		ULONG64 BitmapSize;

		BYTE Buffer[8];
	} BitmapData;

	NTFS_VOLUME_DATA_BUFFER NtfsData;

	ULONG64 FreeBytesToCaller;
	ULONG64 TotalBytes;
	ULONG64 FreeBytes;

	int Result;

	DWORD ErrorCode;

	size_t Length;

	struct __timeb64 Time;

	FILE *Fin;

	WCHAR s1[BUFSIZ];
	WCHAR *p1;

	DWORD w;

	int i;

	JKDefragGui *jkGui = JKDefragGui::getInstance();

	/* Initialize the data. Some items are inherited from the caller and are not
	initialized. */
	Data->Phase = 0;
	Data->Disk.VolumeHandle = NULL;
	Data->Disk.MountPoint = NULL;
	Data->Disk.MountPointSlash = NULL;
	Data->Disk.VolumeName[0] = 0;
	Data->Disk.VolumeNameSlash[0] = 0;
	Data->Disk.Type = UnknownType;
	Data->ItemTree = NULL;
	Data->BalanceCount = 0;
	Data->MftExcludes[0].Start = 0;
	Data->MftExcludes[0].End = 0;
	Data->MftExcludes[1].Start = 0;
	Data->MftExcludes[1].End = 0;
	Data->MftExcludes[2].Start = 0;
	Data->MftExcludes[2].End = 0;
	Data->TotalClusters = 0;
	Data->BytesPerCluster = 0;

	for (i = 0; i < 3; i++) Data->Zones[i] = 0;

	Data->CannotMoveDirs = 0;
	Data->CountDirectories = 0;
	Data->CountAllFiles = 0;
	Data->CountFragmentedItems = 0;
	Data->CountAllBytes = 0;
	Data->CountFragmentedBytes = 0;
	Data->CountAllClusters = 0;
	Data->CountFragmentedClusters = 0;
	Data->CountFreeClusters = 0;
	Data->CountGaps = 0;
	Data->BiggestGap = 0;
	Data->CountGapsLess16 = 0;
	Data->CountClustersLess16 = 0;
	Data->PhaseTodo = 0;
	Data->PhaseDone = 0;

	_ftime64_s(&Time);

	Data->StartTime = Time.time * 1000 + Time.millitm;
	Data->LastCheckpoint = Data->StartTime;
	Data->RunningTime = 0;

	/* Compare the item with the Exclude masks. If a mask matches then return,
	ignoring the item. */
	if (Data->Excludes != NULL)
	{
		for (i = 0; Data->Excludes[i] != NULL; i++)
		{
			if (MatchMask(Path,Data->Excludes[i]) == YES) break;
			if ((wcschr(Data->Excludes[i],L'*') == NULL) &&
				(wcslen(Data->Excludes[i]) <= 3) &&
				(LowerCase(Path[0]) == LowerCase(Data->Excludes[i][0]))) break;
		}

		if (Data->Excludes[i] != NULL)
		{
			/* Show debug message: "Ignoring volume '%s' because of exclude mask '%s'." */
			jkGui->ShowDebug(0,NULL,Data->DebugMsg[47],Path,Data->Excludes[i]);
			return;
		}
	}

	/* Clear the screen and show "Processing '%s'" message. */
	jkGui->ClearScreen(Data->DebugMsg[14],Path);

	/* Try to change our permissions so we can access special files and directories
	such as "C:\System Volume Information". If this does not succeed then quietly
	continue, we'll just have to do with whatever permissions we have.
	SE_BACKUP_NAME = Backup and Restore Privileges.
	*/
	if ((OpenProcessToken(GetCurrentProcess(),TOKEN_ADJUST_PRIVILEGES|TOKEN_QUERY,
		&ProcessTokenHandle) != 0) &&
		(LookupPrivilegeValue(0,SE_BACKUP_NAME,&TakeOwnershipValue) != 0))
	{
		TokenPrivileges.PrivilegeCount = 1;
		TokenPrivileges.Privileges[0].Luid = TakeOwnershipValue;
		TokenPrivileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

		if (AdjustTokenPrivileges(ProcessTokenHandle,FALSE,&TokenPrivileges,
				sizeof(TOKEN_PRIVILEGES),0,0) == FALSE)
		{
			jkGui->ShowDebug(3,NULL,L"Info: could not elevate to SeBackupPrivilege.");
		}
	}
	else
	{
		jkGui->ShowDebug(3,NULL,L"Info: could not elevate to SeBackupPrivilege.");
	}

	/* Try finding the MountPoint by treating the input path as a path to
	something on the disk. If this does not succeed then use the Path as
	a literal MountPoint name. */
	Data->Disk.MountPoint = _wcsdup(Path);
	if (Data->Disk.MountPoint == NULL) return;

	Result = GetVolumePathNameW(Path,Data->Disk.MountPoint,(DWORD)wcslen(Data->Disk.MountPoint)+1);

	if (Result == 0) wcscpy_s(Data->Disk.MountPoint,wcslen(Path)+1,Path);

	/* Make two versions of the MountPoint, one with a trailing backslash and one without. */
	p1 = wcschr(Data->Disk.MountPoint,0);

	if (p1 != Data->Disk.MountPoint)
	{
		p1--;
		if (*p1 == '\\') *p1 = 0;
	}

	Length = wcslen(Data->Disk.MountPoint) + 2;

	Data->Disk.MountPointSlash = (WCHAR *)malloc(sizeof(WCHAR) * Length);

	if (Data->Disk.MountPointSlash == NULL)
	{
		free(Data->Disk.MountPoint);
		return;
	}

	swprintf_s(Data->Disk.MountPointSlash,Length,L"%s\\",Data->Disk.MountPoint);

	/* Determine the name of the volume (something like
	"\\?\Volume{08439462-3004-11da-bbca-806d6172696f}\"). */
	Result = GetVolumeNameForVolumeMountPointW(Data->Disk.MountPointSlash,
		Data->Disk.VolumeNameSlash,MAX_PATH);

	if (Result == 0)
	{
		if (wcslen(Data->Disk.MountPointSlash) > 52 - 1 - 4)
		{
			/* "Cannot find volume name for mountpoint '%s': %s" */
			SystemErrorStr(GetLastError(),s1,BUFSIZ);

			jkGui->ShowDebug(0,NULL,Data->DebugMsg[40],Data->Disk.MountPointSlash,s1);

			free(Data->Disk.MountPoint);
			free(Data->Disk.MountPointSlash);

			return;
		}

		swprintf_s(Data->Disk.VolumeNameSlash,52,L"\\\\.\\%s",Data->Disk.MountPointSlash);
	}

	/* Make a copy of the VolumeName without the trailing backslash. */
	wcscpy_s(Data->Disk.VolumeName,51,Data->Disk.VolumeNameSlash);

	p1 = wcschr(Data->Disk.VolumeName,0);

	if (p1 != Data->Disk.VolumeName)
	{
		p1--;
		if (*p1 == '\\') *p1 = 0;
	}

	/* Exit if the disk is hybernated (if "?/hiberfil.sys" exists and does not begin
	with 4 zero bytes). */
	Length = wcslen(Data->Disk.MountPointSlash) + 14;

	p1 = (WCHAR *)malloc(sizeof(WCHAR) * Length);

	if (p1 == NULL)
	{
		free(Data->Disk.MountPointSlash);
		free(Data->Disk.MountPoint);

		return;
	}

	swprintf_s(p1,Length,L"%s\\hiberfil.sys",Data->Disk.MountPointSlash);

	Result = _wfopen_s(&Fin,p1,L"rb");

	if ((Result == 0) && (Fin != NULL))
	{
		w = 0;

		if ((fread(&w,4,1,Fin) == 1) && (w != 0))
		{
			jkGui->ShowDebug(0,NULL,L"Will not process this disk, it contains hybernated data.");

			free(Data->Disk.MountPoint);
			free(Data->Disk.MountPointSlash);
			free(p1);

			return;
		}
	}

	free(p1);

	/* Show debug message: "Opening volume '%s' at mountpoint '%s'" */
	jkGui->ShowDebug(0,NULL,Data->DebugMsg[29],Data->Disk.VolumeName,Data->Disk.MountPoint);

	/* Open the VolumeHandle. If error then leave. */
	Data->Disk.VolumeHandle = CreateFileW(Data->Disk.VolumeName,GENERIC_READ,
		FILE_SHARE_READ | FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL);

	if (Data->Disk.VolumeHandle == INVALID_HANDLE_VALUE)
	{
		SystemErrorStr(GetLastError(),s1,BUFSIZ);

		jkGui->ShowDebug(1,NULL,L"Cannot open volume '%s' at mountpoint '%s': %s",
			Data->Disk.VolumeName,Data->Disk.MountPoint,s1);

		free(Data->Disk.MountPoint);
		free(Data->Disk.MountPointSlash);

		return;
	}

	/* Determine the maximum LCN (maximum cluster number). A single call to
	FSCTL_GET_VOLUME_BITMAP is enough, we don't have to walk through the
	entire bitmap.
	It's a pity we have to do it in this roundabout manner, because
	there is no system call that reports the total number of clusters
	in a volume. GetDiskFreeSpace() does, but is limited to 2Gb volumes,
	GetDiskFreeSpaceEx() reports in bytes, not clusters, _getdiskfree()
	requires a drive letter so cannot be used on unmounted volumes or
	volumes that are mounted on a directory, and FSCTL_GET_NTFS_VOLUME_DATA
	only works for NTFS volumes. */
	BitmapParam.StartingLcn.QuadPart = 0;

//	long koko = FSCTL_GET_VOLUME_BITMAP;

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

	if ((ErrorCode != NO_ERROR) && (ErrorCode != ERROR_MORE_DATA))
	{
		/* Show debug message: "Cannot defragment volume '%s' at mountpoint '%s'" */
		jkGui->ShowDebug(0,NULL,Data->DebugMsg[32],Data->Disk.VolumeName,Data->Disk.MountPoint);

		CloseHandle(Data->Disk.VolumeHandle);

		free(Data->Disk.MountPoint);
		free(Data->Disk.MountPointSlash);

		return;
	}

	Data->TotalClusters = BitmapData.StartingLcn + BitmapData.BitmapSize;

	/* Determine the number of bytes per cluster.
	Again I have to do this in a roundabout manner. As far as I know there is
	no system call that returns the number of bytes per cluster, so first I have
	to get the total size of the disk and then divide by the number of clusters.
	*/
	ErrorCode = GetDiskFreeSpaceExW(Path,(PULARGE_INTEGER)&FreeBytesToCaller,
		(PULARGE_INTEGER)&TotalBytes,(PULARGE_INTEGER)&FreeBytes);

	if (ErrorCode != 0) Data->BytesPerCluster = TotalBytes / Data->TotalClusters;

	/* Setup the list of clusters that cannot be used. The Master File
	Table cannot be moved and cannot be used by files. All this is
	only necessary for NTFS volumes. */
	ErrorCode = DeviceIoControl(Data->Disk.VolumeHandle,FSCTL_GET_NTFS_VOLUME_DATA,
		NULL,0,&NtfsData,sizeof(NtfsData),&w,NULL);

	if (ErrorCode != 0)
	{
		/* Note: NtfsData.TotalClusters.QuadPart should be exactly the same
		as the Data->TotalClusters that was determined in the previous block. */

		Data->BytesPerCluster = NtfsData.BytesPerCluster;

		Data->MftExcludes[0].Start = NtfsData.MftStartLcn.QuadPart;
		Data->MftExcludes[0].End = NtfsData.MftStartLcn.QuadPart +
			NtfsData.MftValidDataLength.QuadPart / NtfsData.BytesPerCluster;
		Data->MftExcludes[1].Start = NtfsData.MftZoneStart.QuadPart;
		Data->MftExcludes[1].End = NtfsData.MftZoneEnd.QuadPart;
		Data->MftExcludes[2].Start = NtfsData.Mft2StartLcn.QuadPart;
		Data->MftExcludes[2].End = NtfsData.Mft2StartLcn.QuadPart +
			NtfsData.MftValidDataLength.QuadPart / NtfsData.BytesPerCluster;

		/* Show debug message: "MftStartLcn=%I64d, MftZoneStart=%I64d, MftZoneEnd=%I64d, Mft2StartLcn=%I64d, MftValidDataLength=%I64d" */
		jkGui->ShowDebug(3,NULL,Data->DebugMsg[33],
			NtfsData.MftStartLcn.QuadPart,NtfsData.MftZoneStart.QuadPart,
			NtfsData.MftZoneEnd.QuadPart,NtfsData.Mft2StartLcn.QuadPart,
			NtfsData.MftValidDataLength.QuadPart / NtfsData.BytesPerCluster);

		/* Show debug message: "MftExcludes[%u].Start=%I64d, MftExcludes[%u].End=%I64d" */
		jkGui->ShowDebug(3,NULL,Data->DebugMsg[34],0,Data->MftExcludes[0].Start,0,Data->MftExcludes[0].End);
		jkGui->ShowDebug(3,NULL,Data->DebugMsg[34],1,Data->MftExcludes[1].Start,1,Data->MftExcludes[1].End);
		jkGui->ShowDebug(3,NULL,Data->DebugMsg[34],2,Data->MftExcludes[2].Start,2,Data->MftExcludes[2].End);
	}

	/* Fixup the input mask.
	- If the length is 2 or 3 characters then rewrite into "c:\*".
	- If it does not contain a wildcard then append '*'.
	*/
	Length = wcslen(Path) + 3;

	Data->IncludeMask = (WCHAR *)malloc(sizeof(WCHAR) * Length);

	if (Data->IncludeMask == NULL) return;

	wcscpy_s(Data->IncludeMask,Length,Path);

	if ((wcslen(Path) == 2) || (wcslen(Path) == 3))
	{
		swprintf_s(Data->IncludeMask,Length,L"%c:\\*",LowerCase(Path[0]));
	}
	else if (wcschr(Path,L'*') == NULL)
	{
		swprintf_s(Data->IncludeMask,Length,L"%s*",Path);
	}

	jkGui->ShowDebug(0,NULL,L"Input mask: %s",Data->IncludeMask);

	/* Defragment and optimize. */
	jkGui->ShowDiskmap(Data);

	if (*Data->Running == RUNNING) AnalyzeVolume(Data);

	if ((*Data->Running == RUNNING) && (Mode == 1))
	{
		Defragment(Data);
	}

	if ((*Data->Running == RUNNING) && ((Mode == 2) || (Mode == 3)))
	{
		Defragment(Data);

		if (*Data->Running == RUNNING) Fixup(Data);
		if (*Data->Running == RUNNING) OptimizeVolume(Data);
		if (*Data->Running == RUNNING) Fixup(Data);     /* Again, in case of new zone startpoint. */
	}

	if ((*Data->Running == RUNNING) && (Mode == 4))
	{
		ForcedFill(Data);
	}

	if ((*Data->Running == RUNNING) && (Mode == 5))
	{
		OptimizeUp(Data);
	}

	if ((*Data->Running == RUNNING) && (Mode == 6))
	{
		OptimizeSort(Data,0);                        /* Filename */
	}

	if ((*Data->Running == RUNNING) && (Mode == 7))
	{
		OptimizeSort(Data,1);                        /* Filesize */
	}

	if ((*Data->Running == RUNNING) && (Mode == 8))
	{
		OptimizeSort(Data,2);                     /* Last access */
	}

	if ((*Data->Running == RUNNING) && (Mode == 9))
	{
		OptimizeSort(Data,3);                     /* Last change */
	}

	if ((*Data->Running == RUNNING) && (Mode == 10))
	{
		OptimizeSort(Data,4);                        /* Creation */
	}
	/*
	if ((*Data->Running == RUNNING) && (Mode == 11)) {
	MoveMftToBeginOfDisk(Data);
	}
	*/

	CallShowStatus(Data,7,-1);                     /* "Finished." */

	/* Close the volume handles. */
	if ((Data->Disk.VolumeHandle != NULL) &&
		(Data->Disk.VolumeHandle != INVALID_HANDLE_VALUE))
	{
		CloseHandle(Data->Disk.VolumeHandle);
	}

	/* Cleanup. */
	DeleteItemTree(Data->ItemTree);

	if (Data->Disk.MountPoint != NULL) free(Data->Disk.MountPoint);
	if (Data->Disk.MountPointSlash != NULL) free(Data->Disk.MountPointSlash);
}

/* Subfunction for DefragAllDisks(). It will ignore removable disks, and
will iterate for disks that are mounted on a subdirectory of another
disk (instead of being mounted on a drive). */
void JKDefragLib::DefragMountpoints(struct DefragDataStruct *Data, WCHAR *MountPoint, int Mode)
{
	WCHAR VolumeNameSlash[BUFSIZ];
	WCHAR VolumeName[BUFSIZ];

	int DriveType;

	DWORD FileSystemFlags;

	HANDLE FindMountpointHandle;

	WCHAR RootPath[MAX_PATH + BUFSIZ];
	WCHAR *FullRootPath;

	HANDLE VolumeHandle;

	int Result;

	size_t Length;

	DWORD ErrorCode;

	WCHAR s1[BUFSIZ];
	WCHAR *p1;

	DWORD w;

	JKDefragGui *jkGui = JKDefragGui::getInstance();

	if (*Data->Running != RUNNING) return;

	/* Clear the screen and show message "Analyzing volume '%s'" */
	jkGui->ClearScreen(Data->DebugMsg[37],MountPoint);

	/* Return if this is not a fixed disk. */
	DriveType = GetDriveTypeW(MountPoint);

	if (DriveType != DRIVE_FIXED)
	{
		if (DriveType == DRIVE_UNKNOWN)
		{
			jkGui->ClearScreen(L"Ignoring volume '%s' because the drive type cannot be determined.",MountPoint);
		}

		if (DriveType == DRIVE_NO_ROOT_DIR)
		{
			jkGui->ClearScreen(L"Ignoring volume '%s' because there is no volume mounted.",MountPoint);
		}

		if (DriveType == DRIVE_REMOVABLE)
		{
			jkGui->ClearScreen(L"Ignoring volume '%s' because it has removable media.",MountPoint);
		}

		if (DriveType == DRIVE_REMOTE)
		{
			jkGui->ClearScreen(L"Ignoring volume '%s' because it is a remote (network) drive.",MountPoint);
		}

		if (DriveType == DRIVE_CDROM)
		{
			jkGui->ClearScreen(L"Ignoring volume '%s' because it is a CD-ROM drive.",MountPoint);
		}

		if (DriveType == DRIVE_RAMDISK)
		{
			jkGui->ClearScreen(L"Ignoring volume '%s' because it is a RAM disk.",MountPoint);
		}

		return;
	}

	/* Determine the name of the volume, something like
	"\\?\Volume{08439462-3004-11da-bbca-806d6172696f}\". */
	Result = GetVolumeNameForVolumeMountPointW(MountPoint,VolumeNameSlash,BUFSIZ);

	if (Result == 0)
	{
		ErrorCode = GetLastError();

		if (ErrorCode == 3)
		{
			/* "Ignoring volume '%s' because it is not a harddisk." */
			jkGui->ShowDebug(0,NULL,Data->DebugMsg[57],MountPoint);
		}
		else
		{
			/* "Cannot find volume name for mountpoint: %s" */
			SystemErrorStr(ErrorCode,s1,BUFSIZ);

			jkGui->ShowDebug(0,NULL,Data->DebugMsg[40],MountPoint,s1);
		}

		return;
	}

	/* Return if the disk is read-only. */
	GetVolumeInformationW(VolumeNameSlash,NULL,0,NULL,NULL,&FileSystemFlags,NULL,0);

	if ((FileSystemFlags & FILE_READ_ONLY_VOLUME) != 0)
	{
		/* Clear the screen and show message "Ignoring disk '%s' because it is read-only." */
		jkGui->ClearScreen(Data->DebugMsg[36],MountPoint);

		return;
	}

	/* If the volume is not mounted then leave. Unmounted volumes can be
	defragmented, but the system administrator probably has unmounted
	the volume because he wants it untouched. */
	wcscpy_s(VolumeName,BUFSIZ,VolumeNameSlash);

	p1 = wcschr(VolumeName,0);

	if (p1 != VolumeName)
	{
		p1--;
		if (*p1 == '\\') *p1 = 0;
	}

	VolumeHandle = CreateFileW(VolumeName,GENERIC_READ,
		FILE_SHARE_READ | FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL);

	if (VolumeHandle == INVALID_HANDLE_VALUE)
	{
		SystemErrorStr(GetLastError(),s1,BUFSIZ);

		jkGui->ShowDebug(1,NULL,L"Cannot open volume '%s' at mountpoint '%s': %s",
			VolumeName,MountPoint,s1);

		return;
	}

	if (DeviceIoControl(VolumeHandle,FSCTL_IS_VOLUME_MOUNTED,NULL,0,NULL,0,&w,NULL) == 0)
	{
		/* Show debug message: "Volume '%s' at mountpoint '%s' is not mounted." */
		jkGui->ShowDebug(0,NULL,Data->DebugMsg[31],VolumeName,MountPoint);

		CloseHandle(VolumeHandle);

		return;
	}

	CloseHandle(VolumeHandle);

	/* Defrag the disk. */
	Length = wcslen(MountPoint) + 2;

	p1 = (WCHAR *)malloc(sizeof(WCHAR) * Length);

	if (p1 != NULL)
	{
		swprintf_s(p1,Length,L"%s*",MountPoint);

		DefragOnePath(Data,p1,Mode);

		free(p1);
	}

	/* According to Microsoft I should check here if the disk has support for
	reparse points:
	if ((FileSystemFlags & FILE_SUPPORTS_REPARSE_POINTS) == 0) return;
	However, I have found this test will frequently cause a false return
	on Windows 2000. So I've removed it, everything seems to be working
	nicely without it. */

	/* Iterate for all the mountpoints on the disk. */
	FindMountpointHandle = FindFirstVolumeMountPointW(VolumeNameSlash,RootPath,MAX_PATH + BUFSIZ);

	if (FindMountpointHandle == INVALID_HANDLE_VALUE) return;

	do
	{
		Length = wcslen(MountPoint) + wcslen(RootPath) + 1;
		FullRootPath = (WCHAR *)malloc(sizeof(WCHAR) * Length);

		if (FullRootPath != NULL)
		{
			swprintf_s(FullRootPath,Length,L"%s%s",MountPoint,RootPath);

			DefragMountpoints(Data,FullRootPath,Mode);

			free(FullRootPath);
		}
	} while (FindNextVolumeMountPointW(FindMountpointHandle,RootPath,MAX_PATH + BUFSIZ) != 0);

	FindVolumeMountPointClose(FindMountpointHandle);
}

/* Run the defragger/optimizer. See the .h file for a full explanation. */
void JKDefragLib::RunJkDefrag(
				WCHAR *Path,
				int Mode,
				int Speed,
				double FreeSpace,
				WCHAR **Excludes,
				WCHAR **SpaceHogs,
				int *Running,
				WCHAR **DebugMsg)
{
	struct DefragDataStruct Data;

	DWORD DrivesSize;

	WCHAR *Drives;
	WCHAR *Drive;

	int DefaultRunning;
//	int DefaultRedrawScreen;

	DWORD NtfsDisableLastAccessUpdate;

	LONG Result;

	HKEY Key;

	DWORD KeyDisposition;
	DWORD Length;

	WCHAR s1[BUFSIZ];

	int i;

	JKDefragGui *jkGui = JKDefragGui::getInstance();

	/* Copy the input values to the data struct. */
	Data.Speed = Speed;
	Data.FreeSpace = FreeSpace;
	Data.Excludes = Excludes;

	if (Running == NULL)
	{
		Data.Running = &DefaultRunning;
	}
	else 
	{
		Data.Running = Running;
	}

	*Data.Running = RUNNING;

/*
	if (RedrawScreen == NULL) {
		Data.RedrawScreen = &DefaultRedrawScreen;
	} else {
		Data.RedrawScreen = RedrawScreen;
	}
	*Data.RedrawScreen = 0;
*/

	if ((DebugMsg == NULL) || (DebugMsg[0] == NULL))
	{
		Data.DebugMsg = DefaultDebugMsg;
	}
	else
	{
		Data.DebugMsg = DebugMsg;
	}

	/* Make a copy of the SpaceHogs array. */
	Data.SpaceHogs = NULL;
	Data.UseDefaultSpaceHogs = TRUE;

	if (SpaceHogs != NULL)
	{
		for (i = 0; SpaceHogs[i] != NULL; i++)
		{
			if (_wcsicmp(SpaceHogs[i],L"DisableDefaults") == 0)
			{
				Data.UseDefaultSpaceHogs = FALSE;
			}
			else
			{
				Data.SpaceHogs = AddArrayString(Data.SpaceHogs,SpaceHogs[i]);
			}
		}
	}

	if (Data.UseDefaultSpaceHogs == TRUE)
	{
		Data.SpaceHogs = AddArrayString(Data.SpaceHogs,L"?:\\$RECYCLE.BIN\\*");      /* Vista */
		Data.SpaceHogs = AddArrayString(Data.SpaceHogs,L"?:\\RECYCLED\\*");          /* FAT on 2K/XP */
		Data.SpaceHogs = AddArrayString(Data.SpaceHogs,L"?:\\RECYCLER\\*");          /* NTFS on 2K/XP */
		Data.SpaceHogs = AddArrayString(Data.SpaceHogs,L"?:\\WINDOWS\\$*");
		Data.SpaceHogs = AddArrayString(Data.SpaceHogs,L"?:\\WINDOWS\\Downloaded Installations\\*");
		Data.SpaceHogs = AddArrayString(Data.SpaceHogs,L"?:\\WINDOWS\\Ehome\\*");
		Data.SpaceHogs = AddArrayString(Data.SpaceHogs,L"?:\\WINDOWS\\Fonts\\*");
		Data.SpaceHogs = AddArrayString(Data.SpaceHogs,L"?:\\WINDOWS\\Help\\*");
		Data.SpaceHogs = AddArrayString(Data.SpaceHogs,L"?:\\WINDOWS\\I386\\*");
		Data.SpaceHogs = AddArrayString(Data.SpaceHogs,L"?:\\WINDOWS\\IME\\*");
		Data.SpaceHogs = AddArrayString(Data.SpaceHogs,L"?:\\WINDOWS\\Installer\\*");
		Data.SpaceHogs = AddArrayString(Data.SpaceHogs,L"?:\\WINDOWS\\ServicePackFiles\\*");
		Data.SpaceHogs = AddArrayString(Data.SpaceHogs,L"?:\\WINDOWS\\SoftwareDistribution\\*");
		Data.SpaceHogs = AddArrayString(Data.SpaceHogs,L"?:\\WINDOWS\\Speech\\*");
		Data.SpaceHogs = AddArrayString(Data.SpaceHogs,L"?:\\WINDOWS\\Symbols\\*");
		Data.SpaceHogs = AddArrayString(Data.SpaceHogs,L"?:\\WINDOWS\\ie7updates\\*");
		Data.SpaceHogs = AddArrayString(Data.SpaceHogs,L"?:\\WINDOWS\\system32\\dllcache\\*");
		Data.SpaceHogs = AddArrayString(Data.SpaceHogs,L"?:\\WINNT\\$*");
		Data.SpaceHogs = AddArrayString(Data.SpaceHogs,L"?:\\WINNT\\Downloaded Installations\\*");
		Data.SpaceHogs = AddArrayString(Data.SpaceHogs,L"?:\\WINNT\\I386\\*");
		Data.SpaceHogs = AddArrayString(Data.SpaceHogs,L"?:\\WINNT\\Installer\\*");
		Data.SpaceHogs = AddArrayString(Data.SpaceHogs,L"?:\\WINNT\\ServicePackFiles\\*");
		Data.SpaceHogs = AddArrayString(Data.SpaceHogs,L"?:\\WINNT\\SoftwareDistribution\\*");
		Data.SpaceHogs = AddArrayString(Data.SpaceHogs,L"?:\\WINNT\\ie7updates\\*");
		Data.SpaceHogs = AddArrayString(Data.SpaceHogs,L"?:\\*\\Installshield Installation Information\\*");
		Data.SpaceHogs = AddArrayString(Data.SpaceHogs,L"?:\\I386\\*");
		Data.SpaceHogs = AddArrayString(Data.SpaceHogs,L"?:\\System Volume Information\\*");
		Data.SpaceHogs = AddArrayString(Data.SpaceHogs,L"?:\\windows.old\\*");

		Data.SpaceHogs = AddArrayString(Data.SpaceHogs,L"*.7z");
		Data.SpaceHogs = AddArrayString(Data.SpaceHogs,L"*.arj");
		Data.SpaceHogs = AddArrayString(Data.SpaceHogs,L"*.avi");
		Data.SpaceHogs = AddArrayString(Data.SpaceHogs,L"*.bak");
		Data.SpaceHogs = AddArrayString(Data.SpaceHogs,L"*.bup");    /* DVD */
		Data.SpaceHogs = AddArrayString(Data.SpaceHogs,L"*.bz2");
		Data.SpaceHogs = AddArrayString(Data.SpaceHogs,L"*.cab");
		Data.SpaceHogs = AddArrayString(Data.SpaceHogs,L"*.chm");    /* Help files */
		Data.SpaceHogs = AddArrayString(Data.SpaceHogs,L"*.dvr-ms");
		Data.SpaceHogs = AddArrayString(Data.SpaceHogs,L"*.gz");
		Data.SpaceHogs = AddArrayString(Data.SpaceHogs,L"*.ifo");    /* DVD */
		Data.SpaceHogs = AddArrayString(Data.SpaceHogs,L"*.log");
		Data.SpaceHogs = AddArrayString(Data.SpaceHogs,L"*.lzh");
		Data.SpaceHogs = AddArrayString(Data.SpaceHogs,L"*.mp3");
		Data.SpaceHogs = AddArrayString(Data.SpaceHogs,L"*.msi");
		Data.SpaceHogs = AddArrayString(Data.SpaceHogs,L"*.old");
		Data.SpaceHogs = AddArrayString(Data.SpaceHogs,L"*.pdf");
		Data.SpaceHogs = AddArrayString(Data.SpaceHogs,L"*.rar");
		Data.SpaceHogs = AddArrayString(Data.SpaceHogs,L"*.rpm");
		Data.SpaceHogs = AddArrayString(Data.SpaceHogs,L"*.tar");
		Data.SpaceHogs = AddArrayString(Data.SpaceHogs,L"*.wmv");
		Data.SpaceHogs = AddArrayString(Data.SpaceHogs,L"*.vob");    /* DVD */
		Data.SpaceHogs = AddArrayString(Data.SpaceHogs,L"*.z");
		Data.SpaceHogs = AddArrayString(Data.SpaceHogs,L"*.zip");
	}

	/* If the NtfsDisableLastAccessUpdate setting in the registry is 1, then disable
	the LastAccessTime check for the spacehogs. */
	Data.UseLastAccessTime = TRUE;

	if (Data.UseDefaultSpaceHogs == TRUE)
	{
		Result = RegCreateKeyExW(HKEY_LOCAL_MACHINE,
			L"SYSTEM\\CurrentControlSet\\Control\\FileSystem",0,
			NULL,REG_OPTION_NON_VOLATILE,KEY_READ,NULL,&Key,&KeyDisposition);

		if (Result == ERROR_SUCCESS)
		{
			Length = sizeof(NtfsDisableLastAccessUpdate);

			Result = RegQueryValueExW(Key,L"NtfsDisableLastAccessUpdate",NULL,NULL,
				(BYTE *)&NtfsDisableLastAccessUpdate,&Length);

			if ((Result == ERROR_SUCCESS) && (NtfsDisableLastAccessUpdate == 1))
			{
				Data.UseLastAccessTime = FALSE;
			}

			RegCloseKey(Key);
		}

		if (Data.UseLastAccessTime == TRUE)
		{
			jkGui->ShowDebug(1,NULL,L"NtfsDisableLastAccessUpdate is inactive, using LastAccessTime for SpaceHogs.");
		}
		else
		{
			jkGui->ShowDebug(1,NULL,L"NtfsDisableLastAccessUpdate is active, ignoring LastAccessTime for SpaceHogs.");
		}
	}

	/* If a Path is specified then call DefragOnePath() for that path. Otherwise call
	DefragMountpoints() for every disk in the system. */
	if ((Path != NULL) && (*Path != 0))
	{
		DefragOnePath(&Data,Path,Mode);
	}
	else
	{
		DrivesSize = GetLogicalDriveStringsW(0,NULL);

		Drives = (WCHAR *)malloc(sizeof(WCHAR) * (DrivesSize + 1));

		if (Drives != NULL)
		{
			DrivesSize = GetLogicalDriveStringsW(DrivesSize,Drives);

			if (DrivesSize == 0)
			{
				/* "Could not get list of volumes: %s" */
				SystemErrorStr(GetLastError(),s1,BUFSIZ);

				jkGui->ShowDebug(1,NULL,Data.DebugMsg[39],s1);
			}
			else
			{
				Drive = Drives;

				while (*Drive != '\0')
				{
					DefragMountpoints(&Data,Drive,Mode);
					while (*Drive != '\0') Drive++;
					Drive++;
				}
			}

			free(Drives);
		}

		jkGui->ClearScreen(Data.DebugMsg[38]);
	}

	/* Cleanup. */
	if (Data.SpaceHogs != NULL)
	{
		for (i = 0; Data.SpaceHogs[i] != NULL; i++) free(Data.SpaceHogs[i]);

		free(Data.SpaceHogs);
	}

	*Data.Running = STOPPED;
}

/*

Stop the defragger. The "Running" variable must be the same as what was given to
the RunJkDefrag() subroutine. Wait for a maximum of TimeOut milliseconds for the
defragger to stop. If TimeOut is zero then wait indefinitely. If TimeOut is
negative then immediately return without waiting.

*/
void JKDefragLib::StopJkDefrag(int *Running, int TimeOut)
{
	int TimeWaited;

	/* Sanity check. */
	if (Running == NULL) return;

	/* All loops in the library check if the Running variable is set to
	RUNNING. If not then the loop will exit. In effect this will stop
	the defragger. */
	if (*Running == RUNNING) *Running = STOPPING;

	/* Wait for a maximum of TimeOut milliseconds for the defragger to stop.
	If TimeOut is zero then wait indefinitely. If TimeOut is negative then
	immediately return without waiting. */
	TimeWaited = 0;

	while (TimeWaited <= TimeOut)
	{
		if (*Running == STOPPED) break;

		Sleep(100);

		if (TimeOut > 0) TimeWaited = TimeWaited + 100;
	}
}
