#ifndef __CONTEXT_OPERATION_H__
#define __CONTEXT_OPERATION_H__


#define SHITDRV_RESOURCE_TAG                      'cRdS'
#define SHITDRV_STRING_TAG                        'iSdS'
#define SHITDRV_STREAM_CONTEXT_TAG                'cSdS'
#define SHITDRV_STREAM_HANDLE_TAG				  'cHsS'

typedef struct _SHITDRV_STREAM_CONTEXT
{
    BOOLEAN bIsInit;
    BOOLEAN bIsPreModifyCopied;
    BOOLEAN bIsPostModifyCopied;
    CHAR PreModifyCopiedMd5Sum[100];
    CHAR PostModifyCopiedMd5Sum[100];
    UNICODE_STRING UniFileName;
	UNICODE_STRING UniVolumeName;
	ULONG ulCreateCount;
    ULONG ulOpenedCount;
    ULONG ulWriteCount;
    ULONG ulCleanupCount;
    ULONG ulCloseCount;
    HANDLE hProcessId;
    PERESOURCE pResource;
} SHITDRV_STREAM_CONTEXT,*PSHITDRV_STREAM_CONTEXT;

typedef struct _SHITDRV_STREAMHANDLE_CONTEXT
{
	ULONG ulType;
    UNICODE_STRING UniSrcFileName;
	UNICODE_STRING UniDstFileName;
    PERESOURCE pResource;
} SHITDRV_STREAMHANDLE_CONTEXT,*PSHITDRV_STREAMHANDLE_CONTEXT;

#define SHITDRV_STREAM_CONTEXT_SIZE                     sizeof(SHITDRV_STREAM_CONTEXT)
#define SHITDRV_STREAMHANDLE_CONTEXT_SIZE               sizeof(SHITDRV_STREAMHANDLE_CONTEXT)



PERESOURCE ShitDrvAllocateResource(VOID);
VOID ShitDrvFreeResource(PERESOURCE Resource);
NTSTATUS CreateStreamContext(PSHITDRV_STREAM_CONTEXT *pArgStreamContext);
NTSTATUS FindOrCreateStreamContext(PFLT_CALLBACK_DATA Cbd, \
                                   BOOLEAN CreateIfNotFound, \
                                   PSHITDRV_STREAM_CONTEXT *pArgStreamContext, \
                                   PBOOLEAN pContextCreated);

NTSTATUS ShitDrvAllocateUnicodeString(PUNICODE_STRING String);
VOID ShitDrvFreeUnicodeString(PUNICODE_STRING String);
VOID ShitDrvAcquireResourceExclusive(PERESOURCE pResource);
VOID ShitDrvAcquireResourceShared(PERESOURCE pResource);
VOID ShitDrvReleaseResource(PERESOURCE pResource);

VOID ShitDrvContextCleanup(PFLT_CONTEXT Context,FLT_CONTEXT_TYPE ContextType);
NTSTATUS UpdateNameInStreamContext(PUNICODE_STRING DirectoryName, \
								   PSHITDRV_STREAM_CONTEXT pArgStreamContext);

NTSTATUS UpdateNameInStreamHandleContext(PUNICODE_STRING UniSrcFileName, \
										 PUNICODE_STRING UniDstFileName, \
										 PSHITDRV_STREAMHANDLE_CONTEXT pArgStreamHandleContext);
NTSTATUS CreateStreamHandleContext(PSHITDRV_STREAMHANDLE_CONTEXT *pArgStreamHandleContext);
#endif