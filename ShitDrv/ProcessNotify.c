#include "ShitDrv.h"
#include "ProcessList.h"
#include "UtilsApi.h"
#include "ProcessNotify.h"

BOOLEAN IsSandboxProcess(HANDLE hProcessId)
{
	BOOLEAN bRet;
	PLIST_ENTRY pCurList;
	PSANDBOX_PROCESS_LIST pSandboxProcList;

	bRet = FALSE;
	pCurList = NULL;
	pSandboxProcList = NULL;

	if (!IsListEmpty(&g_SandboxProcList))
	{
		pCurList = g_SandboxProcList.Flink;
		while (&g_SandboxProcList != pCurList)
		{
			pSandboxProcList = CONTAINING_RECORD(pCurList,SANDBOX_PROCESS_LIST,Next);
			if (pSandboxProcList->hProcessId != NULL)
			{
				if (hProcessId == pSandboxProcList->hProcessId)
				{
					bRet = TRUE;
					break;
				}
			}
			pCurList = pCurList->Flink;
		}
	}
	return bRet;
}
BOOLEAN IsFltParentProcess(HANDLE hParentId)
{
	BOOLEAN bRet;
	PLIST_ENTRY pCurList;
	PSANDBOX_PROCESS_LIST pSandboxProcList;

	bRet = FALSE;
	pCurList = NULL;
	pSandboxProcList = NULL;

	if (!IsListEmpty(&g_SandboxProcList))
	{
		pCurList = g_SandboxProcList.Flink;
		while (&g_SandboxProcList != pCurList)
		{
			pSandboxProcList = CONTAINING_RECORD(pCurList,SANDBOX_PROCESS_LIST,Next);
			if (pSandboxProcList->hProcessId != NULL)
			{
				if (hParentId == pSandboxProcList->hProcessId)
				{
					bRet = TRUE;
					break;
				}
			}
			pCurList = pCurList->Flink;
		}
	}
	return bRet;
}
void ProcessChildProcessId(HANDLE hProcessId)
{
	PVOID pEProc;
	NTSTATUS Status;
	UNICODE_STRING UniProcName;
	PLIST_ENTRY pCurList;
	PSANDBOX_PROCESS_LIST pSandboxProcList;
	PWCHAR pExist;

	UniProcName.Buffer = NULL;
	UniProcName.MaximumLength = 0;
	pCurList = NULL;
	pExist = NULL;
	pSandboxProcList = NULL;
	pEProc = NULL;

	do 
	{
		Status = PsLookupProcessByProcessId(hProcessId,(PEPROCESS *)&pEProc);
		if (NT_ERROR(Status))
		{
			break;
		}
		Status = GetProcessPath(pEProc,&UniProcName);
		if (NT_ERROR(Status))
		{
			break;
		}
		if (!IsListEmpty(&g_SandboxProcList))
		{
			pCurList = g_SandboxProcList.Flink;
			while (&g_SandboxProcList != pCurList)
			{
				pSandboxProcList = (PSANDBOX_PROCESS_LIST)pCurList;
				pExist = wcsstr(UniProcName.Buffer,pSandboxProcList->UniProcessName.Buffer);
				if (pExist)
				{
					if (!pSandboxProcList->hProcessId || \
						pSandboxProcList->hProcessId != hProcessId)
					{
						pSandboxProcList->hProcessId = hProcessId;
					}
					break;
				}
				pCurList = pCurList->Flink;
			}
		}
	} while (0);
	if (pEProc)
	{
		ObDereferenceObject(pEProc);
		pEProc = NULL;
	}
	if (UniProcName.Buffer)
	{
		ExFreePool(UniProcName.Buffer);
	}
}
void ProcessParentProcessId(HANDLE hProcessId)
{
	PVOID pEProc;
	NTSTATUS Status;
	UNICODE_STRING UniProcName;
	PLIST_ENTRY pCurList;
	PSANDBOX_PROCESS_LIST pSandboxProcList;
	BOOLEAN bIsExist;

	UniProcName.Buffer = NULL;
	UniProcName.MaximumLength = 0;
	pCurList = NULL;
	bIsExist = FALSE;
	pSandboxProcList = NULL;
	pEProc = NULL;


	do 
	{
		Status = PsLookupProcessByProcessId(hProcessId,(PEPROCESS *)&pEProc);
		if (NT_ERROR(Status))
		{
			break;
		}
		Status = GetProcessPath(pEProc,&UniProcName);
		if (NT_SUCCESS(Status))
		{
			break;
		}

		pSandboxProcList->UniProcessName.MaximumLength = UniProcName.MaximumLength;
		Status = AllocateUnicodeString(&pSandboxProcList->UniProcessName);
		if(NT_SUCCESS(Status))
		{
			break;
		}
		RtlCopyMemory(pSandboxProcList->UniProcessName.Buffer, \
			UniProcName.Buffer, \
			UniProcName.Length);
		pSandboxProcList->UniProcessName.Length = UniProcName.Length;
		pSandboxProcList->hProcessId = hProcessId;
		InitializeListHead(&pSandboxProcList->Next);
		ExInterlockedInsertHeadList(&g_SandboxProcList,&pSandboxProcList->Next,&g_SandboxSpinLock);
	} while (0);
	if (pEProc)
	{
		ObDereferenceObject(pEProc);
		pEProc = NULL;
	}
	if (UniProcName.Buffer)
	{
		ExFreePool(UniProcName.Buffer);
	}
}
VOID ProcessNotify(HANDLE ParentId,HANDLE ProcessId,BOOLEAN bCreate)
{
	if (bCreate)
	{
		if (g_pFilterCtl->FileFltAccessCtl.ulIsStartFilter != TRUE || \
			g_pFilterCtl->FileFltAccessCtl.ulIsInitialize != TRUE)
		{
			return;
		}
		if (IsFltParentProcess(ParentId) && \
			FALSE == IsSandboxProcess(ProcessId))
		{
			ProcessParentProcessId(ProcessId);
		}
		else
		{
			ProcessChildProcessId(ProcessId);
		}
	}
}

