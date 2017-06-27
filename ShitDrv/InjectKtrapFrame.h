#ifndef __INJECT_KTRAP_FRAME_H__
#define __INJECT_KTRAP_FRAME_H__

#define WINDOWS_VERSION_NONE                                0
#define WINDOWS_VERSION_2K                                  1
#define WINDOWS_VERSION_XP                                  2
#define WINDOWS_VERSION_2K3                                 3
#define WINDOWS_VERSION_2K3_SP1                             4
#define WINDOWS_VERSION_VISTA                               5
#define WINDOWS_VERSION_WIN7                                6

//线程链表偏移地址
#define BASE_PROCESS_PEB_OFFSET_2K                          0			//NT5.0.2195.7133
#define BASE_PROCESS_PEB_OFFSET_XP                          0x01B0      //NT5.1.2600.3093
#define BASE_PROCESS_PEB_OFFSET_2K3                         0           //nt5.2.3790.0
#define BASE_PROCESS_PEB_OFFSET_2K3_SP1                     0           //nt5.2.3790.1830
#define BASE_PROCESS_PEB_OFFSET_VISTA                       0
#define BASE_PROCESS_PEB_OFFSET_WIN7_X86                    0x01a8		//win7
#define BASE_PROCESS_PEB_OFFSET_WIN7_X64                    0x0330 		//win7

#define BASE_PROCESS_NAME_OFFSET_2K                         0x01FC//NT5.0.2195.7133
#define BASE_PROCESS_NAME_OFFSET_XP                         0x0174//NT5.1.2600.3093
#define BASE_PROCESS_NAME_OFFSET_2K3                        0x0154//nt5.2.3790.0
#define BASE_PROCESS_NAME_OFFSET_2K3_SP1                    0x0164//nt5.2.3790.1830
#define BASE_PROCESS_NAME_OFFSET_VISTA                      0x014c
#define BASE_PROCESS_NAME_OFFSET_WIN7_X86                   0x016c  //win7
#define BASE_PROCESS_NAME_OFFSET_WIN7_X64                   0x02d8  //win7

#define BASE_PROCESS_FLINK_OFFSET_2K                        0//NT5.0.2195.7133
#define BASE_PROCESS_FLINK_OFFSET_XP                        0x0088//NT5.1.2600.3093
#define BASE_PROCESS_FLINK_OFFSET_2K3                       0//NT5.2.3790.0
#define BASE_PROCESS_FLINK_OFFSET_2K3_SP1                   0//nt5.2.3790.1830
#define BASE_PROCESS_FLINK_OFFSET_VISTA                     0
#define BASE_PROCESS_FLINK_OFFSET_WIN7_X86                  0x00b8//win7
#define BASE_PROCESS_FLINK_OFFSET_WIN7_X64                  0x0188//win7


//ThreadListEntry偏移
#define BASE_THREAD_LIST_OFFSET_2K                         0
#define BASE_THREAD_LIST_OFFSET_XP                         0x22c
#define BASE_THREAD_LIST_OFFSET_2K3                        0
#define BASE_THREAD_LIST_OFFSET_2K3_SP1                    0
#define BASE_THREAD_LIST_OFFSET_WIN7_X86				   0x268
#define BASE_THREAD_LIST_OFFSET_WIN7_X64				   0x030 

#define BASE_THREAD_SuspendCount_OFFSET_2K                 0
#define BASE_THREAD_SuspendCount_OFFSET_XP                 0x1b9
#define BASE_THREAD_SuspendCount_OFFSET_2K3                0
#define BASE_THREAD_SuspendCount_OFFSET_2K3_SP1            0
#define BASE_THREAD_SuspendCount_OFFSET_WIN7_X86           0x188//win7
#define BASE_THREAD_SuspendCount_OFFSET_WIN7_X64           0x26c//win7

#define BASE_THREAD_CrossThreadFlags_OFFSET_2K             0
#define BASE_THREAD_CrossThreadFlags_OFFSET_XP             0x248
#define BASE_THREAD_CrossThreadFlags_OFFSET_2K3            0
#define BASE_THREAD_CrossThreadFlags_OFFSET_2K3_SP1        0
#define BASE_THREAD_CrossThreadFlags_OFFSET_WIN7_X86       0x280
#define BASE_HREAD_CrossThreadFlags_OFFSET_WIN7_X64        0x448


//KTHREAD中的Cid偏移
#define BASE_THREAD_Cid_OFFSET_XP                          0x1ec
#define BASE_THREAD_Cid_OFFSET_WIN7_X86                    0x22c
#define BASE_THREAD_Cid_OFFSET_WIN7_X64                    0x3b0

//KTHREAD中的TrapFrame_偏移
#define BASE_THREAD_TrapFrame_OFFSET_XP                    0x134
#define BASE_THREAD_TrapFrame_OFFSET_WIN7_X86              0x128
#define BASE_THREAD_TrapFrame_OFFSET_WIN7_X64              0x1d8

//_EPROCESS中的ThreadListHead偏移
#define BASE_PROCESS_ThreadListHead_OFFSET_XP              0x190
#define BASE_PROCESS_ThreadListHead_OFFSET_WIN7_X86        0x188
#define BASE_PROCESS_ThreadListHead_OFFSET_WIN7_X64        0x300

//_EPROCESS中的Pid偏移
#define BASE_PROCESS_Pid_OFFSET_XP                         0x84
#define BASE_PROCESS_Pid_OFFSET_WIN7_X86                   0xB4
#define BASE_PROCESS_Pid_OFFSET_WIN7_X64                   0x180

#define PS_CROSS_THREAD_FLAGS_SYSTEM 0x00000010UL 
//#define IS_SYSTEM_THREAD(Thread) ((((Thread) + 0x280) & PS_CROSS_THREAD_FLAGS_SYSTEM) != 0)

typedef struct _INJECT_X64_KTRAP_FRAME
{
	ULONG64 P1Home;
	ULONG64 P2Home;
	ULONG64 P3Home;
	ULONG64 P4Home;
	ULONG64 P5;
	KPROCESSOR_MODE PreviousMode;
	KIRQL PreviousIrql;
	UCHAR FaultIndicator;
	UCHAR ExceptionActive;
	ULONG MxCsr;
	ULONG64 Rax;
	ULONG64 Rcx;
	ULONG64 Rdx;
	ULONG64 R8;
	ULONG64 R9;
	ULONG64 R10;
	ULONG64 R11;
	union
	{
		ULONG64 GsBase;
		ULONG64 GsSwap;
	};
	M128A Xmm0;
	M128A Xmm1;
	M128A Xmm2;
	M128A Xmm3;
	M128A Xmm4;
	M128A Xmm5;
	union
	{
		ULONG64 FaultAddress;
		ULONG64 ContextRecord;
		ULONG64 TimeStampCKCL;
	};
	ULONG64 Dr0;
	ULONG64 Dr1;
	ULONG64 Dr2;
	ULONG64 Dr3;
	ULONG64 Dr6;
	ULONG64 Dr7;

	union
	{
		struct
		{
			ULONG64 DebugControl;
			ULONG64 LastBranchToRip;
			ULONG64 LastBranchFromRip;
			ULONG64 LastExceptionToRip;
			ULONG64 LastExceptionFromRip;
		};
		struct
		{
			ULONG64 LastBranchControl;
			ULONG LastBranchMSR;
		};
	};
	USHORT SegDs;
	USHORT SegEs;
	USHORT SegFs;
	USHORT SegGs;
	ULONG64 TrapFrame;
	ULONG64 Rbx;
	ULONG64 Rdi;
	ULONG64 Rsi;
	ULONG64 Rbp;
	union
	{
		ULONG64 ErrorCode;
		ULONG64 ExceptionFrame;
		ULONG64 TimeStampKlog;
	};
	ULONG64 Rip;
	USHORT SegCs;
	UCHAR Fill0;
	UCHAR Logging;
	USHORT Fill1[2];
	ULONG EFlags;
	ULONG Fill2;
	ULONG64 Rsp;
	USHORT SegSs;
	USHORT Fill3;
	LONG CodePatchCycle;
} INJECT_X64_KTRAP_FRAME,*PINJECT_X64_KTRAP_FRAME;

typedef struct _INJECT_X86_KTRAP_FRAME
{
	ULONG   DbgEbp;
	ULONG   DbgEip;
	ULONG   DbgArgMark;
	ULONG   DbgArgPointer;
	ULONG   TempSegCs;
	ULONG   TempEsp;
	ULONG   Dr0;
	ULONG   Dr1;
	ULONG   Dr2;
	ULONG   Dr3;
	ULONG   Dr6;
	ULONG   Dr7;
	ULONG   SegGs;
	ULONG   SegEs;
	ULONG   SegDs;
	ULONG   Edx;
	ULONG   Ecx;
	ULONG   Eax;
	ULONG   PreviousPreviousMode;
	ULONG   ExceptionList;
	ULONG   SegFs;
	ULONG   Edi;
	ULONG   Esi;
	ULONG   Ebx;
	ULONG   Ebp;
	ULONG   ErrCode;
	ULONG   Eip;
	ULONG   SegCs;
	ULONG   EFlags;
	ULONG   HardwareEsp;
	ULONG   HardwareSegSs;
	ULONG   V86Es;
	ULONG   V86Ds;
	ULONG   V86Fs;
	ULONG   V86Gs;
}INJECT_X86_KTRAP_FRAME,*PINJECT_X86_KTRAP_FRAME;

typedef struct _LDR_DATA_TABLE_ENTRY
{
	LIST_ENTRY InLoadOrderLinks;
	LIST_ENTRY InMemoryOrderLinks;
	LIST_ENTRY InInitializationOrderLinks;
	PVOID DllBase;
	PVOID EntryPoint;
	ULONG SizeOfImage;
	UNICODE_STRING FullDllName;
	UNICODE_STRING BaseDllName;
	ULONG Flags;
	unsigned short LoadCount;
	unsigned short TlsIndex;
	union
	{
		LIST_ENTRY HashLinks;
		struct
		{
			PVOID SectionPointer;
			ULONG CheckSum;
		};
	};
	union
	{
		ULONG TimeDateStamp;
		PVOID LoadedImports;
	};
	PVOID EntryPointActivationContext;
	PVOID PatchInformation;
	LIST_ENTRY ForwarderLinks;
	LIST_ENTRY ServiceTagLinks;
	LIST_ENTRY StaticLinks;
}LDR_DATA_TABLE_ENTRY,*PLDR_DATA_TABLE_ENTRY;

typedef struct _INJECT_OBJECT_INFORMATION
{
	PVOID pInjectProcess;
	PVOID pInjectThread;
	CHAR InjectDllPath[100];
}INJECT_OBJECT_INFORMATION,*PINJECT_OBJECT_INFORMATION;

typedef NTSTATUS (__fastcall *FASTCALL_NTSUSPENDTHREAD)(IN HANDLE ThreadHandle,OUT PULONG PreviousSuspendCount OPTIONAL);
typedef NTSTATUS (__stdcall *STDCALL_NTSUSPENDTHREAD)(IN HANDLE ThreadHandle,OUT PULONG PreviousSuspendCount OPTIONAL);
typedef NTSTATUS (__stdcall *STDCALL_NTPROTECTVIRTUALMEMORY)(HANDLE ProcessHandle, \
															 PVOID *BaseAddress , \
															 PSIZE_T RegionSize, \
															 ULONG NewProtect, \
															 PULONG OldProtect);
typedef NTSTATUS (__fastcall *FASTCALL_NTPROTECTVIRTUALMEMORY)(HANDLE ProcessHandle, \
															 PVOID *BaseAddress , \
															 PSIZE_T RegionSize, \
															 ULONG NewProtect, \
															 PULONG OldProtect);

typedef struct _INJECT_API_LIST
{
	FASTCALL_NTSUSPENDTHREAD FastCallKeSuspendThread;
	FASTCALL_NTSUSPENDTHREAD FastCallKeResumeThread;

	STDCALL_NTSUSPENDTHREAD StdCallKeSuspendThread;
	STDCALL_NTSUSPENDTHREAD StdCallKeResumeThread;

	STDCALL_NTPROTECTVIRTUALMEMORY StdCallNtProtectVirtualMemory;
	FASTCALL_NTPROTECTVIRTUALMEMORY FastCallNtProtectVirtualMemory;

	ULONG_PTR ulLoadLibrary;

	BOOLEAN bInitialize;
}INJECT_API_LIST,*PINJECT_API_LIST;

typedef struct _INJECT_PROCESS_INFORMATION
{
	ULONG ulPid;
	CHAR pInjectProcessName[1];
}INJECT_PROCESS_INFORMATION,*PINJECT_PROCESS_INFORMATION;


extern INJECT_API_LIST g_InjectAplList;
extern BOOLEAN g_bIsInjectKtrapFrame;

#endif

#define X86_INJECT_KTRAP_FRAME_LDR_TAG	0x11111111
#define X86_INJECT_KTRAP_FRAME_EIP_TAG	0x40404040
#define X86_INJECT_KTRAP_FRAME_PATH_TAG	0x80808080
#define X86_INJECT_KTRAP_FRAME_PARAMETERS 0x88888888

#define X64_INJECT_KTRAP_FRAME_LDR_TAG	0x1111111111111111
#define X64_INJECT_KTRAP_FRAME_EIP_TAG	0x4040404040404040
#define X64_INJECT_KTRAP_FRAME_PATH_TAG	0x8080808080808080
#define X64_INJECT_KTRAP_FRAME_PARAMETERS 0x8888888888888888

NTSTATUS IsWindows64Bits(PVOID pCurProcess);
PINJECT_OBJECT_INFORMATION FindInjectThread(PINJECT_PROCESS_INFORMATION pInjectProcessInfo);
NTSTATUS InjectKtrapFrame(PINJECT_PROCESS_INFORMATION pInjectProcessInfo,PCHAR pDllPath);
NTSTATUS x86ShellCodeInject(PINJECT_OBJECT_INFORMATION pInjectObjInfo,PCHAR pShellCode,ULONG ulSize);
NTSTATUS x86RiSingShellCodeInject(PINJECT_OBJECT_INFORMATION pInjectObjInfo,PCHAR pShellCode,ULONG ulSize);