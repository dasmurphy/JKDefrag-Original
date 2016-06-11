#ifndef __SCANNTFS_H__
#define __SCANNTFS_H__

#define MFTBUFFERSIZE 256 * 1024         /* 256 KB seems to be the optimum. */

struct INODE_REFERENCE
{
	ULONG InodeNumberLowPart;

	USHORT InodeNumberHighPart;
	USHORT SequenceNumber;
};

struct NTFS_RECORD_HEADER
{
	ULONG Type;                   /* File type, for example 'FILE' */

	USHORT UsaOffset;             /* Offset to the Update Sequence Array */
	USHORT UsaCount;              /* Size in words of Update Sequence Array */

	USN Lsn;                      /* $LogFile Sequence Number (LSN) */
};

struct FILE_RECORD_HEADER
{
	struct NTFS_RECORD_HEADER RecHdr;

	USHORT SequenceNumber;        /* Sequence number */
	USHORT LinkCount;             /* Hard link count */
	USHORT AttributeOffset;       /* Offset to the first Attribute */
	USHORT Flags;                 /* Flags. bit 1 = in use, bit 2 = directory, bit 4 & 8 = unknown. */

	ULONG BytesInUse;             /* Real size of the FILE record */
	ULONG BytesAllocated;         /* Allocated size of the FILE record */

	INODE_REFERENCE BaseFileRecord;     /* File reference to the base FILE record */

	USHORT NextAttributeNumber;   /* Next Attribute Id */
	USHORT Padding;               /* Align to 4 UCHAR boundary (XP) */

	ULONG MFTRecordNumber;        /* Number of this MFT Record (XP) */

	USHORT UpdateSeqNum;          /*  */
};

enum ATTRIBUTE_TYPE
{
	AttributeInvalid              = 0x00,         /* Not defined by Windows */
	AttributeStandardInformation  = 0x10,
	AttributeAttributeList        = 0x20,
	AttributeFileName             = 0x30,
	AttributeObjectId             = 0x40,
	AttributeSecurityDescriptor   = 0x50,
	AttributeVolumeName           = 0x60,
	AttributeVolumeInformation    = 0x70,
	AttributeData                 = 0x80,
	AttributeIndexRoot            = 0x90,
	AttributeIndexAllocation      = 0xA0,
	AttributeBitmap               = 0xB0,
	AttributeReparsePoint         = 0xC0,         /* Reparse Point = Symbolic link */
	AttributeEAInformation        = 0xD0,
	AttributeEA                   = 0xE0,
	AttributePropertySet          = 0xF0,
	AttributeLoggedUtilityStream  = 0x100
};

struct ATTRIBUTE
{
	enum ATTRIBUTE_TYPE AttributeType;

	ULONG Length;

	BOOLEAN Nonresident;

	UCHAR NameLength;

	USHORT NameOffset;
	USHORT Flags;              /* 0x0001 = Compressed, 0x4000 = Encrypted, 0x8000 = Sparse */
	USHORT AttributeNumber;
};

struct RESIDENT_ATTRIBUTE
{
	struct ATTRIBUTE Attribute;

	ULONG ValueLength;

	USHORT ValueOffset;
	USHORT Flags;               // 0x0001 = Indexed
};

struct NONRESIDENT_ATTRIBUTE
{
	struct ATTRIBUTE Attribute;

	ULONGLONG StartingVcn;
	ULONGLONG LastVcn;

	USHORT RunArrayOffset;

	UCHAR CompressionUnit;
	UCHAR AlignmentOrReserved[5];

	ULONGLONG AllocatedSize;
	ULONGLONG DataSize;
	ULONGLONG InitializedSize;
	ULONGLONG CompressedSize;    // Only when compressed
};

struct STANDARD_INFORMATION
{
	ULONG64 CreationTime;
	ULONG64 FileChangeTime;
	ULONG64 MftChangeTime;
	ULONG64 LastAccessTime;

	ULONG FileAttributes;       /* READ_ONLY=0x01, HIDDEN=0x02, SYSTEM=0x04, VOLUME_ID=0x08, ARCHIVE=0x20, DEVICE=0x40 */
	ULONG MaximumVersions;
	ULONG VersionNumber;
	ULONG ClassId;
	ULONG OwnerId;                        // NTFS 3.0 only
	ULONG SecurityId;                     // NTFS 3.0 only

	ULONGLONG QuotaCharge;                // NTFS 3.0 only

	USN Usn;                              // NTFS 3.0 only
};

struct ATTRIBUTE_LIST
{
	enum ATTRIBUTE_TYPE AttributeType;

	USHORT Length;

	UCHAR NameLength;
	UCHAR NameOffset;

	ULONGLONG LowestVcn;

	INODE_REFERENCE FileReferenceNumber;

	USHORT Instance;
	USHORT AlignmentOrReserved[3];
};

struct FILENAME_ATTRIBUTE
{
	struct INODE_REFERENCE ParentDirectory;

	ULONG64 CreationTime;
	ULONG64 ChangeTime;
	ULONG64 LastWriteTime;
	ULONG64 LastAccessTime;

	ULONGLONG AllocatedSize;
	ULONGLONG DataSize;

	ULONG FileAttributes;
	ULONG AlignmentOrReserved;

	UCHAR NameLength;
	UCHAR NameType;                 /* NTFS=0x01, DOS=0x02 */

	WCHAR Name[1];
};

struct OBJECTID_ATTRIBUTE
{
	GUID ObjectId;

	union
	{
		struct
		{
			GUID BirthVolumeId;
			GUID BirthObjectId;
			GUID DomainId;
		};

		UCHAR ExtendedInfo[48];
	};
};

struct VOLUME_INFORMATION
{
	LONGLONG Reserved;

	UCHAR MajorVersion;
	UCHAR MinorVersion;

	USHORT Flags;                /* DIRTY=0x01, RESIZE_LOG_FILE=0x02 */
};

struct DIRECTORY_INDEX
{
	ULONG EntriesOffset;
	ULONG IndexBlockLength;
	ULONG AllocatedSize;
	ULONG Flags;                 /* SMALL=0x00, LARGE=0x01 */
};

struct DIRECTORY_ENTRY
{
	ULONGLONG FileReferenceNumber;

	USHORT Length;
	USHORT AttributeLength;

	ULONG Flags;           // 0x01 = Has trailing VCN, 0x02 = Last entry

	// FILENAME_ATTRIBUTE Name;
	// ULONGLONG Vcn;      // VCN in IndexAllocation of earlier entries
};

struct INDEX_ROOT
{
	enum ATTRIBUTE_TYPE Type;

	ULONG CollationRule;
	ULONG BytesPerIndexBlock;
	ULONG ClustersPerIndexBlock;

	struct DIRECTORY_INDEX DirectoryIndex;
};

struct INDEX_BLOCK_HEADER
{
	struct NTFS_RECORD_HEADER Ntfs;

	ULONGLONG IndexBlockVcn;

	struct DIRECTORY_INDEX DirectoryIndex;
};

struct REPARSE_POINT
{
	ULONG ReparseTag;

	USHORT ReparseDataLength;
	USHORT Reserved;

	UCHAR ReparseData[1];
};

struct EA_INFORMATION
{
	ULONG EaLength;
	ULONG EaQueryLength;
};

struct EA_ATTRIBUTE
{
	ULONG NextEntryOffset;

	UCHAR Flags;
	UCHAR EaNameLength;

	USHORT EaValueLength;

	CHAR EaName[1];
	// UCHAR EaData[];
};

struct ATTRIBUTE_DEFINITION
{
	WCHAR AttributeName[64];

	ULONG AttributeNumber;
	ULONG Unknown[2];
	ULONG Flags;

	ULONGLONG MinimumSize;
	ULONGLONG MaximumSize;
};

/*
   The NTFS scanner will construct an ItemStruct list in memory, but needs some
   extra information while constructing it. The following structs wrap the ItemStruct
   into a new struct with some extra info, discarded when the ItemStruct list is
   ready.

   A single Inode can contain multiple streams of data. Every stream has it's own
   list of fragments. The name of a stream is the same as the filename plus two
   extensions separated by colons:
         filename:"stream name":"stream type"

   For example:
         myfile.dat:stream1:$DATA

   The "stream name" is an empty string for the default stream, which is the data
   of regular files. The "stream type" is one of the following strings:
      0x10      $STANDARD_INFORMATION
      0x20      $ATTRIBUTE_LIST
      0x30      $FILE_NAME
      0x40  NT  $VOLUME_VERSION
      0x40  2K  $OBJECT_ID
      0x50      $SECURITY_DESCRIPTOR
      0x60      $VOLUME_NAME
      0x70      $VOLUME_INFORMATION
      0x80      $DATA
      0x90      $INDEX_ROOT
      0xA0      $INDEX_ALLOCATION
      0xB0      $BITMAP
      0xC0  NT  $SYMBOLIC_LINK
      0xC0  2K  $REPARSE_POINT
      0xD0      $EA_INFORMATION
      0xE0      $EA
      0xF0  NT  $PROPERTY_SET
      0x100 2K  $LOGGED_UTILITY_STREAM
*/

struct StreamStruct
{
	struct StreamStruct *Next;

	WCHAR *StreamName;                     /* "stream name" */

	ATTRIBUTE_TYPE StreamType;             /* "stream type" */

	struct FragmentListStruct *Fragments;  /* The fragments of the stream. */

	ULONG64 Clusters;                      /* Total number of clusters. */
	ULONG64 Bytes;                         /* Total number of bytes. */
};

struct InodeDataStruct
{
	ULONG64 Inode;                         /* The Inode number. */
	ULONG64 ParentInode;                   /* The Inode number of the parent directory. */

	BOOL Directory;                        /* YES: it's a directory. */

	WCHAR *LongFilename;                   /* Long filename. */
	WCHAR *ShortFilename;                  /* Short filename (8.3 DOS). */

	ULONG64 Bytes;                         /* Total number of bytes. */
	ULONG64 CreationTime;                  /* 1 second = 10000000 */
	ULONG64 MftChangeTime;
	ULONG64 LastAccessTime;

	struct StreamStruct *Streams;          /* List of StreamStruct. */
	struct FragmentListStruct *MftDataFragments;   /* The Fragments of the $MFT::$DATA stream. */

	ULONG64 MftDataBytes;                  /* Length of the $MFT::$DATA. */

	struct FragmentListStruct *MftBitmapFragments; /* The Fragments of the $MFT::$BITMAP stream. */

	ULONG64 MftBitmapBytes;                /* Length of the $MFT::$BITMAP. */
};

struct NtfsDiskInfoStruct
{
	ULONG64 BytesPerSector;
	ULONG64 SectorsPerCluster;
	ULONG64 TotalSectors;
	ULONG64 MftStartLcn;
	ULONG64 Mft2StartLcn;
	ULONG64 BytesPerMftRecord;
	ULONG64 ClustersPerIndexRecord;

	struct
	{
		BYTE Buffer[MFTBUFFERSIZE];

		ULONG64 Offset;

		int Age;
	} Buffers[3];
};

class JKScanNtfs
{
public:
	JKScanNtfs();
	~JKScanNtfs();

	// Get instance of the class
	static JKScanNtfs *getInstance();

	BOOL AnalyzeNtfsVolume(struct DefragDataStruct *Data);

private:
	WCHAR *StreamTypeNames(ATTRIBUTE_TYPE StreamType);

	BOOL FixupRawMftdata(struct DefragDataStruct *Data,struct NtfsDiskInfoStruct *DiskInfo,	BYTE *Buffer, ULONG64 BufLength);

	BYTE *ReadNonResidentData(struct DefragDataStruct *Data, struct NtfsDiskInfoStruct *DiskInfo, BYTE *RunData, DWORD RunDataLength,
		ULONG64 Offset, ULONG64 WantedLength);

	BOOL TranslateRundataToFragmentlist(
			struct DefragDataStruct *Data,
			struct InodeDataStruct *InodeData,
			WCHAR *StreamName,
			ATTRIBUTE_TYPE StreamType,
			BYTE *RunData,
			DWORD RunDataLength,
			ULONG64 StartingVcn,
			ULONG64 Bytes);

	void CleanupStreams(struct InodeDataStruct *InodeData, BOOL CleanupFragments);

	WCHAR *ConstructStreamName(WCHAR *FileName1, WCHAR *FileName2, struct StreamStruct *Stream);

	BOOL ProcessAttributes(
			struct DefragDataStruct *Data,
			struct NtfsDiskInfoStruct *DiskInfo,
			struct InodeDataStruct *InodeData,
			BYTE *Buffer,
			ULONG64 BufLength,
			USHORT Instance,
			int Depth);

	void ProcessAttributeList(
			struct DefragDataStruct *Data,
			struct NtfsDiskInfoStruct *DiskInfo,
			struct InodeDataStruct *InodeData,
			BYTE *Buffer,
			ULONG64 BufLength,
			int Depth);

	BOOL InterpretMftRecord(
			struct DefragDataStruct *Data,
			struct NtfsDiskInfoStruct *DiskInfo,
			struct ItemStruct **InodeArray,
			ULONG64 InodeNumber,
			ULONG64 MaxInode,
			struct FragmentListStruct **MftDataFragments,
			ULONG64 *MftDataBytes,
			struct FragmentListStruct **MftBitmapFragments,
			ULONG64 *MftBitmapBytes,
			BYTE *Buffer,
			ULONG64 BufLength);

	// static member that is an instance of itself
	static JKScanNtfs *m_jkScanNtfs;

//	JKDefragGui *m_jkGui;
	JKDefragLib *m_jkLib;
};

#endif
