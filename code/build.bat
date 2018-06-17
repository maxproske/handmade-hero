@echo off

:: Run shell if cl is not a recognized command
cl >NUL 2>&1 || ( call ../misc/shell.bat )

:: `cl win32_handmade.cpp` builds an .obj (intermediate file) and .exe
:: `-Zi` flag builds with .pdp debug files

:: Push directory and build here
mkdir ..\build 2> NUL
pushd ..\build
:: Unresolved external symbol means we are missing a .lib (Lookup DLL in MSDN)
cl -Zi ..\code\win32_handmade.cpp user32.lib gdi32.lib

:: Pop directory to return to starting directory
popd