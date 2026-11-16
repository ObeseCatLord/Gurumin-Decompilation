@echo off
rem Applies symbols.tsv to the existing Ghidra project and re-exports re\export\APP.dll\
setlocal
set "RE=%~dp0"
set "GHIDRA=E:\Claude Typing of the Dead\ghidra_12.2_PUBLIC"
call "%GHIDRA%\support\analyzeHeadless.bat" "%RE%project" GURUMIN -process APP.dll -noanalysis -scriptPath "%RE%scripts" -postScript ApplySymbols.java "%RE%..\symbols.tsv" -postScript ExportAll.java "%RE%export" -log "%RE%ghidra_apply.log"
