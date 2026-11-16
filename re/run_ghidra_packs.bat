@echo off
setlocal
set "RE=%~dp0"
set "GHIDRA=E:\Claude Typing of the Dead\ghidra_12.2_PUBLIC"
if not exist "%RE%project" mkdir "%RE%project"
call "%GHIDRA%\support\analyzeHeadless.bat" "%RE%project" PACKS -import "%RE%bin\Packs.dll" -overwrite -scriptPath "%RE%scripts" -postScript ExportAll.java "%RE%export" -log "%RE%ghidra_packs.log"
