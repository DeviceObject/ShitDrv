#include "ShitDrv.h"
#include "ProcessList.h"

LIST_ENTRY g_SandboxProcList;
KSPIN_LOCK g_SandboxSpinLock;

void InitializeSandboxProcList()
{
	InitializeListHead(&g_SandboxProcList);
	KeInitializeSpinLock(&g_SandboxSpinLock);
}
KIRQL GetSandboxLock(PKSPIN_LOCK pKspinLock)
{
	KIRQL OldIrql,LowerIrql;

	if (KeGetCurrentIrql() <= DISPATCH_LEVEL)
	{
		KeAcquireSpinLock(pKspinLock,&OldIrql);
	}
	else
	{
		KeRaiseIrql(DISPATCH_LEVEL,&LowerIrql);
		if (KeGetCurrentIrql() <= DISPATCH_LEVEL)
		{
			KeAcquireSpinLock(pKspinLock,&OldIrql);
		}
		KeLowerIrql(LowerIrql);
	}
	return OldIrql;
}
void ReleaseSandboxLock(PKSPIN_LOCK pKspinLock,KIRQL Irql)
{
	KIRQL LowerIrql;
	if (KeGetCurrentIrql() <= DISPATCH_LEVEL)
	{
		KeReleaseSpinLock(pKspinLock,Irql);
	}
	else
	{
		KeRaiseIrql(DISPATCH_LEVEL,&LowerIrql);
		if (KeGetCurrentIrql() <= DISPATCH_LEVEL)
		{
			KeReleaseSpinLock(pKspinLock,Irql);
		}
		KeLowerIrql(LowerIrql);
	}
	return;
}
PERESOURCE AllocateResource(VOID)
{
	return ExAllocatePoolWithTag(NonPagedPool,sizeof(ERESOURCE),SANDBOX_PROC_RES_TAG);
}
VOID FreeResource(PERESOURCE Resource)
{
	ExFreePoolWithTag(Resource,SANDBOX_PROC_RES_TAG);
}
NTSTATUS AllocateUnicodeString(PUNICODE_STRING String)
/*++
Routine Description:
    This routine allocates a unicode string
Arguments:
    String - supplies the size of the string to be allocated in the MaximumLength field
             return the unicode string
Return Value:
    STATUS_SUCCESS                  - success
    STATUS_INSUFFICIENT_RESOURCES   - failure
--*/
{
    PAGED_CODE();

    String->Buffer = ExAllocatePoolWithTag(NonPagedPool,String->MaximumLength,SANDBOX_PROC_STR_TAG);
    if (String->Buffer == NULL)
    {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
	RtlZeroMemory(String->Buffer,String->MaximumLength);
    String->Length = 0;
    return STATUS_SUCCESS;
}

VOID FreeUnicodeString(PUNICODE_STRING String)
/*++
Routine Description:
    This routine frees a unicode string
Arguments:
    String - supplies the string to be freed
Return Value:
    None
--*/
{
    PAGED_CODE();
    ExFreePoolWithTag(String->Buffer,SANDBOX_PROC_STR_TAG);
    String->Length = String->MaximumLength = 0;
    String->Buffer = NULL;
}
PSANDBOX_PROCESS_LIST AllocateSandboxProcList()
{
	PSANDBOX_PROCESS_LIST pSandboxProcList;

	pSandboxProcList = NULL;

	do 
	{
		pSandboxProcList = (PSANDBOX_PROCESS_LIST)ExAllocatePoolWithTag(NonPagedPool, \
			sizeof(SANDBOX_PROCESS_LIST), \
			SANDBOX_PROCESS_TAG);
	} while (NULL == pSandboxProcList);
	RtlZeroMemory(pSandboxProcList,sizeof(SANDBOX_PROCESS_LIST));
	pSandboxProcList->UniProcessName.Buffer = NULL;
	pSandboxProcList->UniProcessName.Length = 0;
	InitializeListHead(&pSandboxProcList->Next);
	return pSandboxProcList;
}
VOID AcquireResourceExclusive(PERESOURCE pResource)
{
	ASSERT(KeGetCurrentIrql() <= APC_LEVEL);
	ASSERT(ExIsResourceAcquiredExclusiveLite(pResource) || \
		!ExIsResourceAcquiredSharedLite(pResource));

	KeEnterCriticalRegion();

	(VOID)ExAcquireResourceExclusiveLite(pResource,TRUE);
}
VOID AcquireResourceShared(PERESOURCE pResource)
{
	ASSERT(KeGetCurrentIrql() <= APC_LEVEL);

	KeEnterCriticalRegion();

	(VOID)ExAcquireResourceSharedLite(pResource,TRUE);
}
VOID ReleaseResource(PERESOURCE pResource)
{
	ASSERT(KeGetCurrentIrql() <= APC_LEVEL);
	ASSERT(ExIsResourceAcquiredExclusiveLite(pResource) ||
		ExIsResourceAcquiredSharedLite(pResource));

	ExReleaseResourceLite(pResource);

	KeLeaveCriticalRegion();
}
BOOLEAN FreeSandboxProcList(PSANDBOX_PROCESS_LIST pSandboxProcList)
{
	if (NULL == pSandboxProcList)
	{
		return FALSE;
	}
	if (NULL != pSandboxProcList->UniProcessName.Buffer)
	{
		FreeUnicodeString(&pSandboxProcList->UniProcessName);
	}
	return TRUE;
}

