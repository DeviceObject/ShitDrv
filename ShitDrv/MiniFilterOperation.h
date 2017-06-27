#ifndef __MINIFILTER_OPERATION_H__
#define __MINIFILTER_OPERATION_H__

FLT_PREOP_CALLBACK_STATUS FltPreCreateFiles(PFLT_CALLBACK_DATA Data, \
											PCFLT_RELATED_OBJECTS FltObjects, \
											PVOID *CompletionContext);

FLT_POSTOP_CALLBACK_STATUS FltPostCreateFiles(PFLT_CALLBACK_DATA Data, \
											  PCFLT_RELATED_OBJECTS FltObjects, \
											  PVOID CompletionContext, \
											  FLT_POST_OPERATION_FLAGS Flags);

FLT_PREOP_CALLBACK_STATUS FltPreSetFileformationFiles(PFLT_CALLBACK_DATA Data, \
													PCFLT_RELATED_OBJECTS FltObjects, \
													PVOID *CompletionContext);

FLT_POSTOP_CALLBACK_STATUS FltPostSetFileInformationFiles(PFLT_CALLBACK_DATA Data, \
													  PCFLT_RELATED_OBJECTS FltObjects, \
													  PVOID CompletionContext, \
													  FLT_POST_OPERATION_FLAGS Flags);

FLT_PREOP_CALLBACK_STATUS FltPreReadFiles(PFLT_CALLBACK_DATA Data, \
										  PCFLT_RELATED_OBJECTS FltObjects, \
										  PVOID *CompletionContext);

FLT_POSTOP_CALLBACK_STATUS FltPostReadFiles(PFLT_CALLBACK_DATA Data, \
											PCFLT_RELATED_OBJECTS FltObjects, \
											PVOID CompletionContext, \
											FLT_POST_OPERATION_FLAGS Flags);

FLT_PREOP_CALLBACK_STATUS FltPreWriteFiles(PFLT_CALLBACK_DATA Data, \
										   PCFLT_RELATED_OBJECTS FltObjects, \
										   PVOID *CompletionContext);

FLT_POSTOP_CALLBACK_STATUS FltPostWriteFiles(PFLT_CALLBACK_DATA Data, \
											 PCFLT_RELATED_OBJECTS FltObjects, \
											 PVOID CompletionContext, \
											 FLT_POST_OPERATION_FLAGS Flags);

FLT_PREOP_CALLBACK_STATUS FltPreClose(PFLT_CALLBACK_DATA Data, \
                                      PCFLT_RELATED_OBJECTS FltObjects, \
                                      PVOID *CompletionContext);

FLT_POSTOP_CALLBACK_STATUS FltPreClose(PFLT_CALLBACK_DATA Data, \
									   PCFLT_RELATED_OBJECTS FltObjects, \
									   PVOID *CompletionContext);

#endif