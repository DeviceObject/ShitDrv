
#define OBJECT_TO_OBJECT_HEADER(o)\
	CONTAINING_RECORD((o),OBJECT_HEADER,Body)

typedef struct _OBJECT_TYPE_INITIALIZER
{
	USHORT Length;
	BOOLEAN UseDefaultObject;
	BOOLEAN CaseInsensitive;
	ULONG InvalidAttributes;
	GENERIC_MAPPING GenericMapping;
	ULONG ValidAccessMask;
	BOOLEAN SecurityRequired;
	BOOLEAN MaintainHandleCount;
	BOOLEAN MaintainTypeList;
	POOL_TYPE PoolType;
	ULONG DefaultPagedPoolCharge;
	ULONG DefaultNonPagedPoolCharge;
	PVOID DumpProcedure;
	PVOID OpenProcedure;
	PVOID CloseProcedure;
	PVOID DeleteProcedure;
	PVOID ParseProcedure;
	PVOID SecurityProcedure;
	PVOID QueryNameProcedure;
	PVOID OkayToCloseProcedure;
} OBJECT_TYPE_INITIALIZER,*POBJECT_TYPE_INITIALIZER;
typedef struct _OBJECT_TYPE
{ 
	ERESOURCE Mutex; 
	LIST_ENTRY TypeList; 
	UNICODE_STRING Name; 
	PVOID DefaultObject; 
	ULONG Index; 
	ULONG TotalNumberOfObjects; 
	ULONG TotalNumberOfHandles; 
	ULONG HighWaterNumberOfObjects; 
	ULONG HighWaterNumberOfHandles; 
	OBJECT_TYPE_INITIALIZER TypeInfo; 
#ifdef POOL_TAGGING 
	ULONG Key; 
#endif 
} OBJECT_TYPE,*POBJECT_TYPE;

typedef struct _OBJECT_CREATE_INFORMATION
{ 
	ULONG Attributes; 
	HANDLE RootDirectory; 
	PVOID ParseContext; 
	KPROCESSOR_MODE ProbeMode; 
	ULONG PagedPoolCharge; 
	ULONG NonPagedPoolCharge; 
	ULONG SecurityDescriptorCharge; 
	PSECURITY_DESCRIPTOR SecurityDescriptor; 
	PSECURITY_QUALITY_OF_SERVICE SecurityQos; 
	SECURITY_QUALITY_OF_SERVICE SecurityQualityOfService; 
} OBJECT_CREATE_INFORMATION,*POBJECT_CREATE_INFORMATION;

typedef struct _OBJECT_HEADER
{ 
	LONG PointerCount; 
	union { 
		LONG HandleCount; 
		PSINGLE_LIST_ENTRY SEntry; 
	}; 
	POBJECT_TYPE Type; 
	UCHAR NameInfoOffset; 
	UCHAR HandleInfoOffset; 
	UCHAR QuotaInfoOffset; 
	UCHAR Flags; 
	union 
	{ 
		POBJECT_CREATE_INFORMATION ObjectCreateInfo; 
		PVOID QuotaBlockCharged; 
	};

	PSECURITY_DESCRIPTOR SecurityDescriptor; 
	QUAD Body; 
} OBJECT_HEADER,*POBJECT_HEADER;