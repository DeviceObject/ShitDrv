#ifndef __TEST_INTERFACE_CFG_H__
#define __TEST_INTERFACE_CFG_H__
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <Windows.h>
#include <strsafe.h>
#include <fltUser.h>
#include <Tlhelp32.h>
#include "../ShitDrvInterface/Record.h"
#include "../ShitDrv/IoCtlCode.h"
#include "../ShitDrvInterface/Callback.h"
#include "global.h"
#include "md5.h"

#pragma comment(lib,"../../bin/ShitDrvInterface.lib")
#pragma comment(lib,"fltLib.lib")
#pragma comment(lib,"fltMgr.lib")


ULONG UnicodeToAnsi(PWCHAR pSrc,PCHAR pDst,ULONG ulSize);

#endif