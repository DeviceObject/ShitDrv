#include "ShitDrv.h"
#include <ntifs.h>
#include "UtilsApi.h"
#include "global.h"
#include "md5.h"

NTSTATUS FltCalcFileDatMd5(PFLT_FILTER pFltHandle,PFLT_INSTANCE pFltInstance,PUNICODE_STRING pUniFileName,PUCHAR pOutMd5)
{
    NTSTATUS Status;
    LONGLONG ulFileSize;
    PUCHAR pFileDat;
    HANDLE hFile;
    OBJECT_ATTRIBUTES ObjectAttributes;
    IO_STATUS_BLOCK IoStatus;
    PFILE_OBJECT pFileObject;
    ULONG ulReadByteSize;
    MD5_CTX Md5Ctx;
	LARGE_INTEGER ByteOffset;
    

    Status = STATUS_SUCCESS;
    ulFileSize = 0;
    pFileDat = NULL;
    pFileObject = NULL;
    ulReadByteSize = 0;
	ByteOffset.QuadPart = 0;
	hFile = NULL;

    do 
    {
        if (NULL == pFltHandle || \
            NULL == pUniFileName->Buffer || \
            0 >= pUniFileName->Length || \
            NULL == pOutMd5)
        {
            Status = STATUS_INVALID_PARAMETER;
            break;
        }
        ulFileSize = FltGetFileLength(pFltHandle,pFltInstance,pUniFileName);
        if (0 == ulFileSize)
        {
            Status = STATUS_UNSUCCESSFUL;
            break;
        }
        pFileDat = ExAllocatePool(PagedPool,PAGE_SIZE);
        if (NULL == pFileDat)
        {
            Status = STATUS_MEMORY_NOT_ALLOCATED;
            break;
        }
        InitializeObjectAttributes(&ObjectAttributes,pUniFileName,OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE,NULL,0);
        Status = FltCreateFileEx(pFltHandle, \
            pFltInstance, \
            &hFile, \
            &pFileObject, \
            FILE_READ_ATTRIBUTES | FILE_READ_DATA | SYNCHRONIZE, \
            &ObjectAttributes, \
            &IoStatus, \
            NULL, \
            FILE_ATTRIBUTE_NORMAL, \
            FILE_SHARE_READ | FILE_SHARE_DELETE, \
            FILE_OPEN, \
            FILE_SYNCHRONOUS_IO_NONALERT | FILE_NON_DIRECTORY_FILE, \
            NULL, \
            0, \
            IO_NO_PARAMETER_CHECKING);
        if (NT_ERROR(Status))
        {
            break;
        }
		RtlZeroMemory(&Md5Ctx,sizeof(MD5_CTX));
		MD5Init(&Md5Ctx);
		while (ByteOffset.QuadPart < ulFileSize)
		{
			Status = FltReadFile(pFltInstance, \
				pFileObject, \
				&ByteOffset, \
				PAGE_SIZE, \
				pFileDat, \
				FLTFL_IO_OPERATION_NON_CACHED | FLTFL_IO_OPERATION_DO_NOT_UPDATE_BYTE_OFFSET, \
				&ulReadByteSize, \
				NULL, \
				NULL);
			if (NT_ERROR(Status) || ulReadByteSize <= 0)
			{
				break;
			}
			MD5Update(&Md5Ctx,pFileDat,ulReadByteSize);
			ByteOffset.QuadPart += ulReadByteSize;
		}
    } while (0);
	if (ByteOffset.QuadPart)
	{
		MD5Final(pOutMd5,&Md5Ctx);
	}
	if (pFileDat)
	{
		ExFreePool(pFileDat);
	}
    if (pFileObject)
    {
        ObDereferenceObject(pFileObject);
    }
    if (hFile)
    {
        FltClose(hFile);
    }
    return Status;
}
NTSTATUS CalcFileDatMd5(PUNICODE_STRING pUniFileName,PUCHAR pOutMd5)
{
	NTSTATUS Status;
	LONGLONG ulFileSize;
	PUCHAR pFileDat;
	HANDLE hFile;
	OBJECT_ATTRIBUTES ObjectAttributes;
	IO_STATUS_BLOCK IoStatus;
	//ULONG ulReadByteSize;
	MD5_CTX Md5Ctx;
	LARGE_INTEGER ByteOffset;


	Status = STATUS_SUCCESS;
	ulFileSize = 0;
	pFileDat = NULL;
	//ulReadByteSize = 0;
	ByteOffset.QuadPart = 0;
	hFile = NULL;

	do 
	{
		if (NULL == pUniFileName->Buffer || \
			0 >= pUniFileName->Length || \
			NULL == pOutMd5)
		{
			Status = STATUS_INVALID_PARAMETER;
			break;
		}
		ulFileSize = GetFileLength(pUniFileName);
		if (0 == ulFileSize)
		{
			Status = STATUS_UNSUCCESSFUL;
			break;
		}
		pFileDat = ExAllocatePool(PagedPool,PAGE_SIZE);
		if (NULL == pFileDat)
		{
			Status = STATUS_MEMORY_NOT_ALLOCATED;
			break;
		}
		InitializeObjectAttributes(&ObjectAttributes,pUniFileName,OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE,NULL,0);
		Status = ZwCreateFile(&hFile, \
			FILE_READ_ATTRIBUTES | FILE_READ_DATA | SYNCHRONIZE, \
			&ObjectAttributes, \
			&IoStatus, \
			NULL, \
			FILE_ATTRIBUTE_NORMAL, \
			FILE_SHARE_READ | FILE_SHARE_DELETE, \
			FILE_OPEN, \
			FILE_SYNCHRONOUS_IO_NONALERT | FILE_NON_DIRECTORY_FILE, \
			NULL, \
			0);
		if (NT_ERROR(Status))
		{
			break;
		}
		RtlZeroMemory(&Md5Ctx,sizeof(MD5_CTX));
		MD5Init(&Md5Ctx);
		while (ByteOffset.QuadPart < ulFileSize)
		{
			Status = ZwReadFile(hFile, \
				NULL, \
				NULL, \
				NULL, \
				&IoStatus, \
				pFileDat, \
				PAGE_SIZE, \
				&ByteOffset, \
				NULL);
			if (NT_ERROR(Status) || IoStatus.Information <= 0)
			{
				break;
			}
			MD5Update(&Md5Ctx,pFileDat,(ULONG)IoStatus.Information);
			ByteOffset.QuadPart += IoStatus.Information;
		}
	} while (0);
	if (ByteOffset.QuadPart)
	{
		MD5Final(pOutMd5,&Md5Ctx);
	}
	if (pFileDat)
	{
		ExFreePool(pFileDat);
	}
	if (hFile)
	{
		ZwClose(hFile);
	}
	return Status;
}
VOID System_Sleep(LONGLONG sec)
{
    LARGE_INTEGER interval;

    interval.QuadPart = (sec * DELAY_ONE_SECOND);
    KeDelayExecutionThread( KernelMode, FALSE, &interval );		
}
PVOID MyAllocateMemory(POOL_TYPE PoolType,SIZE_T NumberOfBytes)
{
    PVOID pBuffer;

    pBuffer = NULL;

    do 
    {
        pBuffer = ExAllocatePoolWithTag(PoolType,NumberOfBytes,'MLAM');
    } while (pBuffer == NULL);
    RtlZeroMemory(pBuffer,NumberOfBytes);
    return pBuffer;
}
//输入\\??\\c:-->\\device\\\harddiskvolume1
//LinkTarget.Buffer注意要释放
NTSTATUS QuerySymbolicLink(PUNICODE_STRING pUniSymbolicLinkName, \
                           PUNICODE_STRING pUniLinkTarget)                                  
{
    OBJECT_ATTRIBUTES ObjectAttributes;
    NTSTATUS Status;
    HANDLE hSymLink;

    Status = STATUS_SUCCESS;
    hSymLink = NULL;

    InitializeObjectAttributes(&ObjectAttributes, \
        pUniSymbolicLinkName, \
        OBJ_CASE_INSENSITIVE, \
        0, \
        0);
    Status = ZwOpenSymbolicLinkObject(&hSymLink, \
        GENERIC_READ, \
        &ObjectAttributes);
    if (!NT_SUCCESS(Status))
    {
        return Status;
    }

    pUniLinkTarget->MaximumLength = MAX_PATH * sizeof(WCHAR);
    pUniLinkTarget->Length = 0;
    pUniLinkTarget->Buffer = ExAllocatePoolWithTag(NonPagedPool, \
        pUniLinkTarget->MaximumLength, \
        'SOD');
    if (!pUniLinkTarget->Buffer)
    {
        ZwClose(hSymLink);
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    RtlZeroMemory(pUniLinkTarget->Buffer,pUniLinkTarget->MaximumLength);
    Status = ZwQuerySymbolicLinkObject(hSymLink,pUniLinkTarget,NULL);
    ZwClose(hSymLink);
    if (!NT_SUCCESS(Status))
    {
        ExFreePool(pUniLinkTarget->Buffer);
    }
    return Status;
}
//输入\\Device\\harddiskvolume1
//输出C:
//DosName.Buffer的内存记得释放
NTSTATUS MyRtlVolumeDeviceToDosName(PUNICODE_STRING pUniDeviceName, \
                                    PUNICODE_STRING pUniDosName)
{
    NTSTATUS Status;
    UNICODE_STRING UniDriveLetterName = {0};
    WCHAR wDriveLetterNameBuf[128] = {0};
    WCHAR wC = L'\0';
    WCHAR DriLetter[3] = {0};
    UNICODE_STRING UniLinkTarget = {0};

    for (wC = L'A';wC <= L'Z';wC++)
    {
        RtlInitEmptyUnicodeString(&UniDriveLetterName,wDriveLetterNameBuf,sizeof(wDriveLetterNameBuf));
        RtlAppendUnicodeToString(&UniDriveLetterName, L"\\??\\");
        DriLetter[0] = wC;
        DriLetter[1] = L':';
        DriLetter[2] = 0;
        RtlAppendUnicodeToString(&UniDriveLetterName,DriLetter);
        Status = QuerySymbolicLink(&UniDriveLetterName,&UniLinkTarget);
        if (!NT_SUCCESS(Status))
        {
            continue;
        }
        if (RtlEqualUnicodeString(&UniLinkTarget,pUniDeviceName,TRUE))
        {
            ExFreePool(UniLinkTarget.Buffer);
            break;
        }
        ExFreePool(UniLinkTarget.Buffer);
    }
    if (wC <= L'Z')
    {
        pUniDosName->Buffer = ExAllocatePoolWithTag(PagedPool,3 * sizeof(WCHAR),'SOD');
        if (!pUniDosName->Buffer)
        {
            return STATUS_INSUFFICIENT_RESOURCES;
        }
        pUniDosName->MaximumLength = 6;
        pUniDosName->Length   = 4;
        *pUniDosName->Buffer  = wC;
        *(pUniDosName->Buffer+ 1) = ':';
        *(pUniDosName->Buffer+ 2) = 0;
        return STATUS_SUCCESS;
    }
    return Status;
} 
//c:\\windows\\hi.txt<--\\device\\harddiskvolume1\\windows\\hi.txt
BOOLEAN GetNTLinkName(WCHAR *wNTName,WCHAR *wFileName)
{
    UNICODE_STRING UniFileName = {0};
    UNICODE_STRING UniDosName = {0};
    UNICODE_STRING UniDeviceName = {0};

    WCHAR *wPath = NULL;
    ULONG i = 0;
    ULONG ulSepNum = 0;


    if (wFileName == NULL || \
        wNTName == NULL || \
        _wcsnicmp(wNTName, \
        L"\\device\\harddiskvolume", \
        wcslen(L"\\device\\harddiskvolume")) != 0)
    {
        return FALSE;
    }
    UniFileName.Buffer = wFileName;
    UniFileName.Length = 0;
    UniFileName.MaximumLength = sizeof(WCHAR) * MAX_PATH;

    while(wNTName[i] != L'\0')
    {

        if (wNTName[i] == L'\0')
        {
            break;
        }
        if (wNTName[i] == L'\\')
        {
            ulSepNum++;
        }
        if (ulSepNum == 3)
        {
            wNTName[i] = UNICODE_NULL;
            wPath = &wNTName[i + 1];
            break;
        }
        i++;
    }
    if (wPath == NULL)
    {
        return FALSE;
    }
    RtlInitUnicodeString(&UniDeviceName,wNTName);
    if (!NT_SUCCESS(MyRtlVolumeDeviceToDosName(&UniDeviceName,&UniDosName)))
    {
        return FALSE;
    }
    RtlCopyUnicodeString(&UniFileName,&UniDosName);
    RtlAppendUnicodeToString(&UniFileName,L"\\");
    RtlAppendUnicodeToString(&UniFileName,wPath);
    ExFreePool(UniDosName.Buffer);
    return TRUE;
}
BOOLEAN QueryVolumeName(WCHAR wCh,WCHAR* wName,USHORT uSize)
{
    WCHAR wVolume[7] = L"\\??\\C:";
    UNICODE_STRING LinkName;
    UNICODE_STRING VolName;
    UNICODE_STRING ustrTarget;
    NTSTATUS Status = 0;

    RtlInitUnicodeString(&LinkName,wVolume);

    wVolume[4] = wCh;

    ustrTarget.Buffer = wName;
    ustrTarget.Length = 0;
    ustrTarget.MaximumLength = uSize;

    Status = QuerySymbolicLink(&LinkName,&VolName);
    if (NT_SUCCESS(Status))
    {
        RtlCopyUnicodeString(&ustrTarget,&VolName);
        ExFreePool(VolName.Buffer);
    }
    return NT_SUCCESS(Status);
}
NTSTATUS GetProcessPath(PVOID pEProc,PUNICODE_STRING pUniProcPath)
{
	NTSTATUS Status;
	HANDLE hProcessHandle;
	ULONG ulNeedSize;
	PVOID pDat;

	ulNeedSize = 0;
	Status = STATUS_SUCCESS;
	pDat = NULL;
	hProcessHandle = NULL;
	pUniProcPath->Buffer = NULL;

	do 
	{
		if (!MmIsAddressValid(pUniProcPath))
		{
			Status = STATUS_INVALID_PARAMETER_2;
			break;
		}
		Status = ObOpenObjectByPointer(pEProc, \
			OBJ_KERNEL_HANDLE, \
			NULL, \
			GENERIC_READ, \
			*PsProcessType, \
			KernelMode, \
			&hProcessHandle);
		if (NT_ERROR(Status))
		{
			break;
		}
		Status = ZwQueryInformationProcess(hProcessHandle, \
			ProcessImageFileName, \
			NULL, \
			0, \
			&ulNeedSize);
		if (STATUS_INFO_LENGTH_MISMATCH != Status)
		{
			Status = STATUS_MEMORY_NOT_ALLOCATED;
			break; 
		}
		do 
		{
			pDat = ExAllocatePool(NonPagedPool,ulNeedSize);
		} while (NULL == pDat);
		RtlZeroMemory(pDat,ulNeedSize);
		Status = ZwQueryInformationProcess(hProcessHandle, \
			ProcessImageFileName, \
			pDat, \
			ulNeedSize, \
			&ulNeedSize);
		if (NT_ERROR(Status))
		{
			break;
		}
		if (MmIsAddressValid(pDat))
		{
			if (((PUNICODE_STRING)pDat)->Length <= 0 || \
				((PUNICODE_STRING)pDat)->Buffer == NULL)
			{
				break;
			}
		}
		pUniProcPath->Buffer = ExAllocatePool(NonPagedPool, \
			((PUNICODE_STRING)pDat)->MaximumLength);
		if (NULL == pUniProcPath->Buffer)
		{
			break;
		}
		pUniProcPath->MaximumLength = ((PUNICODE_STRING)pDat)->MaximumLength;
		RtlZeroMemory(pUniProcPath->Buffer,pUniProcPath->MaximumLength);
		RtlCopyMemory(pUniProcPath->Buffer, \
			((PUNICODE_STRING)pDat)->Buffer, \
			((PUNICODE_STRING)pDat)->Length);
		pUniProcPath->Length = ((PUNICODE_STRING)pDat)->Length;
	} while (0);
	if (pDat)
	{
		ExFreePool(pDat);
	}
	if (hProcessHandle)
	{
		ZwClose(hProcessHandle);
	}
	return Status;
}
//\\??\\c:\\windows\\hi.txt-->\\device\\harddiskvolume1\\windows\\hi.txt
BOOLEAN GetNtDeviceName(WCHAR *wFilename,WCHAR *wNtName)
{
    UNICODE_STRING UniVolName = {0,0,0};
    WCHAR wVolName[MAX_PATH] = L"";
    WCHAR wTmpName[MAX_PATH] = L"";
    WCHAR wChVol = L'\0';
    WCHAR *wPath = NULL;
    int i = 0;


    RtlStringCbCopyW(wTmpName,MAX_PATH * sizeof(WCHAR),wFilename);
    for(i = 1;i < MAX_PATH - 1;i++)
    {
        if(wTmpName[i] == L':')
        {
            wPath = &wTmpName[(i + 1) % MAX_PATH];
            wChVol = wTmpName[i - 1];
            break;
        }
    }

    if(wPath == NULL)
    {
        return FALSE;
    }

    if(wChVol == L'?')
    {
        UniVolName.Length = 0;
        UniVolName.MaximumLength = MAX_PATH * sizeof(WCHAR);
        UniVolName.Buffer = wNtName;
        RtlAppendUnicodeToString(&UniVolName,L"\\Device\\HarddiskVolume?");
        RtlAppendUnicodeToString(&UniVolName,wPath);
        return TRUE;
    }
    else if(QueryVolumeName(wChVol,wVolName,MAX_PATH * sizeof(WCHAR)))
    {
        UniVolName.Length = 0;
        UniVolName.MaximumLength = MAX_PATH * sizeof(WCHAR);
        UniVolName.Buffer = wNtName;
        RtlAppendUnicodeToString(&UniVolName,wVolName);
        RtlAppendUnicodeToString(&UniVolName,wPath);
        return TRUE;
    }
    return FALSE;
}
NTSTATUS GetProcessFullName(PFLT_FILTER pFltHandle,PFLT_INSTANCE pFltInstance,PVOID pEProcess,PUNICODE_STRING pUniFullName)
{
	NTSTATUS Status;
	KAPC_STATE ApcState;
	ULONG ulNeedSize;
	PVOID pDat;
	HANDLE hProcessHandle;

	Status = STATUS_UNSUCCESSFUL;
	pDat = NULL;
	hProcessHandle = NULL;

	KeStackAttachProcess(pEProcess,&ApcState);
	do 
	{
		Status = ObOpenObjectByPointer(pEProcess, \
			OBJ_KERNEL_HANDLE, \
			NULL, \
			GENERIC_READ, \
			*PsProcessType, \
			KernelMode, \
			&hProcessHandle);
		if (NT_ERROR(Status))
		{
			break;
		}
		Status = ZwQueryInformationProcess(hProcessHandle, \
			ProcessImageFileName, \
			NULL, \
			0, \
			&ulNeedSize);
		if (STATUS_INFO_LENGTH_MISMATCH != Status)
		{
			Status = STATUS_MEMORY_NOT_ALLOCATED;
			break; 
		}
		do 
		{
			pDat = ExAllocatePoolWithTag(NonPagedPool,ulNeedSize,SHITDRV_TAG);
		} while (NULL == pDat);
		RtlZeroMemory(pDat,ulNeedSize);
		Status = ZwQueryInformationProcess(hProcessHandle, \
			ProcessImageFileName, \
			pDat, \
			ulNeedSize, \
			&ulNeedSize);
		if (NT_ERROR(Status))
		{
			break;
		}
		if (((PUNICODE_STRING)pDat)->Length <= 0 && ((PUNICODE_STRING)pDat)->Buffer == NULL)
		{
			break;
		}
		pUniFullName->MaximumLength = ((PUNICODE_STRING)pDat)->MaximumLength;
		do 
		{
			pUniFullName->Buffer = ExAllocatePool(NonPagedPool, \
				pUniFullName->MaximumLength);
		} while (NULL == pUniFullName->Buffer);

		RtlZeroMemory(pUniFullName->Buffer, \
			pUniFullName->MaximumLength);
		RtlCopyMemory(pUniFullName->Buffer, \
			((PUNICODE_STRING)pDat)->Buffer, \
			((PUNICODE_STRING)pDat)->Length);
		pUniFullName->Length = ((PUNICODE_STRING)pDat)->Length;
	} while (0);
	if (pDat)
	{
		ExFreePoolWithTag(pDat,SHITDRV_TAG);
	}
	if (hProcessHandle)
	{
		ZwClose(hProcessHandle);
	}
	KeUnstackDetachProcess(&ApcState);
	return Status;
}
NTSTATUS GetProcessFullName_Bak(PFLT_FILTER pFltHandle,PFLT_INSTANCE pFltInstance,HANDLE hProcessId,PVOID pEProcess,PUNICODE_STRING pUniFullName)
{
	NTSTATUS Status,DeviceStatus;
	KAPC_STATE ApcState;
	ULONG ulNeedSize;
	PVOID pDat;
	HANDLE hFile;
	IO_STATUS_BLOCK IoStatus;
	OBJECT_ATTRIBUTES ObjectAttributes;
	PFILE_OBJECT pFileObject;
	WCHAR wFileBuffer[MAX_PATH*2];
    WCHAR *wCleanPath;
	PFILE_NAME_INFORMATION FileNameInfo;
	UNICODE_STRING UniProcessPath;
	UNICODE_STRING UniDosDeviceName;
    HANDLE hProcessHandle;
    POBJECT_NAME_INFORMATION pObjNameInfo;

	Status = STATUS_UNSUCCESSFUL;
	pDat = NULL;
	pFileObject = NULL;
    hFile = NULL;
    wCleanPath = NULL;

    UniProcessPath.Length = 0;
    UniProcessPath.MaximumLength = 0;
    UniProcessPath.Buffer = NULL;

    UniDosDeviceName.Buffer = NULL;
    UniDosDeviceName.Length = 0;
    UniDosDeviceName.MaximumLength = 0;
    hProcessHandle = NULL;
    pObjNameInfo = NULL;

    RtlZeroMemory(wFileBuffer,sizeof(WCHAR) * MAX_PATH*2);
    
    if (NULL == pUniFullName->Buffer || \
        !MmIsAddressValid(pUniFullName->Buffer))
    {
        return STATUS_INVALID_PARAMETER_3;
    }
    KeStackAttachProcess(pEProcess,&ApcState);
	do 
	{
       Status = ObOpenObjectByPointer(pEProcess, \
           OBJ_KERNEL_HANDLE, \
           NULL, \
           GENERIC_READ, \
           *PsProcessType, \
           KernelMode, \
           &hProcessHandle);
       if (NT_ERROR(Status))
       {
           break;
       }
		Status = ZwQueryInformationProcess(hProcessHandle, \
			ProcessImageFileName, \
			NULL, \
			0, \
			&ulNeedSize);
		if (STATUS_INFO_LENGTH_MISMATCH != Status)
		{
			Status = STATUS_MEMORY_NOT_ALLOCATED;
			break; 
		}
		do 
		{
			pDat = ExAllocatePoolWithTag(NonPagedPool,ulNeedSize,SHITDRV_TAG);
		} while (NULL == pDat);
		RtlZeroMemory(pDat,ulNeedSize);
		Status = ZwQueryInformationProcess(hProcessHandle, \
			ProcessImageFileName, \
			pDat, \
			ulNeedSize, \
			&ulNeedSize);
		if (NT_ERROR(Status))
		{
			break;
		}
        if (((PUNICODE_STRING)pDat)->Length <= 0 && ((PUNICODE_STRING)pDat)->Buffer == NULL)
        {
            break;
        }
        do 
        {
            UniProcessPath.Buffer = ExAllocatePoolWithTag(NonPagedPool, \
                ((PUNICODE_STRING)pDat)->MaximumLength, \
                SHITDRV_TAG);
        } while (NULL == UniProcessPath.Buffer);

        RtlZeroMemory(UniProcessPath.Buffer, \
            ((PUNICODE_STRING)pDat)->MaximumLength);
        RtlCopyMemory(UniProcessPath.Buffer, \
            ((PUNICODE_STRING)pDat)->Buffer, \
            ((PUNICODE_STRING)pDat)->Length);

        UniProcessPath.Length = ((PUNICODE_STRING)pDat)->Length;
        UniProcessPath.MaximumLength = ((PUNICODE_STRING)pDat)->MaximumLength;
		InitializeObjectAttributes(&ObjectAttributes,&UniProcessPath,OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE,NULL,0);
		Status = FltCreateFileEx(pFltHandle, \
            pFltInstance, \
            &hFile, \
            &pFileObject, \
			FILE_READ_ATTRIBUTES, \
			&ObjectAttributes, \
			&IoStatus, \
			NULL, \
			FILE_ATTRIBUTE_NORMAL, \
			FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, \
			FILE_OPEN, \
			FILE_SYNCHRONOUS_IO_NONALERT | FILE_NON_DIRECTORY_FILE, \
			NULL, \
			0, \
            IO_NO_PARAMETER_CHECKING);
		if (NT_ERROR(Status))
		{
			break;
		}
		//Status = ObReferenceObjectByHandle(hFile, \
		//	0, \
		//	*IoFileObjectType, \
		//	KernelMode, \
		//	(PVOID *)&pFileObject, \
		//	NULL);
		//if (NT_ERROR(Status))
		//{
		//	break;
		//}
		FileNameInfo = (PFILE_NAME_INFORMATION)wFileBuffer;
        Status = FltQueryInformationFile(pFltInstance, \
            pFileObject, \
            FileNameInfo, \
            sizeof(WCHAR) * MAX_PATH, \
            FileNameInformation, \
            &ulNeedSize);
        if (NT_ERROR(Status))
        {
            break;
        }
		if (pFileObject->DeviceObject == NULL)
		{
			DeviceStatus = STATUS_DEVICE_DOES_NOT_EXIST;
			break;
		}
        //PsReferenceProcessFilePointer
        Status = ObQueryNameString(pFileObject->DeviceObject,pObjNameInfo,0,&ulNeedSize);
        if (Status == STATUS_INFO_LENGTH_MISMATCH)
        {
            pObjNameInfo = ExAllocatePool(NonPagedPool,ulNeedSize);
            if (NULL == pObjNameInfo)
            {
                break;
            }
            RtlZeroMemory(pObjNameInfo,ulNeedSize);
            Status = ObQueryNameString(pFileObject->DeviceObject,pObjNameInfo,ulNeedSize,&ulNeedSize);
            if (NT_SUCCESS(Status))
            {
                do 
                {
                    UniDosDeviceName.Buffer = ExAllocatePool(NonPagedPool,pObjNameInfo->Name.MaximumLength);
                } while (NULL == UniDosDeviceName.Buffer);
                RtlZeroMemory(UniDosDeviceName.Buffer,pObjNameInfo->Name.MaximumLength);
                RtlCopyUnicodeString(&UniDosDeviceName,&pObjNameInfo->Name);
            }
            else
            {
                if (KeAreAllApcsDisabled() == FALSE)
                {
                    DeviceStatus = IoVolumeDeviceToDosName(pFileObject->DeviceObject,&UniDosDeviceName);
                }
                else
                {
                    do 
                    {
                        UniDosDeviceName.Buffer = ExAllocatePool(NonPagedPool,sizeof(WCHAR) * MAX_PATH);
                    } while (NULL == UniDosDeviceName.Buffer);
                    RtlZeroMemory(UniDosDeviceName.Buffer,sizeof(WCHAR) * MAX_PATH);
                    DeviceStatus = RtlVolumeDeviceToDosName(pFileObject->DeviceObject,&UniDosDeviceName);
                }
            }
        }
	} while (0);
    if (NT_SUCCESS(Status))
    {
        if (UniProcessPath.Buffer)
        {
            ExFreePoolWithTag(UniProcessPath.Buffer,SHITDRV_TAG);
        }
        if (MmIsAddressValid(FileNameInfo->FileName))
        {
            if (NULL == wCleanPath)
            {
                wCleanPath = ExAllocatePoolWithTag(NonPagedPool,FileNameInfo->FileNameLength,'ClrP');
                if (NULL == wCleanPath)
                {
                    wCleanPath = ExAllocatePoolWithTag(NonPagedPool,FileNameInfo->FileNameLength,'ClrP');
                }
                RtlZeroMemory(wCleanPath,FileNameInfo->FileNameLength);
            }
            RtlCopyMemory(wCleanPath,FileNameInfo->FileName,FileNameInfo->FileNameLength/* * sizeof(WCHAR)*/);
            RtlInitUnicodeString(&UniProcessPath,wCleanPath);
            if (NT_SUCCESS(DeviceStatus))
            {
                RtlCopyUnicodeString(pUniFullName,&UniDosDeviceName);
                RtlUnicodeStringCatEx(pUniFullName,&UniProcessPath,NULL,STRSAFE_IGNORE_NULLS);
            }
            else
            {
                RtlCopyUnicodeString(pUniFullName,&UniProcessPath);
            }
            if (wCleanPath)
            {
                ExFreePoolWithTag(wCleanPath,'ClrP');
            }
        }
    }
    if (pObjNameInfo)
    {
        ExFreePool(pObjNameInfo);
    }
    if (pFileObject)
    {
        ObDereferenceObject(pFileObject);
    }
    if (hFile)
    {
        FltClose(hFile);
    }
    if (pDat)
    {
        ExFreePoolWithTag(pDat,SHITDRV_TAG);
    }
    if (UniDosDeviceName.Buffer)
    {
        ExFreePoolWithTag(UniDosDeviceName.Buffer,SHITDRV_TAG);
    }
    if (hProcessHandle)
    {
        ZwClose(hProcessHandle);
    }
    //if (UniDosDeviceName.Buffer)
    //{
    //    ExFreePool(UniDosDeviceName.Buffer);
    //}
	KeUnstackDetachProcess(&ApcState);
	return Status;
}
NTSTATUS GetProcessFullName_2(PFLT_FILTER pFltHandle,PFLT_INSTANCE pFltInstance,PVOID pEProcess,PUNICODE_STRING pUniFullName)
{
    NTSTATUS Status,DeviceStatus;
    KAPC_STATE ApcState;
    ULONG ulNeedSize;
    PVOID pDat;
    HANDLE hFile;
    IO_STATUS_BLOCK IoStatus;
    OBJECT_ATTRIBUTES ObjectAttributes;
    PFILE_OBJECT pFileObject;
    WCHAR wFileBuffer[MAX_PATH];
    WCHAR wCleanPath[MAX_PATH];
    PFILE_NAME_INFORMATION FileNameInfo;
    UNICODE_STRING UniProcessPath;
    UNICODE_STRING UniDosDeviceName;

    Status = STATUS_UNSUCCESSFUL;
    pDat = NULL;
    pFileObject = NULL;
    hFile = NULL;
    UniProcessPath.Length = 0;
    UniProcessPath.MaximumLength = 0;
    UniProcessPath.Buffer = NULL;

    UniDosDeviceName.Buffer = NULL;
    UniDosDeviceName.Length = 0;
    UniDosDeviceName.MaximumLength = 0;

    RtlZeroMemory(wFileBuffer,sizeof(WCHAR) * MAX_PATH);
    RtlZeroMemory(wCleanPath,sizeof(WCHAR) * MAX_PATH);

    if (NULL == pUniFullName->Buffer || \
        !MmIsAddressValid(pUniFullName->Buffer))
    {
        return STATUS_INVALID_PARAMETER_3;
    }
    KeStackAttachProcess(pEProcess,&ApcState);
    do 
    {
        Status = ZwQueryInformationProcess(NtCurrentProcess(), \
            ProcessImageFileName, \
            NULL, \
            0, \
            &ulNeedSize);
        if (STATUS_INFO_LENGTH_MISMATCH != Status)
        {
            Status = STATUS_MEMORY_NOT_ALLOCATED;
            break; 
        }
        do 
        {
            pDat = ExAllocatePoolWithTag(NonPagedPool,ulNeedSize,SHITDRV_TAG);
        } while (NULL == pDat);
        RtlZeroMemory(pDat,ulNeedSize);

        Status = ZwQueryInformationProcess(NtCurrentProcess(), \
            ProcessImageFileName, \
            pDat, \
            ulNeedSize, \
            &ulNeedSize);
        if (NT_ERROR(Status))
        {
            break;
        }
        do 
        {
            UniProcessPath.Buffer = ExAllocatePoolWithTag(NonPagedPool, \
                ((PUNICODE_STRING)pDat)->MaximumLength, \
                SHITDRV_TAG);
        } while (NULL == UniProcessPath.Buffer);
        RtlZeroMemory(UniProcessPath.Buffer,((PUNICODE_STRING)pDat)->MaximumLength);
        //RtlCopyUnicodeString(&UniProcessPath,(PUNICODE_STRING)pDat);
        //StringCchCopyW(UniProcessPath.Buffer,MAX_PATH,((PUNICODE_STRING)pDat)->Buffer);
        RtlCopyMemory(UniProcessPath.Buffer,((PUNICODE_STRING)pDat)->Buffer,((PUNICODE_STRING)pDat)->Length);
        UniProcessPath.Length = ((PUNICODE_STRING)pDat)->Length;
        UniProcessPath.MaximumLength = ((PUNICODE_STRING)pDat)->MaximumLength;
        InitializeObjectAttributes(&ObjectAttributes,&UniProcessPath,OBJ_CASE_INSENSITIVE,NULL,0);
        Status = FltCreateFile(pFltHandle, \
            pFltInstance, \
            &hFile, \
            FILE_READ_ATTRIBUTES, \
            &ObjectAttributes, \
            &IoStatus, \
            NULL, \
            FILE_ATTRIBUTE_NORMAL, \
            0, \
            FILE_OPEN, \
            FILE_SYNCHRONOUS_IO_NONALERT | FILE_NON_DIRECTORY_FILE, \
            NULL, \
            0, \
            IO_NO_PARAMETER_CHECKING);
        if (NT_ERROR(Status))
        {
            break;
        }
        Status = ObReferenceObjectByHandle(hFile, \
            0, \
            *IoFileObjectType, \
            KernelMode, \
            (PVOID *)&pFileObject, \
            NULL);
        if (NT_ERROR(Status))
        {
            break;
        }
        FileNameInfo = (PFILE_NAME_INFORMATION)wFileBuffer;
        Status = ZwQueryInformationFile(hFile, \
            &IoStatus, \
            FileNameInfo, \
            sizeof(WCHAR) * MAX_PATH, \
            FileNameInformation);
        if (NT_ERROR(Status))
        {
            break;
        }
        //Status = ZwQueryInformationFile(hFile, \
        //	&IoStatus, \
        //	FileNameInfo, \
        //	sizeof(WCHAR) * MAX_PATH, \
        //	FileNameInformation);
        //if (NT_ERROR(Status))
        //{
        //	break;
        //}
        if (pFileObject->DeviceObject == NULL)
        {
            DeviceStatus = STATUS_DEVICE_DOES_NOT_EXIST;
            break;
        }
        do 
        {
            UniDosDeviceName.Buffer = ExAllocatePoolWithTag(NonPagedPool,sizeof(WCHAR) * MAX_PATH,SHITDRV_TAG);
        } while (NULL == UniDosDeviceName.Buffer);
        RtlZeroMemory(UniDosDeviceName.Buffer,sizeof(WCHAR) * MAX_PATH);
        //DeviceStatus = RtlVolumeDeviceToDosName(pFileObject->DeviceObject,&UniDosDeviceName);
        DeviceStatus = RtlVolumeDeviceToDosName(pFileObject->DeviceObject,&UniDosDeviceName);
    } while (0);

    if (NULL != pFileObject)
    {
        ObDereferenceObject(pFileObject);
    }
    if (hFile)
    {
        FltClose(hFile);
    }
    if (pDat)
    {
        ExFreePoolWithTag(pDat,SHITDRV_TAG);
    }
    if (NT_SUCCESS(Status))
    {
        if (UniProcessPath.Buffer)
        {
            ExFreePoolWithTag(UniProcessPath.Buffer,SHITDRV_TAG);
        }
        RtlCopyMemory(wCleanPath,FileNameInfo->FileName,FileNameInfo->FileNameLength * sizeof(WCHAR));
        RtlInitUnicodeString(&UniProcessPath,wCleanPath);
        if (NT_SUCCESS(DeviceStatus))
        {
            RtlCopyUnicodeString(pUniFullName,&UniDosDeviceName);
            RtlUnicodeStringCatEx(pUniFullName,&UniProcessPath,NULL,STRSAFE_IGNORE_NULLS);
        }
        else
        {
            RtlCopyUnicodeString(pUniFullName,&UniProcessPath);
        }
    }
    if (UniDosDeviceName.Buffer)
    {
        ExFreePoolWithTag(UniDosDeviceName.Buffer,SHITDRV_TAG);
    }
    KeUnstackDetachProcess(&ApcState);
    return Status;
}
NTSTATUS GetCurProcFullPath(PFLT_FILTER pFltHandle,PFLT_INSTANCE pFltInstance,WCHAR *wOutFullPath,ULONG ulMaximum)
{
    NTSTATUS Status;
    UNICODE_STRING UniProcessFullName;

    Status = STATUS_SUCCESS;
    UniProcessFullName.Buffer = NULL;
    UniProcessFullName.Length = 0;
    UniProcessFullName.MaximumLength = 0;

    Status = GetProcessFullName(pFltHandle,pFltInstance,PsGetCurrentProcess(),&UniProcessFullName);
    if (NT_SUCCESS(Status) && UniProcessFullName.Buffer != NULL && UniProcessFullName.Length > 0)
    {
        StringCchCopyW(wOutFullPath,ulMaximum,UniProcessFullName.Buffer);
    }
    if (UniProcessFullName.Buffer)
    {
        ExFreePool(UniProcessFullName.Buffer);
    }
    return Status;
}
BOOLEAN GetRegistryObjectCompleteName(PUNICODE_STRING pRegistryPath, \
                                      PUNICODE_STRING pPartialRegistryPath, \
                                      PVOID pRegistryObject)
{
    BOOLEAN bRet;
    BOOLEAN bFoundCompleteName;
    BOOLEAN bPartial;
    ULONG ulReturnedLength;
    POBJECT_NAME_INFORMATION pObjectName;
    NTSTATUS Status;

    bRet = FALSE;
    bFoundCompleteName = FALSE;
    bPartial = FALSE;
    pObjectName = NULL;
    Status = STATUS_SUCCESS;

    if((!MmIsAddressValid(pRegistryObject)) || (pRegistryObject == NULL))
    {
        return bRet;
    }
    /* Check to see if the partial name is really the complete name */
    if(pPartialRegistryPath != NULL)
    {
        if((((pPartialRegistryPath->Buffer[0] == '\\') || \
              (pPartialRegistryPath->Buffer[0] == '%')) || \
              ((pPartialRegistryPath->Buffer[0] == 'T') && \
              (pPartialRegistryPath->Buffer[1] == 'R') && \
              (pPartialRegistryPath->Buffer[2] == 'Y') && \
              (pPartialRegistryPath->Buffer[3] == '\\'))))
        {
            RtlCopyUnicodeString(pRegistryPath,pPartialRegistryPath);
            bPartial = TRUE;
            bFoundCompleteName = TRUE;
        }
    }
    if(!bFoundCompleteName)
    {
        /* Query the object manager in the kernel for the complete name */
        Status = ObQueryNameString(pRegistryObject,NULL,0,&ulReturnedLength);
        if(Status == STATUS_INFO_LENGTH_MISMATCH)
        {
            pObjectName = ExAllocatePoolWithTag(NonPagedPool,ulReturnedLength,'NCOR');
            Status = ObQueryNameString(pRegistryObject,pObjectName,ulReturnedLength,&ulReturnedLength);
            if(NT_SUCCESS(Status))
            {
                RtlCopyUnicodeString(pRegistryPath,&pObjectName->Name);
                bFoundCompleteName = TRUE;
            }
            if (pObjectName)
            {
                ExFreePoolWithTag(pObjectName,'NCOR');
            }
        }
    }
    return bFoundCompleteName;
}
NTSTATUS FltQueryInformationFileSyncronous(PFLT_INSTANCE Instance, \
                                           PFILE_OBJECT FileObject, \
                                           PVOID FileInformation, \
                                           ULONG Length, \
                                           FILE_INFORMATION_CLASS FileInformationClass, \
                                           PULONG LengthReturned)
/*++

Routine Description:

    This routine returns the requested information about a specified file.
    The information returned is determined by the FileInformationClass that
    is specified, and it is placed into the caller's FileInformation buffer.

Arguments:

    Instance - Supplies the Instance initiating this IO.

    FileObject - Supplies the file object about which the requested
        information should be returned.

    FileInformationClass - Specifies the type of information which should be
        returned about the file.

    Length - Supplies the length, in bytes, of the FileInformation buffer.

    FileInformation - Supplies a buffer to receive the requested information
        returned about the file.  This must be a buffer allocated from kernel
        space.

    LengthReturned - the number of bytes returned if the operation was
        successful.

Return Value:

    The status returned is the final completion status of the operation.

--*/

{
#if (NTDDI_VERSION >= NTDDI_LONGHORN)

	return FltQueryInformationFile(Instance,
									FileObject,
									FileInformation,
									Length,
									FileInformationClass,
									LengthReturned);

#else
    PFLT_CALLBACK_DATA data;
    NTSTATUS Status;

    PAGED_CODE();

    Status = FltAllocateCallbackData(Instance,FileObject,&data);

    if (!NT_SUCCESS(Status))
    {
        return Status;
    }

    //
    //  Fill out callback data
    //

    data->Iopb->MajorFunction = IRP_MJ_QUERY_INFORMATION;
    data->Iopb->Parameters.QueryFileInformation.FileInformationClass = FileInformationClass;
    data->Iopb->Parameters.QueryFileInformation.Length = Length;
    data->Iopb->Parameters.QueryFileInformation.InfoBuffer = FileInformation;
    data->Iopb->IrpFlags = IRP_SYNCHRONOUS_API;


    FltPerformSynchronousIo(data);

    //
    //  Return Results
    //

    Status = data->IoStatus.Status;

    if (NT_SUCCESS(Status) &&
        ARGUMENT_PRESENT(LengthReturned))
    {
        *LengthReturned = (ULONG)data->IoStatus.Information;
    }
    FltFreeCallbackData(data);
    return Status;
#endif
}
NTSTATUS CopyModifyFile(PFLT_FILTER hFltHandle, \
                        PFLT_INSTANCE pSrcInstance, \
                        PFILE_OBJECT pSrcFileObj, \
                        PUNICODE_STRING pUniSrcFileName, \
                        PFLT_INSTANCE pDstInstance, \
                        PUNICODE_STRING pUniDstFileName, \
                        BOOLEAN bDirectory)
{
    NTSTATUS Status;
    HANDLE hSrc,hDst;
    OBJECT_ATTRIBUTES ObjAttrSrc;
    //OBJECT_ATTRIBUTES ObjAttrDst;
    IO_STATUS_BLOCK IoStatus;
    PFILE_OBJECT pSrcFileObject,pDstFileObject;
    PVOID pStreamBuffer;
    ULONG uStreamInfoSize;
    FILE_FS_ATTRIBUTE_INFORMATION* pFsAttribInfomation;
    ULONG ulLength;
    PFILE_STREAM_INFORMATION pStreamInfo;
    UNICODE_STRING UniTmpName;
    UNICODE_STRING UniDataStreamName;
    UNICODE_STRING UniSrcFileName;
    UNICODE_STRING UniDstFileName;

    Status = STATUS_SUCCESS;
    pSrcFileObject = NULL;
    pDstFileObject = NULL;
    hSrc = NULL;
    hDst = NULL;
    pStreamBuffer = NULL;
    uStreamInfoSize = 0;
    pFsAttribInfomation = NULL;
    pStreamInfo = NULL;
    UniTmpName.Length = 0;
    UniTmpName.MaximumLength = 0;
    UniTmpName.Buffer = NULL;
    UniSrcFileName.Length = 0;
    UniSrcFileName.MaximumLength = 0;
    UniSrcFileName.Buffer = NULL;
    UniDstFileName.Length = 0;
    UniDstFileName.MaximumLength = 0;
    UniDstFileName.Buffer = NULL;
    ulLength = sizeof(FILE_FS_ATTRIBUTE_INFORMATION) + 20;

    do 
    {
        InitializeObjectAttributes(&ObjAttrSrc, \
            pUniSrcFileName, \
            OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, \
            NULL, \
            0);
        Status = FltCreateFile(hFltHandle, \
            pSrcInstance, \
            &hSrc, \
            FILE_READ_DATA | FILE_READ_ATTRIBUTES | SYNCHRONIZE, \
            &ObjAttrSrc, \
            &IoStatus, \
            NULL, \
            FILE_ATTRIBUTE_NORMAL, \
            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, \
            FILE_OPEN, \
            FILE_SYNCHRONOUS_IO_NONALERT, \
            NULL, \
            0, \
            IO_IGNORE_SHARE_ACCESS_CHECK);
        if (NT_ERROR(Status))
        {
            break;
        }
        Status = ObReferenceObjectByHandle(hSrc, \
            FILE_ANY_ACCESS, \
            NULL, \
            KernelMode, \
            &pSrcFileObject, \
            NULL);
        if (NT_ERROR(Status))
        {
            break;
        }
        uStreamInfoSize = PAGE_SIZE;
        pStreamBuffer = MyAllocateMemory(PagedPool,uStreamInfoSize);
        if(pStreamBuffer == NULL)
        {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            break;
        }
        do 
        {
            Status = FltQueryInformationFileSyncronous(pSrcInstance,
                pSrcFileObj,
                pStreamBuffer,
                uStreamInfoSize,
                FileStreamInformation,
                NULL);
            if(!NT_SUCCESS(Status))
            {
                uStreamInfoSize += PAGE_SIZE;
                ExFreePool(pStreamBuffer);	
                pStreamBuffer = NULL;
            }
        } while (Status == STATUS_BUFFER_OVERFLOW || Status == STATUS_BUFFER_TOO_SMALL);
        if (Status == STATUS_INVALID_PARAMETER)
        {
            do 
            {
                pFsAttribInfomation = ExAllocatePoolWithTag(NonPagedPool,ulLength,'FMPC');
            } while (NULL == pFsAttribInfomation);
            RtlZeroMemory(pFsAttribInfomation,ulLength);
            Status = FltQueryVolumeInformation(pSrcInstance, \
                &IoStatus, \
                pFsAttribInfomation, \
                ulLength, \
                FileFsAttributeInformation);
            if (NT_ERROR(Status))
            {
                break;
            }
            if(0 != _wcsnicmp(L"NTFS", \
                pFsAttribInfomation->FileSystemName, \
                pFsAttribInfomation->FileSystemNameLength / sizeof(WCHAR)))
            {
                Status = DoCopyFile(hFltHandle,
                    pSrcFileObject,
                    pSrcInstance,
                    pUniSrcFileName,
                    pDstInstance,
                    pUniDstFileName,
                    bDirectory,
                    FALSE);
            }
        }
        if (NT_ERROR(Status))
        {
            break;
        }
        pStreamInfo = (PFILE_STREAM_INFORMATION)pStreamBuffer;
        RtlInitUnicodeString(&UniDataStreamName,L"::$DATA");
        while (TRUE)
        {
            UniTmpName.MaximumLength = UniTmpName.Length = (USHORT)pStreamInfo->StreamNameLength;
            UniTmpName.Buffer = pStreamInfo->StreamName;
            if (RtlEqualUnicodeString(&UniTmpName,&UniDataStreamName,TRUE))
            {
                Status = DoCopyFile(hFltHandle,
                    pSrcFileObject,
                    pSrcInstance,
                    pUniSrcFileName,
                    pDstInstance,
                    pUniDstFileName,
                    bDirectory,
                    FALSE);
                if (NT_ERROR(Status))
                {
                    break;
                }
                if (pStreamInfo->NextEntryOffset == 0)
                {
                    break;
                }
                pStreamInfo = (PFILE_STREAM_INFORMATION)((ULONG_PTR)pStreamInfo + pStreamInfo->NextEntryOffset);
                continue;
            }
            UniSrcFileName.MaximumLength = UniSrcFileName.Length = pUniSrcFileName->Length + (USHORT)pStreamInfo->StreamNameLength;
            UniSrcFileName.Buffer = MyAllocateMemory(PagedPool,UniSrcFileName.Length);
            
            UniDstFileName.MaximumLength = UniDstFileName.Length = pUniDstFileName->Length + (USHORT)pStreamInfo->StreamNameLength;
            UniDstFileName.Buffer = MyAllocateMemory(PagedPool,UniDstFileName.Length);

            if(UniSrcFileName.Buffer == NULL || UniDstFileName.Buffer == NULL)
            {
                if(UniSrcFileName.Buffer != NULL)
                {
                    ExFreePool(UniSrcFileName.Buffer);
                    UniSrcFileName.Buffer = NULL;	
                }
                if(UniDstFileName.Buffer != NULL)
                {
                    ExFreePool(UniDstFileName.Buffer);
                    UniDstFileName.Buffer = NULL;
                }
                Status = STATUS_INSUFFICIENT_RESOURCES;
                break;
            }
            RtlCopyMemory(UniSrcFileName.Buffer,pUniSrcFileName->Buffer,pUniSrcFileName->Length);
            RtlCopyMemory(UniSrcFileName.Buffer + pUniSrcFileName->Length / sizeof(WCHAR),
                pStreamInfo->StreamName,
                pStreamInfo->StreamNameLength);

            RtlCopyMemory(UniDstFileName.Buffer, pUniDstFileName->Buffer, pUniDstFileName->Length);
            RtlCopyMemory(UniDstFileName.Buffer + pUniDstFileName->Length / sizeof(WCHAR),
                pStreamInfo->StreamName,
                pStreamInfo->StreamNameLength);

            ExFreePool(UniSrcFileName.Buffer);
            UniSrcFileName.Buffer = NULL;

            ExFreePool(UniDstFileName.Buffer);
            UniDstFileName.Buffer = NULL;
            if(NT_ERROR(Status))
            {
                break;
            }
            if(pStreamInfo->NextEntryOffset == 0)
            {
                break;
            }
            pStreamInfo = (PFILE_STREAM_INFORMATION)((ULONG_PTR)pStreamInfo + pStreamInfo->NextEntryOffset);
        }
    } while (0);
    if (pStreamBuffer)
    {
        ExFreePoolWithTag(pStreamBuffer,'MLAM');
    }
    if (pFsAttribInfomation)
    {
        ExFreePoolWithTag(pFsAttribInfomation,'FMPC');
    }
    if (hSrc > 0)
    {
        FltClose(hSrc);
    }
    if (pSrcFileObject)
    {
        ObDereferenceObject(pSrcFileObject);
    }
    return Status;
}
NTSTATUS DoCopyFile(PFLT_FILTER pFilter, \
                    PFILE_OBJECT pSrcFileObj, \
                    PFLT_INSTANCE pSrcInstance, \
                    PUNICODE_STRING pUniSrcFileName, \
                    PFLT_INSTANCE pDstInstance, \
                    PUNICODE_STRING pUniDstFileName, \
                    BOOLEAN bDirectory, \
                    BOOLEAN bReplace)
{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjSrcAttrib;
    OBJECT_ATTRIBUTES ObjDstAttrib;
    HANDLE hSrcFile;
    HANDLE hDstFile;
    PFILE_OBJECT pSrcFileObject;
    PFILE_OBJECT pDstFileObject;
    IO_STATUS_BLOCK IoStatus;
    LARGE_INTEGER liOffset;
    ULONG ulReadSize;
    ULONG ulWriteSize;
    PVOID pBuffer;
    ULONG ulCreateOptions;
    ULONG ulCreateDisposition;

    Status = STATUS_SUCCESS;
    hSrcFile = NULL;
    hDstFile = NULL;
    pSrcFileObject = NULL;
    pDstFileObject = NULL;
    pBuffer = NULL;
    ulCreateOptions = FILE_SYNCHRONOUS_IO_NONALERT;
    ulCreateDisposition = FILE_OPEN_IF;

    do 
    {
        if(pFilter == NULL || \
            pSrcInstance == NULL || \
            pUniSrcFileName == NULL || \
            pUniDstFileName == NULL)
        {
            Status = STATUS_INVALID_PARAMETER;
            break;
        }
        if(bDirectory)
        {
            ulCreateOptions |= FILE_DIRECTORY_FILE;
        }
        if(!bDirectory)
        {
            if(!pSrcFileObj)
            {
                InitializeObjectAttributes(&ObjSrcAttrib,
                    pUniSrcFileName,
                    OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                    NULL,
                    NULL);

                Status = FltCreateFile(pFilter,
                    pSrcInstance,    
                    &hSrcFile,
                    FILE_READ_DATA | FILE_READ_ATTRIBUTES | SYNCHRONIZE,
                    &ObjSrcAttrib,
                    &IoStatus,
                    0,
                    FILE_ATTRIBUTE_NORMAL,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    FILE_OPEN,
                    ulCreateOptions,
                    NULL,
                    0,
                    IO_IGNORE_SHARE_ACCESS_CHECK);
                if(!NT_SUCCESS(Status))
                {
                    break;
                }
                Status = ObReferenceObjectByHandle(hSrcFile,
                    FILE_ANY_ACCESS,
                    NULL,
                    KernelMode,
                    &pSrcFileObject,
                    NULL);
                if(!NT_SUCCESS(Status))
                {
                    break;
                }
            }
            else
            {
                pSrcFileObject = pSrcFileObj;
                //Status = ObOpenObjectByPointer(pSrcFileObj, \
                //    (ULONG)NULL, \
                //    NULL, \
                //    GENERIC_READ, \
                //    *IoFileObjectType, \
                //    ExGetPreviousMode(), \
                //    &hSrcFile);
            }
        }
        InitializeObjectAttributes(&ObjDstAttrib, \
            pUniDstFileName, \
            OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, \
            NULL, \
            0);
        if (bReplace)
        {
            ulCreateDisposition = FILE_OVERWRITE_IF;
        }
        Status = FltCreateFile(pFilter, \
            pDstInstance, \
            &hDstFile, \
            GENERIC_WRITE | SYNCHRONIZE, \
            &ObjDstAttrib, \
            &IoStatus, \
            0, \
            FILE_ATTRIBUTE_NORMAL, \
            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, \
            ulCreateDisposition, \
            ulCreateOptions, \
            (PVOID)NULL, \
            0, \
            0);
        if (NT_ERROR(Status))
        {
            break;
        }
        Status = ObReferenceObjectByHandle(hDstFile, \
            FILE_ANY_ACCESS, \
            NULL, \
            KernelMode, \
            &pDstFileObject, \
            NULL);
        if (NT_ERROR(Status))
        {
            break;
        }
        //pBuffer = MyAllocateMemory(PagedPool,PAGE_SIZE);
        //if (NULL == pBuffer)
        //{
        //    Status = STATUS_INSUFFICIENT_RESOURCES;
        //    break;
        //}
        pBuffer = FltAllocatePoolAlignedWithTag(pSrcInstance,NonPagedPool,PAGE_SIZE,'PasP');
        if (NULL == pBuffer)
        {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            break;
        }
        RtlZeroMemory(pBuffer,PAGE_SIZE);
        liOffset.QuadPart = pSrcFileObject->CurrentByteOffset.QuadPart;
    } while (0);
    if (hSrcFile && hDstFile)
    {
        while(NT_SUCCESS(Status))
        {
            ulReadSize = 0;
            ulWriteSize = 0;

            Status = FltReadFile(pSrcInstance, \
                pSrcFileObject, \
                0, \
                PAGE_SIZE, \
                pBuffer, \
                FLTFL_IO_OPERATION_NON_CACHED | FLTFL_IO_OPERATION_DO_NOT_UPDATE_BYTE_OFFSET, \
                &ulReadSize, \
                NULL, \
                NULL);
            if (NT_ERROR(Status) || (ulReadSize == 0))
            {
                break;
            }
            pSrcFileObject->CurrentByteOffset.QuadPart += ulReadSize;
            Status = FltWriteFile(pDstInstance, \
                pDstFileObject, \
                0, \
                ulReadSize, \
                pBuffer, \
                0,\
                &ulWriteSize, \
                NULL, \
                NULL);
            if (NT_ERROR(Status) || ulWriteSize < PAGE_SIZE)
            {
                break;
            }
        }
        pSrcFileObject->CurrentByteOffset.QuadPart = liOffset.QuadPart;
        if(Status == STATUS_END_OF_FILE)
        {
            Status = STATUS_SUCCESS;
        }
    }
    if(pBuffer)
    {
        FltFreePoolAlignedWithTag(pSrcInstance,pBuffer,'PasP');
        pBuffer = NULL;
    }
    if(pDstFileObject)
    {
        ObDereferenceObject(pDstFileObject);
        pDstFileObject = NULL;
    }
    if(hDstFile)
    {
        FltClose(hDstFile);
    }
    if(pSrcFileObject)
    {
        ObDereferenceObject(pSrcFileObject);
        pSrcFileObject = NULL;
    }
    if(hSrcFile)
    {
        FltClose(hSrcFile);
    }
    return Status;
}
NTSTATUS ConvertSandboxShortName(PUNICODE_STRING pSandboxPath, \
                                PUNICODE_STRING pSrcFullName, \
                                PUNICODE_STRING pDstFullName, \
                                PUNICODE_STRING pVolumeName)
{
    NTSTATUS Status;
    ULONG ulNeedSize;
    UNICODE_STRING UniTmpName;
	UNICODE_STRING UniFullName;
    ULONG uli;
    WCHAR wConvertPath[MAX_OBJNAME_LENGTH/2];

    uli = 0;
    ulNeedSize = 0;
    Status = STATUS_INVALID_PARAMETER;
	UniFullName.Buffer = NULL;

    do 
    {
		UniFullName.MaximumLength = pSrcFullName->MaximumLength;
		UniFullName.Length = pSrcFullName->Length;
		UniFullName.Buffer = ExAllocatePool(NonPagedPool,UniFullName.MaximumLength);
		if (NULL == UniFullName.Buffer)
		{
			break;
		}
		RtlZeroMemory(UniFullName.Buffer,UniFullName.MaximumLength);
		RtlCopyMemory(UniFullName.Buffer,pSrcFullName->Buffer,pSrcFullName->Length);
		UniFullName.Buffer[UniFullName.Length/2] = L'\0';
        for (uli = UniFullName.Length/2;uli > 0;uli--)
        {
            if (UniFullName.Buffer[uli] == L'\\')
            {
				RtlZeroMemory(wConvertPath,MAX_OBJNAME_LENGTH);
				StringCchCopyW(wConvertPath, \
					MAX_OBJNAME_LENGTH/2, \
					(WCHAR*)&UniFullName.Buffer[uli]);

                ulNeedSize = pSandboxPath->Length + (ULONG)wcslen(&UniFullName.Buffer[uli]) * sizeof(WCHAR) + 0x50;
                pDstFullName->Buffer = ExAllocatePool(NonPagedPool,ulNeedSize);
                if (NULL == pDstFullName)
                {
                    break;
                }
                RtlZeroMemory(pDstFullName->Buffer,ulNeedSize);
                pDstFullName->Length = 0;
                pDstFullName->MaximumLength = (USHORT)ulNeedSize;
				RtlCopyMemory(pDstFullName->Buffer, \
					pSandboxPath->Buffer, \
					pSandboxPath->Length);
				pDstFullName->Length = pSandboxPath->Length;
                RtlInitUnicodeString(&UniTmpName,wConvertPath);
                RtlUnicodeStringCat(pDstFullName,&UniTmpName);
                Status = STATUS_SUCCESS;
                break;
            }
        }
    } while (0);
	if (UniFullName.Buffer)
	{
		ExFreePool(UniFullName.Buffer);
	}
    return Status;
}
NTSTATUS ConvertSandboxName(PUNICODE_STRING pSandboxPath, \
                            PUNICODE_STRING pSrcFullName, \
                            PUNICODE_STRING pDstFullName, \
                            PUNICODE_STRING pVolumeName)
{
    NTSTATUS Status;
    ULONG ulNeedSize;
    WCHAR *wTmpName;
    UNICODE_STRING UniTmpName;

    ulNeedSize = 0;
    Status = STATUS_INVALID_PARAMETER;
    wTmpName = NULL;

    do 
    {
        if (NULL == pSandboxPath || \
            NULL == pSrcFullName || \
            NULL == pDstFullName)
        {
            break;
        }
        ulNeedSize = pSandboxPath->Length + pSrcFullName->Length - pVolumeName->Length;
        pDstFullName->Buffer = ExAllocatePool(NonPagedPool,ulNeedSize * sizeof(WCHAR));
        if (NULL == pDstFullName)
        {
            break;
        }
        RtlZeroMemory(pDstFullName->Buffer,ulNeedSize * sizeof(WCHAR));
        pDstFullName->Length = 0;
        pDstFullName->MaximumLength = (USHORT)ulNeedSize;
        wTmpName = ExAllocatePool(NonPagedPool,pSrcFullName->Length - pVolumeName->Length + sizeof(WCHAR));
        if (NULL == wTmpName)
        {
            break;
        }
        RtlZeroMemory(wTmpName,pSrcFullName->Length - pVolumeName->Length + sizeof(WCHAR));
        RtlCopyMemory(wTmpName, \
            (WCHAR*)((ULONG)pSrcFullName->Buffer + (ULONG)pVolumeName->Length), \
            pSrcFullName->Length - pVolumeName->Length);
        RtlCopyUnicodeString(pDstFullName,pSandboxPath);
        RtlInitUnicodeString(&UniTmpName,wTmpName);
        RtlUnicodeStringCat(pDstFullName,&UniTmpName);
        Status = STATUS_SUCCESS;
    } while (0);
    if (wTmpName)
    {
        ExFreePool(wTmpName);
    }
    return Status;
}
BOOLEAN IsSandboxFileExist(PFLT_FILTER pFilter, \
                           PFLT_INSTANCE pInstance, \
                           PUNICODE_STRING pFileName)
{
    BOOLEAN bRet;
    HANDLE hFile;
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatus;
    OBJECT_ATTRIBUTES ObjAttributes;

    do 
    {
        hFile = NULL;
        bRet = FALSE;
        Status = STATUS_SUCCESS;

        InitializeObjectAttributes(&ObjAttributes, \
            pFileName, \
            OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, \
            NULL, \
            0);
        Status = FltCreateFile(pFilter, \
            pInstance, \
            &hFile, \
            FILE_READ_DATA | FILE_READ_ATTRIBUTES | SYNCHRONIZE, \
            &ObjAttributes, \
            &IoStatus, \
            NULL, \
            FILE_ATTRIBUTE_NORMAL, \
            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, \
            FILE_OPEN, \
            FILE_SYNCHRONOUS_IO_NONALERT, \
            NULL, \
            0, \
            0);
        if (NT_SUCCESS(Status))
        {
            bRet = TRUE;
        }
    } while (0);
    if (hFile)
    {
        FltClose(hFile);
    }
    return bRet;
}
NTSTATUS SandboxIsDirectory(PFILE_OBJECT pFileObject, \
                            PUNICODE_STRING pUniDirName, \
                            PFLT_FILTER pFilter, \
                            PFLT_INSTANCE pInstance, \
                            BOOLEAN* bIsDirectory)
{
    PFILE_OBJECT pLocalFileObject;
    HANDLE hFile;
    FILE_STANDARD_INFORMATION StdInfo;
    NTSTATUS Status;
    OBJECT_ATTRIBUTES objAttrib;
    IO_STATUS_BLOCK IoStatus;

    *bIsDirectory = FALSE;
    pLocalFileObject = NULL;
    hFile = NULL;
    Status = STATUS_UNSUCCESSFUL;

    __try
    {
        if(pFileObject == NULL)
        {

            InitializeObjectAttributes(&objAttrib,
                pUniDirName,
                OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                NULL,
                NULL);
            Status = FltCreateFile(pFilter,
                pInstance,    
                &hFile,
                GENERIC_READ | SYNCHRONIZE,
                &objAttrib,
                &IoStatus,
                0,
                FILE_ATTRIBUTE_NORMAL,
                FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                FILE_OPEN,
                FILE_SYNCHRONOUS_IO_NONALERT,
                NULL,
                0,
                0);
            if(!NT_SUCCESS(Status))
            {
                __leave;
            }
            Status = ObReferenceObjectByHandle(hFile,
                FILE_ANY_ACCESS,
                NULL,
                KernelMode,
                &pFileObject,
                NULL);
            if(!NT_SUCCESS(Status))
            {
                __leave;
            }
        }
        else
        {
            pFileObject = pLocalFileObject;
        }
        Status = FltQueryInformationFileSyncronous(pInstance,
            pFileObject,
            &StdInfo,
            sizeof(FILE_STANDARD_INFORMATION),
            FileStandardInformation,
            NULL);

        if(NT_SUCCESS(Status))
        {
            *bIsDirectory = StdInfo.Directory;
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
    }

    if(pLocalFileObject && !pFileObject)
    {
        ObDereferenceObject(pLocalFileObject);
        pLocalFileObject = NULL;
    }
    if(hFile)
    {
        FltClose(hFile);
        hFile = NULL;
    }
    return Status;
}
BOOLEAN SandboxCreateOneFile(PFLT_FILTER pFilter, \
                            PFLT_INSTANCE pInstance, \
                            PFILE_OBJECT* pFileObject, \
                            PUNICODE_STRING pUniFileName, \
                            BOOLEAN bRetFileObj, \
                            ACCESS_MASK AccessMask, \
                            ULONG ulCreateDisposition, \
                            BOOLEAN bDirectory)
{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjAttributes;
    IO_STATUS_BLOCK IoStatus;
    HANDLE hFile;
    ULONG CreateOptions;
    ACCESS_MASK AccessMack;
    ULONG CreateDisposition;

    Status = STATUS_UNSUCCESSFUL;
    hFile = NULL;
    AccessMack = 0;
    CreateDisposition = 0;
    CreateOptions = FILE_SYNCHRONOUS_IO_NONALERT;

    if((!pFileObject && bRetFileObj) || !pUniFileName)
    {
        return FALSE;
    }
    if(bDirectory)
    {
        CreateOptions |= FILE_DIRECTORY_FILE;
    }
    if(!AccessMask)
    {
        AccessMack = FILE_READ_ATTRIBUTES | SYNCHRONIZE;
    }
    else
    {
        AccessMack = AccessMack;
    }

    if(ulCreateDisposition)
    {
        CreateDisposition = ulCreateDisposition;
    }
    else
    {
        CreateDisposition = FILE_OPEN_IF;
    }
    InitializeObjectAttributes(&ObjAttributes,
        pUniFileName,
        OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
        NULL,
        NULL);

    Status = FltCreateFile(pFilter,
        pInstance,    
        &hFile,
        AccessMack,
        &ObjAttributes,
        &IoStatus,
        0,
        FILE_ATTRIBUTE_NORMAL,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        CreateDisposition,
        CreateOptions,
        NULL,
        0,
        0  );
    if(!NT_SUCCESS(Status) || hFile == NULL)
    {
        return FALSE;
    }
    if(bRetFileObj)
    {
        Status = ObReferenceObjectByHandle(hFile,
            FILE_ANY_ACCESS,
            NULL,
            KernelMode,
            pFileObject,
            NULL);
    }
    if (hFile)
    {
        FltClose(hFile);
        hFile = NULL;
    }
    return NT_SUCCESS(Status);
}
NTSTATUS SandboxRedirectFile(PFLT_CALLBACK_DATA Data, \
                             PCFLT_RELATED_OBJECTS FltObjects, \
                             PUNICODE_STRING pUstrDstFileName)
{
    PFILE_OBJECT pFileObject;

    pFileObject = NULL;
    __try
    {
        pFileObject = Data->Iopb->TargetFileObject;
        if(pFileObject == NULL)
        {
            return STATUS_INVALID_PARAMETER;
        }
        if(pFileObject->FileName.Length > 0 && pFileObject->FileName.Buffer != NULL)
        {
            ExFreePool(pFileObject->FileName.Buffer);
            pFileObject->FileName.Buffer = NULL;
        }	
        pFileObject->FileName = *pUstrDstFileName;
        pFileObject->RelatedFileObject = NULL;
        Data->IoStatus.Status = STATUS_REPARSE; 
        Data->IoStatus.Information = IO_REPARSE;
        FltSetCallbackDataDirty(Data);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
    }
    return STATUS_SUCCESS;
}
PFLT_INSTANCE SandboxGetVolumeInstance(PFLT_FILTER pFilter, \
                                  PUNICODE_STRING pVolumeName)
{
    NTSTATUS Status;
    PFLT_INSTANCE pInstance;
    PFLT_VOLUME pVolumeList[MAX_VOLUME_CHARS];
    BOOLEAN bDone;
    ULONG uRet;
    UNICODE_STRING UniName;
    ULONG ulIndex;
    UNICODE_STRING UniConstInstance;
    WCHAR wNameBuffer[MAX_PATH];


    pInstance = NULL;
    ulIndex = 0;
    bDone = FALSE;
    RtlZeroMemory(wNameBuffer,sizeof(WCHAR) * MAX_PATH);
    RtlInitUnicodeString(&UniConstInstance,L"GlobalAttach");
    Status = FltEnumerateVolumes(pFilter,
        NULL,
        0,
        &uRet);
    if(Status != STATUS_BUFFER_TOO_SMALL)
    {
        return NULL;
    }
    Status = FltEnumerateVolumes(pFilter,
        pVolumeList,
        uRet,
        &uRet);
    if(!NT_SUCCESS(Status))
    {

        return NULL;
    }
    UniName.Buffer = wNameBuffer;
    if (UniName.Buffer == NULL)
    {
        for(ulIndex = 0;ulIndex< uRet;ulIndex++)
        {
            FltObjectDereference(pVolumeList[ulIndex]);
        }
        return NULL;
    }
    UniName.MaximumLength = MAX_PATH*sizeof(WCHAR);
    for (ulIndex = 0;ulIndex < uRet;ulIndex++)
    {
        UniName.Length = 0;
        Status = FltGetVolumeName( pVolumeList[ulIndex],
            &UniName,
            NULL);
        if(!NT_SUCCESS(Status))
        {
            continue;
        }
        if(RtlCompareUnicodeString(&UniName,
            pVolumeName,
            TRUE) != 0)
        {
            continue;
        }
        Status = FltGetVolumeInstanceFromName(pFilter,
            pVolumeList[ulIndex],
            NULL,
            &pInstance);
        if(NT_SUCCESS(Status))
        {
            FltObjectDereference(pInstance);
            break;
        }
    }
    for (ulIndex = 0;ulIndex< uRet;ulIndex++)
    {
        FltObjectDereference(pVolumeList[ulIndex]);
    }
    return pInstance;
}
PVOID GetFileFullPath(PFILE_OBJECT pFileObject, \
                         PFLT_INSTANCE pFltInstance)
{
    NTSTATUS Status;
    ULONG ulNeedSize;
    PVOID pFileNameInfo;

    ulNeedSize = 0;
    Status = STATUS_SUCCESS;
    pFileNameInfo = NULL;

    do 
    {
        if (NULL == pFltInstance || \
            NULL == pFileObject)
        {
            Status = STATUS_INVALID_PARAMETER;
            break;
        }
        pFileNameInfo = ExAllocatePool(NonPagedPool, \
            sizeof(FILE_NAME_INFORMATION) + sizeof(WCHAR) * MAX_PATH);
        if (NULL == pFileNameInfo)
        {
            Status = STATUS_UNSUCCESSFUL;
            break;
        }
        RtlZeroMemory(pFileNameInfo,sizeof(FILE_NAME_INFORMATION) + sizeof(WCHAR) * MAX_PATH);
        Status = FltQueryInformationFile(pFltInstance, \
            pFileObject, \
            pFileNameInfo, \
            sizeof(FILE_NAME_INFORMATION) + sizeof(WCHAR) * MAX_PATH, \
            FileNameInformation, \
            &ulNeedSize);
        if(Status == STATUS_BUFFER_TOO_SMALL || \
            Status == STATUS_OBJECT_TYPE_MISMATCH)
        {
            if (pFileNameInfo)
            {
                ExFreePool(pFileNameInfo);
                pFileNameInfo = NULL;
            }
            pFileNameInfo = ExAllocatePool(NonPagedPool,ulNeedSize);
            if (NULL == pFileNameInfo)
            {
                Status = STATUS_UNSUCCESSFUL;
                break;
            }
            RtlZeroMemory(pFileNameInfo,ulNeedSize);
            Status = FltQueryInformationFile(pFltInstance, \
                pFileObject, \
                pFileNameInfo, \
                ulNeedSize, \
                FileNameInformation, \
                &ulNeedSize);
            if (NT_ERROR(Status))
            {
                break;
            }
        }

    } while (0);
    if (NT_SUCCESS(Status))
    {
        return pFileNameInfo;
    }
    return NULL;
}
NTSTATUS SandboxGetFileNameInformation(PFLT_VOLUME pVolume,
                                  PFLT_INSTANCE	pInstance,
                                  PFILE_OBJECT pFileObject,
                                  BOOLEAN bGetFromCache,
                                  PFLT_FILE_NAME_INFORMATION *pNameInfo)
{

    NTSTATUS			ntStatus = STATUS_UNSUCCESSFUL;
    NTSTATUS			tmpStatus = STATUS_SUCCESS;
    PWCHAR				wszName = NULL;
    PWCHAR				tmpName = NULL;
    UNICODE_STRING		ustrVolumeName={0,0,NULL};
    UNICODE_STRING		ustrFileName={0,0,NULL};
    PWSTR				pFinalName = NULL;  
    PFILE_OBJECT 		pTmpFileObject = NULL;
    ULONG				uRet = 0;
    BOOLEAN 			bParentObjects = FALSE;
    PFLT_FILE_NAME_INFORMATION		pName = NULL;
	PFILE_STREAMHANDLE_CONTEXT pStreamHandleContext = NULL;

    __try
    {
        if( pVolume == NULL ||
            pFileObject == NULL ||
            pNameInfo == NULL )
            __leave;

        wszName = MyNew(WCHAR, MAX_PATH);
        if(!wszName)
            __leave;

        tmpName = MyNew(WCHAR, MAX_PATH);
        if(!tmpName)
            __leave;

        pName = (PFLT_FILE_NAME_INFORMATION)MyNew(BYTE, MAX_PATH * sizeof(WCHAR)+ sizeof(FLT_FILE_NAME_INFORMATION));
        if(pName == NULL)
            __leave;
        wszName[0] = 0;
        tmpName[0] = 0;
        ustrVolumeName.Buffer = MyNew(WCHAR, MAX_PATH);
        if(ustrVolumeName.Buffer == NULL)
        {
            MyDelete(pName);
            __leave;
        }
        ustrVolumeName.Length = 0;
        ustrVolumeName.MaximumLength = MAX_PATH * sizeof(WCHAR);

        tmpStatus = FltGetVolumeName(pVolume,
            &ustrVolumeName,
            &uRet);
        if(! NT_SUCCESS(tmpStatus))
        {
            MyDelete(pName);
            __leave;
        }

        if (bGetFromCache && pFileObject->RelatedFileObject)
        {
            ntStatus = FltGetStreamHandleContext(pInstance,
                pFileObject,
                &pStreamHandleContext);
            if(!NT_SUCCESS( ntStatus))
            {
                MyDelete(pName);
                pName = NULL;
                __leave;
            }

            if (pStreamHandleContext->m_FileName[0]!=0)
            {
                RtlCopyMemory(tmpName, pStreamHandleContext->m_FileName, (MAX_PATH-1)*sizeof(WCHAR));
                tmpName[MAX_PATH -1] = 0;
                FltReleaseContext(pStreamHandleContext);
            }
            else
            {
                MyDelete(pName);
                pName = NULL;
                FltReleaseContext(pStreamHandleContext);
                ntStatus = STATUS_NOT_FOUND;
                __leave;
            }
        }
        else
        {
            pTmpFileObject = pFileObject;
            while(pTmpFileObject != NULL && pTmpFileObject->FileName.Length > 0 && pTmpFileObject->Type == IO_TYPE_FILE)
            {		
                RtlZeroMemory(tmpName, MAX_PATH * sizeof(WCHAR) );

                tmpStatus = RtlStringCbCopyNW(tmpName,
                    MAX_PATH*sizeof(WCHAR),
                    pTmpFileObject->FileName.Buffer,
                    pTmpFileObject->FileName.Length);
                if(! NT_SUCCESS(tmpStatus))
                    break;
                if (bParentObjects )
                {
                    if (pTmpFileObject->FileName.Length >= sizeof(WCHAR)&&
                        pTmpFileObject->FileName.Buffer[pTmpFileObject->FileName.Length / sizeof(WCHAR) -1] != L'\\'&&
                        wszName[0] != '\\')
                    {
                        RtlStringCchCatW(tmpName, MAX_PATH, L"\\");
                    }

                }
                else
                    bParentObjects = TRUE;
                tmpStatus = RtlStringCchCatW(tmpName,
                    MAX_PATH,
                    wszName);
                if(! NT_SUCCESS(tmpStatus))
                    break;

                tmpName[MAX_PATH-1] = 0;
                RtlZeroMemory(wszName, MAX_PATH * sizeof(WCHAR) );

                tmpStatus = RtlStringCchCopyW(wszName,
                    MAX_PATH,
                    tmpName);
                if(! NT_SUCCESS(tmpStatus))
                    break;

                wszName[MAX_PATH-1] = 0;
                pTmpFileObject = pTmpFileObject->RelatedFileObject;
            }

            if(! NT_SUCCESS(tmpStatus))
            {
                MyDelete(pName);
                __leave;
            }

            RtlZeroMemory(tmpName, MAX_PATH * sizeof(WCHAR) );

            tmpStatus = RtlStringCbCopyNW(tmpName,
                MAX_PATH*sizeof(WCHAR),
                ustrVolumeName.Buffer,
                ustrVolumeName.Length);
            if(! NT_SUCCESS(tmpStatus))
            {
                MyDelete(pName);
                __leave;
            }

            tmpStatus = RtlStringCchCatW(tmpName,
                MAX_PATH,
                wszName);
            if(! NT_SUCCESS(tmpStatus))
            {
                MyDelete(pName);	
                __leave;
            }
        }
        pFinalName = tmpName;

        RtlInitUnicodeString(&ustrFileName, pFinalName);


        if(ustrFileName.Length < MAX_PATH*sizeof(WCHAR) )
        {
            pName->Size = sizeof(FLT_FILE_NAME_INFORMATION);
            pName->NamesParsed = 0; 
            pName->Format = FLT_FILE_NAME_NORMALIZED;

            pName->Name.Buffer = (PWSTR)( (PBYTE)pName+sizeof(FLT_FILE_NAME_INFORMATION)+sizeof(USHORT)*2 );
            pName->Name.Length = ustrFileName.Length;
            pName->Name.MaximumLength = pName->Name.Length;
            RtlCopyMemory(pName->Name.Buffer, ustrFileName.Buffer, ustrFileName.Length);

            pName->Volume.Buffer = pName->Name.Buffer;
            pName->Volume.Length = ustrVolumeName.Length;
            pName->Volume.MaximumLength  = pName->Volume.Length;

            *pNameInfo = pName;

            ntStatus = STATUS_SUCCESS;
        }
        else
        {
            MyDelete(pName);
            pName = NULL;
            ntStatus = STATUS_UNSUCCESSFUL;
        }
    }


    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        *pNameInfo = NULL;
        ntStatus = STATUS_UNSUCCESSFUL;

        if(pName != NULL)
        {
            MyDelete(pName);
            pName = NULL;
        }

    }

    if(ustrVolumeName.Buffer != NULL)
        MyDelete(ustrVolumeName.Buffer);

    if(NULL != wszName)
        MyDelete(wszName);

    if(NULL != tmpName)
        MyDelete(tmpName);

    return ntStatus;
}
ULONGLONG FltGetFileLength(PFLT_FILTER pFltHandle, \
                            PFLT_INSTANCE pFltInstance, \
                            PUNICODE_STRING pUniFileName)
{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE hFile;
    IO_STATUS_BLOCK IoStatus;
    PFILE_OBJECT pFileObject;
    PFILE_STANDARD_INFORMATION pFileStandardInfo;
    ULONG ulLength;
    LARGE_INTEGER liFileSize;

    Status = STATUS_SUCCESS;
    hFile = NULL;
    ulLength = 0;
    pFileStandardInfo = NULL;
    pFileObject = NULL;
    liFileSize.QuadPart = 0;

    InitializeObjectAttributes(&ObjectAttributes, \
        pUniFileName, \
        OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, \
        NULL, \
        0);
    do 
    {
        Status = FltCreateFile(pFltHandle, \
            pFltInstance, \
            &hFile, \
            FILE_READ_DATA | FILE_READ_ATTRIBUTES | SYNCHRONIZE, \
            &ObjectAttributes, \
            &IoStatus, \
            NULL, \
            FILE_ATTRIBUTE_NORMAL, \
            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, \
            FILE_OPEN, \
            FILE_SYNCHRONOUS_IO_NONALERT, \
            NULL, \
            0, \
            IO_IGNORE_SHARE_ACCESS_CHECK);
        if (NT_ERROR(Status))
        {
            break;
        }
        Status = ObReferenceObjectByHandle(hFile, \
            FILE_ANY_ACCESS, \
            NULL, \
            KernelMode, \
            &pFileObject, \
            NULL);
        if (NT_ERROR(Status))
        {
            break;
        }
        ulLength = sizeof(FILE_STANDARD_INFORMATION);
        pFileStandardInfo = ExAllocatePool(NonPagedPool, \
            ulLength);
        if (NULL == pFileStandardInfo)
        {
            Status = STATUS_MEMORY_NOT_ALLOCATED;
            break;
        }
        RtlZeroMemory(pFileStandardInfo,ulLength);
        Status = FltQueryInformationFile(pFltInstance, \
            pFileObject, \
            pFileStandardInfo, \
            ulLength, \
            FileStandardInformation, \
            &ulLength);
        if (Status == STATUS_BUFFER_TOO_SMALL || \
            Status == STATUS_OBJECT_TYPE_MISMATCH)
        {
            if (pFileStandardInfo)
            {
                ExFreePool(pFileStandardInfo);
                pFileStandardInfo = NULL;
            }
            pFileStandardInfo = ExAllocatePool(NonPagedPool,ulLength);
            if (NT_ERROR(Status))
            {
                Status = STATUS_MEMORY_NOT_ALLOCATED;
                break;
            }
            RtlZeroMemory(pFileStandardInfo,ulLength);
            Status = FltQueryInformationFile(pFltInstance, \
                pFileObject, \
                pFileStandardInfo, \
                ulLength, \
                FileStandardInformation, \
                &ulLength);
            if (NT_ERROR(Status))
            {
                break;
            }
        }
        liFileSize.QuadPart = pFileStandardInfo->EndOfFile.QuadPart;
    } while (0);
    if (hFile)
    {
        FltClose(hFile);
    }
    if (pFileStandardInfo)
    {
        ExFreePool(pFileStandardInfo);
    }
    if (pFileObject)
    {
        ObDereferenceObject(pFileObject);
    }
    if (NT_SUCCESS(Status))
    {
         return liFileSize.QuadPart;
    }
    return 0;
}
ULONGLONG GetFileLength(PUNICODE_STRING pUniFileName)
{
	NTSTATUS Status;
	OBJECT_ATTRIBUTES ObjectAttributes;
	HANDLE hFile;
	IO_STATUS_BLOCK IoStatus;
	PFILE_STANDARD_INFORMATION pFileStandardInfo;
	ULONG ulLength;
	LARGE_INTEGER liFileSize;

	Status = STATUS_SUCCESS;
	hFile = NULL;
	ulLength = 0;
	pFileStandardInfo = NULL;
	liFileSize.QuadPart = 0;

	InitializeObjectAttributes(&ObjectAttributes, \
		pUniFileName, \
		OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, \
		NULL, \
		0);
	do 
	{
		Status = ZwCreateFile(&hFile, \
			FILE_READ_DATA | FILE_READ_ATTRIBUTES | SYNCHRONIZE, \
			&ObjectAttributes, \
			&IoStatus, \
			NULL, \
			FILE_ATTRIBUTE_NORMAL, \
			FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, \
			FILE_OPEN, \
			FILE_SYNCHRONOUS_IO_NONALERT, \
			NULL, \
			0);
		if (NT_ERROR(Status))
		{
			break;
		}
		ulLength = sizeof(FILE_STANDARD_INFORMATION);
		pFileStandardInfo = ExAllocatePool(NonPagedPool, \
			ulLength);
		if (NULL == pFileStandardInfo)
		{
			Status = STATUS_MEMORY_NOT_ALLOCATED;
			break;
		}
		RtlZeroMemory(pFileStandardInfo,ulLength);
		Status = ZwQueryInformationFile(hFile, \
			&IoStatus, \
			pFileStandardInfo, \
			ulLength, \
			FileStandardInformation);
		if (Status == STATUS_BUFFER_TOO_SMALL || \
			Status == STATUS_OBJECT_TYPE_MISMATCH)
		{
			if (pFileStandardInfo)
			{
				ExFreePool(pFileStandardInfo);
				pFileStandardInfo = NULL;
			}
			pFileStandardInfo = ExAllocatePool(NonPagedPool,ulLength);
			if (NT_ERROR(Status))
			{
				Status = STATUS_MEMORY_NOT_ALLOCATED;
				break;
			}
			RtlZeroMemory(pFileStandardInfo,ulLength);
			Status = ZwQueryInformationFile(hFile, \
				&IoStatus, \
				pFileStandardInfo, \
				ulLength, \
				FileStandardInformation);
			if (NT_ERROR(Status))
			{
				break;
			}
		}
		liFileSize.QuadPart = pFileStandardInfo->EndOfFile.QuadPart;
	} while (0);
	if (hFile)
	{
		ZwClose(hFile);
	}
	if (pFileStandardInfo)
	{
		ExFreePool(pFileStandardInfo);
	}
	if (NT_SUCCESS(Status))
	{
		return liFileSize.QuadPart;
	}
	return 0;
}