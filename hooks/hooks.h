#ifndef HOOK_H
#define HOOK_H

#include <windows.h>
#ifndef NDEBUG
#include <stdio.h>
#endif

#ifdef __GNUC__
#define __try
#define __except(x) if (0)
#endif

typedef struct HOOKLISTDATA
{
    char function_name[32];
    PROC new_function;
    PROC *function;
    DWORD flags;
} HOOKLISTDATA;
typedef struct HOOKLIST
{
    char module_name[32];
    HOOKLISTDATA data[16];
} HOOKLIST;

typedef BOOL(WINAPI *SETFILEATTRIBUTESAPROC)(LPCSTR lpFileName, DWORD dwFileAttributes);
typedef BOOL(WINAPI *SETFILEATTRIBUTESWPROC)(LPCWSTR lpFileName, DWORD dwFileAttributes);

extern SETFILEATTRIBUTESAPROC real_SetFileAttributesA;
extern SETFILEATTRIBUTESWPROC real_SetFileAttributesW;

BOOL WINAPI fake_SetFileAttributesA(LPCSTR lpFileName, DWORD dwFileAttributes);
BOOL WINAPI fake_SetFileAttributesW(LPCWSTR lpFileName, DWORD dwFileAttributes);

void hook_patch_iat_list(HMODULE hmod, BOOL unhook, HOOKLIST *hooks);

int IAT_InstallHook();
int IAT_UnInstallHook();

void hook_init();
void hook_exit();

#endif