#include "ShitDrv.h"
#include "UtilsApi.h"
#include "RecordInfo.h"
#include "RegisterCallBack.h"

LIST_ENTRY g_RecordListHead;

KSPIN_LOCK g_RecordSpinLock;

void InitializeRecordInfo()
{
    InitializeListHead(&g_RecordListHead);
    KeInitializeSpinLock(&g_RecordSpinLock);
}
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
                             ULONG ulType)
{
    if (NULL == pRecordList || \
        NULL == wProcessFullPath)
    {
        return FALSE;
    }
    if (wProcessFullPath && ulFullPathLength)
    {
        RtlZeroMemory(pRecordList->RecordDat.wProcessFullPath, \
            MAX_PATH * sizeof(WCHAR));

        RtlCopyMemory(pRecordList->RecordDat.wProcessFullPath, \
            wProcessFullPath, \
            ulFullPathLength);
    }
    if (wTargetPath && ulTargetPathLength)
    {
        RtlZeroMemory(pRecordList->RecordDat.wTargetPath, \
            MAX_PATH * sizeof(WCHAR));

        RtlCopyMemory(pRecordList->RecordDat.wTargetPath, \
            wTargetPath, \
            ulTargetPathLength);
    }
    if (ulOperationLength && wOperation)
    {
        RtlCopyMemory(pRecordList->RecordDat.wOperation, \
            wOperation, \
            ulOperationLength);
    }
    if (hProcessId)
    {
        pRecordList->RecordDat.hProcessId = hProcessId;
    }
    if (ulOperType)
    {
        pRecordList->RecordDat.ulOperType = ulOperType;
    }
    KeQuerySystemTime(&pRecordList->RecordDat.OperationTime);
    if (ulType == IsFsFile)
    {
        pRecordList->RecordDat.ulSignature = SHIT_DRV_SIGNATURE_FS;
    }
    else if (ulType == IsRegister)
    {
        pRecordList->RecordDat.ulSignature = SHIT_DRV_SIGNATURE_RG;
    }
    else
    {

    }
    
    return TRUE;
}
PRECORD_LIST AllocateRecordListDat()
{
    PRECORD_LIST pRecordList;

    pRecordList = NULL;

    do 
    {
        pRecordList = (PRECORD_LIST)ExAllocatePoolWithTag(NonPagedPool, \
            sizeof(RECORD_LIST), \
            SHIT_DRV_RECORD);
    } while (NULL == pRecordList);
    RtlZeroMemory(pRecordList,sizeof(RECORD_LIST));
    return pRecordList;
}
void FreeRecordListDat(PRECORD_LIST pRecordList)
{
    if (pRecordList)
    {
        ExFreePoolWithTag(pRecordList,SHIT_DRV_RECORD);
    }
    return;
}
VOID SendRecordThread(PVOID StartContext)
{
    while (TRUE)
    {
        if (!IsListEmpty(&g_RecordListHead))
        {
            if (g_pFilterCtl->FileFltAccessCtl.ulIsStartFilter == FALSE)
            {
                KeSetEvent(g_pEventWaitKrl,IO_NO_INCREMENT,FALSE);
                KeClearEvent(g_pEventWaitKrl);
            }
        }
        System_Sleep(1);
    }
    PsTerminateSystemThread(STATUS_SUCCESS);
    return;
}