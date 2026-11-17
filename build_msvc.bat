@echo off

rd /s /q msvc
del *.obj
del res\*.res
md msvc
rc /r /c65001 /DWIN32 res\libcurl.rc
cl /LD /utf-8 /DUNICODE /D_UNICODE /DWIN32 /D_WINDOWS /DWIN32_LEAN_AND_MEAN /O2 /MP /std:c17 /bigobj /Zc:preprocessor /permissive- /Zc:inline /GL /DNDEBUG /Fe:msvc\libcurl.dll *.c hooks\*.c User32.lib res\libcurlx.lib res\libcurl.res /link /DEBUG:NONE /INCREMENTAL:NO /LTCG /DEF:res\libcurl.def