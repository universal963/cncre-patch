#include "hooks.h"

void hook_init()
{
    if (IAT_InstallHook())
    {
        MessageBoxA(NULL, "Error: kernel32.dll functions hook failed!\nWhat happened?", "Error", MB_ICONERROR);
    }
}

void hook_exit()
{
    IAT_UnInstallHook();
}