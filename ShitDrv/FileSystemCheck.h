#ifndef __FILE_SYSTEM_CHECK_H__
#define __FILE_SYSTEM_CHECK_H__

BOOLEAN DrvCheckFileSystemIsOK();
VOID DrvReInitCallback(  //����ļ�ϵͳ�Ƿ�׼�������Ļص�����
                       IN PDRIVER_OBJECT DriverObject,
                       IN PVOID pContext,
                       IN ULONG Count
                       );
void CheckFileSystem(PDRIVER_OBJECT pDriverObject);
#endif