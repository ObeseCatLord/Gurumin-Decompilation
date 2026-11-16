@echo off
rem Applies ..\symbols.tsv to the Ghidra project, re-exports into export_new\, then swaps it in for export\APP.dll
setlocal
set "RE=%~dp0"
set "GHIDRA=E:\Claude Typing of the Dead\ghidra_12.2_PUBLIC"
if exist "%RE%export_new" rmdir /s /q "%RE%export_new"
mkdir "%RE%export_new"
call "%GHIDRA%\support\analyzeHeadless.bat" "%RE%project" GURUMIN -process APP.dll -noanalysis -scriptPath "%RE%scripts" -postScript ApplySymbols.java "%RE%..\symbols.tsv" -postScript ExportAll.java "%RE%export_new" -log "%RE%ghidra_apply.log"
if exist "%RE%export_new\APP.dll\decompiled.c" (
  rmdir /s /q "%RE%export\APP.dll"
  move "%RE%export_new\APP.dll" "%RE%export\APP.dll" >nul
)
