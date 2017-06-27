#ifndef __FILE_SYSTEM_CHECK_H__
#define __FILE_SYSTEM_CHECK_H__

BOOLEAN DrvCheckFileSystemIsOK();
VOID DrvReInitCallback(  //检查文件系统是否准备就绪的回调函数
                       IN PDRIVER_OBJECT DriverObject,
                       IN PVOID pContext,
                       IN ULONG Count
                       );
void CheckFileSystem(PDRIVER_OBJECT pDriverObject);
#endif