#ifndef __IO_CTL_CODE_H__
#define __IO_CTL_CODE_H__


#define MAX_PATH    260
#define SHITDRV_BASENAME			L"ShitDrv"
#define SHITDRV_DEVNAME				L"\\Device\\" SHITDRV_BASENAME
#define SHITDRV_LINKNAME			L"\\DosDevices\\" SHITDRV_BASENAME
#define SHITDRV_USERLINK			L"\\\\.\\" SHITDRV_BASENAME

#define SERVER_PORT					L"\\ShitDrvFltPort"
#define EVENT_NAME_WAIT_KERNEL      L"\\BaseNamedObjects\\ShitDrvWaitKernel"


#define IOCTL(n) CTL_CODE(FILE_DEVICE_UNKNOWN, n, METHOD_BUFFERED, 0)

#define IOC(ops) IOCTL_ ## ops
#define IOP(ops) struct ops ## _params
#define IOR(ops) struct ops ## _returns
#define IOS(ops) struct ops ## _shared

enum
{
	IOC(SetCurrentProcess) = IOCTL('SPCS')
};

IOP(SetCurrentProcess)
{
	HANDLE ProcessId;
};

enum
{
    IOC(SetCurClientPath) = IOCTL('PCCS')
};

IOP(SetCurClientPath)
{
    WCHAR wCurClientPath[MAX_PATH];
};
enum
{
	IOC(SetSandBoxProcName) = IOCTL('NPSS')
};
IOP(SetSandBoxProcName)
{
	CHAR ImageName[MAX_PATH];
};
enum
{
	IOC(GetFileMd5Sum) = IOCTL('SMFG')
};
IOP(GetFileMd5Sum)
{
	WCHAR wImageName[MAX_PATH];
	UCHAR Md5Sum[16];
};
enum
{
	IOC(InjectKtrapFrame) = IOCTL('IjkF')
};

IOP(InjectKtrapFrame)
{
	ULONG ulPid;
	char pInjectProcessName[260];
	char pInjectDllPath[260];
};
#endif