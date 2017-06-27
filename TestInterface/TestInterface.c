#include "TestInterfaceCfg.h"

static ULONG ulRecordCount = 0;

HANDLE GetPid(wchar_t *proc_name)
{
	HANDLE hSnapshot;
	PROCESSENTRY32W pe32;

	hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
	if (hSnapshot == INVALID_HANDLE_VALUE)
	{
		return NULL;
	}
	pe32.dwSize = sizeof(PROCESSENTRY32W);
	if (!Process32FirstW(hSnapshot,&pe32))
	{
		CloseHandle(hSnapshot);
		return NULL;
	}
	do 
	{
		if (_wcsnicmp(pe32.szExeFile,proc_name,wcslen(proc_name)) == 0)
		{
			CloseHandle(hSnapshot);
			return (HANDLE)pe32.th32ProcessID;
		}
	} while (Process32NextW(hSnapshot,&pe32));
	CloseHandle(hSnapshot);
	return NULL;
}
ULONG UnicodeToAnsi(PWCHAR pSrc,PCHAR pDst,ULONG ulSize)
{
	ULONG ulNeedSize;

	ulNeedSize = 0;
	if (ulSize)
	{
		ulNeedSize = WideCharToMultiByte(CP_OEMCP, \
			0, \
			pSrc, \
			-1, \
			pDst, \
			ulSize, \
			NULL, \
			FALSE);
	}
	else
	{
		ulNeedSize = WideCharToMultiByte(CP_OEMCP, \
			0, \
			pSrc, \
			-1, \
			NULL, \
			0, \
			NULL, \
			FALSE);
	}
	return ulNeedSize;
}
void DoRecord(PRECORD_INFORMATION pRecordInfo)
{
	PCHAR pShowInfo;
	ULONG ulNeedSize;

	pShowInfo = NULL;
	ulNeedSize = 0;

    printf("%d\n",ulRecordCount);
	if (pRecordInfo->wProcessFullPath)
	{
		ulNeedSize = UnicodeToAnsi(pRecordInfo->wProcessFullPath,pShowInfo,0);
		if (ulNeedSize)
		{
			pShowInfo = (PCHAR)malloc(ulNeedSize);
			RtlZeroMemory(pShowInfo,ulNeedSize);
		}
		ulNeedSize = UnicodeToAnsi(pRecordInfo->wProcessFullPath,pShowInfo,ulNeedSize);
		if (ulNeedSize)
		{
			printf("%s\n",pShowInfo);
		}
	}
	if (pShowInfo)
	{
		free(pShowInfo);
		pShowInfo = NULL;
		ulNeedSize = 0;
	}

	if (pRecordInfo->wOperation)
	{
		ulNeedSize = UnicodeToAnsi(pRecordInfo->wOperation,pShowInfo,0);
		if (ulNeedSize)
		{
			pShowInfo = (PCHAR)malloc(ulNeedSize);
			RtlZeroMemory(pShowInfo,ulNeedSize);
		}
		ulNeedSize = UnicodeToAnsi(pRecordInfo->wOperation,pShowInfo,ulNeedSize);
		if (ulNeedSize)
		{
			printf("%s\n",pShowInfo);
		}
	}
	if (pShowInfo)
	{
		free(pShowInfo);
		pShowInfo = NULL;
		ulNeedSize = 0;
	}
	if (pRecordInfo->wTargetPath)
	{
		ulNeedSize = UnicodeToAnsi(pRecordInfo->wTargetPath,pShowInfo,0);
		if (ulNeedSize)
		{
			pShowInfo = (PCHAR)malloc(ulNeedSize);
			RtlZeroMemory(pShowInfo,ulNeedSize);
		}
		ulNeedSize = UnicodeToAnsi(pRecordInfo->wTargetPath,pShowInfo,ulNeedSize);
		if (ulNeedSize)
		{
			printf("%s\n",pShowInfo);
		}
	}
	if (pShowInfo)
	{
		free(pShowInfo);
		pShowInfo = NULL;
		ulNeedSize = 0;
	}

	if (pRecordInfo->ulOperType == IsRenNameFile)
	{
		if (pRecordInfo->wBakPath)
		{
			ulNeedSize = UnicodeToAnsi(pRecordInfo->wBakPath,pShowInfo,0);
			if (ulNeedSize)
			{
				pShowInfo = (PCHAR)malloc(ulNeedSize);
				RtlZeroMemory(pShowInfo,ulNeedSize);
			}
			ulNeedSize = UnicodeToAnsi(pRecordInfo->wBakPath,pShowInfo,ulNeedSize);
			if (ulNeedSize)
			{
				printf("%s\n",pShowInfo);
			}
		}
		if (pShowInfo)
		{
			free(pShowInfo);
			pShowInfo = NULL;
			ulNeedSize = 0;
		}
	}
	printf("%s\n",pRecordInfo->Md5Sum);
	printf("=========================================================\n");
    ulRecordCount++;
}

int _cdecl main(int argc, CHAR* argv[])
{
	BOOLEAN bRet;
	FILTER_DAT_CONTROL FltDatCtl;
	WCHAR wCurPath[MAX_PATH];
	UCHAR Md5Sum[16];
	CHAR FormatMd5Sum[MAX_PATH];

	bRet = FALSE;
	RtlZeroMemory(&FltDatCtl,sizeof(FILTER_DAT_CONTROL));
	RtlZeroMemory(FormatMd5Sum,MAX_PATH);

	RtlZeroMemory(wCurPath,sizeof(WCHAR) * MAX_PATH);
	GetCurrentDirectoryW(MAX_PATH,wCurPath);
	bRet = SetCurClientPath(wCurPath);

	bRet = SetCurrentProcess((HANDLE)GetCurrentProcessId());

	bRet = SetSandBoxProcName("Trojans.exe");

	bRet = SetRecordCallback(DoRecord);
	FltDatCtl.ulIsStartFilter = 1;
	FltDatCtl.ulIsFilterRenName = 1;
	FltDatCtl.ulIsFilterDelete = 1;
	SetFltCtl(FltDatCtl);
	StartInterface();
	GetFileMd5Sum(L"\\??\\c:\\windows\\system32\\calc.exe",Md5Sum);
	sprintf_s(FormatMd5Sum,MAX_PATH,"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",Md5Sum[0], \
		Md5Sum[1],Md5Sum[2], Md5Sum[3],Md5Sum[4],Md5Sum[5],Md5Sum[6],Md5Sum[7],Md5Sum[8], \
		Md5Sum[9],Md5Sum[10],Md5Sum[11],Md5Sum[12],Md5Sum[13],Md5Sum[14],Md5Sum[15]);
	printf("\n\n\n");
	printf("%s",FormatMd5Sum);
	printf("\n\n\n");

	InjectKtrapFrame("calc.exe",(ULONG)GetPid(L"calc.exe"),"crypt32.dll");
    system("pause");
	return 0;
}

