#ifndef __SHIT_DRV_H__
#define __SHIT_DRV_H__

#include <ntifs.h>
#include <ntdef.h>
#include <fltKernel.h>
#include <strsafe.h>
#include "IoCtlCode.h"
#include <Ntstrsafe.h>
#include <WINDEF.H>
#include <ntimage.h>

#define SHITDRV_TAG		'PaSD'
#define MAX_PATH 260
#define MAX_OBJNAME_LENGTH  1024
#define	MAX_VOLUME_CHARS 26

#define UNICODE_STRING_CONST(x) \
{sizeof(L##x)-2, sizeof(L##x), L##x}

#pragma pack(1)
typedef struct _FILTER_ACCESS_CONTROL
{
    ULONG ulIsInitialize:1;
    ULONG ulIsStartFilter:1;
    ULONG ulIsFilterRead:1;
    ULONG ulIsFilterWrite:1;
    ULONG ulIsFilterCreate:1;
    ULONG ulIsFilterOpen:1;
    ULONG ulIsFilterDelete:1;
    ULONG ulIsFilterRenName:1;
    ULONG ulIsFilterSetSize:1;
    ULONG ulIsFilterSetInfo:1;
    ULONG ulReserved:22;
}FILTER_ACCESS_CONTROL,*PFILTER_ACCESS_CONTROL;

typedef struct _FILTER_DAT_CONTROL
{
    ULONG ulIsStartFilter:1;
    ULONG ulIsFilterWrite:1;
    ULONG ulIsFilterDelete:1;
    ULONG ulIsFilterRenName:1;
    ULONG ulIsFilterSetSize:1;
    ULONG ulIsFilterSetInfo:1;
    ULONG ulReserved:26;
} FILTER_DAT_CONTROL,*PFILTER_DAT_CONTROL;

typedef struct _MY_FILTER_CONTROL 
{
    FILTER_ACCESS_CONTROL FileFltAccessCtl;
	PFLT_FILTER pFilterHandle;
    PFLT_INSTANCE pFltInstance;
    PFLT_PORT ServerPort;
    PFLT_PORT ClientPort;
	PVOID pMyProc;
	UNICODE_STRING UniCurPath;
    UNICODE_STRING UniSandBoxPath;
    UNICODE_STRING UniFloppy;
    PFLT_INSTANCE pFltSandboxVolInstance;

	HANDLE hFltProcessId;

    
}MY_FILTER_CONTROL,*PMY_FILTER_CONTROL;
#pragma pack()
extern PMY_FILTER_CONTROL g_pFilterCtl;
extern USHORT *NtBuildNumber;

extern PKEVENT g_pEventWaitKrl;
extern HANDLE g_hEventWaitKrl;


UCHAR *PsGetProcessImageFileName(IN PEPROCESS Process);
NTSTATUS
NTAPI
ZwQueryInformationProcess(HANDLE ProcessHandle, \
						  PROCESSINFOCLASS ProcessInformationClass, \
						  PVOID ProcessInformation, \
						  ULONG ProcessInformationLength, \
						  PULONG ReturnLength);

#endif