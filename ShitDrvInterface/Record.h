#ifndef __RECORD_H__
#define __RECORD_H__

typedef enum _RECORD_TYPE
{
    IsFsFile,
    IsRegister
}RECORD_TYPE,*PRECORD_TYPE;

#define SHIT_DRV_SIGNATURE_FS  'PaFs'
#define SHIT_DRV_SIGNATURE_RG  'PsRg'

typedef enum _FILE_OPERATIONS
{
	IsOpenFile,
	IsCreateFile,
	IsReadFile,
	IsWriteFile,
	IsSetFileSize,
	IsDeleteFile,
	IsRenNameFile
}FILE_OPERATIONS,*PFILE_OPERATIONS;

typedef struct _RECORD_INFORMATION 
{
    ULONG ulSignature;
    WCHAR wProcessFullPath[MAX_PATH];
    WCHAR wTargetPath[MAX_PATH];
	WCHAR wBakPath[MAX_PATH];
    CHAR Md5Sum[100];
    ULONG ulOperType;
    WCHAR wOperation[50];
    HANDLE hProcessId;
	ULONG ulCreateCount;
    ULONG ulOpenCount;
    ULONG ulWriteCount;
    ULONG ulCleanupCount;
    ULONG ulCloseCount;
    LARGE_INTEGER OperationTime;
}RECORD_INFORMATION,*PRECORD_INFORMATION;

#pragma pack(1)
typedef struct _FILTER_DAT_CONTROL
{
    ULONG ulIsStartFilter:1;
    ULONG ulIsFilterWrite:1;
    ULONG ulIsFilterDelete:1;
    ULONG ulIsFilterRenName:1;
    ULONG ulIsFilterSetSize:1;
    ULONG ulIsFilterSetInfo:1;
    ULONG ulReserved:26;
} FILTER_DAT_CONTROL,*PFILTER_DAT_CONTROL;
#pragma pack()

#endif