@echo off
rem Environment for the EN (Steam, VS2013) build: VS2013 Update 4, cl 18.00.31101 / link 12.00.31101,
rem the exact build in game_en_steam.exe's Rich header. Set GURUMIN_VC12 to your "Microsoft Visual Studio 12.0" folder.
set "VS12=E:\Microsoft Visual Studio 12.0"
if defined GURUMIN_VC12 set "VS12=%GURUMIN_VC12%"
call "%VS12%\VC\bin\vcvars32.bat" >nul
set "INCLUDE=%~dp0include;%INCLUDE%"
if not defined GURUMIN_PY set "GURUMIN_PY=C:\Users\Michael\AppData\Local\Programs\Python\Python313\python.exe"
if not exist "%GURUMIN_PY%" set "GURUMIN_PY=python"
rem VS2013's front end crashes (C1001, p0io.c) when the Windows ANSI code page is UTF-8 (65001):
rem tools\run_cl.py runs cl with GetACP patched to GURUMIN_EN_ACP (default 932), or directly on other systems.
set CLRUN="%GURUMIN_PY%" "%~dp0tools\run_cl.py"
