#include "hooks/hooks.h"

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
    {
        hook_init();
        break;
    }
    case DLL_PROCESS_DETACH:
    {
        hook_exit();
        break;
    }
    }
    return TRUE;
}