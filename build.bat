@echo off

if not exist build mkdir build

set compiler_flags=-std=c99 -g -Wall -Wextra -Werror -Wvla -Wno-unused-function
set linker_flags=-lkernel32 -luser32 -lgdi32
set defines=-D_DEBUG -D_CRT_SECURE_NO_WARNINGS
set include_path=code/

clang code/tgui_win32.c -o build/tgui.exe -I%include_path% %compiler_flags% %defines% %linker_flags%
