#include "ShitDrv.h"
#include <ntddk.h>
#include "LoadPlugin.h"

LIST_ENTRY g_LdrPluginList;

PLDR_PLUGIN_LIST AllocateLdrPlugin()
{
    PLDR_PLUGIN_LIST pLdrPluginList;

    pLdrPluginList = NULL;
    do 
    {
        pLdrPluginList = (PLDR_PLUGIN_LIST)ExAllocatePoolWithTag(NonPagedPool, \
            sizeof(LDR_PLUGIN_LIST), \
            LDR_PLUGIN_TAG);
    } while (NULL == pLdrPluginList);
    RtlZeroMemory(pLdrPluginList,sizeof(LDR_PLUGIN_LIST));
    return pLdrPluginList;
}
NTSTATUS GetDrvPluginInfo(PLDR_PLUGIN_LIST pLdrPluginList,WCHAR *wDrvPluginPath)
{
    OBJECT_ATTRIBUTES ObjectSubPlugin;
    UNICODE_STRING UniNewPluginRegPath;
    UNICODE_STRING UniValueKeyName;
    PKEY_VALUE_PARTIAL_INFORMATION pValuePartialInfo;
    HANDLE hSubPluginKey;
    NTSTATUS Status;
    WCHAR wPluginRegPath[MAX_PATH];
    ULONG ulNeedSize;

    hSubPluginKey = NULL;
    UniNewPluginRegPath.Buffer = NULL;
    UniNewPluginRegPath.Length = 0;
    UniNewPluginRegPath.MaximumLength = 0;
    ulNeedSize = 0;
    pValuePartialInfo = NULL;
    Status = STATUS_SUCCESS;

    RtlZeroMemory(wPluginRegPath,sizeof(WCHAR) * MAX_PATH);

    do 
    {
        StringCchCopyW(wPluginRegPath,MAX_PATH,wDrvPluginPath);
        RtlInitUnicodeString(&UniNewPluginRegPath,wPluginRegPath);
        InitializeObjectAttributes(&ObjectSubPlugin, \
        &UniNewPluginRegPath, \
        OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, \
        NULL, \
        0);
        Status = ZwOpenKey(&hSubPluginKey, \
            KEY_READ | KEY_QUERY_VALUE, \
            &ObjectSubPlugin);
        if (NT_ERROR(Status))
        {
            break;
        }
        RtlInitUnicodeString(&UniValueKeyName,L"Path");
        Status = ZwQueryValueKey(hSubPluginKey, \
            &UniValueKeyName, \
            KeyValueFullInformation, \
            NULL, \
            0, \
            &ulNeedSize);
        if (Status == STATUS_BUFFER_OVERFLOW || \
            Status == STATUS_BUFFER_TOO_SMALL)
        {
            pValuePartialInfo = (PKEY_VALUE_PARTIAL_INFORMATION)ExAllocatePoolWithTag(NonPagedPool, \
                ulNeedSize, \
                LDR_PLUGIN_TAG);
            if (NULL == pValuePartialInfo)
            {
                break;
            }
            RtlZeroMemory(pValuePartialInfo,ulNeedSize);
            Status = ZwQueryValueKey(hSubPluginKey, \
                &UniValueKeyName, \
                KeyValueFullInformation, \
                pValuePartialInfo, \
                ulNeedSize, \
                &ulNeedSize);
            if (NT_SUCCESS(Status))
            {
                RtlCopyMemory(pLdrPluginList->CheckFileSignature.wPluginFileFullPath, \
                    pValuePartialInfo->Data, \
                    pValuePartialInfo->DataLength);
            }
            if (pValuePartialInfo)
            {
                ExFreePoolWithTag(pValuePartialInfo,LDR_PLUGIN_TAG);
                pValuePartialInfo = NULL;
            }
        }
        RtlInitUnicodeString(&UniValueKeyName,L"LdrCtl");
        Status = ZwQueryValueKey(hSubPluginKey, \
            &UniValueKeyName, \
            KeyValueFullInformation, \
            NULL, \
            0, \
            &ulNeedSize);
        if (Status == STATUS_BUFFER_OVERFLOW || \
            Status == STATUS_BUFFER_TOO_SMALL)
        {
            pValuePartialInfo = (PKEY_VALUE_PARTIAL_INFORMATION)ExAllocatePoolWithTag(NonPagedPool, \
                ulNeedSize, \
                LDR_PLUGIN_TAG);
            if (NULL == pValuePartialInfo)
            {
                break;
            }
            RtlZeroMemory(pValuePartialInfo,ulNeedSize);
            Status = ZwQueryValueKey(hSubPluginKey, \
                &UniValueKeyName, \
                KeyValueFullInformation, \
                pValuePartialInfo, \
                ulNeedSize, \
                &ulNeedSize);
            if (NT_SUCCESS(Status))
            {
                RtlCopyMemory(&pLdrPluginList->CheckFileSignature.LdrCtl, \
                    pValuePartialInfo->Data, \
                    pValuePartialInfo->DataLength);
            }
            if (pValuePartialInfo)
            {
                ExFreePoolWithTag(pValuePartialInfo,LDR_PLUGIN_TAG);
                pValuePartialInfo = NULL;
            }
        }
    } while (0);
    if (pValuePartialInfo)
    {
        ExFreePoolWithTag(pValuePartialInfo,LDR_PLUGIN_TAG);
        pValuePartialInfo = NULL;
    }
    if (hSubPluginKey)
    {
        ZwClose(hSubPluginKey);
    }
    return Status;
}
NTSTATUS EnumPlugin(PUNICODE_STRING pUniDrvPlugin)
{
    NTSTATUS Status;
    HANDLE hKey;
    OBJECT_ATTRIBUTES ObjectAttributes;
    ULONG uli;
    PKEY_FULL_INFORMATION pKeyFullInfo;
    PKEY_BASIC_INFORMATION pKeyBasicInfo;
    PLDR_PLUGIN_LIST pLdrPluginList;
    ULONG ulNeedSize;
    WCHAR wPluginRegPath[MAX_PATH];
    WCHAR wPluginSubRegPath[MAX_PATH];

    hKey = NULL;
    Status = STATUS_SUCCESS;
    uli = 0;
    pKeyFullInfo = NULL;
    pKeyBasicInfo = NULL;
    ulNeedSize = 0;
    pLdrPluginList = NULL;

    RtlZeroMemory(wPluginRegPath,sizeof(WCHAR) * MAX_PATH);
    RtlZeroMemory(wPluginSubRegPath,sizeof(WCHAR) * MAX_PATH);

    do 
    {
        if (NULL == pUniDrvPlugin)
        {
            break;
        }
        InitializeObjectAttributes(&ObjectAttributes, \
            pUniDrvPlugin, \
            OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, \
            NULL, \
            0);
        Status = ZwOpenKey(&hKey,KEY_READ | KEY_QUERY_VALUE,&ObjectAttributes);
        if (NT_ERROR(Status))
        {
            break;
        }
        Status = ZwQueryKey(hKey,KeyFullInformation,NULL,0,&ulNeedSize);
        if (Status == STATUS_BUFFER_OVERFLOW || \
            Status == STATUS_BUFFER_TOO_SMALL)
        {
            pKeyFullInfo = ExAllocatePool(NonPagedPool,ulNeedSize);
            if (NULL == pKeyFullInfo)
            {
                break;
            }
            RtlZeroMemory(pKeyFullInfo,ulNeedSize);
            Status = ZwQueryKey(hKey,KeyFullInformation,pKeyFullInfo,ulNeedSize,&ulNeedSize);
            if (NT_SUCCESS(Status))
            {
                for (uli = 0;uli < pKeyFullInfo->SubKeys;uli++)
                {
                    Status = ZwEnumerateKey(hKey,uli,KeyBasicInformation,NULL,0,&ulNeedSize);
                    if (Status == STATUS_BUFFER_OVERFLOW || \
                        Status == STATUS_BUFFER_TOO_SMALL)
                    {
                        pKeyBasicInfo = ExAllocatePool(NonPagedPool,ulNeedSize);
                        if (NULL == pKeyBasicInfo)
                        {
                            Status = STATUS_BUFFER_TOO_SMALL;
                            break; 
                        }
                        RtlZeroMemory(pKeyBasicInfo,ulNeedSize);
                        Status = ZwEnumerateKey(hKey,uli,KeyBasicInformation,pKeyBasicInfo,ulNeedSize,&ulNeedSize);
                        if (NT_SUCCESS(Status))
                        {
                            pLdrPluginList = AllocateLdrPlugin();
                            
                            RtlCopyMemory(pLdrPluginList->CheckFileSignature.wPluginName, \
                                pKeyBasicInfo->Name, \
                                pKeyBasicInfo->NameLength);

                            RtlZeroMemory(wPluginRegPath,sizeof(WCHAR) * MAX_PATH);
                            RtlZeroMemory(wPluginSubRegPath,sizeof(WCHAR) * MAX_PATH);

                            StringCchCopyW(wPluginRegPath,MAX_PATH,pUniDrvPlugin->Buffer);
                            StringCchPrintfW(wPluginSubRegPath, \
                                MAX_PATH, \
                                L"%ws\\%ws\\", \
                                wPluginRegPath, \
                                pLdrPluginList->CheckFileSignature.wPluginName);

                            if (NT_SUCCESS(GetDrvPluginInfo(pLdrPluginList,wPluginSubRegPath)))
                            {
                                InitializeListHead(&pLdrPluginList->List);
                                InsertTailList(&g_LdrPluginList,&pLdrPluginList->List);
                            }
                            if (pKeyBasicInfo)
                            {
                                ExFreePool(pKeyBasicInfo);
                                pKeyBasicInfo = NULL;
                            }
                        }
                    }
                }
            }
        }
    } while (0);
    if (pKeyFullInfo)
    {
        ExFreePool(pKeyFullInfo);
        pKeyFullInfo = NULL;
    }
    if (hKey)
    {
        ZwClose(hKey);
    }
    return Status;
}
NTSTATUS GetPluginRootPath(PUNICODE_STRING pUniDrvRegPath, \
                           PUNICODE_STRING pUniPluginPath)
{
    NTSTATUS Status;
    UNICODE_STRING UniDrvRootKey;
    //UNICODE_STRING UniRetPluginPath;
    WCHAR wOutPluginRootPath[MAX_PATH];
    ULONG ulLength;


    RtlZeroMemory(wOutPluginRootPath,sizeof(WCHAR) * MAX_PATH);
    Status = STATUS_SUCCESS;
    ulLength = 0;

    do 
    {
        if (NULL == pUniDrvRegPath)
        {
            Status = STATUS_INVALID_PARAMETER;
            break;
        }
        if (pUniDrvRegPath->Length <= 0 || \
            pUniDrvRegPath->Buffer == NULL)
        {
            Status = STATUS_INVALID_PARAMETER;
            break;
        }
        RtlInitUnicodeString(&UniDrvRootKey,DRV_REGISTER_ROOT_KEY);
        if (!RtlCompareUnicodeString(pUniDrvRegPath,&UniDrvRootKey,TRUE))
        {
            break;
        }
        //StringCchCopyW(wOutPluginRootPath,MAX_PATH,pUniDrvRegPath->Buffer);
        //RtlCopyMemory(wOutPluginRootPath,pUniDrvRegPath->Buffer,pUniDrvRegPath->Length);
        //StringCchCatW(wOutPluginRootPath,MAX_PATH,L"\\Plugin\\");
        //RtlInitUnicodeString(pUniPluginPath,wOutPluginRootPath);

        RtlCopyUnicodeString(pUniPluginPath,pUniDrvRegPath);
        RtlAppendUnicodeToString(pUniPluginPath,L"\\Plugin\\");

 
    } while (0);
    return Status;
}
VOID LoadBootPlugin(PVOID StartContext)
{
    PUNICODE_STRING pUniRegPath;
    UNICODE_STRING UniDrvPluginPath;
    UNICODE_STRING UniIoCreateDriver;
    NTSTATUS Status;

    Status = STATUS_SUCCESS;
    pUniRegPath = (PUNICODE_STRING)StartContext;

    do 
    {
        if (g_pFilterCtl->FileFltAccessCtl.ulIsFileSystemInit == 1)
        {
            if (NULL == g_pFilterCtl->ulIoCreateDriver)
            {
                RtlInitUnicodeString(&UniIoCreateDriver,L"IoCreateDriver");
                g_pFilterCtl->ulIoCreateDriver = MmGetSystemRoutineAddress(&UniIoCreateDriver);
                InitializeListHead(&g_LdrPluginList);
            }
            Status = GetPluginRootPath(pUniRegPath,&UniDrvPluginPath);
            if (NT_SUCCESS(Status))
            {
                Status = EnumPlugin(&UniDrvPluginPath);
                if (NT_SUCCESS(Status))
                {

                }
            }
        }
    } while (TRUE);
    PsTerminateSystemThread(Status);
}
