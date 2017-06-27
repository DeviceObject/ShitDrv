
// Protection flags for memory pages (Executable, Readable, Writeable)
static int ProtectionFlags[2][2][2] = {
	{
		// not executable
		{PAGE_NOACCESS, PAGE_WRITECOPY},
		{PAGE_READONLY, PAGE_READWRITE},
	}, {
		// executable
		{PAGE_EXECUTE, PAGE_EXECUTE_WRITECOPY},
		{PAGE_EXECUTE_READ, PAGE_EXECUTE_READWRITE},
	},
};

// 因为要远程线程注入只能获取远程线程内部的函数地址
typedef struct _RemoteParam {
	int ProtectionFlags[2][2][2];
	WCHAR wzFileName[256];			//要注入的DLL名称
}RemoteParam, *PRemoteParam;




extern PRemoteParam g_pLibRemoteR0;
extern PVOID g_pLoadFuncR0;
extern ULONG g_ulRiSingShellCodeLength;

NTSTATUS NTAPI InitRiSingShellCode(PCHAR InjectPath);

