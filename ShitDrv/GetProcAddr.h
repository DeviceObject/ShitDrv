#ifndef __GET_PROC_ADDR_H__
#define __GET_PROC_ADDR_H__

#endif

#define SEC_IMAGE 0x1000000

typedef struct _SECTION_IMAGE_INFORMATION
{
	PULONG TransferAddress;
	ULONG ZeroBits;
	ULONG MaximumStackSize;
	ULONG CommittedStackSize;
	ULONG SubSysmtemType;
	USHORT SubSystemMinorVersion;
	USHORT SubSystemMajorVersion;
	ULONG GpValue;
	USHORT Imagecharacteristics;
	USHORT DllCharacteristics;
	USHORT Machine;
	UCHAR  ImageContainsCode;
	UCHAR  Spare1;
	ULONG LoaderFlags;
	ULONG ImageFileSize;
	ULONG Reserved;
}SECTION_IMAGE_INFORMATION,*PSECTION_IMAGE_INFORMATION;
typedef struct _PEB_LDR_DATA
{  
	ULONG Length;
	BOOLEAN Initialized;
	PVOID SsHandle;
	LIST_ENTRY InLoadOrderModuleList;	//按加载顺序
	LIST_ENTRY InMemoryOrderModuleList;	//按内存顺序
	LIST_ENTRY InInitializationOrderModuleList;//按初始化顺序
	PVOID EntryInProgress;  
}PEB_LDR_DATA,*PPEB_LDR_DATA;
typedef struct _LDR_MODULE
{  
	LIST_ENTRY InLoadOrderModuleList;
	LIST_ENTRY InMemoryOrderModuleList;
	LIST_ENTRY InInitializationOrderModuleList;
	PVOID BaseAddress;
	PVOID EntryPoint;
	ULONG SizeOfImage;
	UNICODE_STRING FullDllName;
	UNICODE_STRING BaseDllName;
	ULONG Flags;
	SHORT LoadCount;
	SHORT TlsIndex;
	LIST_ENTRY HashTableEntry;
	ULONG TimeDateStamp;
}LDR_MODULE,*PLDR_MODULE;


typedef struct ServiceDescriptorEntry
{
	PVOID pServiceTableBase;
	ULONG_PTR *ServiceCounterTableBase; //Used only in checked build
	ULONG_PTR NumberOfServices;
	PVOID ParamTableBase;
}ServiceDescriptorTableEntry,*PServiceDescriptorTableEntry;
#ifndef _WIN64
extern PServiceDescriptorTableEntry KeServiceDescriptorTable;//SSDT
PServiceDescriptorTableEntry ShadowKeServiceDescriptorTable;//ShadowSSDT
#else
PServiceDescriptorTableEntry KeServiceDescriptorTable;
#endif

ULONG_PTR GetDllFunctionAddress(PCHAR pFunctionName,PUNICODE_STRING pDllName);
ULONG_PTR GetExportFunction(PVOID pBaseAddress,PCHAR pFunctionName);
ULONG_PTR SearchApiFromPeb(PINJECT_OBJECT_INFORMATION pInjectObjInfo,PCHAR pModuleName,PCHAR pApiName);
ULONG_PTR GetModuleBaseFromPeb(PINJECT_OBJECT_INFORMATION pInjectObjInfo,PCHAR pModuleName);