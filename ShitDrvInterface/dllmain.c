// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "DllCfg.h"
#include "Record.h"
#include "Callback.h"

HANDLE g_hServerPort = NULL;
HANDLE g_hEventWaitKrl = NULL;

HANDLE g_hShitDrvDevObj = NULL;
HANDLE g_hWorkRecv = NULL;

BOOLEAN OpenDevice(PWCHAR pTargetDevName)
{
	BOOLEAN bStatus;

	do 
	{
		bStatus = TRUE;
		if (pTargetDevName == NULL)
		{
			bStatus = FALSE;
			break;
		}
		g_hShitDrvDevObj = CreateFile(pTargetDevName, \
			GENERIC_ALL, \
			FILE_SHARE_READ | FILE_SHARE_WRITE, \
			NULL, \
			OPEN_EXISTING, \
			FILE_ATTRIBUTE_DEVICE, \
			NULL);
		if (g_hShitDrvDevObj == NULL || \
			g_hShitDrvDevObj == INVALID_HANDLE_VALUE)
		{
			bStatus = FALSE;
			break;
		}
	} while (0);
	return bStatus;
}

BOOLEAN CloseDevice(HANDLE hDevice)
{
	if (hDevice)
	{
		CloseHandle(hDevice);
		hDevice = NULL;
		return TRUE;
	}
	return FALSE;
}
//void InitializeDrvInterface()
//{
//	
//	FILTER_DAT_CONTROL FltDatCtl;
//
//	RtlZeroMemory(&FltDatCtl,sizeof(FILTER_DAT_CONTROL));
//
//
//	FltDatCtl.ulIsStartFilter = 1;
//	SetFltCtl(FltDatCtl);
//	
//}
HANDLE StartInterface()
{
	return CreateThread(NULL,0,DoRecvWork,NULL,0,NULL);
}
BOOL APIENTRY DllMain(HMODULE hModule,DWORD ul_reason_for_call,LPVOID lpReserved)
{
	BOOLEAN bRet;
	HRESULT hResult;

	bRet = FALSE;

	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		{
			bRet = OpenDevice(SHITDRV_USERLINK);
			if (bRet)
			{
				g_hEventWaitKrl = OpenEvent(SYNCHRONIZE,FALSE,EVENT_NAME_WAIT_KERNEL);
				hResult = FilterConnectCommunicationPort(SERVER_PORT, \
					0, \
					NULL, \
					0, \
					NULL, \
					&g_hServerPort);
				if (S_OK != hResult)
				{
					return -1;
				}
				InitializeRecordCallback();
			}
		}
		break;
	case DLL_PROCESS_DETACH:
		{
			if (g_hServerPort)
			{
				CloseHandle(g_hServerPort);
				g_hServerPort = NULL;
			}
			bRet = CloseDevice(g_hShitDrvDevObj);
		}
		break;
	}
	return bRet;
}

