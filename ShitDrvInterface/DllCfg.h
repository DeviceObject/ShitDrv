#ifndef __DLL_CFG_H__
#define __DLL_CFG_H__

#include <Windows.h>
#include "../ShitDrv/IoCtlCode.h"
#include <strsafe.h>
#include <Tlhelp32.h>
#include <fltUser.h>

#pragma comment(lib,"fltLib.lib")
#pragma comment(lib,"fltMgr.lib")

extern HANDLE g_hShitDrvDevObj;
extern HANDLE g_hWorkRecv;
extern HANDLE g_hServerPort;
extern HANDLE g_hEventWaitKrl;



#endif