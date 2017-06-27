#include "ShitDrv.h"
#include "UtilsApi.h"
#include "RecordInfo.h"
#include "RegisterCallBack.h"
#include "ProcessNotify.h"
#include "RegisterOperation.h"

NTSTATUS FilterPostCreateKey(PVOID pCallbackContext,PVOID Argument2)
{
	NTSTATUS Status;
	HANDLE hProcessId;
	WCHAR wProcessFullPath[MAX_PATH];
	PREG_POST_CREATE_KEY_INFORMATION pRegPostCreateKeyInfo;
	PRECORD_LIST pRecordList;
	HANDLE hKey;
	BOOLEAN bIsCreated;
	OBJECT_ATTRIBUTES ObjKey;
	ULONG ulDisposition;
	WCHAR wOperation[50];

	hKey = NULL;
	bIsCreated = FALSE;
	ulDisposition = 0;
	RtlZeroMemory(wOperation,sizeof(WCHAR) * 50);

	if (FALSE == g_pFilterCtl->FileFltAccessCtl.ulIsStartFilter || \
		FALSE == g_pFilterCtl->FileFltAccessCtl.ulIsInitialize)
	{
		return STATUS_SUCCESS;
	}
	if (KeGetCurrentIrql() > APC_LEVEL || \
		ExGetPreviousMode() == KernelMode)
	{
		return STATUS_SUCCESS;
	}
	if (g_pFilterCtl->pMyProc == PsGetCurrentProcess())
	{
		return STATUS_SUCCESS;
	}
	hProcessId = PsGetCurrentProcessId();
	if (FALSE == IsSandboxProcess(hProcessId))
	{
		return STATUS_SUCCESS;
	}
	pRegPostCreateKeyInfo = NULL;
	Status = STATUS_SUCCESS;
	RtlZeroMemory(wProcessFullPath,sizeof(WCHAR) * MAX_PATH);

	do 
	{   
		if (Argument2 == NULL)
		{
			break;
		}
		pRegPostCreateKeyInfo = (PREG_POST_CREATE_KEY_INFORMATION)Argument2;

		Status = GetCurProcFullPath(g_pFilterCtl->pFilterHandle, \
			g_pFilterCtl->pFltInstance, \
			wProcessFullPath, \
			MAX_PATH);
		if (NT_ERROR(Status))
		{
			break;
		}
		InitializeObjectAttributes(&ObjKey, \
			pRegPostCreateKeyInfo->CompleteName, \
			OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, \
			NULL, \
			NULL);
		Status = ZwCreateKey(&hKey, \
			KEY_ALL_ACCESS, \
			&ObjKey, \
			0, \
			NULL, \
			REG_OPTION_NON_VOLATILE, \
			&ulDisposition);
		if (NT_SUCCESS(Status))
		{
			if (ulDisposition == REG_CREATED_NEW_KEY)
			{
				bIsCreated = TRUE;
			}
			else
			{
				bIsCreated = FALSE;
			}
		}
		if (hKey)
		{
			ZwClose(hKey);
		}
		if (bIsCreated)
		{
			RtlCopyMemory(wOperation,L"创建键",wcslen(L"创建键") * sizeof(WCHAR));
		}
		else
		{
			RtlCopyMemory(wOperation,L"打开键",wcslen(L"打开键") * sizeof(WCHAR));
		}
		pRecordList = AllocateRecordListDat();
		if (pRecordList && InitializeRecordDat(pRecordList, \
			wProcessFullPath, \
			(ULONG)wcslen(wProcessFullPath) * sizeof(WCHAR), \
			pRegPostCreateKeyInfo->CompleteName->Buffer, \
			pRegPostCreateKeyInfo->CompleteName->Length, \
			wOperation, \
			(ULONG)wcslen(wOperation) * sizeof(WCHAR), \
			hProcessId, \
			0, \
			0, \
			0, \
			0, \
			0, \
			0, \
			IsRegister))
		{
			InitializeListHead(&pRecordList->RecordList);
			ExInterlockedInsertTailList(&g_RecordListHead,&pRecordList->RecordList,&g_RecordSpinLock);
			KeSetEvent(g_pEventWaitKrl,IO_NO_INCREMENT,FALSE);
			KeClearEvent(g_pEventWaitKrl);
		}
	} while (0);
	return STATUS_SUCCESS;

}
NTSTATUS FilterPostCreateKeyEx(PVOID pCallbackContext,PVOID Argument2,ULONG ulType)
{
	NTSTATUS Status;
	HANDLE hProcessId;
	PREG_POST_OPERATION_INFORMATION pPostInfo;
	HANDLE hKey;
	BOOLEAN bIsCreated;
	ULONG ulDisposition;
	WCHAR wOperation[50];
	WCHAR wProcessFullPath[MAX_PATH];

	hKey = NULL;
	bIsCreated = FALSE;
	ulDisposition = 0;
	Status = STATUS_SUCCESS;
	pPostInfo = NULL;
	RtlZeroMemory(wOperation,sizeof(WCHAR) * 50);
	RtlZeroMemory(wProcessFullPath,sizeof(WCHAR) * MAX_PATH);


	if (FALSE == g_pFilterCtl->FileFltAccessCtl.ulIsStartFilter || \
		FALSE == g_pFilterCtl->FileFltAccessCtl.ulIsInitialize)
	{
		return STATUS_SUCCESS;
	}
	if (KeGetCurrentIrql() > APC_LEVEL || \
		ExGetPreviousMode() == KernelMode)
	{
		return STATUS_SUCCESS;
	}
	if (g_pFilterCtl->pMyProc == PsGetCurrentProcess())
	{
		return STATUS_SUCCESS;
	}
	hProcessId = PsGetCurrentProcessId();
	if (FALSE == IsSandboxProcess(hProcessId))
	{
		return STATUS_SUCCESS;
	}
	do 
	{   
		if (Argument2 == NULL)
		{
			break;
		}
		pPostInfo = (PREG_POST_OPERATION_INFORMATION)Argument2;
		Status = GetCurProcFullPath(g_pFilterCtl->pFilterHandle, \
			g_pFilterCtl->pFltInstance, \
			wProcessFullPath, \
			MAX_PATH);
		if (NT_ERROR(Status))
		{
			break;
		}
		Status = FilterPost(pPostInfo,ulType);
	} while (0);
	return STATUS_SUCCESS;
}

NTSTATUS FilterPreCreateKey(PVOID pCallbackContext,PVOID Argument2)
{
	NTSTATUS Status;
	HANDLE hProcessId;
	WCHAR wProcessFullPath[MAX_PATH];
	PREG_PRE_CREATE_KEY_INFORMATION pRegCreateKeyInfo;
	PRECORD_LIST pRecordList;
	HANDLE hKey;
	BOOLEAN bIsCreated;
	OBJECT_ATTRIBUTES ObjKey;
	ULONG ulDisposition;
	WCHAR wOperation[50];

	hKey = NULL;
	bIsCreated = FALSE;
	ulDisposition = 0;
	RtlZeroMemory(wOperation,sizeof(WCHAR) * 50);

	if (FALSE == g_pFilterCtl->FileFltAccessCtl.ulIsStartFilter || \
		FALSE == g_pFilterCtl->FileFltAccessCtl.ulIsInitialize)
	{
		return STATUS_SUCCESS;
	}
	if (KeGetCurrentIrql() > APC_LEVEL || \
		ExGetPreviousMode() == KernelMode)
	{
		return STATUS_SUCCESS;
	}
	if (g_pFilterCtl->pMyProc == PsGetCurrentProcess())
	{
		return STATUS_SUCCESS;
	}
	hProcessId = PsGetCurrentProcessId();
	if (FALSE == IsSandboxProcess(hProcessId))
	{
		return STATUS_SUCCESS;
	}
	pRegCreateKeyInfo = NULL;
	Status = STATUS_SUCCESS;
	RtlZeroMemory(wProcessFullPath,sizeof(WCHAR) * MAX_PATH);

	do 
	{   
		if (Argument2 == NULL)
		{
			break;
		}
		pRegCreateKeyInfo = (PREG_PRE_CREATE_KEY_INFORMATION)Argument2;

		Status = GetCurProcFullPath(g_pFilterCtl->pFilterHandle, \
			g_pFilterCtl->pFltInstance, \
			wProcessFullPath, \
			MAX_PATH);
		if (NT_ERROR(Status))
		{
			break;
		}
		InitializeObjectAttributes(&ObjKey, \
			pRegCreateKeyInfo->CompleteName, \
			OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, \
			NULL, \
			NULL);
		Status = ZwCreateKey(&hKey, \
			KEY_ALL_ACCESS, \
			&ObjKey, \
			0, \
			NULL, \
			REG_OPTION_NON_VOLATILE, \
			&ulDisposition);
		if (NT_SUCCESS(Status))
		{
			if (ulDisposition == REG_CREATED_NEW_KEY)
			{
				bIsCreated = TRUE;
			}
			else
			{
				bIsCreated = FALSE;
			}
		}
		if (hKey)
		{
			ZwClose(hKey);
		}
		if (bIsCreated)
		{
			RtlCopyMemory(wOperation,L"创建键",wcslen(L"创建键") * sizeof(WCHAR));
		}
		else
		{
			RtlCopyMemory(wOperation,L"打开键",wcslen(L"打开键") * sizeof(WCHAR));
		}
		pRecordList = AllocateRecordListDat();
		if (pRecordList && InitializeRecordDat(pRecordList, \
			wProcessFullPath, \
			(ULONG)wcslen(wProcessFullPath) * sizeof(WCHAR), \
			pRegCreateKeyInfo->CompleteName->Buffer, \
			pRegCreateKeyInfo->CompleteName->Length, \
			wOperation, \
			(ULONG)wcslen(wOperation) * sizeof(WCHAR), \
			hProcessId, \
			0, \
			0, \
			0, \
			0, \
			0, \
			0, \
			IsRegister))
		{
			InitializeListHead(&pRecordList->RecordList);
			ExInterlockedInsertTailList(&g_RecordListHead,&pRecordList->RecordList,&g_RecordSpinLock);
			KeSetEvent(g_pEventWaitKrl,IO_NO_INCREMENT,FALSE);
			KeClearEvent(g_pEventWaitKrl);
		}
	} while (0);
	return STATUS_SUCCESS;

}
NTSTATUS FilterPreCreateKeyEx(PVOID pCallbackContext,PVOID Argument2,ULONG ulType)
{
	NTSTATUS Status;
	HANDLE hProcessId;
	PREG_CREATE_KEY_INFORMATION pCreateKeyInfo;
	PRECORD_LIST pRecordList;
	HANDLE hKey;
	BOOLEAN bIsCreated;
	OBJECT_ATTRIBUTES ObjKey;
	ULONG ulDisposition;
	WCHAR wOperation[50];
	WCHAR wProcessFullPath[MAX_PATH];

	hKey = NULL;
	bIsCreated = FALSE;
	ulDisposition = 0;
	Status = STATUS_SUCCESS;
	pCreateKeyInfo = NULL;
	RtlZeroMemory(wOperation,sizeof(WCHAR) * 50);
	RtlZeroMemory(wProcessFullPath,sizeof(WCHAR) * MAX_PATH);


	if (FALSE == g_pFilterCtl->FileFltAccessCtl.ulIsStartFilter || \
		FALSE == g_pFilterCtl->FileFltAccessCtl.ulIsInitialize)
	{
		return STATUS_SUCCESS;
	}
	if (KeGetCurrentIrql() > APC_LEVEL || \
		ExGetPreviousMode() == KernelMode)
	{
		return STATUS_SUCCESS;
	}
	if (g_pFilterCtl->pMyProc == PsGetCurrentProcess())
	{
		return STATUS_SUCCESS;
	}
	hProcessId = PsGetCurrentProcessId();
	if (FALSE == IsSandboxProcess(hProcessId))
	{
		return STATUS_SUCCESS;
	}
	do 
	{   
		if (Argument2 == NULL)
		{
			break;
		}
		pCreateKeyInfo = (PREG_CREATE_KEY_INFORMATION)Argument2;
		Status = GetCurProcFullPath(g_pFilterCtl->pFilterHandle, \
			g_pFilterCtl->pFltInstance, \
			wProcessFullPath, \
			MAX_PATH);
		if (NT_ERROR(Status))
		{
			break;
		}
		InitializeObjectAttributes(&ObjKey, \
			pCreateKeyInfo->CompleteName, \
			OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, \
			NULL, \
			NULL);
		Status = ZwCreateKey(&hKey, \
			KEY_ALL_ACCESS, \
			&ObjKey, \
			0, \
			NULL, \
			REG_OPTION_NON_VOLATILE, \
			&ulDisposition);
		if (NT_SUCCESS(Status))
		{
			if (ulDisposition == REG_CREATED_NEW_KEY)
			{
				bIsCreated = TRUE;
			}
			else
			{
				bIsCreated = FALSE;
			}
		}
		if (hKey)
		{
			ZwClose(hKey);
		}
		if (bIsCreated)
		{
			RtlCopyMemory(wOperation,L"创建键",wcslen(L"创建键") * sizeof(WCHAR));
		}
		else
		{
			RtlCopyMemory(wOperation,L"打开键",wcslen(L"打开键") * sizeof(WCHAR));
		}
		pRecordList = AllocateRecordListDat();
		if (pRecordList && InitializeRecordDat(pRecordList, \
			wProcessFullPath, \
			(ULONG)wcslen(wProcessFullPath) * sizeof(WCHAR), \
			pCreateKeyInfo->CompleteName->Buffer, \
			pCreateKeyInfo->CompleteName->Length, \
			wOperation, \
			(ULONG)wcslen(wOperation) * sizeof(WCHAR), \
			hProcessId, \
			0, \
			0, \
			0, \
			0, \
			0, \
			0, \
			IsRegister))
		{
			InitializeListHead(&pRecordList->RecordList);
			ExInterlockedInsertTailList(&g_RecordListHead,&pRecordList->RecordList,&g_RecordSpinLock);
			KeSetEvent(g_pEventWaitKrl,IO_NO_INCREMENT,FALSE);
			KeClearEvent(g_pEventWaitKrl);
		}
	} while (0);
	return STATUS_SUCCESS;
}
NTSTATUS FilterPostSetKey(PVOID pCallbackContext,PVOID Argument2,ULONG ulType)
{
	NTSTATUS Status;
	HANDLE hProcessId;
	PREG_POST_OPERATION_INFORMATION pPostOperationInfo;

	pPostOperationInfo = NULL;
	Status = STATUS_SUCCESS;

	if (FALSE == g_pFilterCtl->FileFltAccessCtl.ulIsStartFilter || \
		FALSE == g_pFilterCtl->FileFltAccessCtl.ulIsInitialize)
	{
		return STATUS_SUCCESS;
	}
	if (KeGetCurrentIrql() > APC_LEVEL || \
		ExGetPreviousMode() == KernelMode)
	{
		return STATUS_SUCCESS;
	}
	if (g_pFilterCtl->pMyProc == PsGetCurrentProcess())
	{
		return STATUS_SUCCESS;
	}
	hProcessId = PsGetCurrentProcessId();
	if (FALSE == IsSandboxProcess(hProcessId))
	{
		return STATUS_SUCCESS;
	}

	do 
	{
		if (Argument2 == NULL)
		{
			break;
		}
		pPostOperationInfo = (PREG_POST_OPERATION_INFORMATION)Argument2;
		if (pPostOperationInfo->Object && \
			NT_SUCCESS(pPostOperationInfo->Status))
		{
			Status = FilterPost(pPostOperationInfo,ulType);
		}
	} while (0);
	return Status;
}
NTSTATUS FilterPostSetInformationKey(PVOID pCallbackContext,PVOID Argument2,ULONG ulType)
{
	NTSTATUS Status;
	PREG_POST_OPERATION_INFORMATION pPostOperationInfo;
	HANDLE hProcessId;

	pPostOperationInfo = NULL;
	Status = STATUS_SUCCESS;

	if (FALSE == g_pFilterCtl->FileFltAccessCtl.ulIsStartFilter || \
		FALSE == g_pFilterCtl->FileFltAccessCtl.ulIsInitialize)
	{
		return STATUS_SUCCESS;
	}
	if (KeGetCurrentIrql() > APC_LEVEL || \
		ExGetPreviousMode() == KernelMode)
	{
		return STATUS_SUCCESS;
	}
	if (g_pFilterCtl->pMyProc == PsGetCurrentProcess())
	{
		return STATUS_SUCCESS;
	}
	hProcessId = PsGetCurrentProcessId();
	if (FALSE == IsSandboxProcess(hProcessId))
	{
		return STATUS_SUCCESS;
	}

	do 
	{
		if (Argument2 == NULL)
		{
			break;
		}
		pPostOperationInfo = (PREG_POST_OPERATION_INFORMATION)Argument2;
		if (pPostOperationInfo->Object && \
			NT_SUCCESS(pPostOperationInfo->Status))
		{
			Status = FilterPost(pPostOperationInfo,ulType);
		}
	} while (0);
	return Status;
}
NTSTATUS FilterPostRenNameKey(PVOID pCallbackContext,PVOID Argument2,ULONG ulType)
{
	NTSTATUS Status;
	PREG_POST_OPERATION_INFORMATION pPostOperationInfo;
	HANDLE hProcessId;

	pPostOperationInfo = NULL;
	Status = STATUS_SUCCESS;

	if (FALSE == g_pFilterCtl->FileFltAccessCtl.ulIsStartFilter || \
		FALSE == g_pFilterCtl->FileFltAccessCtl.ulIsInitialize)
	{
		return STATUS_SUCCESS;
	}
	if (KeGetCurrentIrql() > APC_LEVEL || \
		ExGetPreviousMode() == KernelMode)
	{
		return STATUS_SUCCESS;
	}
	if (g_pFilterCtl->pMyProc == PsGetCurrentProcess())
	{
		return STATUS_SUCCESS;
	}
	hProcessId = PsGetCurrentProcessId();
	if (FALSE == IsSandboxProcess(hProcessId))
	{
		return STATUS_SUCCESS;
	}

	do 
	{
		if (Argument2 == NULL)
		{
			break;
		}
		pPostOperationInfo = (PREG_POST_OPERATION_INFORMATION)Argument2;
		if (pPostOperationInfo->Object && \
			NT_SUCCESS(pPostOperationInfo->Status))
		{
			Status = FilterPost(pPostOperationInfo,ulType);
		}
	} while (0);
	return Status;
}
NTSTATUS FilterPostDeleteKey(PVOID pCallbackContext,PVOID Argument2,ULONG ulType)
{
	NTSTATUS Status;
	PREG_POST_OPERATION_INFORMATION pPostOperationInfo;
	HANDLE hProcessId;

	pPostOperationInfo = NULL;
	Status = STATUS_SUCCESS;

	if (FALSE == g_pFilterCtl->FileFltAccessCtl.ulIsStartFilter || \
		FALSE == g_pFilterCtl->FileFltAccessCtl.ulIsInitialize)
	{
		return STATUS_SUCCESS;
	}
	if (KeGetCurrentIrql() > APC_LEVEL || \
		ExGetPreviousMode() == KernelMode)
	{
		return STATUS_SUCCESS;
	}
	if (g_pFilterCtl->pMyProc == PsGetCurrentProcess())
	{
		return STATUS_SUCCESS;
	}
	hProcessId = PsGetCurrentProcessId();
	if (FALSE == IsSandboxProcess(hProcessId))
	{
		return STATUS_SUCCESS;
	}

	do 
	{
		if (Argument2 == NULL)
		{
			break;
		}
		pPostOperationInfo = (PREG_POST_OPERATION_INFORMATION)Argument2;
		if (pPostOperationInfo->Object && \
			NT_SUCCESS(pPostOperationInfo->Status))
		{
			Status = FilterPost(pPostOperationInfo,ulType);
		}
	} while (0);
	return Status;
}
NTSTATUS FilterPostDeleteValueKey(PVOID pCallbackContext,PVOID Argument2,ULONG ulType)
{
	NTSTATUS Status;
	PREG_POST_OPERATION_INFORMATION pPostOperationInfo;
	HANDLE hProcessId;

	pPostOperationInfo = NULL;
	Status = STATUS_SUCCESS;

	if (FALSE == g_pFilterCtl->FileFltAccessCtl.ulIsStartFilter || \
		FALSE == g_pFilterCtl->FileFltAccessCtl.ulIsInitialize)
	{
		return STATUS_SUCCESS;
	}
	if (KeGetCurrentIrql() > APC_LEVEL || \
		ExGetPreviousMode() == KernelMode)
	{
		return STATUS_SUCCESS;
	}
	if (g_pFilterCtl->pMyProc == PsGetCurrentProcess())
	{
		return STATUS_SUCCESS;
	}
	hProcessId = PsGetCurrentProcessId();
	if (FALSE == IsSandboxProcess(hProcessId))
	{
		return STATUS_SUCCESS;
	}

	do 
	{
		if (Argument2 == NULL)
		{
			break;
		}
		pPostOperationInfo = (PREG_POST_OPERATION_INFORMATION)Argument2;
		if (pPostOperationInfo->Object && \
			NT_SUCCESS(pPostOperationInfo->Status))
		{
			Status = FilterPost(pPostOperationInfo,ulType);
		}
	} while (0);
	return Status;
}
NTSTATUS FilterPost(PREG_POST_OPERATION_INFORMATION pPostInfo,ULONG ulOperationType)
{
	NTSTATUS Status;
	PREG_POST_OPERATION_INFORMATION pPostOperationInfo;
	WCHAR wProcessFullPath[MAX_PATH];
	WCHAR wOperation[50];
	UNICODE_STRING UniRegistryPath;
	PRECORD_LIST pRecordList;
	HANDLE hPid;

	Status = STATUS_SUCCESS;
	pPostOperationInfo = NULL;
	UniRegistryPath.Buffer = NULL;
	UniRegistryPath.Length = 0;
	pRecordList = NULL;
	hPid = 0;
	UniRegistryPath.MaximumLength = MAX_PATH * 4;

	do 
	{
		UniRegistryPath.Buffer = ExAllocatePoolWithTag(NonPagedPool,sizeof(WCHAR) * MAX_PATH * 4,'KClF');
	} while (UniRegistryPath.Buffer == NULL);
	RtlZeroMemory(UniRegistryPath.Buffer,sizeof(WCHAR) * MAX_PATH * 4);
	RtlZeroMemory(wProcessFullPath,sizeof(WCHAR) * MAX_PATH);
	RtlZeroMemory(wOperation,sizeof(WCHAR) * 50);

	do 
	{   
		if (pPostInfo == NULL)
		{
			break;
		}
		pPostOperationInfo = pPostInfo;
		if (pPostOperationInfo->Object && \
			NT_SUCCESS(pPostOperationInfo->Status))
		{
			hPid = PsGetCurrentProcessId();
			Status = GetCurProcFullPath(g_pFilterCtl->pFilterHandle, \
				g_pFilterCtl->pFltInstance, \
				wProcessFullPath, \
				MAX_PATH);
			if (NT_ERROR(Status))
			{
				break;
			}
			if (GetRegistryObjectCompleteName(&UniRegistryPath,NULL,pPostOperationInfo->Object))
			{
				switch(ulOperationType)
				{
				case RegNtPostSetValueKey:
					RtlCopyMemory(wOperation,L"修改键值",wcslen(L"修改键值") * sizeof(WCHAR));
					break;
				case RegNtPostSetInformationKey:
					RtlCopyMemory(wOperation,L"设置",wcslen(L"设置") * sizeof(WCHAR));
					break;
				case RegNtPostRenameKey:
					RtlCopyMemory(wOperation,L"重命名",wcslen(L"重命名") * sizeof(WCHAR));
					break;
				case RegNtPostDeleteKey:
					RtlCopyMemory(wOperation,L"删除",wcslen(L"删除") * sizeof(WCHAR));
				case RegNtPostDeleteValueKey:
					RtlCopyMemory(wOperation,L"删除键",wcslen(L"删除键") * sizeof(WCHAR));
					break;
				default:
					RtlCopyMemory(wOperation,L"Unknow",wcslen(L"Unknow") * sizeof(WCHAR));
					break;
				}
				DbgPrint("Process : %ws  %ws  %wZ\n", \
					wProcessFullPath, \
					wOperation, \
					&UniRegistryPath);
				pRecordList = AllocateRecordListDat();
				if (pRecordList && InitializeRecordDat(pRecordList, \
					wProcessFullPath, \
					(ULONG)wcslen(wProcessFullPath) * sizeof(WCHAR), \
					UniRegistryPath.Buffer, \
					UniRegistryPath.Length, \
					wOperation, \
					(ULONG)wcslen(wOperation) * sizeof(WCHAR), \
					hPid, \
					ulOperationType, \
					0, \
					0, \
					0, \
					0, \
					0, \
					IsRegister))
				{
					InitializeListHead(&pRecordList->RecordList);
					ExInterlockedInsertTailList(&g_RecordListHead,&pRecordList->RecordList,&g_RecordSpinLock);
					KeSetEvent(g_pEventWaitKrl,IO_NO_INCREMENT,FALSE);
					KeClearEvent(g_pEventWaitKrl);
				}
			}
		}
	} while (0);
	if (UniRegistryPath.Buffer)
	{
		ExFreePoolWithTag(UniRegistryPath.Buffer,'KClF');
	}
	return STATUS_SUCCESS;
}