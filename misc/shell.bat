:: @ says don't echo "echo off"
@echo off

:: `cl` needs environment initialized for x64
:: `call` needed to launch a new bat, and continue running code from here
call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" x64

:: `set path` shows environment variables
:: Add path to shell if it doesn't exist in environment variables
set path=D:\Users\Max\source\repos\handmade-hero\misc;%path%

:: `devenv` launches Visual Studio