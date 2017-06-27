#ifndef __CALL_BACK_H__
#define __CALL_BACK_H__


#define MAX_CALLBACK        32

typedef void (*DOSHITDRVRECORD)(PRECORD_INFORMATION pRecordDat);

typedef struct _RECORD_CALLBACK
{
    ULONG ulRecordCallbackCount;
    DOSHITDRVRECORD RecordCallBack[MAX_CALLBACK];
    CRITICAL_SECTION CriticalSection;
}RECORD_CALLBACK,*PRECORD_CALLBACK;

extern PRECORD_CALLBACK g_pRecordDrvCallBack;


HRESULT SetFltCtl(FILTER_DAT_CONTROL FltDatCtlInfo);
BOOLEAN InitializeRecordCallback();
BOOLEAN SetRecordCallback(DOSHITDRVRECORD RecordCallback);
void ReleaseRecordCallback(DOSHITDRVRECORD RecordCallback);
DWORD DoRecvWork(LPVOID lpParamter);
BOOLEAN SetCurClientPath(PWCHAR pClientPath);
BOOLEAN SetSandBoxProcName(PCHAR pImagName);
BOOLEAN SetCurrentProcess(HANDLE hProcessId);
BOOLEAN GetFileMd5Sum(PWCHAR pFilePath,PUCHAR pOutMd5Sum);
HANDLE StartInterface();


BOOLEAN InjectKtrapFrame(PCHAR pInjectProcessName,ULONG ulPid,PCHAR pInjectDllPath);

#endif