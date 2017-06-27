#include "InjectShell.h"

BOOL APIENTRY DllMain(HMODULE hModule,DWORD ul_reason_for_call,LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		MessageBox(NULL,L"InjectShell",L"Inject",MB_OK);
		break;
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}
