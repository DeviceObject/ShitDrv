#ifndef __LOAD_PLUGIN_H__
#define __LOAD_PLUGIN_H__

#define MAX_PATH 260
#define LDR_PLUGIN_TAG      'GPDL'
#pragma pack(1)
typedef struct _LOAD_MODE_CONTROL
{
    ULONG IsBootLoad:1;
    ULONG IsGeneralLoad:1;
    ULONG IsShutdownLoad:1;
    ULONG ulReserved:29;
} LOAD_MODE_CONTROL,*PLOAD_MODE_CONTROL;
typedef struct _CHECK_FILE_SIGNATURE
{
    PUNICODE_STRING pUniRegPath;
    LOAD_MODE_CONTROL LdrCtl;
    WCHAR wPluginName[MAX_PATH];
    WCHAR wPluginFileFullPath[MAX_PATH];
} CHECK_FILE_SIGNATURE,*PCHECK_FILE_SIGNATURE;
typedef struct _LDR_PLUGIN_LIST
{
    LIST_ENTRY List;
    CHECK_FILE_SIGNATURE CheckFileSignature;
    PDRIVER_INITIALIZE PluginDriverEntry;
} LDR_PLUGIN_LIST,*PLDR_PLUGIN_LIST;
#pragma pack()

extern LIST_ENTRY g_LdrPluginList;
#define DRV_REGISTER_ROOT_KEY L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\"

NTSTATUS GetPluginRootPath(PUNICODE_STRING pUniDrvRegPath, \
                           PUNICODE_STRING pUniPluginPath);
NTSTATUS EnumPlugin(PUNICODE_STRING pUniDrvPlugin);
VOID LoadBootPlugin(PVOID StartContext);
PLDR_PLUGIN_LIST AllocateLdrPlugin();
NTSTATUS GetDrvPluginInfo(PLDR_PLUGIN_LIST pLdrPluginList,WCHAR *wDrvPluginPath);
#endif