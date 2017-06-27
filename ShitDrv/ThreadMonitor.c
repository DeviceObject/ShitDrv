#include "ShitDrv.h"
#include "ThreadMonitor.h"

VOID ThreadMonitor(HANDLE ProcessId,HANDLE ThreadId,BOOLEAN bCreate)
{
	if (bCreate)
	{

	}
	else
	{

	}
}

NTSTATUS RegisterThreadMonitor(ULONG_PTR *ulMonitorRoutine,BOOLEAN bIsUnload)
{
	NTSTATUS Status;

	Status = STATUS_UNSUCCESSFUL;

	do 
	{
		if (bIsUnload)
		{
			Status = PsRemoveCreateThreadNotifyRoutine(ThreadMonitor);
			break;
		}
		else
		{
			Status = PsSetCreateThreadNotifyRoutine(ThreadMonitor);
			break;
		}
	} while (0);
	return Status;
}