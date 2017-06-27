#include "ShitDrv.h"
#include "MiniFilterOperation.h"
#include "UtilsApi.h"
#include "RegisterMiniFilter.h"
#include "ContextOperation.h"

PMY_FILTER_CONTROL g_pFilterCtl = NULL;

const FLT_CONTEXT_REGISTRATION MiniMonitorContext[] =
{

	{
		FLT_STREAM_CONTEXT,
		0,
		ShitDrvContextCleanup,
		SHITDRV_STREAM_CONTEXT_SIZE,
		SHITDRV_STREAM_CONTEXT_TAG
	},
	{
		FLT_STREAMHANDLE_CONTEXT,
		0,
		ShitDrvContextCleanup,
		SHITDRV_STREAMHANDLE_CONTEXT_SIZE,
		SHITDRV_STREAM_HANDLE_TAG
	},
	{
		FLT_CONTEXT_END
	}
};
const FLT_OPERATION_REGISTRATION MiniFilterCallbacks[] =
{
	{
		IRP_MJ_CREATE,
		0,
		NULL,
		FltPostCreateFiles
	},
	{
		IRP_MJ_WRITE,
		0,
		FltPreWriteFiles,
		FltPostWriteFiles
	},
	{
		IRP_MJ_SET_INFORMATION,
		0,
		FltPreSetFileformationFiles,
		FltPostSetFileInformationFiles
	},
    {
        IRP_MJ_CLOSE,
        0,
        FltPreClose,
        NULL
    },
	{
		IRP_MJ_OPERATION_END
	}
};

const FLT_REGISTRATION MiniMonitorRegistration = {

	sizeof(FLT_REGISTRATION),         //  Size
	FLT_REGISTRATION_VERSION,           //  Version
	0,                                  //  Flags
	MiniMonitorContext,					//  ContextRegistration
    //NULL,
	MiniFilterCallbacks,               //  Operation callbacks
	MiniFilterUnload,					//  FilterUnload
	MiniFilterInstanceSetup,			//  InstanceSetup
	NULL,								//  InstanceQueryTeardown
	MiniFilterInstanceTeardownStart,    //  InstanceTeardownStart
	NULL,                               //  InstanceTeardownComplete
	NULL,                               //  GenerateFileName
	NULL,                               //  GenerateDestinationFileName
	NULL                                //  NormalizeNameComponent
};


NTSTATUS InitializeMiniFilter(PDRIVER_OBJECT pDrvObj)
{
	NTSTATUS Status;
    UNICODE_STRING UniSymLinkName;
	
	Status = STATUS_UNSUCCESSFUL;

	do 
	{
		g_pFilterCtl = (PMY_FILTER_CONTROL)ExAllocatePoolWithTag(NonPagedPool, \
			sizeof(MY_FILTER_CONTROL), \
			SHITDRV_TAG);
	} while (NULL == g_pFilterCtl);
	RtlZeroMemory(g_pFilterCtl,sizeof(MY_FILTER_CONTROL));

    RtlInitUnicodeString(&UniSymLinkName,L"\\??\\c:");
    Status = QuerySymbolicLink(&UniSymLinkName,&g_pFilterCtl->UniSandBoxPath);
    if (NT_SUCCESS(Status))
    {
        RtlUnicodeStringCatString(&g_pFilterCtl->UniSandBoxPath,L"\\sandbox");
    }
    else
    {
        RtlInitUnicodeString(&g_pFilterCtl->UniSandBoxPath, L"\\Device\\HarddiskVolume1\\sandbox");
    }
    RtlInitUnicodeString(&g_pFilterCtl->UniFloppy,L"\\Device\\Floppy");
	g_pFilterCtl->FileFltAccessCtl.ulIsInitialize = TRUE;
    if (g_pFilterCtl->FileFltAccessCtl.ulIsInitialize)
    {
        Status = FltRegisterFilter(pDrvObj,&MiniMonitorRegistration,&g_pFilterCtl->pFilterHandle);
    }
	return Status;
}
NTSTATUS MiniFilterInstanceSetup(PCFLT_RELATED_OBJECTS FltObjects, \
								 FLT_INSTANCE_SETUP_FLAGS Flags, \
								 DEVICE_TYPE VolumeDeviceType, \
								 FLT_FILESYSTEM_TYPE VolumeFilesystemType)
{
	return STATUS_SUCCESS;
}
NTSTATUS MiniFilterUnload(FLT_FILTER_UNLOAD_FLAGS Flags)
{
	return STATUS_SUCCESS;
}
VOID MiniFilterInstanceTeardownStart(PCFLT_RELATED_OBJECTS FltObjects, \
									 FLT_INSTANCE_TEARDOWN_FLAGS Reason)
{

}
