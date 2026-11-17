# 修复《命令与征服：重制版合集》（以及可能同引擎、需稍作修改的其他游戏）中 `AppData` 目录被解除隐藏的问题

## 描述

这个 bug 很久以前就有人描述过（[见此](https://steamcommunity.com/app/1213210/discussions/0/3048360012548450402/)），但遗憾的是，一直没有人给予足够重视。

事实证明，这只是一个很小（甚至有些愚蠢）的 bug：游戏会尝试对路径中的每个**文件夹**调用 `SetFileAttributes()` 并传入 `FILE_ATTRIBUTE_DIRECTORY`，但根目录除外——根目录传入的是 `0`，这甚至不是一个有效标志。根据[文档](https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-setfileattributesa)：

> 文件不能转换为目录。要创建目录，请使用 `CreateDirectory` 或 `CreateDirectoryEx` 函数。

尽管该 API 的具体实现尚不清楚，但最终结果是：该 API 会清除指定文件夹上的所有属性。

例如，假设路径为 `C:\Users\<Username>\AppData\Roaming\CnCRemastered\UGC\PreviewFiles`，游戏会对 `C:` 调用 `SetFileAttributes()` 并传入 `0`，并对 `Users`、`<Username>`、**`AppData`**、`Roaming`、`CnCRemastered`、`UGC` 和 `PreviewFiles` 调用 `SetFileAttributes()` 并传入 `FILE_ATTRIBUTE_DIRECTORY`，从而清除 `AppData` 的隐藏属性；如果以提升权限运行，甚至会清除 `Users` 的只读属性。

此补丁会挂钩这些 API，检查 `dwFileAttributes`；如果该值错误，则直接返回 `TRUE`，从而修复该 bug。

## 用法

将游戏文件夹中的原始 `libcurl.dll` 重命名为 `libcurlx.dll`，然后将此修复 DLL 放入该文件夹，并命名为 `libcurl.dll`。

## 致谢

- [cnc-ddraw](https://github.com/FunkyFr3sh/cnc-ddraw) 提供的 IAT 挂钩代码。
- [curl](https://github.com/curl/curl) 提供的 `.rc` 文件。