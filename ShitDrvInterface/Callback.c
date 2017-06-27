#include "DllCfg.h"
#include "Record.h"
#include "Callback.h"



PRECORD_CALLBACK g_pRecordDrvCallBack = NULL;

BOOLEAN GetFileMd5Sum(PWCHAR pFilePath,PUCHAR pOutMd5Sum)
{
	ULONG ulBytesReturned;
	BOOLEAN bRet;
	IOP(GetFileMd5Sum) Params;


	bRet = FALSE;
	RtlZeroMemory(&Params,sizeof(Params));
	RtlCopyMemory(Params.wImageName,pFilePath,wcslen(pFilePath) * sizeof(WCHAR));

	if (NULL == pOutMd5Sum)
	{
		return bRet;
	}
	bRet = DeviceIoControl(g_hShitDrvDevObj, \
		IOC(GetFileMd5Sum), \
		&Params, \
		sizeof(Params), \
		&Params, \
		sizeof(Params), \
		&ulBytesReturned, \
		NULL);
	if (bRet)
	{
		RtlCopyMemory(pOutMd5Sum,Params.Md5Sum,16);
	}
	return bRet;
}
BOOLEAN SetCurrentProcess(HANDLE hProcessId)
{
	ULONG ulBytesReturned;
	IOP(SetCurrentProcess) Params;

	RtlZeroMemory(&Params,sizeof(Params));
	Params.ProcessId = hProcessId;
	return DeviceIoControl(g_hShitDrvDevObj, \
		IOC(SetCurrentProcess), \
		&Params, \
		sizeof(Params), \
		NULL, \
		0, \
		&ulBytesReturned, \
		NULL);
}
BOOLEAN InjectKtrapFrame(PCHAR pInjectProcessName,ULONG ulPid,PCHAR pInjectDllPath)
{
	ULONG ulBytesReturned;
	IOP(InjectKtrapFrame) Params;

	if (NULL == pInjectDllPath)
	{
		return 0;
	}
	Params.ulPid = 0;
	RtlZeroMemory(Params.pInjectProcessName,MAX_PATH);
	RtlZeroMemory(Params.pInjectDllPath,MAX_PATH);

	if (ulPid)
	{
		Params.ulPid = ulPid;
	}
	if (pInjectProcessName)
	{
		RtlCopyMemory(Params.pInjectProcessName,pInjectProcessName,strlen(pInjectProcessName));
	}
	RtlCopyMemory(Params.pInjectDllPath,pInjectDllPath,strlen(pInjectDllPath));
	return DeviceIoControl(g_hShitDrvDevObj, \
		IOC(InjectKtrapFrame), \
		&Params, \
		sizeof(Params), \
		NULL, \
		0, \
		&ulBytesReturned, \
		NULL);
}
BOOLEAN SetSandBoxProcName(PCHAR pImagName)
{
	ULONG ulBytesReturned;
	IOP(SetSandBoxProcName) Params;

	RtlZeroMemory(&Params,sizeof(Params));
	StringCchCopyA(Params.ImageName,MAX_PATH,pImagName);
	return DeviceIoControl(g_hShitDrvDevObj, \
		IOC(SetSandBoxProcName), \
		&Params, \
		sizeof(Params), \
		NULL, \
		0, \
		&ulBytesReturned, \
		NULL);
}
BOOLEAN SetCurClientPath(PWCHAR pClientPath)
{
	ULONG ulBytesReturned;
	IOP(SetCurClientPath) Params;
	
	RtlZeroMemory(&Params,sizeof(Params));
	StringCchCopyW(Params.wCurClientPath,MAX_PATH,pClientPath);
	return DeviceIoControl(g_hShitDrvDevObj, \
		IOC(SetCurClientPath), \
		&Params, \
		sizeof(Params), \
		NULL, \
		0, \
		&ulBytesReturned, \
		NULL);
}
HRESULT SetFltCtl(FILTER_DAT_CONTROL FltDatCtlInfo)
{
    ULONG ulBytesReturned;
    HRESULT hResult;
    hResult = FilterSendMessage(g_hServerPort, \
        &FltDatCtlInfo, \
        sizeof(FltDatCtlInfo), \
        NULL, \
        0, \
        &ulBytesReturned);
    if (hResult != S_OK)
    {
        return hResult;
    }
    return hResult;
}
DWORD DoRecvWork(LPVOID lpParamter)
{
	HRESULT hResult;
    BOOLEAN bFind;
    ULONG uli;
    DOSHITDRVRECORD RecordCallBack;
    ULONG ulBytesReturned = 0;
    PRECORD_INFORMATION pRecordInfo = NULL;
    bFind = FALSE;

    pRecordInfo = (PRECORD_INFORMATION)malloc(sizeof(RECORD_INFORMATION));
    if (NULL == pRecordInfo)
    {
        return -1;
    }
    while (WaitForSingleObject(g_hEventWaitKrl,INFINITE))
    {
        RtlZeroMemory(pRecordInfo,sizeof(RECORD_INFORMATION));
        hResult = FilterSendMessage(g_hServerPort, \
            L"Test", \
            8, \
            pRecordInfo, \
            sizeof(RECORD_INFORMATION), \
            &ulBytesReturned);
        if (hResult != S_OK || \
			ulBytesReturned == 0)
        {
            continue;
        }
        if (g_pRecordDrvCallBack->ulRecordCallbackCount > 0)
        {
            for (uli = 0;uli < MAX_CALLBACK;uli++)
            {
                if (g_pRecordDrvCallBack->RecordCallBack[uli] != NULL)
                {
                    RecordCallBack = g_pRecordDrvCallBack->RecordCallBack[uli];
                    RecordCallBack(pRecordInfo);
                }
            }
        }
    }
    return 0;
}
BOOLEAN InitializeRecordCallback()
{
    BOOLEAN bRet;

    do 
    {
        if (NULL == g_pRecordDrvCallBack)
        {
            g_pRecordDrvCallBack = (PRECORD_CALLBACK)VirtualAlloc(NULL, \
                sizeof(RECORD_CALLBACK), \
                MEM_COMMIT | MEM_RESERVE, \
                PAGE_READWRITE);
            if (NULL == g_pRecordDrvCallBack)
            {
                bRet = FALSE;
            }
            RtlZeroMemory(g_pRecordDrvCallBack,sizeof(RECORD_CALLBACK));
        }
        InitializeCriticalSection(&g_pRecordDrvCallBack->CriticalSection);
        bRet = TRUE;
    } while (0);
    return bRet;
}
BOOLEAN SetRecordCallback(DOSHITDRVRECORD RecordCallback)
{
    BOOLEAN bRet;
    ULONG uli;

    bRet = FALSE;

    do 
    {
        if (NULL == RecordCallback)
        {
            break;
        }
        for (uli = 0;uli < MAX_CALLBACK;uli++)
        {
            if (g_pRecordDrvCallBack->RecordCallBack[uli] == RecordCallback)
            {
                break;
            }
        }
        for (uli = 0;uli < MAX_CALLBACK;uli++)
        {
            if (g_pRecordDrvCallBack->RecordCallBack[uli] == NULL)
            {
                EnterCriticalSection(&g_pRecordDrvCallBack->CriticalSection);
                g_pRecordDrvCallBack->RecordCallBack[uli] = RecordCallback;
                g_pRecordDrvCallBack->ulRecordCallbackCount++;
                LeaveCriticalSection(&g_pRecordDrvCallBack->CriticalSection);
                bRet = TRUE;
				break;
            }
        }
    } while (0);
    return bRet;
}
void ReleaseRecordCallback(DOSHITDRVRECORD RecordCallback)
{
    ULONG uli;
    if (RecordCallback)
    {
        for (uli = 0;uli < MAX_CALLBACK;uli++)
        {
            if (g_pRecordDrvCallBack->RecordCallBack[uli] == RecordCallback)
            {
                EnterCriticalSection(&g_pRecordDrvCallBack->CriticalSection);
                g_pRecordDrvCallBack->RecordCallBack[uli] = NULL;
                g_pRecordDrvCallBack->ulRecordCallbackCount--;
                LeaveCriticalSection(&g_pRecordDrvCallBack->CriticalSection);
            }
        }
    }
    else
    {
        for (uli = 0;uli < MAX_CALLBACK;uli++)
        {
            if (g_pRecordDrvCallBack->RecordCallBack[uli])
            {
                EnterCriticalSection(&g_pRecordDrvCallBack->CriticalSection);
                g_pRecordDrvCallBack->RecordCallBack[uli] = NULL;
                g_pRecordDrvCallBack->ulRecordCallbackCount--;
                LeaveCriticalSection(&g_pRecordDrvCallBack->CriticalSection);
            }
        }
        DeleteCriticalSection(&g_pRecordDrvCallBack->CriticalSection);
    }
}