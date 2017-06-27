#ifndef __PROCESS_NOTIFY_H__
#define __PROCESS_NOTIFY_H__

BOOLEAN IsFltParentProcess(HANDLE hParentId);
VOID ProcessNotify(HANDLE ParentId,HANDLE ProcessId,BOOLEAN bCreate);
BOOLEAN IsSandboxProcess(HANDLE hProcessId);
void ProcessParentProcessId(HANDLE hProcessId);
void ProcessChildProcessId(HANDLE hProcessId);

#endif
