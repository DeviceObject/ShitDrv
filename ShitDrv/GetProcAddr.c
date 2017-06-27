#include "ShitDrv.h"
#include "InitializeInjectRelevantInfo.h"
#include "InjectKtrapFrame.h"
#include "GetProcAddr.h"

ULONG_PTR GetModuleBaseFromPeb(PINJECT_OBJECT_INFORMATION pInjectObjInfo,PCHAR pModuleName)
{
	PPEB Peb;
	PPEB_LDR_DATA pPebLdrDat;
	PLDR_MODULE pLdrModule;
	PLIST_ENTRY pCurListEntry;
	PLIST_ENTRY pHeaderListEntry;
	ANSI_STRING aStrModuleName;
	UNICODE_STRING unStrModuleName;

	if (NULL == pInjectObjInfo || \
		NULL == pModuleName)
	{
		return (ULONG_PTR)NULL;
	}
	RtlInitAnsiString(&aStrModuleName,pModuleName);
	RtlAnsiStringToUnicodeString(&unStrModuleName,&aStrModuleName,TRUE);
	Peb = (PPEB)*(ULONG_PTR*)((ULONG_PTR)pInjectObjInfo->pInjectProcess + g_InjectRelevantOffset.ulOffsetPeb);
	if (NULL == Peb)
	{
		RtlFreeUnicodeString(&unStrModuleName);
		return (ULONG_PTR)NULL;
	}
	pPebLdrDat = (PPEB_LDR_DATA)*(ULONG_PTR*)((ULONG_PTR)Peb + g_InjectRelevantOffset.ulOffsetPebLdr);
	if (NULL == pPebLdrDat)
	{
		RtlFreeUnicodeString(&unStrModuleName);
		return (ULONG_PTR)NULL;
	}
	pHeaderListEntry = pCurListEntry = pPebLdrDat->InLoadOrderModuleList.Flink;
	while (pHeaderListEntry != pCurListEntry->Flink)
	{
		pLdrModule = (PLDR_MODULE)pCurListEntry;
		if (NULL == pLdrModule)
		{
			continue;
		}
		if (RtlCompareUnicodeString(&pLdrModule->BaseDllName,&unStrModuleName,TRUE) == 0)
		{
			RtlFreeUnicodeString(&unStrModuleName);
			return (ULONG_PTR)pLdrModule->BaseAddress;
		}
		pCurListEntry = pCurListEntry->Flink;
	}
	RtlFreeUnicodeString(&unStrModuleName);
	return (ULONG_PTR)NULL;
}
ULONG_PTR GetExportFunction(PVOID pBaseAddress,PCHAR pFunctionName)
{
	PIMAGE_DOS_HEADER pDosHeader;
	PIMAGE_NT_HEADERS pNtHeader;
	PIMAGE_EXPORT_DIRECTORY pEat;
	BOOLEAN bIsWow64Bits;
	ULONG ulNumberOfNames,uli;
	ULONG *ulAddressOfNames;
	ULONG *ulAddressOfFunctions;
	USHORT *ulAddressOfNameOrdinals;
	USHORT ulFunctionsOrdinals;
	PCHAR pTargetApiName,pTargetApiAddress;

	if (NULL == pBaseAddress || \
		NULL == pFunctionName)
	{
		return (ULONG_PTR)NULL;
	}
	pDosHeader = (PIMAGE_DOS_HEADER)pBaseAddress;
	if (pDosHeader->e_magic != IMAGE_DOS_SIGNATURE)
	{
		return (ULONG_PTR)NULL;
	}
	pNtHeader = (PIMAGE_NT_HEADERS)((ULONG_PTR)pDosHeader + pDosHeader->e_lfanew);
	if (pNtHeader->Signature != IMAGE_NT_SIGNATURE)
	{
		return (ULONG_PTR)NULL;
	}
	if (pNtHeader->FileHeader.Machine == IMAGE_FILE_MACHINE_I386)
	{
		PIMAGE_OPTIONAL_HEADER32 pOptHeader32;

		pOptHeader32 = (PIMAGE_OPTIONAL_HEADER32)((ULONG)pNtHeader + sizeof(ULONG) + sizeof(IMAGE_FILE_HEADER));
		pEat = (PIMAGE_EXPORT_DIRECTORY)((ULONG)pBaseAddress + pOptHeader32->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
		bIsWow64Bits = FALSE;
	}
	else if (pNtHeader->FileHeader.Machine == IMAGE_FILE_MACHINE_IA64 || \
		pNtHeader->FileHeader.Machine == IMAGE_FILE_MACHINE_AMD64)
	{
		PIMAGE_OPTIONAL_HEADER64 pOptHeader64;

		pOptHeader64 = (PIMAGE_OPTIONAL_HEADER64)((ULONG64)pNtHeader + sizeof(ULONG) + sizeof(IMAGE_FILE_HEADER));
		pEat = (PIMAGE_EXPORT_DIRECTORY)((ULONGLONG)pBaseAddress + pOptHeader64->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
		bIsWow64Bits = TRUE;
	}
	else
	{
		bIsWow64Bits = FALSE;
	}
	ulNumberOfNames = pEat->NumberOfFunctions;
	ulAddressOfNames = (ULONG *)((ULONG_PTR)pBaseAddress + pEat->AddressOfNames);
	ulAddressOfFunctions = (ULONG *)((ULONG_PTR)pBaseAddress + pEat->AddressOfFunctions);
	ulAddressOfNameOrdinals = (USHORT *)((ULONG_PTR)pBaseAddress + pEat->AddressOfNameOrdinals);
	for (uli = 0;uli < ulNumberOfNames;uli++)
	{
		pTargetApiName = (PCHAR)((ULONG_PTR)pBaseAddress + ulAddressOfNames[uli]);
		if (_strnicmp(pTargetApiName,pFunctionName,strlen(pFunctionName)) == 0)
		{
			ulFunctionsOrdinals = (USHORT)(ulAddressOfNameOrdinals[uli] + pEat->Base - 1);
			pTargetApiAddress = (PCHAR)((ULONG_PTR)pBaseAddress + ulAddressOfFunctions[ulFunctionsOrdinals]);
			return (ULONG_PTR)pTargetApiAddress;
		}
	}
	return (ULONG_PTR)NULL;
}
ULONG_PTR SearchApiFromPeb(PINJECT_OBJECT_INFORMATION pInjectObjInfo,PCHAR pModuleName,PCHAR pApiName)
{
	PPEB Peb;
	PPEB_LDR_DATA pPebLdrDat;
	PLDR_MODULE pLdrModule;
	PLIST_ENTRY pCurListEntry;
	PLIST_ENTRY pHeaderListEntry;
	ANSI_STRING aStrModuleName;
	UNICODE_STRING unStrModuleName;

	if (NULL == pInjectObjInfo || \
		NULL == pModuleName || \
		NULL == pApiName)
	{
		return (ULONG_PTR)NULL;
	}
	RtlInitAnsiString(&aStrModuleName,pModuleName);
	RtlAnsiStringToUnicodeString(&unStrModuleName,&aStrModuleName,TRUE);
	Peb = (PPEB)*(ULONG_PTR*)((ULONG_PTR)pInjectObjInfo->pInjectProcess + g_InjectRelevantOffset.ulOffsetPeb);
	if (NULL == Peb)
	{
		RtlFreeUnicodeString(&unStrModuleName);
		return (ULONG_PTR)NULL;
	}
	pPebLdrDat = (PPEB_LDR_DATA)*(ULONG_PTR*)((ULONG_PTR)Peb + g_InjectRelevantOffset.ulOffsetPebLdr);
	if (NULL == pPebLdrDat)
	{
		RtlFreeUnicodeString(&unStrModuleName);
		return (ULONG_PTR)NULL;
	}
	pHeaderListEntry = pCurListEntry = pPebLdrDat->InLoadOrderModuleList.Flink;
	while (pHeaderListEntry != pCurListEntry->Flink)
	{
		pLdrModule = (PLDR_MODULE)pCurListEntry;
		if (NULL == pLdrModule)
		{
			continue;
		}
		if (RtlCompareUnicodeString(&pLdrModule->BaseDllName,&unStrModuleName,TRUE) == 0)
		{
			g_InjectAplList.ulLoadLibrary = GetExportFunction(pLdrModule->BaseAddress,pApiName);
			RtlFreeUnicodeString(&unStrModuleName);
			return g_InjectAplList.ulLoadLibrary;
		}
		pCurListEntry = pCurListEntry->Flink;
	}
	RtlFreeUnicodeString(&unStrModuleName);
	return (ULONG_PTR)NULL;
}
ULONG_PTR GetDllFunctionAddress(PCHAR pFunctionName,PUNICODE_STRING pDllName) 
{
	NTSTATUS Status;
	USHORT uMachine;
	HANDLE hSection,hFile;
	OBJECT_ATTRIBUTES ObjectAttributes;
	PIMAGE_DOS_HEADER pDosHeader;
	PIMAGE_EXPORT_DIRECTORY pEat;
	IO_STATUS_BLOCK IoStatusBlock;
	ULONG* ulAddressOfFunctions;
	ULONG* ulAddressOfNames;
	SHORT* AddressOfNameOrdinals;
	ULONG ulFunctionOrdinal;
	ULONG uli,FunctionAddress;
	PCHAR FunctionName;
	STRING ntFunctionName,ntFunctionNameSearch;
	PVOID pBaseAddress;
	SIZE_T Size;

	Size = 0;
	pBaseAddress = NULL;
	InitializeObjectAttributes(&ObjectAttributes,pDllName,OBJ_CASE_INSENSITIVE,NULL,0);

	Status = ZwOpenFile(&hFile, \
		FILE_EXECUTE | SYNCHRONIZE, \
		&ObjectAttributes, \
		&IoStatusBlock, \
		FILE_SHARE_READ, \
		FILE_SYNCHRONOUS_IO_NONALERT);
	if (NT_ERROR(Status))
	{
		return Status;
	}
	Status = ZwCreateSection(&hSection, \
		SECTION_MAP_READ, \
		NULL/*&ObjectAttributes*/, \
		NULL, \
		PAGE_READONLY, \
		SEC_IMAGE, \
		hFile);
	if (NT_ERROR(Status))
	{
		ZwClose(hFile);
		return Status;
	}
	Status = ZwMapViewOfSection(hSection, \
		NtCurrentProcess(), \
		&pBaseAddress, \
		0, \
		0, \
		0, \
		&Size, \
		ViewShare, \
		0, \
		PAGE_READONLY); 
	if (NT_ERROR(Status))
	{
		if (Status == STATUS_IMAGE_NOT_AT_BASE && \
			Size > 0)
		{
			
		}
		else
		{
			ZwClose(hFile);
			return Status;
		}
	}
	ZwClose(hFile);
	pDosHeader = (PIMAGE_DOS_HEADER)pBaseAddress;
	uMachine = ((PIMAGE_NT_HEADERS)((PCHAR)pDosHeader + pDosHeader->e_lfanew))->FileHeader.Machine;
	if (IMAGE_FILE_MACHINE_AMD64 == uMachine || \
		IMAGE_FILE_MACHINE_IA64 == uMachine)
	{
		PIMAGE_NT_HEADERS64 pNtHeaders64;

		pNtHeaders64 = (PIMAGE_NT_HEADERS64)((PCHAR)pDosHeader + pDosHeader->e_lfanew);
		pEat = (PIMAGE_EXPORT_DIRECTORY)((PCHAR)pBaseAddress +  \
			pNtHeaders64->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
	}
	else
	{
		PIMAGE_NT_HEADERS32 pNtHeaders32;

		pNtHeaders32 = (PIMAGE_NT_HEADERS32)((PCHAR)pDosHeader + pDosHeader->e_lfanew);
		pEat = (PIMAGE_EXPORT_DIRECTORY)((PCHAR)pBaseAddress +  \
			pNtHeaders32->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
		DbgPrint("%p",pEat);
	}
	ulAddressOfFunctions = (ULONG *)((PCHAR)pBaseAddress + pEat->AddressOfFunctions);
	ulAddressOfNames = (ULONG *)((PCHAR)pBaseAddress + pEat->AddressOfNames);
	AddressOfNameOrdinals = (SHORT*)pEat->AddressOfNameOrdinals;

	RtlInitString(&ntFunctionNameSearch,pFunctionName);

	for(uli = 0;uli < pEat->NumberOfFunctions;uli++)
	{
		FunctionName = (PCHAR)((PCHAR)pBaseAddress + ulAddressOfNames[uli]);
		RtlInitString(&ntFunctionName,FunctionName);
		ulFunctionOrdinal = AddressOfNameOrdinals[uli] + pEat->Base - 1;
		FunctionAddress = (ULONG)((PCHAR)pBaseAddress + ulAddressOfFunctions[ulFunctionOrdinal]);
		if (RtlCompareString(&ntFunctionName,&ntFunctionNameSearch,TRUE) == 0) 
		{
			ZwClose(hSection);
			return FunctionAddress;
		}
	}
	ZwClose(hSection);
	Status = STATUS_UNSUCCESSFUL;
	return Status;
}