@echo off

:: Build (with -Zi debug info) and open in Visual Studio
call build.bat
devenv ..\build\win32_handmade.exe