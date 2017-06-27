#ifndef __DISPATCH_H__
#define __DISPATCH_H__

NTSTATUS Dispatch(PDEVICE_OBJECT pDevObj,PIRP pIrp);
NTSTATUS ReadDispatch(PDEVICE_OBJECT pDevObj,PIRP pIrp);
NTSTATUS DispatchIoCtl(PDEVICE_OBJECT pDevObj,PIRP pIrp);
#endif