#include "ShitDrv.h"
#include "RegisterMiniFilter.h"
#include "Dispatch.h"
#include "RegisterCallBack.h"
#include "RecordInfo.h"
#include "UtilsApi.h"
#include "ProcessList.h"
#include "ProcessNotify.h"
#include "Communication.h"

PKEVENT g_pEventWaitKrl = NULL;
HANDLE g_hEventWaitKrl = NULL;


void DrvUnLoad(PDRIVER_OBJECT pDrvObj)
{
	UNICODE_STRING UniSymName;

    g_pFilterCtl->FileFltAccessCtl.ulIsStartFilter = FALSE;
	PsSetCreateProcessNotifyRoutine(ProcessNotify,TRUE);
	if (g_pFilterCtl->ServerPort)
	{
		FltCloseCommunicationPort(g_pFilterCtl->ServerPort);
	}
    InitializeCmCallBack(pDrvObj,NULL,NULL,TRUE);
    if (g_pFilterCtl->pFilterHandle)
    {
        FltUnregisterFilter(g_pFilterCtl->pFilterHandle);
        g_pFilterCtl->pFilterHandle = NULL;
    }
    if (pDrvObj->DeviceObject)
    {
        RtlInitUnicodeString(&UniSymName,SHITDRV_LINKNAME);
        IoDeleteSymbolicLink(&UniSymName);
        IoDeleteDevice(pDrvObj->DeviceObject);
    }
	if (g_hEventWaitKrl)
	{
		ZwClose(g_hEventWaitKrl);
	}
	if (g_pFilterCtl->UniSandBoxPath.Buffer)
	{
		ExFreePoolWithTag(g_pFilterCtl->UniSandBoxPath.Buffer,'SOD');
	}
	if (g_pRegisterCallBack)
	{
		ExFreePoolWithTag(g_pRegisterCallBack,SHITDRV_TAG);
	}
    if (g_pFilterCtl)
    {
        ExFreePoolWithTag(g_pFilterCtl,SHITDRV_TAG);
    }
	return;
}

NTSTATUS DriverEntry(PDRIVER_OBJECT pDrvObj,PUNICODE_STRING pUniRegPath)
{
	NTSTATUS Status;
	UNICODE_STRING UniDevName;
	UNICODE_STRING UniSymName;
    UNICODE_STRING UniAltitude;
	PDEVICE_OBJECT pDevObj;

	Status = STATUS_UNSUCCESSFUL;
    
	do 
	{
		if (*NtBuildNumber < 2600)
		{
			Status = STATUS_NOT_SUPPORTED;
			break;
		}
		RtlInitUnicodeString(&UniDevName,SHITDRV_DEVNAME);
		RtlInitUnicodeString(&UniSymName,SHITDRV_LINKNAME);

		Status = IoCreateDevice(pDrvObj, \
			(ULONG)NULL, \
			&UniDevName, \
			FILE_DEVICE_UNKNOWN, \
			0, \
			FALSE, \
			&pDevObj);
		if (NT_ERROR(Status))
		{
			break;
		}

		Status = IoCreateSymbolicLink(&UniSymName,&UniDevName);
		if (NT_ERROR(Status))
		{
			IoDeleteDevice(pDevObj);
			break;
		}

		pDrvObj->MajorFunction[IRP_MJ_CREATE] = (PDRIVER_DISPATCH)Dispatch;
		pDrvObj->MajorFunction[IRP_MJ_CLOSE] = (PDRIVER_DISPATCH)Dispatch;
		pDrvObj->MajorFunction[IRP_MJ_READ] = (PDRIVER_DISPATCH)Dispatch;
		pDrvObj->MajorFunction[IRP_MJ_WRITE] = (PDRIVER_DISPATCH)Dispatch;
		pDrvObj->MajorFunction[IRP_MJ_DEVICE_CONTROL] = (PDRIVER_DISPATCH)DispatchIoCtl;
		pDrvObj->DriverUnload = DrvUnLoad;

		Status = InitializeMiniFilter(pDrvObj);
		if (NT_ERROR(Status))
		{
			break;
		}
        //if (g_pFilterCtl && \
        //    g_pFilterCtl->FileFltAccessCtl.ulIsInitialize)
        //{
        //    RtlCopyMemory(g_pFilterCtl->wDrvRegPath,pUniRegPath->Buffer,pUniRegPath->Length);
        //    RtlInitUnicodeString(&g_pFilterCtl->UniDrvPath,g_pFilterCtl->wDrvRegPath);
        //}
		Status = FltStartFiltering(g_pFilterCtl->pFilterHandle);
		if (NT_ERROR(Status))
		{
			if (g_pFilterCtl->pFilterHandle)
			{
				FltUnregisterFilter(g_pFilterCtl->pFilterHandle);
				g_pFilterCtl->pFilterHandle = NULL;
			}
			break;
		}
        Status = InitializeCommunication(g_pFilterCtl->pFilterHandle);
        if (NT_SUCCESS(Status))
        {
            //g_pFilterCtl->FileFltAccessCtl.ulIsStartFilter = 1;
        }
        RtlInitUnicodeString(&UniAltitude,L"370090");
        InitializeCmCallBack(pDrvObj,&UniAltitude,NULL,FALSE);

        InitializeRecordInfo();
		InitializeSandboxProcList();
		PsSetCreateProcessNotifyRoutine(ProcessNotify,FALSE);

	} while (0);
	if (NT_ERROR(Status))
	{
		if (pDrvObj->DeviceObject)
		{
            if (1 == g_pFilterCtl->FileFltAccessCtl.ulIsStartFilter && g_pFilterCtl->pFilterHandle)
            {
                FltUnregisterFilter(g_pFilterCtl->pFilterHandle);
                g_pFilterCtl->pFilterHandle = NULL;
                g_pFilterCtl->FileFltAccessCtl.ulIsStartFilter = 0;
            }
			RtlInitUnicodeString(&UniSymName,SHITDRV_LINKNAME);
			IoDeleteSymbolicLink(&UniSymName);
			IoDeleteDevice(pDrvObj->DeviceObject);
		}
	}
	return Status;
}
