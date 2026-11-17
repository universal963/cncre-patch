#include "hooks.h"

SETFILEATTRIBUTESAPROC real_SetFileAttributesA = NULL;
SETFILEATTRIBUTESWPROC real_SetFileAttributesW = NULL;

static HOOKLIST g_hooks[] =
    {
        {"kernel32.dll",
         {{"SetFileAttributesA", (PROC)fake_SetFileAttributesA, (PROC *)&real_SetFileAttributesA, 0},
          {"SetFileAttributesW", (PROC)fake_SetFileAttributesW, (PROC *)&real_SetFileAttributesW, 0},
          {"", NULL, NULL, 0}}},
        {"",
         {{"", NULL, NULL, 0}}}};

BOOL WINAPI fake_SetFileAttributesA(LPCSTR lpFileName, DWORD dwFileAttributes)
{
    if (dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY || dwFileAttributes == 0)
    {
        return TRUE;
    }
    return real_SetFileAttributesA(lpFileName, dwFileAttributes);
}

BOOL WINAPI fake_SetFileAttributesW(LPCWSTR lpFileName, DWORD dwFileAttributes)
{
    if (dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY || dwFileAttributes == 0)
    {
        return TRUE;
    }
    return real_SetFileAttributesW(lpFileName, dwFileAttributes);
}

static BOOL util_is_bad_read_ptr(void *p)
{
    MEMORY_BASIC_INFORMATION mbi = {0};
    if (VirtualQuery(p, &mbi, sizeof(mbi)))
    {
        DWORD mask = (PAGE_READONLY |
                      PAGE_READWRITE |
                      PAGE_WRITECOPY |
                      PAGE_EXECUTE_READ |
                      PAGE_EXECUTE_READWRITE |
                      PAGE_EXECUTE_WRITECOPY);

        BOOL b = !(mbi.Protect & mask);

        if (mbi.Protect & (PAGE_GUARD | PAGE_NOACCESS))
            b = TRUE;

        return b;
    }

    return TRUE;
}

void hook_patch_iat_list(HMODULE hmod, BOOL unhook, HOOKLIST *hooks)
{
    if (!hmod || hmod == INVALID_HANDLE_VALUE || !hooks)
        return;

    __try
    {
        PIMAGE_DOS_HEADER dos_header = (PIMAGE_DOS_HEADER)hmod;
        if (dos_header->e_magic != IMAGE_DOS_SIGNATURE)
            return;

        PIMAGE_NT_HEADERS nt_headers = (PIMAGE_NT_HEADERS)((size_t)dos_header + (size_t)dos_header->e_lfanew);
        if (nt_headers->Signature != IMAGE_NT_SIGNATURE)
            return;

        DWORD import_desc_rva = nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
        DWORD import_desc_size = nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size;

        if (import_desc_rva == 0 || import_desc_size == 0)
            return;

        PIMAGE_IMPORT_DESCRIPTOR import_desc = (PIMAGE_IMPORT_DESCRIPTOR)((size_t)dos_header + import_desc_rva);

        while (import_desc->FirstThunk)
        {
            if (!import_desc->OriginalFirstThunk || !import_desc->Name)
            {
                import_desc++;
                continue;
            }

            for (int i = 0; hooks[i].module_name[0]; i++)
            {
                char *imp_module_name = (char *)((size_t)dos_header + import_desc->Name);

                if (_stricmp(imp_module_name, hooks[i].module_name) == 0)
                {
                    PIMAGE_THUNK_DATA first_thunk = (void *)((size_t)dos_header + import_desc->FirstThunk);
                    PIMAGE_THUNK_DATA o_first_thunk = (void *)((size_t)dos_header + import_desc->OriginalFirstThunk);

                    while (first_thunk->u1.Function)
                    {
                        if (!o_first_thunk->u1.AddressOfData)
                        {
                            first_thunk++;
                            o_first_thunk++;
                            continue;
                        }

                        PIMAGE_IMPORT_BY_NAME import = (void *)((size_t)dos_header + o_first_thunk->u1.AddressOfData);

                        if ((o_first_thunk->u1.Ordinal & IMAGE_ORDINAL_FLAG) == 0)
                        {
                            for (int x = 0; hooks[i].data[x].function_name[0]; x++)
                            {
                                if (!unhook && !hooks[i].data[x].new_function)
                                    continue;

#if defined(__GNUC__)
                                if (util_is_bad_read_ptr((void *)import->Name))
                                    continue;
#endif

                                if (strcmp((const char *)import->Name, hooks[i].data[x].function_name) == 0)
                                {
                                    DWORD op;

                                    if (VirtualProtect(
                                            &first_thunk->u1.Function,
                                            sizeof(size_t),
                                            PAGE_READWRITE,
                                            &op))
                                    {
                                        if (unhook)
                                        {
                                            size_t org =
                                                (size_t)GetProcAddress(
                                                    GetModuleHandleA(hooks[i].module_name),
                                                    hooks[i].data[x].function_name);

                                            if (org && first_thunk->u1.Function != org)
                                            {
                                                first_thunk->u1.Function = org;
                                            }
                                        }
                                        else
                                        {
                                            if (first_thunk->u1.Function != (size_t)hooks[i].data[x].new_function)
                                                first_thunk->u1.Function = (size_t)hooks[i].data[x].new_function;
                                        }

                                        VirtualProtect(&first_thunk->u1.Function, sizeof(size_t), op, &op);
                                    }

                                    break;
                                }
                            }
                        }

                        first_thunk++;
                        o_first_thunk++;
                    }

                    break;
                }
            }

            import_desc++;
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
    }
}

int IAT_InstallHook()
{
    HMODULE kernel32_dll = GetModuleHandleA("kernel32.dll");
    if (kernel32_dll)
    {
        real_SetFileAttributesA = (SETFILEATTRIBUTESAPROC)GetProcAddress(kernel32_dll, "SetFileAttributesA");
        real_SetFileAttributesW = (SETFILEATTRIBUTESWPROC)GetProcAddress(kernel32_dll, "SetFileAttributesW");
        hook_patch_iat_list(GetModuleHandleA(NULL), FALSE, (HOOKLIST *)&g_hooks);
    }
    else
    {
        return 1;
    }
    return 0;
}

int IAT_UnInstallHook()
{
    hook_patch_iat_list(GetModuleHandleA(NULL), TRUE, (HOOKLIST *)&g_hooks);
    return 0;
}