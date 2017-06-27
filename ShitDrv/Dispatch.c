#include "ShitDrv.h"
#include "InjectKtrapFrame.h"
#include "DrvFunction.h"
#include "Dispatch.h"

NTSTATUS Dispatch(PDEVICE_OBJECT pDevObj,PIRP pIrp)
{
    pIrp->IoStatus.Status = STATUS_SUCCESS;
    pIrp->IoStatus.Information = 0;
    
    IoCompleteRequest(pIrp,IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}
NTSTATUS ReadDispatch(PDEVICE_OBJECT pDevObj,PIRP pIrp)
{
    pIrp->IoStatus.Status = STATUS_SUCCESS;
    pIrp->IoStatus.Information = 0;
    
    IoCompleteRequest(pIrp,IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}
NTSTATUS DispatchIoCtl(PDEVICE_OBJECT pDevObj,PIRP pIrp)
{
	NTSTATUS Status;
	PIO_STACK_LOCATION pIrpStack;
	PUCHAR pInBuffer,pOutBuffer;
	ULONG ulInLength,ulOutLength,ulIoCtlCode,ulInfo;

	Status = STATUS_UNSUCCESSFUL;
	ulInfo = 0;

	pIrpStack = IoGetCurrentIrpStackLocation(pIrp);
	ulInLength = pIrpStack->Parameters.DeviceIoControl.InputBufferLength;
	pInBuffer = pIrp->AssociatedIrp.SystemBuffer;
	ulOutLength = pIrpStack->Parameters.DeviceIoControl.OutputBufferLength;
	pOutBuffer = pIrp->AssociatedIrp.SystemBuffer;
	ulIoCtlCode = pIrpStack->Parameters.DeviceIoControl.IoControlCode;

	switch(ulIoCtlCode)
	{
	case IOC(SetCurrentProcess):
		{
			IOP(SetCurrentProcess) *Params;

			if (ulInLength < sizeof(*Params))
			{
				break;
			}
			Params = (IOP(SetCurrentProcess)*)pIrp->AssociatedIrp.SystemBuffer;
			if (SetCurrentProcess(Params->ProcessId))
			{
				Status = STATUS_SUCCESS;
			}
		}
		break;
	case IOC(SetCurClientPath):
		{
			IOP(SetCurClientPath) *Params;

			if (ulInLength < sizeof(*Params))
			{
				break;
			}
			Params = (IOP(SetCurClientPath)*)pIrp->AssociatedIrp.SystemBuffer;
			if (SetCurClientPath(Params->wCurClientPath))
			{
				Status = STATUS_SUCCESS;
			}
		}
		break;
	case IOC(SetSandBoxProcName):
		{
			IOP(SetSandBoxProcName) *Params;

			if (ulInLength < sizeof(*Params))
			{
				break;
			}
			Params = (IOP(SetSandBoxProcName)*)pIrp->AssociatedIrp.SystemBuffer;
			if (SetSandBoxProcName(Params->ImageName))
			{
				Status = STATUS_SUCCESS;
			}
		}
		break;
	case IOC(GetFileMd5Sum):
		{
			IOP(GetFileMd5Sum) *Params;

			if (ulInLength < sizeof(*Params))
			{
				break;
			}
			Params = (IOP(GetFileMd5Sum)*)pIrp->AssociatedIrp.SystemBuffer;
			if (GetFileMd5Sum(Params->wImageName,Params->Md5Sum))
			{
				Status = STATUS_SUCCESS;
				ulInfo = sizeof(*Params);
			}
		}
		break;
	case IOC(InjectKtrapFrame):
		{
			IOP(InjectKtrapFrame) *Params;
			PINJECT_PROCESS_INFORMATION pInjectProcessInfo;

			if (ulInLength < sizeof(*Params))
			{
				break;
			}
			do 
			{
				pInjectProcessInfo = ExAllocatePoolWithTag(NonPagedPool,sizeof(INJECT_PROCESS_INFORMATION) + 260,'PasP');
			} while (NULL == pInjectProcessInfo);
			RtlZeroMemory(pInjectProcessInfo,sizeof(INJECT_PROCESS_INFORMATION) + 260);
			Params = (IOP(InjectKtrapFrame) *)pIrp->AssociatedIrp.SystemBuffer;
			if (Params->ulPid)
			{
				pInjectProcessInfo->ulPid = Params->ulPid;
			}
			else
			{
				RtlCopyMemory(pInjectProcessInfo->pInjectProcessName,Params->pInjectProcessName,strlen(Params->pInjectProcessName));
			}
			Status = InjectKtrapFrame(pInjectProcessInfo,Params->pInjectDllPath);
			if (pInjectProcessInfo)
			{
				ExFreePoolWithTag(pInjectProcessInfo,'PasP');
			}
		}
	default:
		Status = STATUS_INVALID_DEVICE_REQUEST;
		break;
	}
	pIrp->IoStatus.Status = Status;
	pIrp->IoStatus.Information = ulInfo;
	IoCompleteRequest(pIrp,IO_NO_INCREMENT);
	return Status;
}