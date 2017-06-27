#ifndef __REGISTER_CALLBACK_H__
#define __REGISTER_CALLBACK_H__

typedef struct _REGISTER_CALLBACK 
{
    LARGE_INTEGER CmHandleCookie;
    PVOID pMyProc;
}REGISTER_CALLBACK,*PREGISTER_CALLBACK;

NTSTATUS InitializeCmCallBack(PDRIVER_OBJECT pDrvObj, \
                              PUNICODE_STRING pUniAltitude, \
                              PVOID pContext, \
                              BOOLEAN bRemove);
NTSTATUS RegistryCallback(PVOID CallbackContext,PVOID Argument1,PVOID Argument2);

extern PREGISTER_CALLBACK g_pRegisterCallBack;
#endif