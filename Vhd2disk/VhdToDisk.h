#pragma once


typedef struct
{
	UINT16 cylinders;
	UCHAR heads;
	UCHAR sectors;
} VHD_GEOMETRY;


typedef struct _VHD_FOOTER
{
	CHAR cookie[8];
	UINT32 features;
	UINT32 version;
	UINT64 dataOffset;
	UINT32 timeStamp;
	CHAR creatorApplication[4];
	UINT32 creatorVersion;
	CHAR creatorOS[4];
	UINT64 originalSize;
	UINT64 currentSize;
	VHD_GEOMETRY diskGeometry;
	UINT32 diskType;
	UINT32 checksum;
	UCHAR uniqueId[16];
	UCHAR savedState;
	UCHAR padding[427];
}VHD_FOOTER, *PVHD_FOOTER;


typedef struct
{
	CHAR cookie[8];
	UINT64 dataOffset;
	UINT64 tableOffset;
	UINT32 headerVersion;
	UINT32 maxTableEntries;
	UINT32 blockSize;
	UINT32 checksum;
	UCHAR parentUniqueId[16];
	UINT32 parentTimeStamp;
	UINT32 reserved1;
	UCHAR parentUnicodeName[512];
	struct
	{
		UCHAR platformCode[4];
		UINT32 platformDataSpace;
		UINT32 platformDataLength;
		UINT32 reserved;
		UINT64 platformDataOffset;
	} partentLocator[8];
	UCHAR reserved2[256];
} VHD_DYNAMIC;


class CVhdToDisk
{
	VHD_FOOTER	m_Foot;
	VHD_DYNAMIC m_Dyn;

	HANDLE		m_hVhdFile;
	HANDLE		m_hPhysicalDrive;

public:
	CVhdToDisk(void);
	CVhdToDisk(LPWSTR sPath);
	~CVhdToDisk(void);

	BOOL DumpVhdToDisk(const LPWSTR sPath, const LPWSTR sDrive, const HWND);


	BOOL ParseFirstSector(HWND hDlg);

protected:

	BOOL OpenVhdFile(LPWSTR sPath);
	BOOL CloseVhdFile();

	BOOL OpenPhysicalDrive(LPWSTR sDrive);
	BOOL ClosePhysicalDrive();

	BOOL ReadFooter();
	BOOL ReadDynHeader();

	
	UINT64 GetFirstSectorAddress();
	
	BOOL Dump(HWND);
};
