#include "ShitDrv.h"
#include "RecordInfo.h"
#include "Communication.h"

NTSTATUS CommuniConnect(PFLT_PORT ClientPort, \
                        PVOID ServerPortCookie, \
                        PVOID ConnectionContext, \
                        ULONG SizeOfContext, \
                        PVOID *ConnectionPortCookie)
{
    NTSTATUS Status;

    Status = STATUS_SUCCESS;
    if (ClientPort && \
        g_pFilterCtl->FileFltAccessCtl.ulIsInitialize && \
        NULL == g_pFilterCtl->ClientPort)
    {
        g_pFilterCtl->ClientPort = ClientPort;
    }
    return Status;
}
VOID CommunDisConnect(PVOID ConnectionCookie)
{
    if (ConnectionCookie && \
        g_pFilterCtl->ClientPort)
    {
        FltCloseClientPort(g_pFilterCtl->pFilterHandle, \
            &g_pFilterCtl->ClientPort);
        g_pFilterCtl->ClientPort = NULL;
    }
    return;
}
void SetFilterFlag(PFILTER_DAT_CONTROL pFltDatCtl)
{
    g_pFilterCtl->FileFltAccessCtl.ulIsFilterWrite = pFltDatCtl->ulIsFilterWrite;
    g_pFilterCtl->FileFltAccessCtl.ulIsFilterDelete = pFltDatCtl->ulIsFilterDelete;
    g_pFilterCtl->FileFltAccessCtl.ulIsFilterRenName = pFltDatCtl->ulIsFilterRenName;
    g_pFilterCtl->FileFltAccessCtl.ulIsFilterSetSize = pFltDatCtl->ulIsFilterSetSize;
    g_pFilterCtl->FileFltAccessCtl.ulIsFilterSetInfo = pFltDatCtl->ulIsFilterSetInfo;
    g_pFilterCtl->FileFltAccessCtl.ulIsStartFilter = pFltDatCtl->ulIsStartFilter;

}
NTSTATUS CommunMsgNotify(PVOID PortCookie, \
                         PVOID InputBuffer, \
                         ULONG InputBufferLength, \
                         PVOID OutputBuffer, \
                         ULONG OutputBufferLength, \
                         PULONG ReturnOutputBufferLength)
{
    NTSTATUS Status;
    KIRQL OldIrql;
    PRECORD_LIST pRecordList;
    PFILTER_DAT_CONTROL pFltDatCtl;

    Status = STATUS_UNSUCCESSFUL;
	*ReturnOutputBufferLength = 0;
    if (InputBufferLength == sizeof(FILTER_DAT_CONTROL))
    {
        pFltDatCtl = (PFILTER_DAT_CONTROL)InputBuffer;
        SetFilterFlag(pFltDatCtl);
    }
    if (OutputBufferLength == sizeof(RECORD_INFORMATION))
    {
        pRecordList = NULL;
        if (!IsListEmpty(&g_RecordListHead))
        {
            KeAcquireSpinLock(&g_RecordSpinLock,&OldIrql);
            pRecordList = (PRECORD_LIST)RemoveHeadList(&g_RecordListHead);
            KeReleaseSpinLock(&g_RecordSpinLock,OldIrql);
            if (pRecordList && \
                OutputBuffer && OutputBufferLength >= sizeof(RECORD_INFORMATION))
            {
                RtlCopyMemory(OutputBuffer,&pRecordList->RecordDat,sizeof(RECORD_INFORMATION));
                *ReturnOutputBufferLength = sizeof(RECORD_INFORMATION);
                Status = STATUS_SUCCESS;
                KeClearEvent(g_pEventWaitKrl);
            }
            if (pRecordList)
            {
                FreeRecordListDat(pRecordList);
            }
        }
    }
    return Status;
}
NTSTATUS InitializeCommunication(PFLT_FILTER pFltHandle)
{
    NTSTATUS Status;
    PSECURITY_DESCRIPTOR pSecurityDesc;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING UniFltPort;
    UNICODE_STRING UniEventNameWaitKernel;


    Status = STATUS_UNSUCCESSFUL;
    pSecurityDesc = NULL;
    RtlInitUnicodeString(&UniEventNameWaitKernel,EVENT_NAME_WAIT_KERNEL);
    do 
    {
        g_pEventWaitKrl = IoCreateNotificationEvent(&UniEventNameWaitKernel,&g_hEventWaitKrl);
        if (g_pEventWaitKrl != NULL)
        {
            KeClearEvent(g_pEventWaitKrl);
        }
        Status = FltBuildDefaultSecurityDescriptor(&pSecurityDesc,FLT_PORT_ALL_ACCESS);
        if (NT_ERROR(Status))
        {
            break;
        }
        RtlInitUnicodeString(&UniFltPort,SERVER_PORT);
        InitializeObjectAttributes(&ObjectAttributes, \
            &UniFltPort, \
            OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, \
            NULL, \
            pSecurityDesc);
        Status = FltCreateCommunicationPort(pFltHandle, \
            &g_pFilterCtl->ServerPort, \
            &ObjectAttributes, \
            NULL, \
            CommuniConnect, \
            CommunDisConnect, \
            CommunMsgNotify, \
            1);
        if (NT_ERROR(Status))
        {
            break;
        }
        Status = STATUS_SUCCESS;
    } while (0);
    if (pSecurityDesc)
    {
        FltFreeSecurityDescriptor(pSecurityDesc);
    }
    return Status;
}