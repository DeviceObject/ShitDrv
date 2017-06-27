#ifndef __REGISTER_OPERATION_H__
#define __REGISTER_OPERATION_H__

NTSTATUS FilterPreCreateKeyEx(PVOID pCallbackContext,PVOID Argument2,ULONG ulType);
NTSTATUS FilterPreCreateKey(PVOID pCallbackContext,PVOID Argument2);
NTSTATUS FilterPost(PREG_POST_OPERATION_INFORMATION pPostInfo,ULONG ulOperationType);
NTSTATUS FilterPostSetKey(PVOID pCallbackContext,PVOID Argument2,ULONG ulType);
NTSTATUS FilterPostSetInformationKey(PVOID pCallbackContext,PVOID Argument2,ULONG ulType);
NTSTATUS FilterPostRenNameKey(PVOID pCallbackContext,PVOID Argument2,ULONG ulType);
NTSTATUS FilterPostDeleteKey(PVOID pCallbackContext,PVOID Argument2,ULONG ulType);
NTSTATUS FilterPostDeleteValueKey(PVOID pCallbackContext,PVOID Argument2,ULONG ulType);

#endif