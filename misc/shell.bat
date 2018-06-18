:: @ says don't echo "echo off"
@echo off

:: initialize `cl` for x64
:: `call` launchs a new bat and continues
call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" x64

:: Add path to shell if it doesn't exist in environment variables
set path=D:\Users\Max\source\repos\handmade-hero\misc;%path%