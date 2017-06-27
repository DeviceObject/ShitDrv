#include "ShitDrv.h"
#include "InitializeInjectRelevantInfo.h"
#include "InjectKtrapFrame.h"
INJECT_RELEVANT_OFFSET g_InjectRelevantOffset = {0};

PVOID x64GetKeServiceDescriptorTable()
{
	PUCHAR pSearchStartAddress;
	PUCHAR pSearchEndAddress;
	PUCHAR pCount;
	ULONG ulOffset;

	pSearchStartAddress = (PUCHAR)__readmsr(0xC0000082);
	pSearchEndAddress = pSearchStartAddress + PAGE_SIZE;
	ulOffset = 0;

	for (pCount = pSearchStartAddress;pCount < pSearchEndAddress;pCount++)
	{
		if(MmIsAddressValid(pCount) && \
			MmIsAddressValid(pCount + 1) && \
			MmIsAddressValid(pCount + 2))
		{
			if (*pCount == 0x4c && \
				*(pCount + 1) == 0x8D && \
				*(pCount + 2) == 0x15)
			{
				ulOffset = *(ULONG*)(pCount + 3);
				if (ulOffset)
				{
					if (MmIsAddressValid((PVOID)((ULONG_PTR)ulOffset + (ULONG_PTR)(pCount + 7))))
					{
						return (PVOID)((ULONGLONG)ulOffset + (ULONGLONG)(pCount + 7));
					}
				}
			}
		}
	}
	return NULL;
}
ULONG GetProcessNameOffset()
{
	PVOID pCurProc;
	ULONG uli;
	uli = 0;
	pCurProc = (PVOID)PsGetCurrentProcess();
	for(uli = 0; uli < 3 * PAGE_SIZE;uli++)
	{
		if(strncmp((PCHAR)((ULONG)pCurProc + uli),NT_SYSTEM_NAME,strlen(NT_SYSTEM_NAME)) == 0)
		{
			return uli;
		}
	}
	return 0;
}
BOOLEAN InitializeWindows2k(PINJECT_RELEVANT_OFFSET pInjectRelevantOffset)
{
	if (NULL == pInjectRelevantOffset)
	{
		return FALSE;
	}
	pInjectRelevantOffset->WindowsVersion.bIs64Bit = FALSE;
	pInjectRelevantOffset->WindowsVersion.bIsWindows2000 = TRUE;
	pInjectRelevantOffset->ulOffsetPeb = 0x00;
	pInjectRelevantOffset->ulOffsetName = 0x01FC;
	pInjectRelevantOffset->ulOffsetFlink = 0x00;
	pInjectRelevantOffset->ulOffsetThreadListHead = 0x00;
	pInjectRelevantOffset->ulOffsetPid = 0x00;

	pInjectRelevantOffset->ulOffsetSuspendCount = 0x00;
	pInjectRelevantOffset->ulOffsetCrossThreadFlags = 0x00;
	pInjectRelevantOffset->ulOffsetCid = 0x00;
	pInjectRelevantOffset->ulOffsetTrapFrame = 0x00;
	pInjectRelevantOffset->ulOffsetThreadListEntry = 0x00;
	
	return TRUE;
}
BOOLEAN InitializeWindowsXp(PINJECT_RELEVANT_OFFSET pInjectRelevantOffset)
{
	if (NULL == pInjectRelevantOffset)
	{
		return FALSE;
	}
	pInjectRelevantOffset->WindowsVersion.bIs64Bit = FALSE;
	pInjectRelevantOffset->WindowsVersion.bIsWindowsXp = TRUE;
	pInjectRelevantOffset->ulOffsetPeb = 0x1B0;
	pInjectRelevantOffset->ulOffsetName = 0x174;
	pInjectRelevantOffset->ulOffsetFlink = 0x88;
	pInjectRelevantOffset->ulOffsetThreadListHead = 0x190;
	pInjectRelevantOffset->ulOffsetPid = 0x84;

	pInjectRelevantOffset->ulOffsetSuspendCount = 0x1b9;
	pInjectRelevantOffset->ulOffsetCrossThreadFlags = 0x248;
	pInjectRelevantOffset->ulOffsetCid = 0x1ec;
	pInjectRelevantOffset->ulOffsetTrapFrame = 0x134;
	pInjectRelevantOffset->ulOffsetThreadListEntry = 0x22c;
	pInjectRelevantOffset->ulOffsetTeb = 0x20;

	pInjectRelevantOffset->ulOffsetPebLdr = 0x0c;
	pInjectRelevantOffset->ulOffsetPebModuleListEntry = 0x0c;

	pInjectRelevantOffset->ulOffsetActivationContextStackPointer = 0x1A8;
	return TRUE;
}
BOOLEAN InitializeWindows2003(PINJECT_RELEVANT_OFFSET pInjectRelevantOffset)
{
	if (NULL == pInjectRelevantOffset)
	{
		return FALSE;
	}
	pInjectRelevantOffset->WindowsVersion.bIs64Bit = FALSE;
	pInjectRelevantOffset->WindowsVersion.bIsWindows2003 = TRUE;
	pInjectRelevantOffset->ulOffsetPeb = 0x00;
	pInjectRelevantOffset->ulOffsetName = 0x00;
	pInjectRelevantOffset->ulOffsetFlink = 0x00;
	pInjectRelevantOffset->ulOffsetThreadListHead = 0x00;
	pInjectRelevantOffset->ulOffsetPid = 0x00;

	pInjectRelevantOffset->ulOffsetSuspendCount = 0x00;
	pInjectRelevantOffset->ulOffsetCrossThreadFlags = 0x00;
	pInjectRelevantOffset->ulOffsetCid = 0x00;
	pInjectRelevantOffset->ulOffsetTrapFrame = 0x00;
	pInjectRelevantOffset->ulOffsetThreadListEntry = 0x00;
	pInjectRelevantOffset->ulOffsetTeb = 0x20;

	pInjectRelevantOffset->ulOffsetPebLdr = 0x0c;
	pInjectRelevantOffset->ulOffsetPebModuleListEntry = 0x0c;

	pInjectRelevantOffset->ulOffsetActivationContextStackPointer = 0x1A8;
	return TRUE;
}
NTSTATUS InitializeWindows7(PINJECT_RELEVANT_OFFSET pInjectRelevantOffset)
{
	NTSTATUS Status;

	if (NULL == pInjectRelevantOffset)
	{
		return FALSE;
	}
	pInjectRelevantOffset->WindowsVersion.bIsWindows7 = TRUE;
	Status = IsWindows64Bits(PsGetCurrentProcess());
	if (Status == 0x86)
	{
		pInjectRelevantOffset->WindowsVersion.bIs64Bit = FALSE;
		pInjectRelevantOffset->ulOffsetPeb = 0x01a8;
		pInjectRelevantOffset->ulOffsetName = 0x016c;
		pInjectRelevantOffset->ulOffsetFlink = 0x00b8;
		pInjectRelevantOffset->ulOffsetThreadListHead = 0x188;
		pInjectRelevantOffset->ulOffsetPid = 0xB4;

		pInjectRelevantOffset->ulOffsetSuspendCount = 0x188;
		pInjectRelevantOffset->ulOffsetCrossThreadFlags = 0x280;
		pInjectRelevantOffset->ulOffsetCid = 0x22c;
		pInjectRelevantOffset->ulOffsetTrapFrame = 0x128;
		pInjectRelevantOffset->ulOffsetThreadListEntry = 0x268;
		pInjectRelevantOffset->ulOffsetTeb = 0x88;

		pInjectRelevantOffset->ulOffsetPebLdr = 0x0c;
		pInjectRelevantOffset->ulOffsetPebModuleListEntry = 0x0c;

		pInjectRelevantOffset->ulOffsetActivationContextStackPointer = 0x1A8;
		return TRUE;
	}
	else if (Status == 0x64)
	{
		pInjectRelevantOffset->WindowsVersion.bIs64Bit = TRUE;
		pInjectRelevantOffset->ulOffsetPeb = 0x330 + 0x08;
		pInjectRelevantOffset->ulOffsetName = 0x2d8 + 0x08;
		pInjectRelevantOffset->ulOffsetFlink = 0x188;
		pInjectRelevantOffset->ulOffsetThreadListHead = 0x30;
		pInjectRelevantOffset->ulOffsetPid = 0x180;

		pInjectRelevantOffset->ulOffsetSuspendCount = 0x26c;
		pInjectRelevantOffset->ulOffsetCrossThreadFlags = 0x448;
		pInjectRelevantOffset->ulOffsetCid = 0x3b0;
		pInjectRelevantOffset->ulOffsetTrapFrame = 0x1d8;
		pInjectRelevantOffset->ulOffsetThreadListEntry = 0x2f8;
		pInjectRelevantOffset->ulOffsetTeb = 0x0b8;

		pInjectRelevantOffset->ulOffsetPebLdr = 0x18;
		pInjectRelevantOffset->ulOffsetPebModuleListEntry = 0x10;

		pInjectRelevantOffset->ulOffsetActivationContextStackPointer = 0x2c8;
		return TRUE;
	}
	else
	{
	}
	if (NT_ERROR(Status))
	{
		return FALSE;
	}
	return Status;
}
NTSTATUS InitializeWindows8_1(PINJECT_RELEVANT_OFFSET pInjectRelevantOffset)
{
	NTSTATUS Status;

	if (NULL == pInjectRelevantOffset)
	{
		return FALSE;
	}
	pInjectRelevantOffset->WindowsVersion.bIsWindows81 = TRUE;
	Status = IsWindows64Bits(PsGetCurrentProcess());
	if (Status == 0x86)
	{
		pInjectRelevantOffset->WindowsVersion.bIs64Bit = FALSE;
		pInjectRelevantOffset->ulOffsetPeb = 0x140;
		pInjectRelevantOffset->ulOffsetName = 0x170;
		pInjectRelevantOffset->ulOffsetFlink = 0x0b8;
		pInjectRelevantOffset->ulOffsetThreadListHead = 0x194;
		pInjectRelevantOffset->ulOffsetPid = 0xB4;

		pInjectRelevantOffset->ulOffsetSuspendCount = 0x18c;
		pInjectRelevantOffset->ulOffsetCrossThreadFlags = 0x3b8;
		pInjectRelevantOffset->ulOffsetCid = 0x364;
		pInjectRelevantOffset->ulOffsetTrapFrame = 0x06c;
		pInjectRelevantOffset->ulOffsetThreadListEntry = 0x39c;
		pInjectRelevantOffset->ulOffsetTeb = 0xa8;


		pInjectRelevantOffset->ulOffsetPebLdr = 0x0c;
		pInjectRelevantOffset->ulOffsetPebModuleListEntry = 0x0c;
		pInjectRelevantOffset->ulOffsetActivationContextStackPointer = 0x1A8;
		return TRUE;
	}
	else if (Status == 0x64)
	{
		pInjectRelevantOffset->WindowsVersion.bIs64Bit = TRUE;
		pInjectRelevantOffset->ulOffsetPeb = 0x330;
		pInjectRelevantOffset->ulOffsetName = 0x2d8;
		pInjectRelevantOffset->ulOffsetFlink = 0x188;
		pInjectRelevantOffset->ulOffsetThreadListHead = 0x300;
		pInjectRelevantOffset->ulOffsetPid = 0x180;

		pInjectRelevantOffset->ulOffsetSuspendCount = 0x26c;
		pInjectRelevantOffset->ulOffsetCrossThreadFlags = 0x448;
		pInjectRelevantOffset->ulOffsetCid = 0x3b0;
		pInjectRelevantOffset->ulOffsetTrapFrame = 0x1d8;
		pInjectRelevantOffset->ulOffsetThreadListEntry = 0x030;
		return TRUE;
	}
	else
	{
	}
	if (NT_ERROR(Status))
	{
		return FALSE;
	}
	return Status;
}
NTSTATUS InitializeWindows8(PINJECT_RELEVANT_OFFSET pInjectRelevantOffset)
{
	NTSTATUS Status;

	if (NULL == pInjectRelevantOffset)
	{
		return FALSE;
	}
	pInjectRelevantOffset->WindowsVersion.bIsWindows8 = TRUE;
	Status = IsWindows64Bits(PsGetCurrentProcess());
	if (Status == 0x86)
	{
		pInjectRelevantOffset->WindowsVersion.bIs64Bit = FALSE;
		pInjectRelevantOffset->ulOffsetPeb = 0x140;
		pInjectRelevantOffset->ulOffsetName = 0x170;
		pInjectRelevantOffset->ulOffsetFlink = 0x0b8;
		pInjectRelevantOffset->ulOffsetThreadListHead = 0x194;
		pInjectRelevantOffset->ulOffsetPid = 0xB4;

		pInjectRelevantOffset->ulOffsetSuspendCount = 0x18c;
		pInjectRelevantOffset->ulOffsetCrossThreadFlags = 0x268;
		pInjectRelevantOffset->ulOffsetCid = 0x214;
		pInjectRelevantOffset->ulOffsetTrapFrame = 0x06c;
		pInjectRelevantOffset->ulOffsetThreadListEntry = 0x24c;
		pInjectRelevantOffset->ulOffsetTeb = 0xa8;


		pInjectRelevantOffset->ulOffsetPebLdr = 0x0c;
		pInjectRelevantOffset->ulOffsetPebModuleListEntry = 0x0c;
		pInjectRelevantOffset->ulOffsetActivationContextStackPointer = 0x1A8;
		return TRUE;
	}
	else if (Status == 0x64)
	{
		pInjectRelevantOffset->WindowsVersion.bIs64Bit = TRUE;
		pInjectRelevantOffset->ulOffsetPeb = 0x330;
		pInjectRelevantOffset->ulOffsetName = 0x2d8;
		pInjectRelevantOffset->ulOffsetFlink = 0x188;
		pInjectRelevantOffset->ulOffsetThreadListHead = 0x300;
		pInjectRelevantOffset->ulOffsetPid = 0x180;

		pInjectRelevantOffset->ulOffsetSuspendCount = 0x26c;
		pInjectRelevantOffset->ulOffsetCrossThreadFlags = 0x448;
		pInjectRelevantOffset->ulOffsetCid = 0x3b0;
		pInjectRelevantOffset->ulOffsetTrapFrame = 0x1d8;
		pInjectRelevantOffset->ulOffsetThreadListEntry = 0x030;
		return TRUE;
	}
	else
	{
	}
	if (NT_ERROR(Status))
	{
		return FALSE;
	}
	return Status;
}
NTSTATUS InitializeWindows10(PINJECT_RELEVANT_OFFSET pInjectRelevantOffset)
{
	NTSTATUS Status;

	if (NULL == pInjectRelevantOffset)
	{
		return FALSE;
	}
	pInjectRelevantOffset->WindowsVersion.bIsWindows10 = TRUE;
	Status = IsWindows64Bits(PsGetCurrentProcess());
	if (Status == 0x86)
	{
		pInjectRelevantOffset->WindowsVersion.bIs64Bit = FALSE;
		pInjectRelevantOffset->ulOffsetPeb = 0x144;
		pInjectRelevantOffset->ulOffsetName = 0x174;
		pInjectRelevantOffset->ulOffsetFlink = 0x0b8;
		pInjectRelevantOffset->ulOffsetThreadListHead = 0x198;
		pInjectRelevantOffset->ulOffsetPid = 0xB4;

		pInjectRelevantOffset->ulOffsetSuspendCount = 0x18c;
		pInjectRelevantOffset->ulOffsetCrossThreadFlags = 0x3c8;
		pInjectRelevantOffset->ulOffsetCid = 0x374;
		pInjectRelevantOffset->ulOffsetTrapFrame = 0x06c;
		pInjectRelevantOffset->ulOffsetThreadListEntry = 0x3ac;
		pInjectRelevantOffset->ulOffsetTeb = 0xa8;

		pInjectRelevantOffset->ulOffsetPebLdr = 0x0c;
		pInjectRelevantOffset->ulOffsetPebModuleListEntry = 0x0c;

		pInjectRelevantOffset->ulOffsetActivationContextStackPointer = 0x1A8;
		return TRUE;
	}
	else if (Status == 0x64)
	{
		pInjectRelevantOffset->WindowsVersion.bIs64Bit = TRUE;
		pInjectRelevantOffset->ulOffsetPeb = 0x330;
		pInjectRelevantOffset->ulOffsetName = 0x2d8;
		pInjectRelevantOffset->ulOffsetFlink = 0x188;
		pInjectRelevantOffset->ulOffsetThreadListHead = 0x300;
		pInjectRelevantOffset->ulOffsetPid = 0x180;

		pInjectRelevantOffset->ulOffsetSuspendCount = 0x26c;
		pInjectRelevantOffset->ulOffsetCrossThreadFlags = 0x448;
		pInjectRelevantOffset->ulOffsetCid = 0x3b0;
		pInjectRelevantOffset->ulOffsetTrapFrame = 0x1d8;
		pInjectRelevantOffset->ulOffsetThreadListEntry = 0x030;
		return TRUE;
	}
	else
	{
	}
	if (NT_ERROR(Status))
	{
		return FALSE;
	}
	return Status;
}
BOOLEAN InitializeInjectInformation(PINJECT_RELEVANT_OFFSET pInjectRelevantOffset)
{
	SYSTEM_VERSION SystemVersion;
	
	RtlZeroMemory(pInjectRelevantOffset,sizeof(INJECT_RELEVANT_OFFSET));
	RtlZeroMemory(&SystemVersion,sizeof(SYSTEM_VERSION));
	PsGetVersion(&SystemVersion.ulMajorVersion, \
		&SystemVersion.ulMinorVersion, \
		&SystemVersion.ulBuildNumber, \
		SystemVersion.unStrCSDVersion);
	//从vista开始取NtBuildNumber就可以知道是什么系统和sp了vista 6000 sp1 6001 sp2 6002 win7 7600 win8 9200 win10 10586
	//OsMajorVersion = 6 OsMinorVersion =0 VISTA 
	//OsMajorVersion = 6 OsMinorVersion =1 win7
	//OsMajorVersion = 6 OsMinorVersion =2 win8
	//OsMajorVersion = 0xA OsMinorVersion =0 win10
	if (SystemVersion.ulMajorVersion == 0x0A && SystemVersion.ulMinorVersion == 0)
	{
		DbgPrint("Windows 10\r\n");
		if (InitializeWindows10(pInjectRelevantOffset) == TRUE)
		{
			//if (pInjectRelevantOffset->ulOffsetName == GetProcessNameOffset())
			//{
			//	return TRUE;
			//}
			return TRUE;
		}
		return FALSE;
	}
	else if (SystemVersion.ulMajorVersion == 6 && SystemVersion.ulMinorVersion == 3)
	{
		DbgPrint("Windows 8.1\r\n");
		if (InitializeWindows8_1(pInjectRelevantOffset) == TRUE)
		{
			//if (pInjectRelevantOffset->ulOffsetName == GetProcessNameOffset())
			//{
			//	return TRUE;
			//}
			return TRUE;
		}
		return FALSE;
	}
	else if (SystemVersion.ulMajorVersion == 6 && SystemVersion.ulMinorVersion == 2)
	{
		DbgPrint("Windows 8\r\n");
		if (InitializeWindows8(pInjectRelevantOffset) == TRUE)
		{
			//if (pInjectRelevantOffset->ulOffsetName == GetProcessNameOffset())
			//{
			//	return TRUE;
			//}
			return TRUE;
		}
		return FALSE;
	}
	else if (SystemVersion.ulMajorVersion == 6 && SystemVersion.ulMinorVersion == 1)
	{
		DbgPrint("Windows 7\r\n");
		if (InitializeWindows7(pInjectRelevantOffset) == TRUE)
		{
			//if (pInjectRelevantOffset->ulOffsetName == GetProcessNameOffset())
			//{
			//	return TRUE;
			//}
			return TRUE;
		}
		return FALSE;
	}
	else if (SystemVersion.ulMajorVersion == 6 && SystemVersion.ulMinorVersion == 0)
	{
		DbgPrint("Windows Vista");
		if (SystemVersion.ulBuildNumber == 6001)
		{
			DbgPrint(" Sp 1\r\n");
		}
		else if (SystemVersion.ulBuildNumber == 6002)
		{
			DbgPrint(" Sp 2\r\n");
		}
		else
		{
			DbgPrint("\r\n");
		}

	}
	else if (SystemVersion.ulMajorVersion == 5 && SystemVersion.ulMinorVersion == 2)
	{
		DbgPrint("Windows 2003\r\n");
		if (InitializeWindows2003(pInjectRelevantOffset) == TRUE)
		{
			//if (pInjectRelevantOffset->ulOffsetName == GetProcessNameOffset())
			//{
			//	return TRUE;
			//}
			return TRUE;
		}
		return FALSE;
		
	}
	else if (SystemVersion.ulMajorVersion == 5 && SystemVersion.ulMinorVersion == 1)
	{
		DbgPrint("Windows XP\r\n");
		if (InitializeWindowsXp(pInjectRelevantOffset) == TRUE)
		{
			//if (pInjectRelevantOffset->ulOffsetName == GetProcessNameOffset())
			//{
			//	return TRUE;
			//}
			return TRUE;
		}
		return FALSE;
	}
	else if (SystemVersion.ulMajorVersion == 5 && SystemVersion.ulMinorVersion == 0)
	{
		DbgPrint("Windows 2000\r\n");
		if (InitializeWindows2k(pInjectRelevantOffset) == TRUE)
		{
			if (pInjectRelevantOffset->ulOffsetName == GetProcessNameOffset())
			{
				return TRUE;
			}
		}
		return FALSE;
	}
	else if (SystemVersion.ulMajorVersion == 4 && SystemVersion.ulMinorVersion == 0)
	{
		DbgPrint("Windows 40\r\n");
	}
	return FALSE;
}
