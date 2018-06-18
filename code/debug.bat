@echo off

:: Build for debug and open in Visual Studio
call build.bat
devenv ..\build\win32_handmade.exe