#ifndef __RECORD_INFO_H__
#define __RECORD_INFO_H__

#define SHIT_DRV_RECORD     'drDS'
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

typedef struct _RECORD_LIST
{
    LIST_ENTRY RecordList;
    RECORD_INFORMATION RecordDat;
}RECORD_LIST,*PRECORD_LIST;

extern LIST_ENTRY g_RecordListHead;

extern KSPIN_LOCK g_RecordSpinLock;


VOID SendRecordThread(PVOID StartContext);
void InitializeRecordInfo();
PRECORD_LIST AllocateRecordListDat();
void FreeRecordListDat(PRECORD_LIST pRecordList);
BOOLEAN InitializeRecordDat(PRECORD_LIST pRecordList, \
                            WCHAR *wProcessFullPath, \
                            ULONG ulFullPathLength, \
                            WCHAR *wTargetPath, \
                            ULONG ulTargetPathLength, \
                            WCHAR *wOperation,
                            ULONG ulOperationLength,
                            HANDLE hProcessId, \
                            ULONG ulOperType, \
							ULONG ulCreateCount, \
                            ULONG ulOpenCount, \
                            ULONG ulWriteCount, \
                            ULONG ulCleanupCount, \
                            ULONG ulCloseCount, \
                            ULONG ulType);

#endif