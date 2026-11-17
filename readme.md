# Fix for the `AppData` directory being unhidden in Command & Conquer Remastered Collection (and probably other games on the same engine, with slight changes)

## Description

This bug was described a long time ago ([here](https://steamcommunity.com/app/1213210/discussions/0/3048360012548450402/)), but unfortunately no one paid enough attention to it.

It turns out to be just a small (and maybe silly) bug: the game tries to call `SetFileAttributes()` with `FILE_ATTRIBUTE_DIRECTORY` on every **folder** in a path, excluding the root directory, which is passed `0`—not even a valid flag. According to the [docs](https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-setfileattributesa):

> Files cannot be converted into directories. To create a directory, use the `CreateDirectory` or `CreateDirectoryEx` function.

Although the exact implementation of this API is unclear, the final result is that the API clears all attributes from the specified folders.

For example, suppose a path like `C:\Users\<Username>\AppData\Roaming\CnCRemastered\UGC\PreviewFiles`; the game calls `SetFileAttributes()` on `C:` with `0`, and on `Users`, `<Username>`, **`AppData`**, `Roaming`, `CnCRemastered`, `UGC`, and `PreviewFiles` with `FILE_ATTRIBUTE_DIRECTORY`, clearing the hidden attribute on `AppData`, and even the read-only attribute on `Users`, if run with elevated privileges.

This patch hooks these APIs, checks `dwFileAttributes`, and returns `TRUE` directly if the value is wrong, thereby fixing the bug.

## Usage

Rename the original `libcurl.dll` in the game folder to `libcurlx.dll`, then place this fix DLL in the folder and name it `libcurl.dll`.

## Credits

- [cnc-ddraw](https://github.com/FunkyFr3sh/cnc-ddraw) for the IAT hooking code.
- [curl](https://github.com/curl/curl) for the `.rc` file.