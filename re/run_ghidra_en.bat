@echo off
setlocal
set "RE=%~dp0"
rem Set GHIDRA to your Ghidra 12.x folder; the default is the maintainer's.
if not defined GHIDRA set "GHIDRA=E:\Claude Typing of the Dead\ghidra_12.2_PUBLIC"
if not exist "%RE%project" mkdir "%RE%project"
call "%GHIDRA%\support\analyzeHeadless.bat" "%RE%project" GURUMIN_EN -import "%RE%bin\game_en_steam.exe" -overwrite -scriptPath "%RE%scripts" -postScript ExportAll.java "%RE%export" -log "%RE%ghidra_en.log"
