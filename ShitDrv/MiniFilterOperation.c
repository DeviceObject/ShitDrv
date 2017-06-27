#include "ShitDrv.h"
#include <ntstrsafe.h>
#include "UtilsApi.h"
#include "RecordInfo.h"
#include "ContextOperation.h"
#include "ProcessNotify.h"
#include "MiniFilterOperation.h"


FLT_POSTOP_CALLBACK_STATUS FltPostCreateFiles(PFLT_CALLBACK_DATA Data, \
											  PCFLT_RELATED_OBJECTS FltObjects, \
											  PVOID CompletionContext, \
											  FLT_POST_OPERATION_FLAGS Flags)
{
	NTSTATUS Status;
	UNICODE_STRING UniProcessName;
	FLT_POSTOP_CALLBACK_STATUS FltCallBackStatus;
	PFLT_FILE_NAME_INFORMATION pFltNameInfo;
	BOOLEAN bIsDirectory;
	HANDLE hProcessId;
	PRECORD_LIST pRecordList;
	WCHAR wOperation[50];
	ULONG ulNewCreate;
	BOOLEAN bIsSandboxFileExist;
	ULONG ulIndexFile;
	UNICODE_STRING UniSandBoxPath;
	BOOLEAN bIsExist;
	//UCHAR Md5Sum[16];
	PSHITDRV_STREAM_CONTEXT pStreamContext;
	BOOLEAN bIsStreamContextCreated;
	//CHAR ExistFileMd5Sum[MAX_PATH];
	//CHAR NowFileMd5Sum[MAX_PATH];
	BOOLEAN bIsCopyMd5Sum;
	ULONG ulDisp;
	//UNICODE_STRING UniExistFileMd5Sum;
	//UNICODE_STRING UniExistName;
	//WCHAR wIndexFile[50];


	pFltNameInfo = NULL;
	UniProcessName.Buffer = NULL;
	UniProcessName.MaximumLength = 0;
	bIsDirectory = FALSE;
	ulNewCreate = 0;
	hProcessId = 0;
	pRecordList = NULL;
	FltCallBackStatus = FLT_POSTOP_FINISHED_PROCESSING;
	bIsSandboxFileExist = FALSE;
	ulIndexFile = 0;
	UniSandBoxPath.Buffer = NULL;
	UniSandBoxPath.Length = 0;
	UniSandBoxPath.MaximumLength = 0;
	bIsExist = FALSE;
	pStreamContext = NULL;
	bIsStreamContextCreated = FALSE;
	bIsCopyMd5Sum = FALSE;
	ulDisp = 0;
	//UniExistFileMd5Sum.Buffer = NULL;
	//UniExistName.Buffer = NULL;

	if (FALSE == g_pFilterCtl->FileFltAccessCtl.ulIsStartFilter || \
		FALSE == g_pFilterCtl->FileFltAccessCtl.ulIsInitialize)
	{
		return FltCallBackStatus;
	}
	if ((Data->Iopb->IrpFlags & IRP_PAGING_IO) || \
		(Data->Iopb->IrpFlags & IRP_SYNCHRONOUS_PAGING_IO))
	{
		return FltCallBackStatus;
	}
	if (KeGetCurrentIrql() > APC_LEVEL || \
		NULL == Data || \
		NULL == FltObjects || \
		NULL == FltObjects->FileObject || \
		IRP_MJ_CREATE != Data->Iopb->MajorFunction || \
		/*STATUS_REPARSE == Data->IoStatus.Status || \*/
		ExGetPreviousMode() == KernelMode)
	{
		return FltCallBackStatus;
	}
	if (g_pFilterCtl->pMyProc == PsGetCurrentProcess())
	{
		return FltCallBackStatus;
	}
	hProcessId = PsGetCurrentProcessId();
	if (FALSE == IsSandboxProcess(hProcessId))
	{
		return FltCallBackStatus;
	}
	Status = FltIsDirectory(FltObjects->FileObject,FltObjects->Instance,&bIsDirectory);
	if (NT_SUCCESS(Status))
	{
		if (bIsDirectory)
		{
			return FltCallBackStatus;
		}
	}
	else
	{
		return FltCallBackStatus;
	}
	ulDisp = (Data->Iopb->Parameters.Create.Options >> 24) & 0xFF;
	if (ulDisp & FILE_SUPERSEDE || \
		ulDisp & FILE_OPEN || \
		ulDisp & FILE_CREATE || \
		ulDisp & FILE_OPEN_IF || \
		ulDisp & FILE_OVERWRITE || \
		ulDisp & FILE_OVERWRITE_IF)
	{
		if (NT_SUCCESS(Data->IoStatus.Status) && \
			(Data->Iopb->Parameters.Create.SecurityContext->DesiredAccess & FILE_GENERIC_WRITE))
		{
			if (Data->IoStatus.Information == FILE_CREATED)
			{
				ulNewCreate = FILE_CREATED;
			}
			else if (Data->IoStatus.Information == FILE_OPENED)
			{
				ulNewCreate = FILE_OPENED;
			}
			else if (Data->IoStatus.Information == FILE_OVERWRITTEN)
			{
				ulNewCreate = FILE_OVERWRITTEN;
			}
			else if (Data->IoStatus.Information == FILE_SUPERSEDED)
			{
				ulNewCreate = FILE_SUPERSEDED;
			}
			else
			{
				ulNewCreate = 0;
			}
		}
		else
		{
			return FltCallBackStatus;
		}
	}
	if (ulNewCreate)
	{
		do 
		{
			Status = FltGetFileNameInformation(Data, \
				FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_DEFAULT, \
				&pFltNameInfo);
			if (NT_SUCCESS(Status))
			{
				Status = FltParseFileNameInformation(pFltNameInfo);
				if (NT_ERROR(Status))
				{
					break;
				}
			}
			if (NT_ERROR(Status) || \
				NULL == pFltNameInfo)
			{
				break;
			}
			if (!RtlCompareUnicodeString(&pFltNameInfo->Name,&pFltNameInfo->Volume,TRUE))
			{
				break;
			}
			if (RtlPrefixUnicodeString(&g_pFilterCtl->UniFloppy,&pFltNameInfo->Name,TRUE))
			{
				break;
			}
			Status = GetProcessFullName(g_pFilterCtl->pFilterHandle, \
				FltObjects->Instance, \
				(PVOID)PsGetCurrentProcess(), \
				&UniProcessName);
			if (NT_SUCCESS(Status))
			{
				bIsExist = RtlPrefixUnicodeString(&g_pFilterCtl->UniSandBoxPath,&pFltNameInfo->Name,TRUE);
				if (bIsExist)
				{
					break;
				}
				if (ulNewCreate == FILE_CREATED)
				{
					Status = FindOrCreateStreamContext(Data,TRUE,&pStreamContext,&bIsStreamContextCreated);
					if (NT_ERROR(Status))
					{
						break;
					}
					ShitDrvAcquireResourceExclusive(pStreamContext->pResource);
					if (pStreamContext->bIsInit)
					{
						pStreamContext->UniFileName.MaximumLength = pFltNameInfo->Name.MaximumLength;
						pStreamContext->UniFileName.Buffer = NULL;
						ShitDrvAllocateUnicodeString(&pStreamContext->UniFileName);
						RtlCopyMemory(pStreamContext->UniFileName.Buffer, \
							pFltNameInfo->Name.Buffer, \
							pFltNameInfo->Name.Length);
						pStreamContext->UniFileName.Length = pFltNameInfo->Name.Length;
						pStreamContext->UniFileName.Buffer[pStreamContext->UniFileName.Length/2] = L'\0';

						pStreamContext->UniVolumeName.MaximumLength = pFltNameInfo->Volume.MaximumLength;
						pStreamContext->UniVolumeName.Buffer = NULL;
						ShitDrvAllocateUnicodeString(&pStreamContext->UniVolumeName);
						RtlCopyMemory(pStreamContext->UniVolumeName.Buffer, \
							pFltNameInfo->Volume.Buffer, \
							pFltNameInfo->Volume.Length);
						pStreamContext->UniVolumeName.Length = pFltNameInfo->Volume.Length;
						pStreamContext->UniVolumeName.Buffer[pStreamContext->UniVolumeName.Length/2] = L'\0';

						pStreamContext->ulCreateCount++;
						pStreamContext->hProcessId = hProcessId;
						RtlZeroMemory(pStreamContext->PreModifyCopiedMd5Sum,100);
						pStreamContext->bIsPreModifyCopied = FALSE;
					}
					ShitDrvReleaseResource(pStreamContext->pResource);

					RtlZeroMemory(wOperation,sizeof(WCHAR) * 50);
					RtlCopyMemory(wOperation,L"创建文件",wcslen(L"创建文件") * sizeof(WCHAR));
					pRecordList = AllocateRecordListDat();
					if (pRecordList && InitializeRecordDat(pRecordList, \
						UniProcessName.Buffer, \
						UniProcessName.Length, \
						pFltNameInfo->Name.Buffer, \
						pFltNameInfo->Name.Length, \
						wOperation, \
						(ULONG)wcslen(L"创建文件") * sizeof(WCHAR), \
						hProcessId, \
						IsCreateFile, \
						pStreamContext->ulCreateCount, \
						pStreamContext->ulOpenedCount, \
						pStreamContext->ulWriteCount, \
						pStreamContext->ulCleanupCount, \
						pStreamContext->ulCloseCount, \
						IsFsFile))
					{
						InitializeListHead(&pRecordList->RecordList);
						ExInterlockedInsertTailList(&g_RecordListHead,&pRecordList->RecordList,&g_RecordSpinLock);
						KeSetEvent(g_pEventWaitKrl,IO_NO_INCREMENT,FALSE);
						KeClearEvent(g_pEventWaitKrl);
					}
				}
				else if (ulNewCreate == FILE_OVERWRITTEN || \
					ulNewCreate == FILE_SUPERSEDED || \
					ulNewCreate == FILE_OPENED)
				{
					////Isn't From Sandbox
					//Status = ConvertSandboxShortName(&g_pFilterCtl->UniSandBoxPath, \
					//	&pFltNameInfo->Name, \
					//	&UniSandBoxPath, \
					//	&pFltNameInfo->Volume);
					//if (NT_SUCCESS(Status))
					//{
					//	if (FltGetFileLength(FltObjects->Filter, \
					//		FltObjects->Instance, \
					//		&pFltNameInfo->Name) <= 0xA00000)
					//	{
					//		RtlZeroMemory(Md5Sum,16);
					//		Status = FltCalcFileDatMd5(FltObjects->Filter, \
					//			FltObjects->Instance, \
					//			&pFltNameInfo->Name, \
					//			Md5Sum);
					//		if (NT_SUCCESS(Status))
					//		{
					//			RtlZeroMemory(NowFileMd5Sum,MAX_PATH);
					//			StringCchPrintfA(NowFileMd5Sum,MAX_PATH,"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",Md5Sum[0], \
					//				Md5Sum[1],Md5Sum[2], Md5Sum[3],Md5Sum[4],Md5Sum[5],Md5Sum[6],Md5Sum[7],Md5Sum[8], \
					//				Md5Sum[9],Md5Sum[10],Md5Sum[11],Md5Sum[12],Md5Sum[13],Md5Sum[14],Md5Sum[15]);
					//			bIsCopyMd5Sum = FALSE;
					//		}
					//	}
					//	bIsSandboxFileExist = IsSandboxFileExist(g_pFilterCtl->pFilterHandle, \
					//		FltObjects->Instance, \
					//		&UniSandBoxPath);
					//	if (bIsSandboxFileExist)
					//	{
					//		//沙箱中存在文件
					//		if (FltGetFileLength(FltObjects->Filter, \
					//			FltObjects->Instance, \
					//			&pFltNameInfo->Name) <= 0xA00000)
					//		{
					//			RtlZeroMemory(Md5Sum,16);
					//			Status = FltCalcFileDatMd5(FltObjects->Filter, \
					//				FltObjects->Instance, \
					//				&UniSandBoxPath, \
					//				Md5Sum);
					//			if (NT_SUCCESS(Status))
					//			{
					//				RtlZeroMemory(ExistFileMd5Sum,MAX_PATH);
					//				StringCchPrintfA(ExistFileMd5Sum,MAX_PATH,"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",Md5Sum[0], \
					//					Md5Sum[1],Md5Sum[2], Md5Sum[3],Md5Sum[4],Md5Sum[5],Md5Sum[6],Md5Sum[7],Md5Sum[8], \
					//					Md5Sum[9],Md5Sum[10],Md5Sum[11],Md5Sum[12],Md5Sum[13],Md5Sum[14],Md5Sum[15]);
					//				if (strncmp(ExistFileMd5Sum,NowFileMd5Sum,strlen(NowFileMd5Sum)) != 0)
					//				{
					//					Status = DoCopyFile(g_pFilterCtl->pFilterHandle,
					//						//FltObjects->FileObject,
					//						NULL,
					//						FltObjects->Instance,
					//						&pFltNameInfo->Name,
					//						FltObjects->Instance,
					//						&UniSandBoxPath,
					//						FALSE,
					//						TRUE);
					//					if (NT_SUCCESS(Status))
					//					{
					//						DbgPrint("CopyFile Success\r\n");
					//						bIsCopyMd5Sum = TRUE;
					//					}
					//				}
					//			}
					//		}
					//	}
					//	else
					//	{
					//		//沙箱中不存在文件,创建一个文件
					//		if (FltGetFileLength(FltObjects->Filter, \
					//			FltObjects->Instance, \
					//			&pFltNameInfo->Name) <= 0xA00000)
					//		{
					//			//RtlAppendUnicodeToString(&UniSandBoxPath,L"0000");
					//			Status = DoCopyFile(g_pFilterCtl->pFilterHandle,
					//				//FltObjects->FileObject,
					//				NULL,
					//				FltObjects->Instance,
					//				&pFltNameInfo->Name,
					//				FltObjects->Instance,
					//				&UniSandBoxPath,
					//				FALSE,
					//				FALSE);
					//			if (NT_SUCCESS(Status))
					//			{
					//				DbgPrint("CopyFile Success\r\n");
					//			}
					//			bIsCopyMd5Sum = FALSE;
					//		}
					//	}
						Status = FindOrCreateStreamContext(Data,TRUE,&pStreamContext,&bIsStreamContextCreated);
						if (NT_ERROR(Status))
						{
							break;
						}
						ShitDrvAcquireResourceExclusive(pStreamContext->pResource);
						if (pStreamContext->bIsInit)
						{
							RtlCopyUnicodeString(&pStreamContext->UniFileName,&pFltNameInfo->Name);
							pStreamContext->ulOpenedCount++;
							pStreamContext->hProcessId = hProcessId;

							pStreamContext->UniFileName.MaximumLength = pFltNameInfo->Name.MaximumLength;
							pStreamContext->UniFileName.Buffer = NULL;
							ShitDrvAllocateUnicodeString(&pStreamContext->UniFileName);
							RtlCopyMemory(pStreamContext->UniFileName.Buffer, \
								pFltNameInfo->Name.Buffer, \
								pFltNameInfo->Name.Length);
							pStreamContext->UniFileName.Length = pFltNameInfo->Name.Length;

							pStreamContext->UniVolumeName.MaximumLength = pFltNameInfo->Volume.MaximumLength;
							pStreamContext->UniVolumeName.Buffer = NULL;
							ShitDrvAllocateUnicodeString(&pStreamContext->UniVolumeName);
							RtlCopyMemory(pStreamContext->UniVolumeName.Buffer, \
								pFltNameInfo->Volume.Buffer, \
								pFltNameInfo->Volume.Length);
							pStreamContext->UniVolumeName.Length = pFltNameInfo->Volume.Length;
							//if (FALSE == bIsCopyMd5Sum)
							//{
							//	RtlCopyMemory(pStreamContext->PreModifyCopiedMd5Sum,NowFileMd5Sum,strlen(NowFileMd5Sum));
							//	pStreamContext->bIsPreModifyCopied = TRUE;
							//}
							//else
							//{
							//	RtlCopyMemory(pStreamContext->PreModifyCopiedMd5Sum,ExistFileMd5Sum,strlen(ExistFileMd5Sum));
							//	pStreamContext->bIsPreModifyCopied = FALSE;
							//}
						}
						ShitDrvReleaseResource(pStreamContext->pResource);
					//}
				}
				else
				{
					break;
				}
			}
		} while (0);
	}
	if (UniSandBoxPath.Buffer)
	{
		ExFreePool(UniSandBoxPath.Buffer);
	}
	if (pFltNameInfo)
	{
		FltReleaseFileNameInformation(pFltNameInfo);
	}
	if (UniProcessName.Buffer)
	{
		ExFreePool(UniProcessName.Buffer);
	}
	if (pStreamContext)
	{
		FltReleaseContext(pStreamContext);
	}
	return FltCallBackStatus;
}
FLT_PREOP_CALLBACK_STATUS FltPreSetFileformationFiles(PFLT_CALLBACK_DATA Data, \
													  PCFLT_RELATED_OBJECTS FltObjects, \
													  PVOID *CompletionContext)
{
	NTSTATUS Status;
	FLT_PREOP_CALLBACK_STATUS FltCallBackStatus;
	PFLT_FILE_NAME_INFORMATION pFltNameInfo;
	//PFILE_RENAME_INFORMATION pRenNameInfo;
	UNICODE_STRING UniProcessName;
	UNICODE_STRING UniSandBoxPath;
	HANDLE hProcessId;
	WCHAR wOperation[50];
	PRECORD_LIST pRecordList;
	BOOLEAN bIsSandboxFileExist;
	BOOLEAN bIsExist;
	UCHAR Md5Sum[16];
	PSHITDRV_STREAM_CONTEXT pStreamContext;
	BOOLEAN bIsStreamContextCreated;
	CHAR ExistFileMd5Sum[MAX_PATH];
	CHAR NowFileMd5Sum[MAX_PATH];
	BOOLEAN bIsCopyMd5Sum;

	FltCallBackStatus = FLT_PREOP_SUCCESS_WITH_CALLBACK;
	bIsSandboxFileExist = FALSE;
	UniSandBoxPath.Buffer = NULL;
	UniSandBoxPath.Length = 0;
	UniSandBoxPath.MaximumLength = 0;
	UniProcessName.Buffer = NULL;
	UniProcessName.MaximumLength = 0;

	do
	{
		if (FALSE == g_pFilterCtl->FileFltAccessCtl.ulIsStartFilter || \
			FALSE == g_pFilterCtl->FileFltAccessCtl.ulIsInitialize)
		{
			return FltCallBackStatus;
		}
		if (KeGetCurrentIrql() > APC_LEVEL || \
			NULL == Data || \
			NULL == FltObjects || \
			NULL == FltObjects->FileObject || \
			IRP_MJ_SET_INFORMATION != Data->Iopb->MajorFunction || \
			ExGetPreviousMode() == KernelMode)
		{
			return FltCallBackStatus;
		}
		if (g_pFilterCtl->pMyProc == PsGetCurrentProcess())
		{
			return FltCallBackStatus;
		}
		hProcessId = PsGetCurrentProcessId();
		if (FALSE == IsSandboxProcess(hProcessId))
		{
			return FltCallBackStatus;
		}
		if (FileRenameInformation == Data->Iopb->Parameters.SetFileInformation.FileInformationClass || \
			FileDispositionInformation == Data->Iopb->Parameters.SetFileInformation.FileInformationClass)
		{
			Status = FltGetFileNameInformation(Data, \
				FLT_FILE_NAME_NORMALIZED, \
				&pFltNameInfo);
			if (NT_SUCCESS(Status))
			{
				Status = FltParseFileNameInformation(pFltNameInfo);
				if (NT_ERROR(Status))
				{
					break;
				}
			}
			if (NT_ERROR(Status) || \
				NULL == pFltNameInfo)
			{
				break;
			}
			bIsExist = RtlPrefixUnicodeString(&g_pFilterCtl->UniSandBoxPath,&pFltNameInfo->Name,TRUE);
			if (bIsExist)
			{
				break;
			}
			Status = GetProcessFullName(g_pFilterCtl->pFilterHandle, \
				FltObjects->Instance, \
				(PVOID)PsGetCurrentProcess(), \
				&UniProcessName);
			if (Data->Iopb->Parameters.SetFileInformation.FileInformationClass == FileDispositionInformation && \
				g_pFilterCtl->FileFltAccessCtl.ulIsFilterDelete == 1)
			{
				Status = ConvertSandboxShortName(&g_pFilterCtl->UniSandBoxPath, \
					&pFltNameInfo->Name, \
					&UniSandBoxPath, \
					&pFltNameInfo->Volume);
				if (NT_SUCCESS(Status))
				{
					if (FltGetFileLength(FltObjects->Filter, \
						FltObjects->Instance, \
						&pFltNameInfo->Name) <= 0xA00000)
					{
						RtlZeroMemory(Md5Sum,16);
						Status = FltCalcFileDatMd5(FltObjects->Filter, \
							FltObjects->Instance, \
							&pFltNameInfo->Name, \
							Md5Sum);
						if (NT_SUCCESS(Status))
						{
							RtlZeroMemory(NowFileMd5Sum,MAX_PATH);
							StringCchPrintfA(NowFileMd5Sum,MAX_PATH,"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",Md5Sum[0], \
								Md5Sum[1],Md5Sum[2], Md5Sum[3],Md5Sum[4],Md5Sum[5],Md5Sum[6],Md5Sum[7],Md5Sum[8], \
								Md5Sum[9],Md5Sum[10],Md5Sum[11],Md5Sum[12],Md5Sum[13],Md5Sum[14],Md5Sum[15]);
							bIsCopyMd5Sum = FALSE;
						}
					}
				}
				bIsSandboxFileExist = IsSandboxFileExist(g_pFilterCtl->pFilterHandle, \
					FltObjects->Instance, \
					&UniSandBoxPath);
				if (bIsSandboxFileExist)
				{
					//沙箱中存在文件
					if (FltGetFileLength(FltObjects->Filter, \
						FltObjects->Instance, \
						&pFltNameInfo->Name) <= 0xA00000)
					{
						RtlZeroMemory(Md5Sum,16);
						Status = FltCalcFileDatMd5(FltObjects->Filter, \
							FltObjects->Instance, \
							&UniSandBoxPath, \
							Md5Sum);
						if (NT_SUCCESS(Status))
						{
							RtlZeroMemory(ExistFileMd5Sum,MAX_PATH);
							StringCchPrintfA(ExistFileMd5Sum,MAX_PATH,"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",Md5Sum[0], \
								Md5Sum[1],Md5Sum[2], Md5Sum[3],Md5Sum[4],Md5Sum[5],Md5Sum[6],Md5Sum[7],Md5Sum[8], \
								Md5Sum[9],Md5Sum[10],Md5Sum[11],Md5Sum[12],Md5Sum[13],Md5Sum[14],Md5Sum[15]);
							if (strncmp(ExistFileMd5Sum,NowFileMd5Sum,strlen(NowFileMd5Sum)) != 0)
							{
								Status = DoCopyFile(g_pFilterCtl->pFilterHandle,
									//FltObjects->FileObject,
									NULL,
									FltObjects->Instance,
									&pFltNameInfo->Name,
									FltObjects->Instance,
									&UniSandBoxPath,
									FALSE,
									TRUE);
								if (NT_SUCCESS(Status))
								{
									DbgPrint("CopyFile Success\r\n");
									bIsCopyMd5Sum = TRUE;
								}
							}
						}
					}
				}
				else
				{
					//沙箱中不存在文件,创建一个文件
					if (FltGetFileLength(FltObjects->Filter, \
						FltObjects->Instance, \
						&pFltNameInfo->Name) <= 0xA00000)
					{
						//RtlAppendUnicodeToString(&UniSandBoxPath,L"0000");
						Status = DoCopyFile(g_pFilterCtl->pFilterHandle,
							//FltObjects->FileObject,
							NULL,
							FltObjects->Instance,
							&pFltNameInfo->Name,
							FltObjects->Instance,
							&UniSandBoxPath,
							FALSE,
							FALSE);
						if (NT_SUCCESS(Status))
						{
							DbgPrint("CopyFile Success\r\n");
						}
						bIsCopyMd5Sum = FALSE;
					}
				}
				Status = FindOrCreateStreamContext(Data,TRUE,&pStreamContext,&bIsStreamContextCreated);
				if (NT_ERROR(Status))
				{
					break;
				}
				ShitDrvAcquireResourceExclusive(pStreamContext->pResource);
				if (pStreamContext->bIsInit)
				{
					RtlCopyUnicodeString(&pStreamContext->UniFileName,&pFltNameInfo->Name);
					pStreamContext->ulCleanupCount++;
					pStreamContext->hProcessId = hProcessId;
					if (FALSE == bIsCopyMd5Sum)
					{
						RtlCopyMemory(pStreamContext->PostModifyCopiedMd5Sum,NowFileMd5Sum,strlen(NowFileMd5Sum));
						pStreamContext->bIsPostModifyCopied = TRUE;
					}
					else
					{
						RtlCopyMemory(pStreamContext->PostModifyCopiedMd5Sum,ExistFileMd5Sum,strlen(ExistFileMd5Sum));
						pStreamContext->bIsPostModifyCopied = TRUE;
					}
				}
				ShitDrvReleaseResource(pStreamContext->pResource);
				RtlZeroMemory(wOperation,sizeof(WCHAR) * 50);
				RtlCopyMemory(wOperation,L"删除文件",wcslen(L"删除文件") * sizeof(WCHAR));
				pRecordList = AllocateRecordListDat();
				if (pRecordList && InitializeRecordDat(pRecordList, \
					UniProcessName.Buffer, \
					UniProcessName.Length, \
					pFltNameInfo->Name.Buffer, \
					pFltNameInfo->Name.Length, \
					wOperation, \
					(ULONG)wcslen(L"删除文件") * sizeof(WCHAR), \
					hProcessId, \
					IsDeleteFile, \
					0,
					0, \
					0, \
					0, \
					0, \
					IsFsFile));
				{
					RtlCopyMemory(pRecordList->RecordDat.Md5Sum, \
						pStreamContext->PostModifyCopiedMd5Sum, \
						strlen(pStreamContext->PostModifyCopiedMd5Sum));
					InitializeListHead(&pRecordList->RecordList);
					ExInterlockedInsertTailList(&g_RecordListHead,&pRecordList->RecordList,&g_RecordSpinLock);
					KeSetEvent(g_pEventWaitKrl,IO_NO_INCREMENT,FALSE);
					KeClearEvent(g_pEventWaitKrl);
				}
			}
			else if (Data->Iopb->Parameters.SetFileInformation.FileInformationClass == FileRenameInformation && \
				g_pFilterCtl->FileFltAccessCtl.ulIsFilterRenName == 1)
			{
				Status = FindOrCreateStreamContext(Data,TRUE,&pStreamContext,&bIsStreamContextCreated);
				if (NT_ERROR(Status))
				{
					break;
				}
				ShitDrvAcquireResourceExclusive(pStreamContext->pResource);
				if (pStreamContext->bIsInit)
				{

					pStreamContext->ulOpenedCount++;
					pStreamContext->hProcessId = hProcessId;
					UpdateNameInStreamContext(&pFltNameInfo->Name,pStreamContext);
				}
				ShitDrvReleaseResource(pStreamContext->pResource);
			}
		}
	} while (0);
	if (UniProcessName.Buffer)
	{
		ExFreePool(UniProcessName.Buffer);
	}
	if (UniSandBoxPath.Buffer)
	{
		ExFreePool(UniSandBoxPath.Buffer);
	}
	return FltCallBackStatus;
}
FLT_POSTOP_CALLBACK_STATUS FltPostSetFileInformationFiles(PFLT_CALLBACK_DATA Data, \
														  PCFLT_RELATED_OBJECTS FltObjects, \
														  PVOID CompletionContext, \
														  FLT_POST_OPERATION_FLAGS Flags)
{
	FLT_POSTOP_CALLBACK_STATUS FltCallBackStatus;
	NTSTATUS Status;
	ULONG ulFileInfoClass;
	//PVOID pCurProc;
	PFLT_FILE_NAME_INFORMATION pFltNameInfo;
	PFILE_RENAME_INFORMATION pRenNameInfo;
	UNICODE_STRING UniProcessName;
	HANDLE hProcessId;
	WCHAR wOperation[50];
	PRECORD_LIST pRecordList;
	ULONG ulOperationType;
	BOOLEAN bRet;
	PSHITDRV_STREAM_CONTEXT pStreamContext;
	BOOLEAN bIsStreamContextCreated,bIsExist;


	bRet = FALSE;
	Status = STATUS_SUCCESS;
	pFltNameInfo = NULL;
	pRenNameInfo = NULL;
	pRecordList = NULL;
	ulOperationType = 0;
	FltCallBackStatus = FLT_PREOP_SUCCESS_WITH_CALLBACK;
	hProcessId = 0;
	UniProcessName.Buffer = NULL;
	UniProcessName.MaximumLength = 0;
	pStreamContext = NULL;
	bIsStreamContextCreated = FALSE;
	bIsExist = FALSE;

	do 
	{
		if (FALSE == g_pFilterCtl->FileFltAccessCtl.ulIsStartFilter || \
			FALSE == g_pFilterCtl->FileFltAccessCtl.ulIsInitialize)
		{
			return FltCallBackStatus;
		}
		if (KeGetCurrentIrql() > APC_LEVEL || \
			NULL == Data || \
			NULL == FltObjects || \
			NULL == FltObjects->FileObject || \
			ExGetPreviousMode() == KernelMode)
		{
			return FltCallBackStatus;
		}
		if (g_pFilterCtl->pMyProc == PsGetCurrentProcess())
		{
			return FltCallBackStatus;
		}
		hProcessId = PsGetCurrentProcessId();
		if (FALSE == IsSandboxProcess(hProcessId))
		{
			return FltCallBackStatus;
		}
		ulFileInfoClass = Data->Iopb->Parameters.SetFileInformation.FileInformationClass;
		if (FileBasicInformation == ulFileInfoClass || \
			FileDispositionInformation == ulFileInfoClass || \
			FileRenameInformation == ulFileInfoClass || \
			FileLinkInformation == ulFileInfoClass || \
			FileEndOfFileInformation == ulFileInfoClass || \
			FileAllocationInformation == ulFileInfoClass)
		{
			Status = FltGetFileNameInformation(Data, \
				FLT_FILE_NAME_NORMALIZED, \
				&pFltNameInfo);
			if (NT_SUCCESS(Status))
			{
				Status = FltParseFileNameInformation(pFltNameInfo);
				if (NT_ERROR(Status))
				{
					break;
				}
			}
			if (NT_ERROR(Status) || \
				NULL == pFltNameInfo)
			{
				break;
			}
			bIsExist = RtlPrefixUnicodeString(&g_pFilterCtl->UniSandBoxPath,&pFltNameInfo->Name,TRUE);
			if (bIsExist)
			{
				break;
			}
			Status = GetProcessFullName(g_pFilterCtl->pFilterHandle, \
				FltObjects->Instance, \
				(PVOID)PsGetCurrentProcess(), \
				&UniProcessName);
			Status = FindOrCreateStreamContext(Data,TRUE,&pStreamContext,&bIsStreamContextCreated);
			if (NT_ERROR(Status))
			{
				break;
			}
			ShitDrvAcquireResourceExclusive(pStreamContext->pResource);
			if (pStreamContext->bIsInit)
			{
				pStreamContext->hProcessId = hProcessId;
			}
			ShitDrvReleaseResource(pStreamContext->pResource);
			if (Data->Iopb->Parameters.SetFileInformation.FileInformationClass == FileRenameInformation && \
				g_pFilterCtl->FileFltAccessCtl.ulIsFilterRenName == 1)
			{
				//RenName
				RtlZeroMemory(wOperation,sizeof(WCHAR) * 50);
				RtlCopyMemory(wOperation,L"重命名文件",wcslen(L"重命名文件") * sizeof(WCHAR));
				pRenNameInfo = (PFILE_RENAME_INFORMATION)Data->Iopb->Parameters.SetFileInformation.InfoBuffer;
				if (pRenNameInfo)
				{
					pRecordList = AllocateRecordListDat();
					if (pRecordList && InitializeRecordDat(pRecordList, \
						UniProcessName.Buffer, \
						UniProcessName.Length, \
						pFltNameInfo->Name.Buffer, \
						pFltNameInfo->Name.Length, \
						wOperation, \
						(ULONG)wcslen(L"重命名文件") * sizeof(WCHAR), \
						hProcessId, \
						IsRenNameFile, \
						0, \
						0, \
						0, \
						0, \
						0, \
						IsFsFile))
					{
						if (pStreamContext->UniFileName.Length < MAX_PATH)
						{
							RtlCopyMemory(pRecordList->RecordDat.wBakPath, \
								pStreamContext->UniFileName.Buffer, \
								pStreamContext->UniFileName.Length);
						}
						InitializeListHead(&pRecordList->RecordList);
						ExInterlockedInsertTailList(&g_RecordListHead,&pRecordList->RecordList,&g_RecordSpinLock);
						KeSetEvent(g_pEventWaitKrl,IO_NO_INCREMENT,FALSE);
						KeClearEvent(g_pEventWaitKrl);
					}
				}
			}
			else if ((Data->Iopb->Parameters.SetFileInformation.FileInformationClass == FileEndOfFileInformation || \
				Data->Iopb->Parameters.SetFileInformation.FileInformationClass == FileAllocationInformation) && \
				g_pFilterCtl->FileFltAccessCtl.ulIsFilterSetSize == 1)
			{
				//Set File Size
				RtlZeroMemory(wOperation,sizeof(WCHAR) * 50);
				RtlCopyMemory(wOperation,L"改变文件属性",wcslen(L"改变文件属性") * sizeof(WCHAR));
				pRecordList = AllocateRecordListDat();
				if (pRecordList && InitializeRecordDat(pRecordList, \
					UniProcessName.Buffer, \
					UniProcessName.Length, \
					pFltNameInfo->Name.Buffer, \
					pFltNameInfo->Name.Length, \
					wOperation, \
					(ULONG)wcslen(L"改变文件属性") * sizeof(WCHAR), \
					hProcessId, \
					IsSetFileSize, \
					0, \
					0, \
					0, \
					0, \
					0, \
					IsFsFile))
				{
					InitializeListHead(&pRecordList->RecordList);
					ExInterlockedInsertTailList(&g_RecordListHead,&pRecordList->RecordList,&g_RecordSpinLock);
					KeSetEvent(g_pEventWaitKrl,IO_NO_INCREMENT,FALSE);
					KeClearEvent(g_pEventWaitKrl);
				}
			}
			else if (Data->Iopb->Parameters.SetFileInformation.FileInformationClass == FileLinkInformation)
			{
			}
			else if (Data->Iopb->Parameters.SetFileInformation.FileInformationClass == FileBasicInformation)
			{
			}
			else
			{

			}
		}
	} while (0);
	if (pFltNameInfo)
	{
		FltReleaseFileNameInformation(pFltNameInfo);
	}
	if (UniProcessName.Buffer)
	{
		ExFreePool(UniProcessName.Buffer);
	}
	return FltCallBackStatus;
}
FLT_PREOP_CALLBACK_STATUS FltPreWriteFiles(PFLT_CALLBACK_DATA Data, \
									   PCFLT_RELATED_OBJECTS FltObjects, \
									   PVOID *CompletionContext)
{
	PRECORD_LIST pRecordList;
	NTSTATUS Status;
	FLT_PREOP_CALLBACK_STATUS FltStatus;
	HANDLE hProcessId;
	//WCHAR wOperation[50];
	BOOLEAN bIsDirectory;
	PSHITDRV_STREAM_CONTEXT pStreamContext;
	BOOLEAN bIsStreamContextCreated,bIsExist;
	BOOLEAN bIsCopyFile;
	BOOLEAN bIsCopyMd5Sum;
	CHAR NowFileMd5Sum[MAX_PATH];
	CHAR ExistFileMd5Sum[MAX_PATH];
	UCHAR Md5Sum[16];
	BOOLEAN bIsSandboxFileExist;
	UNICODE_STRING UniSandBoxPath;

	hProcessId = 0;
	UniSandBoxPath.Buffer = NULL;
	UniSandBoxPath.Length = 0;
	UniSandBoxPath.MaximumLength = 0;
	pRecordList = NULL;
	Status = STATUS_SUCCESS;
	FltStatus = FLT_PREOP_SUCCESS_WITH_CALLBACK;
	bIsDirectory = FALSE;
	bIsStreamContextCreated = FALSE;
	bIsExist = FALSE;
	pStreamContext = NULL;
	bIsCopyFile = FALSE;
	bIsSandboxFileExist = FALSE;
	bIsCopyMd5Sum = FALSE;

	do 
	{
		if (FALSE == g_pFilterCtl->FileFltAccessCtl.ulIsStartFilter || \
			FALSE == g_pFilterCtl->FileFltAccessCtl.ulIsInitialize)
		{
			return FltStatus;
		}
		if (KeGetCurrentIrql() > APC_LEVEL || \
			NULL == Data || \
			NULL == FltObjects || \
			ExGetPreviousMode() == KernelMode || \
			Data->Iopb->MajorFunction != IRP_MJ_WRITE)
		{
			return FltStatus;
		}
		if (g_pFilterCtl->pMyProc == PsGetCurrentProcess())
		{
			return FltStatus;
		}
		hProcessId = PsGetCurrentProcessId();
		if (FALSE == IsSandboxProcess(hProcessId))
		{
			return FltStatus;
		}
		Status = FltIsDirectory(FltObjects->FileObject,FltObjects->Instance,&bIsDirectory);
		if (NT_SUCCESS(Status))
		{
			if (bIsDirectory)
			{
				return FltStatus;
			}
		}
		else
		{
			return FltStatus;
		}
		Status = FindOrCreateStreamContext(Data,TRUE,&pStreamContext,&bIsStreamContextCreated);
		if (NT_ERROR(Status))
		{
			break;
		}
		if (TRUE == bIsStreamContextCreated)
		{
			break;
		}
		if (pStreamContext->ulCreateCount)
		{
			break;
		}
		bIsExist = RtlPrefixUnicodeString(&g_pFilterCtl->UniSandBoxPath,&pStreamContext->UniFileName,TRUE);
		if (bIsExist)
		{
			break;
		}
		if (!RtlCompareUnicodeString(&pStreamContext->UniFileName,&pStreamContext->UniVolumeName,TRUE))
		{
			break;
		}
		if (TRUE == pStreamContext->bIsInit && \
			0 == pStreamContext->ulWriteCount)
		{
			bIsCopyFile = TRUE;
		}
		ShitDrvAcquireResourceExclusive(pStreamContext->pResource);
		pStreamContext->ulWriteCount++;
		ShitDrvReleaseResource(pStreamContext->pResource);
		if (bIsCopyFile)
		{
			//Isn't From Sandbox
			Status = ConvertSandboxShortName(&g_pFilterCtl->UniSandBoxPath, \
				&pStreamContext->UniFileName, \
				&UniSandBoxPath, \
				&pStreamContext->UniVolumeName);
			if (NT_SUCCESS(Status))
			{
				if (FltGetFileLength(FltObjects->Filter, \
					FltObjects->Instance, \
					&pStreamContext->UniFileName) <= 0xA00000)
				{
					RtlZeroMemory(Md5Sum,16);
					Status = FltCalcFileDatMd5(FltObjects->Filter, \
						FltObjects->Instance, \
						&pStreamContext->UniFileName, \
						Md5Sum);
					if (NT_SUCCESS(Status))
					{
						RtlZeroMemory(NowFileMd5Sum,MAX_PATH);
						StringCchPrintfA(NowFileMd5Sum,MAX_PATH,"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",Md5Sum[0], \
							Md5Sum[1],Md5Sum[2], Md5Sum[3],Md5Sum[4],Md5Sum[5],Md5Sum[6],Md5Sum[7],Md5Sum[8], \
							Md5Sum[9],Md5Sum[10],Md5Sum[11],Md5Sum[12],Md5Sum[13],Md5Sum[14],Md5Sum[15]);
						bIsCopyMd5Sum = FALSE;
					}
				}
				bIsSandboxFileExist = IsSandboxFileExist(g_pFilterCtl->pFilterHandle, \
					FltObjects->Instance, \
					&UniSandBoxPath);
				if (bIsSandboxFileExist)
				{
					//沙箱中存在文件
					if (FltGetFileLength(FltObjects->Filter, \
						FltObjects->Instance, \
						&pStreamContext->UniFileName) <= 0xA00000)
					{
						RtlZeroMemory(Md5Sum,16);
						Status = FltCalcFileDatMd5(FltObjects->Filter, \
							FltObjects->Instance, \
							&UniSandBoxPath, \
							Md5Sum);
						if (NT_SUCCESS(Status))
						{
							RtlZeroMemory(ExistFileMd5Sum,MAX_PATH);
							StringCchPrintfA(ExistFileMd5Sum,MAX_PATH,"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",Md5Sum[0], \
								Md5Sum[1],Md5Sum[2], Md5Sum[3],Md5Sum[4],Md5Sum[5],Md5Sum[6],Md5Sum[7],Md5Sum[8], \
								Md5Sum[9],Md5Sum[10],Md5Sum[11],Md5Sum[12],Md5Sum[13],Md5Sum[14],Md5Sum[15]);
							if (strncmp(ExistFileMd5Sum,NowFileMd5Sum,strlen(NowFileMd5Sum)) != 0)
							{
								Status = DoCopyFile(g_pFilterCtl->pFilterHandle,
									//FltObjects->FileObject,
									NULL,
									FltObjects->Instance,
									&pStreamContext->UniFileName,
									FltObjects->Instance,
									&UniSandBoxPath,
									FALSE,
									TRUE);
								if (NT_SUCCESS(Status))
								{
									DbgPrint("CopyFile Success\r\n");
									bIsCopyMd5Sum = TRUE;
								}
							}
						}
					}
				}
				else
				{
					//沙箱中不存在文件,创建一个文件
					if (FltGetFileLength(FltObjects->Filter, \
						FltObjects->Instance, \
						&pStreamContext->UniFileName) <= 0xA00000)
					{
						//RtlAppendUnicodeToString(&UniSandBoxPath,L"0000");
						Status = DoCopyFile(g_pFilterCtl->pFilterHandle,
							//FltObjects->FileObject,
							NULL,
							FltObjects->Instance,
							&pStreamContext->UniFileName,
							FltObjects->Instance,
							&UniSandBoxPath,
							FALSE,
							FALSE);
						if (NT_SUCCESS(Status))
						{
							DbgPrint("CopyFile Success\r\n");
						}
						bIsCopyMd5Sum = FALSE;
					}
				}
			}
			ShitDrvAcquireResourceExclusive(pStreamContext->pResource);
			if (FALSE == bIsCopyMd5Sum)
			{
				RtlCopyMemory(pStreamContext->PostModifyCopiedMd5Sum,NowFileMd5Sum,strlen(NowFileMd5Sum));
				pStreamContext->bIsPostModifyCopied = TRUE;
			}
			else
			{
				RtlCopyMemory(pStreamContext->PostModifyCopiedMd5Sum,ExistFileMd5Sum,strlen(NowFileMd5Sum));
				pStreamContext->bIsPostModifyCopied = TRUE;
			}
			ShitDrvReleaseResource(pStreamContext->pResource);
		}
	} while (0);
	if (UniSandBoxPath.Buffer)
	{
		ExFreePool(UniSandBoxPath.Buffer);
	}
	if (pStreamContext)
	{
		FltReleaseContext(pStreamContext);
	}
	return FltStatus;
}
FLT_POSTOP_CALLBACK_STATUS FltPostWriteFiles(PFLT_CALLBACK_DATA Data, \
											 PCFLT_RELATED_OBJECTS FltObjects, \
											 PVOID CompletionContext, \
											 FLT_POST_OPERATION_FLAGS Flags)
{
	PRECORD_LIST pRecordList;
	NTSTATUS Status;
	PFLT_FILE_NAME_INFORMATION pFltNameInfo;
	FLT_POSTOP_CALLBACK_STATUS FltStatus;
	UNICODE_STRING UniProcessName;
	HANDLE hProcessId;
	//WCHAR wOperation[50];
	BOOLEAN bIsDirectory;
	PSHITDRV_STREAM_CONTEXT pStreamContext;
	BOOLEAN bIsStreamContextCreated,bIsExist;

	hProcessId = 0;
	UniProcessName.Buffer = NULL;
	UniProcessName.MaximumLength = 0;
	pRecordList = NULL;
	Status = STATUS_SUCCESS;
	FltStatus = FLT_POSTOP_FINISHED_PROCESSING;
	pFltNameInfo = NULL;
	bIsDirectory = FALSE;
	bIsStreamContextCreated = FALSE;
	bIsExist = FALSE;
	pStreamContext = NULL;

	do 
	{
		if (FALSE == g_pFilterCtl->FileFltAccessCtl.ulIsStartFilter || \
			FALSE == g_pFilterCtl->FileFltAccessCtl.ulIsInitialize)
		{
			return FltStatus;
		}
		if (KeGetCurrentIrql() > APC_LEVEL || \
			NULL == Data || \
			NULL == FltObjects || \
			ExGetPreviousMode() == KernelMode || \
			Data->Iopb->MajorFunction != IRP_MJ_WRITE)
		{
			return FltStatus;
		}
		if (g_pFilterCtl->pMyProc == PsGetCurrentProcess())
		{
			return FltStatus;
		}
		hProcessId = PsGetCurrentProcessId();
		if (FALSE == IsSandboxProcess(hProcessId))
		{
			return FltStatus;
		}
		Status = FltIsDirectory(FltObjects->FileObject,FltObjects->Instance,&bIsDirectory);
		if (NT_SUCCESS(Status))
		{
			if (bIsDirectory)
			{
				return FltStatus;
			}
		}
		else
		{
			return FltStatus;
		}
		if (Data->Iopb->Parameters.Write.Length > 0/* && \
			Data->Iopb->Parameters.Write.WriteBuffer*/)
		{
			Status = FltGetFileNameInformation(Data, \
				FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_DEFAULT, \
				&pFltNameInfo);
			if (NT_SUCCESS(Status))
			{
				Status = FltParseFileNameInformation(pFltNameInfo);
				if (NT_ERROR(Status))
				{
					break;
				}
			}
			if (NT_ERROR(Status) || \
				NULL == pFltNameInfo)
			{
				break;
			}
			if (!RtlCompareUnicodeString(&pFltNameInfo->Name,&pFltNameInfo->Volume,TRUE))
			{
				break;
			}
			bIsExist = RtlPrefixUnicodeString(&g_pFilterCtl->UniSandBoxPath,&pFltNameInfo->Name,TRUE);
			if (bIsExist)
			{
				break;
			}
			Status = GetProcessFullName(g_pFilterCtl->pFilterHandle, \
				FltObjects->Instance, \
				(PVOID)PsGetCurrentProcess(), \
				&UniProcessName);
			if (NT_SUCCESS(Status))
			{
				Status = FindOrCreateStreamContext(Data,TRUE,&pStreamContext,&bIsStreamContextCreated);
				if (NT_ERROR(Status))
				{
					break;
				}
				ShitDrvAcquireResourceExclusive(pStreamContext->pResource);
				if (pStreamContext->bIsInit)
				{
					pStreamContext->ulWriteCount++;
					pStreamContext->hProcessId = hProcessId;
				}
				ShitDrvReleaseResource(pStreamContext->pResource);
			}
		}
	} while (0);
	if (pFltNameInfo)
	{
		FltReleaseFileNameInformation(pFltNameInfo);
	}
	if (UniProcessName.Buffer)
	{
		ExFreePool(UniProcessName.Buffer);
	}
	if (pStreamContext)
	{
		FltReleaseContext(pStreamContext);
	}
	return FltStatus;
}

FLT_PREOP_CALLBACK_STATUS FltPreClose(PFLT_CALLBACK_DATA Data, \
									   PCFLT_RELATED_OBJECTS FltObjects, \
									   PVOID *CompletionContext)
{
	NTSTATUS Status;
	UNICODE_STRING UniProcessName;
	FLT_PREOP_CALLBACK_STATUS FltCallBackStatus;
	BOOLEAN bIsDirectory;
	HANDLE hProcessId;
	PRECORD_LIST pRecordList;
	ULONG ulNewCreate;
	BOOLEAN bIsSandboxFileExist;
	ULONG ulIndexFile;
	UNICODE_STRING UniSandBoxPath;
	BOOLEAN bIsExist;
	PSHITDRV_STREAM_CONTEXT pStreamContext;
	BOOLEAN bIsStreamContextCreated;
	UCHAR Md5Sum[16];
	CHAR ExistFileMd5Sum[MAX_PATH];
	CHAR NowFileMd5Sum[MAX_PATH];
	BOOLEAN bIsCopyMd5Sum;
	WCHAR wOperation[50];
	//UNICODE_STRING UniExistFileMd5Sum;
	//UNICODE_STRING UniExistName;
	//WCHAR wIndexFile[50];


	UniProcessName.Buffer = NULL;
	UniProcessName.MaximumLength = 0;
	bIsDirectory = FALSE;
	ulNewCreate = 0;
	hProcessId = 0;
	pRecordList = NULL;
	FltCallBackStatus = FLT_PREOP_SUCCESS_NO_CALLBACK;
	bIsSandboxFileExist = FALSE;
	ulIndexFile = 0;
	UniSandBoxPath.Buffer = NULL;
	UniSandBoxPath.Length = 0;
	UniSandBoxPath.MaximumLength = 0;
	bIsExist = FALSE;
	pStreamContext = NULL;
	bIsStreamContextCreated = FALSE;
	bIsCopyMd5Sum = FALSE;
	//UniExistName.Buffer = NULL;
	//UniExistFileMd5Sum.Buffer = NULL;

	if (FALSE == g_pFilterCtl->FileFltAccessCtl.ulIsStartFilter || \
		FALSE == g_pFilterCtl->FileFltAccessCtl.ulIsInitialize)
	{
		return FltCallBackStatus;
	}
	if ((Data->Iopb->IrpFlags & IRP_PAGING_IO) || \
		(Data->Iopb->IrpFlags & IRP_SYNCHRONOUS_PAGING_IO))
	{
		return FltCallBackStatus;
	}
	if (KeGetCurrentIrql() > APC_LEVEL || \
		NULL == Data || \
		NULL == FltObjects || \
		NULL == FltObjects->FileObject || \
		IRP_MJ_CLOSE != Data->Iopb->MajorFunction || \
		/*STATUS_REPARSE == Data->IoStatus.Status || \*/
		ExGetPreviousMode() == KernelMode)
	{
		return FltCallBackStatus;
	}
	if (g_pFilterCtl->pMyProc == PsGetCurrentProcess())
	{
		return FltCallBackStatus;
	}
	hProcessId = PsGetCurrentProcessId();
	if (FALSE == IsSandboxProcess(hProcessId))
	{
		return FltCallBackStatus;
	}
	Status = FltIsDirectory(FltObjects->FileObject,FltObjects->Instance,&bIsDirectory);
	if (NT_SUCCESS(Status))
	{
		if (bIsDirectory)
		{
			return FltCallBackStatus;
		}
	}
	else
	{
		return FltCallBackStatus;
	}
	do 
	{
		Status = GetProcessFullName(g_pFilterCtl->pFilterHandle, \
			FltObjects->Instance, \
			(PVOID)PsGetCurrentProcess(), \
			&UniProcessName);
		if (NT_SUCCESS(Status))
		{
			Status = FindOrCreateStreamContext(Data,TRUE,&pStreamContext,&bIsStreamContextCreated);
			if (NT_ERROR(Status))
			{
				break;
			}
			if (FALSE ==  bIsStreamContextCreated && \
				pStreamContext->ulWriteCount)
			{
				//Isn't From Sandbox
				Status = ConvertSandboxShortName(&g_pFilterCtl->UniSandBoxPath, \
					&pStreamContext->UniFileName, \
					&UniSandBoxPath, \
					&pStreamContext->UniVolumeName);
				if (NT_SUCCESS(Status))
				{
					if (FltGetFileLength(FltObjects->Filter, \
						FltObjects->Instance, \
						&pStreamContext->UniFileName) <= 0xA00000)
					{
						RtlZeroMemory(Md5Sum,16);
						Status = FltCalcFileDatMd5(FltObjects->Filter, \
							FltObjects->Instance, \
							&pStreamContext->UniFileName, \
							Md5Sum);
						RtlZeroMemory(NowFileMd5Sum,MAX_PATH);
						StringCchPrintfA(NowFileMd5Sum,MAX_PATH,"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",Md5Sum[0], \
							Md5Sum[1],Md5Sum[2], Md5Sum[3],Md5Sum[4],Md5Sum[5],Md5Sum[6],Md5Sum[7],Md5Sum[8], \
							Md5Sum[9],Md5Sum[10],Md5Sum[11],Md5Sum[12],Md5Sum[13],Md5Sum[14],Md5Sum[15]);
						if (NT_SUCCESS(Status))
						{
							bIsCopyMd5Sum = FALSE;
						}
					}
					RtlAppendUnicodeToString(&UniSandBoxPath,L".Post");
					bIsSandboxFileExist = IsSandboxFileExist(g_pFilterCtl->pFilterHandle, \
						FltObjects->Instance, \
						&UniSandBoxPath);
					if (bIsSandboxFileExist)
					{
						//沙箱中存在文件
						if (FltGetFileLength(FltObjects->Filter, \
							FltObjects->Instance, \
							&pStreamContext->UniFileName) <= 0xA00000)
						{
							RtlZeroMemory(Md5Sum,16);
							Status = FltCalcFileDatMd5(FltObjects->Filter, \
								FltObjects->Instance, \
								&UniSandBoxPath, \
								Md5Sum);
							if (NT_SUCCESS(Status))
							{
								RtlZeroMemory(ExistFileMd5Sum,MAX_PATH);
								StringCchPrintfA(ExistFileMd5Sum,MAX_PATH,"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",Md5Sum[0], \
									Md5Sum[1],Md5Sum[2], Md5Sum[3],Md5Sum[4],Md5Sum[5],Md5Sum[6],Md5Sum[7],Md5Sum[8], \
									Md5Sum[9],Md5Sum[10],Md5Sum[11],Md5Sum[12],Md5Sum[13],Md5Sum[14],Md5Sum[15]);
								if (strncmp(ExistFileMd5Sum,NowFileMd5Sum,strlen(NowFileMd5Sum)) != 0)
								{
									Status = DoCopyFile(g_pFilterCtl->pFilterHandle,
										//FltObjects->FileObject,
										NULL,
										FltObjects->Instance,
										&pStreamContext->UniFileName,
										FltObjects->Instance,
										&UniSandBoxPath,
										FALSE,
										TRUE);
									if (NT_SUCCESS(Status))
									{
										DbgPrint("CopyFile Success\r\n");
									}
									bIsCopyMd5Sum = TRUE;
								}
							}
						}
					}
					else
					{
						if (FltGetFileLength(FltObjects->Filter, \
							FltObjects->Instance, \
							&pStreamContext->UniFileName) <= 0xA00000)
						{
							Status = DoCopyFile(g_pFilterCtl->pFilterHandle,
								//FltObjects->FileObject,
								NULL,
								FltObjects->Instance,
								&pStreamContext->UniFileName,
								FltObjects->Instance,
								&UniSandBoxPath,
								FALSE,
								FALSE);
							if (NT_SUCCESS(Status))
							{
								DbgPrint("CopyFile Success\r\n");
							}
							bIsCopyMd5Sum = FALSE;
						}
					}
					ShitDrvAcquireResourceExclusive(pStreamContext->pResource);
					if (pStreamContext->bIsInit)
					{
						pStreamContext->ulCloseCount++;
						pStreamContext->hProcessId = hProcessId;
						if (FALSE == bIsCopyMd5Sum)
						{
							RtlCopyMemory(pStreamContext->PostModifyCopiedMd5Sum,NowFileMd5Sum,strlen(NowFileMd5Sum));
							pStreamContext->bIsPostModifyCopied = TRUE;
						}
						else
						{
							RtlCopyMemory(pStreamContext->PostModifyCopiedMd5Sum,ExistFileMd5Sum,strlen(NowFileMd5Sum));
							pStreamContext->bIsPostModifyCopied = TRUE;
						}
					}
					ShitDrvReleaseResource(pStreamContext->pResource);
					RtlZeroMemory(wOperation,sizeof(WCHAR) * 50);
					RtlCopyMemory(wOperation,L"写入文件",wcslen(L"写入文件") * sizeof(WCHAR));
					pRecordList = AllocateRecordListDat();
					if (pRecordList && InitializeRecordDat(pRecordList, \
						UniProcessName.Buffer, \
						UniProcessName.Length, \
						pStreamContext->UniFileName.Buffer, \
						pStreamContext->UniFileName.Length, \
						wOperation, \
						(ULONG)wcslen(L"写入文件") * sizeof(WCHAR), \
						hProcessId, \
						IsWriteFile, \
						pStreamContext->ulCreateCount, \
						pStreamContext->ulOpenedCount, \
						pStreamContext->ulWriteCount, \
						pStreamContext->ulCleanupCount, \
						pStreamContext->ulCloseCount, \
						IsFsFile))
					{
						RtlCopyMemory(pRecordList->RecordDat.Md5Sum, \
							pStreamContext->PostModifyCopiedMd5Sum, \
							strlen(pStreamContext->PostModifyCopiedMd5Sum));
						InitializeListHead(&pRecordList->RecordList);
						ExInterlockedInsertTailList(&g_RecordListHead,&pRecordList->RecordList,&g_RecordSpinLock);
						KeSetEvent(g_pEventWaitKrl,IO_NO_INCREMENT,FALSE);
						KeClearEvent(g_pEventWaitKrl);
					}
				}
			}
		}
	} while (0);
	if (UniSandBoxPath.Buffer)
	{
		ExFreePool(UniSandBoxPath.Buffer);
	}
	if (UniProcessName.Buffer)
	{
		ExFreePool(UniProcessName.Buffer);
	}
	if (pStreamContext)
	{
		FltReleaseContext(pStreamContext);
	}
	return FltCallBackStatus;
}