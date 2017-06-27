#include "ShitDrv.h"
#include "ProcessList.h"
#include "UtilsApi.h"
#include "DrvFunction.h"

BOOLEAN GetFileMd5Sum(PWCHAR pFilePath,PUCHAR pOutMd5Sum)
{
	BOOLEAN bRet;
	UNICODE_STRING UniFileName;
	NTSTATUS Status;

	Status = STATUS_UNSUCCESSFUL;
	bRet = FALSE;

	RtlInitUnicodeString(&UniFileName,pFilePath);
	if (g_pFilterCtl->FileFltAccessCtl.ulIsStartFilter && \
		NULL != g_pFilterCtl->pFltInstance)
	{
		Status = FltCalcFileDatMd5(g_pFilterCtl->pFilterHandle, \
			g_pFilterCtl->pFltInstance, \
			&UniFileName, \
			pOutMd5Sum);
		if (NT_SUCCESS(Status))
		{
			bRet = TRUE;
		}
	}
	else
	{
		Status = CalcFileDatMd5(&UniFileName, \
			pOutMd5Sum);
		if (NT_SUCCESS(Status))
		{
			bRet = TRUE;
		}
	}
	return bRet;
}
BOOLEAN SetCurrentProcess(HANDLE Pid)
{
	PVOID pProcess;

	pProcess = NULL;
	
	if (0 == Pid)
	{
		return FALSE;
	}
	if (g_pFilterCtl->FileFltAccessCtl.ulIsInitialize)
	{
		if (NT_SUCCESS(PsLookupProcessByProcessId(Pid,(PEPROCESS*)&pProcess)))
		{
			g_pFilterCtl->pMyProc = pProcess;
			ObDereferenceObject(pProcess);
			return TRUE;
		}
	}
	return FALSE;
}
BOOLEAN SetCurClientPath(WCHAR *wPath)
{
    if (NULL == wPath)
    {
        return FALSE;
    }
    if (g_pFilterCtl->FileFltAccessCtl.ulIsInitialize)
    {
		RtlInitUnicodeString(&g_pFilterCtl->UniCurPath,wPath);
        return TRUE;
    }
    return FALSE;
}
BOOLEAN SetSandboxPath(PWCHAR pSandboxPath)
{
	if (NULL == pSandboxPath)
	{
		if (g_pFilterCtl->FileFltAccessCtl.ulIsInitialize)
		{
			RtlInitUnicodeString(&g_pFilterCtl->UniSandBoxPath,pSandboxPath);
			return TRUE;
		}
	}
	return FALSE;
}
BOOLEAN SetSandBoxProcName(PCHAR pImagName)
{
	NTSTATUS Status;
	ANSI_STRING AnsiName;
	PSANDBOX_PROCESS_LIST pSandboxList;
	UNICODE_STRING UniProcName;
	//KIRQL OldIrql;

	pSandboxList = NULL;
	UniProcName.Buffer = NULL;
	Status = STATUS_SUCCESS;

	RtlInitAnsiString(&AnsiName,pImagName);
	RtlAnsiStringToUnicodeString(&UniProcName,&AnsiName,TRUE);
	pSandboxList = AllocateSandboxProcList();
	if (NULL == pSandboxList)
	{
		if (UniProcName.Buffer)
		{
			RtlFreeUnicodeString(&UniProcName);
			UniProcName.Buffer = NULL;
		}
		return FALSE;
	}
	RtlZeroMemory(pSandboxList,sizeof(SANDBOX_PROCESS_LIST));
	pSandboxList->UniProcessName.MaximumLength = UniProcName.MaximumLength;
	Status = AllocateUnicodeString(&pSandboxList->UniProcessName);
	if (NT_ERROR(Status))
	{
		if (UniProcName.Buffer)
		{
			RtlFreeUnicodeString(&UniProcName);
			UniProcName.Buffer = NULL;
		}
		return FALSE;
	}
	RtlCopyMemory(pSandboxList->UniProcessName.Buffer, \
		UniProcName.Buffer, \
		UniProcName.Length);
	pSandboxList->UniProcessName.Length = UniProcName.Length;
	InitializeListHead(&pSandboxList->Next);
	//OldIrql = GetSandboxLock(&g_SandboxSpinLock);
	//InsertTailList(&g_SandboxProcList,&pParentProcList->Next);
	//InsertHeadList(&g_SandboxProcList,&pParentProcList->Next);
	ExInterlockedInsertHeadList(&g_SandboxProcList,&pSandboxList->Next,&g_SandboxSpinLock);
	//ReleaseSandboxLock(&g_SandboxSpinLock,OldIrql);
	if (UniProcName.Buffer)
	{
		RtlFreeUnicodeString(&UniProcName);
		UniProcName.Buffer = NULL;
	}
	return TRUE;
}