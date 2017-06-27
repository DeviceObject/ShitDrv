#ifndef __PROCESS_LIST_H__
#define __PROCESS_LIST_H__

#define SANDBOX_PROCESS_TAG			'tpdS'
#define SANDBOX_PROC_RES_TAG		'srdS'
#define SANDBOX_PROC_STR_TAG		'tsdS'
typedef struct _SANDBOX_PROCESS_LIST
{
    LIST_ENTRY Next;
	HANDLE hProcessId;
    UNICODE_STRING UniProcessName;
}SANDBOX_PROCESS_LIST,*PSANDBOX_PROCESS_LIST;


void InitializeSandboxProcList();
PERESOURCE AllocateResource(VOID);
VOID FreeResource(PERESOURCE Resource);
PSANDBOX_PROCESS_LIST AllocateSandboxProcList();
NTSTATUS AllocateUnicodeString(PUNICODE_STRING String);
VOID FreeUnicodeString(PUNICODE_STRING String);
VOID AcquireResourceExclusive(PERESOURCE pResource);
VOID AcquireResourceShared(PERESOURCE pResource);
VOID ReleaseResource(PERESOURCE pResource);
BOOLEAN FreeSandboxProcList(PSANDBOX_PROCESS_LIST pSandboxProcList);

KIRQL GetSandboxLock(PKSPIN_LOCK pKspinLock);
void ReleaseSandboxLock(PKSPIN_LOCK pKspinLock,KIRQL Irql);


extern LIST_ENTRY g_SandboxProcList;
extern KSPIN_LOCK g_SandboxSpinLock;

#endif