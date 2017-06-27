//#include <ntddk.h>
//#include "InjectApcThread.h"
//
//BOOLEAN InjectApcThread()
//{
//	NTSTATUS Status;
//	PSYSTEM_PROCESS_INFORMATION pSystemProcessInfo;
//	ULONG_PTR ulLength;
//
//	pSystemProcessInfo = NULL;
//	ulLength = 0;
//
//	while (TRUE)
//	{
//		Status = ZwQuerySystemInformation(SystemProcessInformation,pSystemProcessInfo,ulLength,&ulLength);
//		if (Status == STATUS_INFO_LENGTH_MISMATCH && ulLength)
//		{
//			if (pSystemProcessInfo)
//			{
//				ExFreePoolWithTag(pSystemProcessInfo,INJECTOR_APC_TAG);
//			}
//			pSystemProcessInfo = ExAllocatePoolWithTag(NonPagedPool,ulLength,INJECTOR_APC_TAG);
//			do 
//			{
//				pSystemProcessInfo = ExAllocatePoolWithTag(NonPagedPool,ulLength,INJECTOR_APC_TAG);
//			} while (NULL == pSystemProcessInfo);
//		}
//		else
//		{
//			break;
//		}
//	}
//	do {
//		if (info->UniqueProcessId == pid)
//		{
//			PSYSTEM_THREAD_INFORMATION thread_info;
//			thread_info = (void *)(info + 1);
//
//			if (thread_info->ThreadState == 14)
//			{
//				Status = PsLookupThreadByThreadId(thread_info->ClientId.UniqueThread, &thread);
//				if (NT_SUCCESS(ns))
//				{		
//					rc = insert_thread_apc(remote_path, thread, is_wow64);
//					break;
//				}
//			}
//
//			while (i < info->NumberOfThreads)
//			{
//				Status = PsLookupThreadByThreadId(thread_info->ClientId.UniqueThread, &thread);
//				if (NT_SUCCESS(ns))
//				{
//					rc |= insert_thread_apc(remote_path, thread, is_wow64);
//				}
//				i++;
//				thread_info++;
//			}
//		}
//		info = (info->NextEntryOffset == 0) ? NULL : (void *)((char *)info + info->NextEntryOffset);
//	} while (info);
//
//}