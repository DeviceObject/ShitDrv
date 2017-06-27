#include "ShitDrv.h"
#include "FileSystemCheck.h"

BOOLEAN DrvCheckFileSystemIsOK()
{
    HANDLE              FileHandle          = 0;
    OBJECT_ATTRIBUTES   ObjectAttributes    = {0};
    UNICODE_STRING      FileName            = {0};
    IO_STATUS_BLOCK     IoStatus            = {0};
    NTSTATUS            Status              = STATUS_SUCCESS;
    BOOLEAN				bRet                = FALSE;

    RtlInitUnicodeString(&FileName, L"\\Device\\HarddiskVolume1");
    InitializeObjectAttributes(&ObjectAttributes,  &FileName, OBJ_CASE_INSENSITIVE|OBJ_KERNEL_HANDLE, NULL, NULL );
    Status = ZwCreateFile(
        &FileHandle, 
        (SYNCHRONIZE | FILE_READ_ATTRIBUTES),
        &ObjectAttributes,
        &IoStatus, 
        NULL, 
        FILE_ATTRIBUTE_NORMAL,
        FILE_SHARE_READ, 
        FILE_OPEN, 
        FILE_DIRECTORY_FILE|FILE_SYNCHRONOUS_IO_NONALERT, 
        NULL, 
        0);

    if(NT_SUCCESS(Status))
    {		
        ZwClose(FileHandle);
        bRet = TRUE;
    }
    return bRet;
}
VOID DrvReInitCallback(  //检查文件系统是否准备就绪的回调函数
                       IN PDRIVER_OBJECT DriverObject,
                       IN PVOID pContext,
                       IN ULONG Count)
{
    if (!DrvCheckFileSystemIsOK())
    {
        //注册一个重新初始化
        IoRegisterDriverReinitialization(DriverObject, DrvReInitCallback, pContext);
    }
    else
    {
        if (g_pFilterCtl->FileFltAccessCtl.ulIsFileSystemInit == 0)
        {
            g_pFilterCtl->FileFltAccessCtl.ulIsFileSystemInit = 1;
        }
    }
}
void CheckFileSystem(PDRIVER_OBJECT pDriverObject)
{
    if (!DrvCheckFileSystemIsOK())
    {
        IoRegisterDriverReinitialization(pDriverObject,DrvReInitCallback,NULL);
    }
    else
    {
        if (g_pFilterCtl->FileFltAccessCtl.ulIsFileSystemInit == 0)
        {
            g_pFilterCtl->FileFltAccessCtl.ulIsFileSystemInit = 1;
        }
    }
}