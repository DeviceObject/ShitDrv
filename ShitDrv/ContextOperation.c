#include "ShitDrv.h"
#include "ContextOperation.h"


VOID ShitDrvContextCleanup(PFLT_CONTEXT Context,FLT_CONTEXT_TYPE ContextType)
{
	PSHITDRV_STREAM_CONTEXT pStreamContext;
	PSHITDRV_STREAMHANDLE_CONTEXT pStreamHandleContext;

    PAGED_CODE();

    switch(ContextType)
    {
    case FLT_STREAM_CONTEXT:

        pStreamContext = (PSHITDRV_STREAM_CONTEXT)Context;

        if (pStreamContext->pResource != NULL)
        {
            ExDeleteResourceLite(pStreamContext->pResource);
            ShitDrvFreeResource(pStreamContext->pResource);
        }
        if (pStreamContext->UniFileName.Buffer != NULL)
        {
            ShitDrvFreeUnicodeString(&pStreamContext->UniFileName);
        }
        break;
	case FLT_STREAMHANDLE_CONTEXT:

		pStreamHandleContext = (PSHITDRV_STREAMHANDLE_CONTEXT)Context;

		if (pStreamHandleContext->pResource != NULL)
		{
			ExDeleteResourceLite(pStreamHandleContext->pResource);
			ShitDrvFreeResource(pStreamHandleContext->pResource);
		}

		if (pStreamHandleContext->UniSrcFileName.Buffer != NULL)
		{
			ShitDrvFreeUnicodeString(&pStreamHandleContext->UniSrcFileName);
		}
		if (pStreamHandleContext->UniDstFileName.Buffer != NULL)
		{
			ShitDrvFreeUnicodeString(&pStreamHandleContext->UniDstFileName);
		}
		break;
    default:
        break;
    }
}
NTSTATUS UpdateNameInStreamHandleContext(PUNICODE_STRING UniSrcFileName, \
										 PUNICODE_STRING UniDstFileName, \
                                         PSHITDRV_STREAMHANDLE_CONTEXT pArgStreamHandleContext)
{
    NTSTATUS Status;

    PAGED_CODE();
 
    if (pArgStreamHandleContext->UniSrcFileName.Buffer != NULL)
    {
        ShitDrvFreeUnicodeString(&pArgStreamHandleContext->UniSrcFileName);

		pArgStreamHandleContext->UniSrcFileName.MaximumLength = UniSrcFileName->Length;
		Status = ShitDrvAllocateUnicodeString(&pArgStreamHandleContext->UniSrcFileName);
		if (NT_SUCCESS(Status))
		{
			RtlCopyUnicodeString(&pArgStreamHandleContext->UniSrcFileName,UniSrcFileName);
		}
    }
	if (pArgStreamHandleContext->UniDstFileName.Buffer != NULL)
	{
		ShitDrvFreeUnicodeString(&pArgStreamHandleContext->UniDstFileName);

		pArgStreamHandleContext->UniDstFileName.MaximumLength = UniDstFileName->Length;
		Status = ShitDrvAllocateUnicodeString(&pArgStreamHandleContext->UniDstFileName);
		if (NT_SUCCESS(Status))
		{
			RtlCopyUnicodeString(&pArgStreamHandleContext->UniDstFileName,UniDstFileName);
		}
	}
    return Status;
}

//NTSTATUS CreateStreamHandleContext(PSHITDRV_STREAMHANDLE_CONTEXT *pArgStreamHandleContext)
///*++
//Routine Description:
//    This routine creates a new stream context
//Arguments:
//    StreamContext         - Returns the stream context
//Return Value:
//    Status
//--*/
//{
//    NTSTATUS Status;
//    PSHITDRV_STREAMHANDLE_CONTEXT pStreamHandleContext;
//
//    PAGED_CODE();
//    //
//    //  Allocate a stream context
//    //
//    Status = FltAllocateContext(g_pFilterCtl->pFilterHandle,
//                                FLT_STREAMHANDLE_CONTEXT,
//                                SHITDRV_STREAMHANDLE_CONTEXT_SIZE,
//                                PagedPool,
//                                &pStreamHandleContext);
//
//    if (!NT_SUCCESS(Status))
//    {
//        return Status;
//    }
//    //
//    //  Initialize the newly created context
//    //
//    RtlZeroMemory(pStreamHandleContext,SHITDRV_STREAMHANDLE_CONTEXT_SIZE);
//    pStreamHandleContext->pResource = ShitDrvAllocateResource();
//    if(pStreamHandleContext->pResource == NULL)
//    {
//        FltReleaseContext(pStreamHandleContext);
//        return STATUS_INSUFFICIENT_RESOURCES;
//    }
//    ExInitializeResourceLite(pStreamHandleContext->pResource);
//    *pArgStreamHandleContext = pStreamHandleContext;
//
//    return STATUS_SUCCESS;
//}
NTSTATUS CreateOrReplaceStreamHandleContext(PFLT_CALLBACK_DATA Cbd, \
                                            BOOLEAN ReplaceIfExists, \
                                            PSHITDRV_STREAMHANDLE_CONTEXT *pArgStreamHandleContext, \
                                            PBOOLEAN pContextReplaced)
/*++
Routine Description:
    This routine creates a stream handle context for the target stream
    handle. Optionally, if the context already exists, this routine
    replaces it with the new context and releases the old context
Arguments:
    Cbd                   - Supplies a pointer to the callbackData which
                            declares the requested operation.
    ReplaceIfExists       - Supplies if the stream handle context must be
                            replaced if already present
    StreamContext         - Returns the stream context
    ContextReplaced       - Returns if an existing context was replaced
Return Value:
    Status
--*/
{
    NTSTATUS Status;
    PSHITDRV_STREAMHANDLE_CONTEXT pStreamHandleContext;
    PSHITDRV_STREAMHANDLE_CONTEXT pOldStreamHandleContext;

    PAGED_CODE();

    *pArgStreamHandleContext = NULL;
    if (pContextReplaced != NULL)
    {
        *pContextReplaced = FALSE;
    }
    //
    //  Create a stream context
    //
    Status = CreateStreamHandleContext(&pStreamHandleContext);
    if (!NT_SUCCESS(Status))
    {
        return Status;
    }
    //
    //  Set the new context we just allocated on the file object
    //
    Status = FltSetStreamHandleContext(Cbd->Iopb->TargetInstance,
                                       Cbd->Iopb->TargetFileObject,
                                       ReplaceIfExists ? FLT_SET_CONTEXT_REPLACE_IF_EXISTS : FLT_SET_CONTEXT_KEEP_IF_EXISTS,
                                       pStreamHandleContext,
                                       &pOldStreamHandleContext);

    if (!NT_SUCCESS(Status))
    {
        //
        //  We release the context here because FltSetStreamContext failed
        //
        //  If FltSetStreamContext succeeded then the context will be returned
        //  to the caller. The caller will use the context and then release it
        //  when he is done with the context.
        //
        FltReleaseContext(pStreamHandleContext);
        if (Status != STATUS_FLT_CONTEXT_ALREADY_DEFINED)
        {
            return Status;
        }
        //
        //  We will reach here only if we have failed with STATUS_FLT_CONTEXT_ALREADY_DEFINED
        //  and we can fail with that code only if the context already exists and we have used
        //  the FLT_SET_CONTEXT_KEEP_IF_EXISTS flag

        ASSERT(ReplaceIfExists  == FALSE);
        //
        //  Race condition. Someone has set a context after we queried it.
        //  Use the already set context instead
        //
        //
        //  Return the existing context. Note that the new context that we allocated has already been
        //  realeased above.
        //
        pStreamHandleContext = pOldStreamHandleContext;
        Status = STATUS_SUCCESS;
    }
    else
    {
        //
        //  FltSetStreamContext has suceeded. The new context will be returned
        //  to the caller. The caller will use the context and then release it
        //  when he is done with the context.
        //
        //  However, if we have replaced an existing context then we need to
        //  release the old context so as to decrement the ref count on it.
        //
        //  Note that the memory allocated to the objects within the context
        //  will be freed in the context cleanup and must not be done here.
        //
        if (ReplaceIfExists && pOldStreamHandleContext != NULL)
        {
            FltReleaseContext(pOldStreamHandleContext);
            if (pContextReplaced != NULL)
            {
                *pContextReplaced = TRUE;
            }
        }
    }
    *pArgStreamHandleContext = pStreamHandleContext;
    return Status;
}
NTSTATUS UpdateNameInStreamContext(PUNICODE_STRING DirectoryName, \
								   PSHITDRV_STREAM_CONTEXT pArgStreamContext)
/*++
Routine Description:
    This routine updates the name of the target in the supplied stream context
Arguments:
    DirectoryName         - Supplies the directory name
    StreamContext    - Returns the updated name in the stream context
Return Value:
    Status
Note:
    The caller must synchronize access to the context. This routine does no
    synchronization
--*/
{
    NTSTATUS Status;

    PAGED_CODE();
    //
    //  Free any existing name
    //
    if (pArgStreamContext->UniFileName.Buffer != NULL)
    {
        ShitDrvFreeUnicodeString(&pArgStreamContext->UniFileName);
    }
    //
    //  Allocate and copy off the directory name
    //
    pArgStreamContext->UniFileName.MaximumLength = DirectoryName->Length;
    Status = ShitDrvAllocateUnicodeString(&pArgStreamContext->UniFileName);
    if (NT_SUCCESS(Status))
    {
        RtlCopyUnicodeString(&pArgStreamContext->UniFileName,DirectoryName);
    }
    return Status;
}
NTSTATUS ShitDrvAllocateUnicodeString(PUNICODE_STRING String)
/*++
Routine Description:
    This routine allocates a unicode string
Arguments:
    String - supplies the size of the string to be allocated in the MaximumLength field
             return the unicode string
Return Value:
    STATUS_SUCCESS                  - success
    STATUS_INSUFFICIENT_RESOURCES   - failure
--*/
{
    PAGED_CODE();

    String->Buffer = ExAllocatePoolWithTag(PagedPool,String->MaximumLength,SHITDRV_STRING_TAG);
    if (String->Buffer == NULL)
    {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    String->Length = 0;
    return STATUS_SUCCESS;
}

VOID ShitDrvFreeUnicodeString(PUNICODE_STRING String)
/*++
Routine Description:
    This routine frees a unicode string
Arguments:
    String - supplies the string to be freed
Return Value:
    None
--*/
{
    PAGED_CODE();
    ExFreePoolWithTag(String->Buffer,SHITDRV_STRING_TAG);
    String->Length = String->MaximumLength = 0;
    String->Buffer = NULL;
}
VOID ShitDrvAcquireResourceExclusive(PERESOURCE pResource)
{
    ASSERT(KeGetCurrentIrql() <= APC_LEVEL);
    ASSERT(ExIsResourceAcquiredExclusiveLite(pResource) || \
        !ExIsResourceAcquiredSharedLite(pResource));

    KeEnterCriticalRegion();

    (VOID)ExAcquireResourceExclusiveLite(pResource,TRUE);
}
VOID ShitDrvAcquireResourceShared(PERESOURCE pResource)
{
    ASSERT(KeGetCurrentIrql() <= APC_LEVEL);

    KeEnterCriticalRegion();

    (VOID)ExAcquireResourceSharedLite(pResource,TRUE);
}
VOID ShitDrvReleaseResource(PERESOURCE pResource)
{
    ASSERT(KeGetCurrentIrql() <= APC_LEVEL);
    ASSERT(ExIsResourceAcquiredExclusiveLite(pResource) ||
        ExIsResourceAcquiredSharedLite(pResource));

    ExReleaseResourceLite(pResource);

    KeLeaveCriticalRegion();
}
PERESOURCE ShitDrvAllocateResource(VOID)
{
    return ExAllocatePoolWithTag(NonPagedPool,sizeof(ERESOURCE),SHITDRV_RESOURCE_TAG);
}
VOID ShitDrvFreeResource(PERESOURCE Resource)
{
    ExFreePoolWithTag(Resource,SHITDRV_RESOURCE_TAG);
}
NTSTATUS CreateStreamContext(PSHITDRV_STREAM_CONTEXT *pArgStreamContext)
/*++
Routine Description:
    This routine creates a new stream context
Arguments:
    StreamContext         - Returns the stream context
Return Value:
    Status
--*/
{
    NTSTATUS Status;
    PSHITDRV_STREAM_CONTEXT pStreamContext;

    PAGED_CODE();
    //
    //  Allocate a stream context
    //
    Status = FltAllocateContext(g_pFilterCtl->pFilterHandle,
                                FLT_STREAM_CONTEXT,
                                SHITDRV_STREAM_CONTEXT_SIZE,
                                PagedPool,
                                &pStreamContext);
    if (!NT_SUCCESS(Status))
    {
        return Status;
    }
    //
    //  Initialize the newly created context
    //
    RtlZeroMemory(pStreamContext,SHITDRV_STREAM_CONTEXT_SIZE);
    pStreamContext->pResource = ShitDrvAllocateResource();
    if(pStreamContext->pResource == NULL)
    {

        FltReleaseContext(pStreamContext);
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    ExInitializeResourceLite(pStreamContext->pResource);
    pStreamContext->bIsInit = TRUE;
    *pArgStreamContext = pStreamContext;
    return STATUS_SUCCESS;
}
NTSTATUS FindOrCreateStreamContext(PFLT_CALLBACK_DATA Cbd, \
                                   BOOLEAN CreateIfNotFound, \
                                   PSHITDRV_STREAM_CONTEXT *pArgStreamContext, \
                                   PBOOLEAN pContextCreated)
/*++

Routine Description:

    This routine finds the stream context for the target stream.
    Optionally, if the context does not exist this routing creates
    a new one and attaches the context to the stream.

Arguments:
    Cbd                   - Supplies a pointer to the callbackData which
                            declares the requested operation.
    CreateIfNotFound      - Supplies if the stream must be created if missing
    StreamContext         - Returns the stream context
    ContextCreated        - Returns if a new context was created

Return Value:

    Status

--*/
{
    NTSTATUS Status;
    PSHITDRV_STREAM_CONTEXT pStreamContext;
    PSHITDRV_STREAM_CONTEXT pOldStreamContext;

    PAGED_CODE();

    *pArgStreamContext = NULL;
    if (pContextCreated != NULL)
    {
        *pContextCreated = FALSE;
    }
    Status = FltGetStreamContext(Cbd->Iopb->TargetInstance,
                                 Cbd->Iopb->TargetFileObject,
                                 &pStreamContext);
    //
    //  If the call failed because the context does not exist
    //  and the user wants to creat a new one, the create a
    //  new context
    //
    if (!NT_SUCCESS(Status) &&
        (Status == STATUS_NOT_FOUND) &&
        CreateIfNotFound)
    {
        //
        //  Create a stream context
        //
        Status = CreateStreamContext(&pStreamContext);
        if (!NT_SUCCESS(Status))
        {
            return Status;
        }
        //
        //  Set the new context we just allocated on the file object
        //
        Status = FltSetStreamContext( Cbd->Iopb->TargetInstance,
                                      Cbd->Iopb->TargetFileObject,
                                      FLT_SET_CONTEXT_KEEP_IF_EXISTS,
                                      pStreamContext,
                                      &pOldStreamContext);
        if (!NT_SUCCESS(Status))
        {
            //  We release the context here because FltSetStreamContext failed
            //  If FltSetStreamContext succeeded then the context will be returned
            //  to the caller. The caller will use the context and then release it
            //  when he is done with the context.
            FltReleaseContext(pStreamContext);
            if (Status != STATUS_FLT_CONTEXT_ALREADY_DEFINED)
            {
                //
                //  FltSetStreamContext failed for a reason other than the context already
                //  existing on the stream. So the object now does not have any context set
                //  on it. So we return failure to the caller.
                //
                return Status;
            }
            //  Race condition. Someone has set a context after we queried it.
            //  Use the already set context instead
            //  Return the existing context. Note that the new context that we allocated has already been
            //  realeased above.
            pStreamContext = pOldStreamContext;
            Status = STATUS_SUCCESS;
        }
        else
        {
            if (pContextCreated != NULL)
            {
                *pContextCreated = TRUE;
            }
        }
    }
    *pArgStreamContext = pStreamContext;
    return Status;
}
NTSTATUS CreateStreamHandleContext(PSHITDRV_STREAMHANDLE_CONTEXT *pArgStreamHandleContext)
{
    NTSTATUS Status;
    PSHITDRV_STREAMHANDLE_CONTEXT pStreamHandleContext;

    PAGED_CODE();

    Status = FltAllocateContext(g_pFilterCtl->pFilterHandle,
                                FLT_STREAMHANDLE_CONTEXT,
                                SHITDRV_STREAMHANDLE_CONTEXT_SIZE,
                                PagedPool,
                                &pStreamHandleContext);
    if (!NT_SUCCESS(Status))
    {
        return Status;
    }
    RtlZeroMemory(pStreamHandleContext,SHITDRV_STREAMHANDLE_CONTEXT_SIZE);
    pStreamHandleContext->pResource = ShitDrvAllocateResource();
    if(pStreamHandleContext->pResource == NULL)
    {

        FltReleaseContext(pStreamHandleContext);
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    ExInitializeResourceLite(pStreamHandleContext->pResource);
    *pArgStreamHandleContext = pStreamHandleContext;
    return STATUS_SUCCESS;
}
