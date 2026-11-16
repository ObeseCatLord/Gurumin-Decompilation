@echo off
rem usage: decompile_at.bat <PROJECT> <program> <out file> <hex>...
setlocal
set "RE=%~dp0"
set "GHIDRA=E:\Claude Typing of the Dead\ghidra_12.2_PUBLIC"
set PROJ=%1
set PROG=%2
set OUT=%3
shift & shift & shift
set ARGS=
:loop
if "%1"=="" goto run
set ARGS=%ARGS% %1
shift
goto loop
:run
call "%GHIDRA%\support\analyzeHeadless.bat" "%RE%project" %PROJ% -process %PROG% -noanalysis -scriptPath "%RE%scripts" -postScript DecompileAt.java "%OUT%" %ARGS% -log "%RE%ghidra_decompile_at.log"
