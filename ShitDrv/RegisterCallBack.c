#include "ShitDrv.h"
#include "UtilsApi.h"
#include "RegisterOperation.h"
#include "RegisterCallBack.h"

PREGISTER_CALLBACK g_pRegisterCallBack = NULL;

NTSTATUS RegistryCallback(PVOID CallbackContext,PVOID Argument1,PVOID Argument2)
{
    NTSTATUS Status;
    REG_NOTIFY_CLASS RegNotifyClass;

    RegNotifyClass = (LONG)Argument1;
    Status = STATUS_SUCCESS;


    switch(RegNotifyClass)
    {
		case RegNtPreCreateKey:
            Status = FilterPreCreateKey(CallbackContext,Argument2);
            break;
		case RegNtPreCreateKeyEx:
            Status = FilterPreCreateKeyEx(CallbackContext,Argument2,RegNotifyClass);
            break;
        case RegNtPostSetValueKey:
            Status = FilterPostSetKey(CallbackContext,Argument2,RegNotifyClass);
            break;
        case RegNtPostSetInformationKey:
            Status = FilterPostSetInformationKey(CallbackContext,Argument2,RegNotifyClass);
            break;
        case RegNtPostRenameKey:
            Status = FilterPostRenNameKey(CallbackContext,Argument2,RegNotifyClass);
            break;
        case RegNtPostDeleteKey:
            Status = FilterPostDeleteKey(CallbackContext,Argument2,RegNotifyClass);
            break;
        case RegNtPostDeleteValueKey:
            Status = FilterPostDeleteValueKey(CallbackContext,Argument2,RegNotifyClass);
            break;
        default:
            break;
    }
    return Status;
}
NTSTATUS InitializeCmCallBack(PDRIVER_OBJECT pDrvObj, \
                              PUNICODE_STRING pUniAltitude, \
                              PVOID pContext, \
                              BOOLEAN bRemove)
{
    NTSTATUS Status;

    Status = STATUS_UNSUCCESSFUL;
    if (g_pRegisterCallBack == NULL)
    {
        do 
        {
            g_pRegisterCallBack = ExAllocatePoolWithTag(NonPagedPool,sizeof(REGISTER_CALLBACK),SHITDRV_TAG);
        } while (NULL == g_pRegisterCallBack);
        RtlZeroMemory(g_pRegisterCallBack,sizeof(REGISTER_CALLBACK));
    }

    if (bRemove)
    {
        if (g_pRegisterCallBack->CmHandleCookie.QuadPart != 0)
        {
            Status = CmUnRegisterCallback(g_pRegisterCallBack->CmHandleCookie);
            if (NT_SUCCESS(Status))
            {
                DbgPrint("CmUnRegisterCallback Success\n");
                g_pRegisterCallBack->CmHandleCookie.QuadPart = 0;
            }
            else
            {
                DbgPrint("CmUnRegisterCallback Failed\n");
            }
        }
    }
    else
    {
        if (g_pRegisterCallBack->CmHandleCookie.QuadPart == 0)
        {
            Status = CmRegisterCallbackEx(RegistryCallback, \
                pUniAltitude, \
                pDrvObj, \
                pContext, \
                &g_pRegisterCallBack->CmHandleCookie, \
                NULL);
            if (NT_SUCCESS(Status))
            {
                DbgPrint("CmRegisterCallback Success\n");
            }
            else
            {
                DbgPrint("CmRegisterCallback Failed\n");
            }
        }
    }
    return Status;
}