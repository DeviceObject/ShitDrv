#ifndef __UTILS_API_H__
#define __UTILS_API_H__


typedef struct _FILE_STREAMHANDLE_CONTEXT 
{
    BOOLEAN		 m_bNewFile;
    FILE_INFORMATION_CLASS m_QueryType;
    PFILE_OBJECT 	outSideSbFileObj;
    PFLT_INSTANCE 	pInstance;
    WCHAR		 	m_Name[MAX_PATH];
    BOOLEAN			m_bDelete;
    HANDLE			handle;
    WCHAR		 	m_FileName[MAX_PATH];
}FILE_STREAMHANDLE_CONTEXT,*PFILE_STREAMHANDLE_CONTEXT;

//typedef struct _FILE_STREAM_CONTEXT {
//    DWORD		 m_BaseVersion;
//    DWORD		 m_Flags;
//}FILE_STREAM_CONTEXT,*PFILE_STREAM_CONTEXT;

#define MyNew(_type, _count) \
    (_type*)ExAllocatePoolWithTag(NonPagedPool, sizeof(_type) * (_count), 'PasP')

#define MyDelete(_p) \
    do{if(!(_p)) break; ExFreePool((_p)); (_p) = NULL;}while(0)


//////////////////////////////////////////////////////////////////////////
#define WATCHDOG_INTERNAL			3  //√Î
#define TERMINATE_PROCESS_TIMEOUT	20  //√Î
#define RUN_TARGET_INERVAL			72  //–° ±
//////////////////////////////////////////////////////////////////////////

#define DELAY_ONE_MICROSECOND   (-10)
#define DELAY_ONE_MILLISECOND   (DELAY_ONE_MICROSECOND*1000)
#define DELAY_ONE_SECOND        (DELAY_ONE_MILLISECOND*1000)
 

VOID System_Sleep(LONGLONG sec);


NTSTATUS GetProcessPath(PVOID pEProc, \
						PUNICODE_STRING pUniProcPath);
NTSTATUS GetProcessFullName(PFLT_FILTER pFltHandle, \
                            PFLT_INSTANCE pFltInstance, \
                            PVOID pEProcess, \
                            PUNICODE_STRING pUniFullName);
NTSTATUS GetProcessFullName_2(PFLT_FILTER pFltHandle, \
                              PFLT_INSTANCE pFltInstance, \
                              PVOID pEProcess, \
                              PUNICODE_STRING pUniFullName);
NTSTATUS QuerySymbolicLink(PUNICODE_STRING pUniSymbolicLinkName, \
                           PUNICODE_STRING pUniLinkTarget);
NTSTATUS MyRtlVolumeDeviceToDosName(PUNICODE_STRING pUniDeviceName, \
                                    PUNICODE_STRING pUniDosName);
BOOLEAN GetNTLinkName(WCHAR *wNTName,WCHAR *wFileName);
BOOLEAN QueryVolumeName(WCHAR wCh,WCHAR* wName,USHORT uSize);
BOOLEAN GetNtDeviceName(WCHAR *wFilename,WCHAR *wNtName);
NTSTATUS GetCurProcFullPath(PFLT_FILTER pFltHandle, \
                            PFLT_INSTANCE pFltInstance, \
                            WCHAR *wOutFullPath, \
                            ULONG ulMaximum);
BOOLEAN GetRegistryObjectCompleteName(PUNICODE_STRING pRegistryPath, \
                                      PUNICODE_STRING pPartialRegistryPath, \
                                      PVOID pRegistryObject);
PVOID MyAllocateMemory(POOL_TYPE PoolType,SIZE_T NumberOfBytes);
NTSTATUS FltQueryInformationFileSyncronous(PFLT_INSTANCE Instance, \
                                           PFILE_OBJECT FileObject, \
                                           PVOID FileInformation, \
                                           ULONG Length, \
                                           FILE_INFORMATION_CLASS FileInformationClass, \
                                           PULONG LengthReturned);
NTSTATUS ConvertSandboxName(PUNICODE_STRING pSandboxPath, \
                            PUNICODE_STRING pSrcFullName, \
                            PUNICODE_STRING pDstFullName, \
                            PUNICODE_STRING pVolumeName);
BOOLEAN IsSandboxFileExist(PFLT_FILTER pFilter, \
                           PFLT_INSTANCE pInstance, \
                           PUNICODE_STRING pFileName);
NTSTATUS SandboxIsDirectory(PFILE_OBJECT pFileObject, \
                            PUNICODE_STRING pUniDirName, \
                            PFLT_FILTER pFilter, \
                            PFLT_INSTANCE pInstance, \
                            BOOLEAN* bIsDirectory);
NTSTATUS SandboxRedirectFile(PFLT_CALLBACK_DATA Data, \
                             PCFLT_RELATED_OBJECTS FltObjects, \
                             PUNICODE_STRING pUstrDstFileName);
BOOLEAN SandboxCreateOneFile(PFLT_FILTER pFilter, \
                             PFLT_INSTANCE pInstance, \
                             PFILE_OBJECT* pFileObject, \
                             PUNICODE_STRING pUniFileName, \
                             BOOLEAN bRetFileObj, \
                             ACCESS_MASK AccessMask, \
                             ULONG ulCreateDisposition, \
                             BOOLEAN bDirectory);
PFLT_INSTANCE SandboxGetVolumeInstance(PFLT_FILTER pFilter, \
                                  PUNICODE_STRING pVolumeName);
NTSTATUS DoCopyFile(PFLT_FILTER pFilter, \
                    PFILE_OBJECT pSrcFileObj, \
                    PFLT_INSTANCE pSrcInstance, \
                    PUNICODE_STRING pUniSrcFileName, \
                    PFLT_INSTANCE pDstInstance, \
                    PUNICODE_STRING pUniDstFileName, \
                    BOOLEAN bDirectory, \
                    BOOLEAN bReplace);
NTSTATUS CopyModifyFile(PFLT_FILTER hFltHandle, \
                        PFLT_INSTANCE pSrcInstance, \
                        PFILE_OBJECT pSrcFileObj, \
                        PUNICODE_STRING pUniSrcFileName, \
                        PFLT_INSTANCE pDstInstance, \
                        PUNICODE_STRING pUniDstFileName, \
                        BOOLEAN bDirectory);
PVOID GetFileFullPath(PFILE_OBJECT pFileObject, \
                      PFLT_INSTANCE pFltInstance);
NTSTATUS ConvertSandboxShortName(PUNICODE_STRING pSandboxPath, \
                                 PUNICODE_STRING pSrcFullName, \
                                 PUNICODE_STRING pDstFullName, \
                                 PUNICODE_STRING pVolumeName);
NTSTATUS SandboxGetFileNameInformation(PFLT_VOLUME pVolume,
                                       PFLT_INSTANCE	pInstance,
                                       PFILE_OBJECT pFileObject,
                                       BOOLEAN bGetFromCache,
                                       PFLT_FILE_NAME_INFORMATION *pNameInfo);
ULONGLONG FltGetFileLength(PFLT_FILTER pFltHandle, \
                    PFLT_INSTANCE pFltInstance, \
                    PUNICODE_STRING pUniFileName);
NTSTATUS FltCalcFileDatMd5(PFLT_FILTER pFltHandle, \
                        PFLT_INSTANCE pFltInstance, \
                        PUNICODE_STRING pUniFileName, \
                        PUCHAR pOutMd5);

ULONGLONG GetFileLength(PUNICODE_STRING pUniFileName);
NTSTATUS CalcFileDatMd5(PUNICODE_STRING pUniFileName,PUCHAR pOutMd5);

#endif