#ifndef __DRV_FUNCTION_H__
#define __DRV_FUNCTION_H__

BOOLEAN SetCurrentProcess(HANDLE Pid);
BOOLEAN SetCurClientPath(WCHAR *wPath);
BOOLEAN SetSandboxPath(PWCHAR pSandboxPath);
BOOLEAN SetSandBoxProcName(PCHAR pImagName);
BOOLEAN GetFileMd5Sum(PWCHAR pFilePath,PUCHAR pOutMd5Sum);

#endif