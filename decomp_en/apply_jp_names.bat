@echo off
rem Applies the JP names (and safe JP signatures) from jp_en_map.tsv to the EN Ghidra project, then re-exports it.
rem 1. ExportJpSignatures.java on the JP project (read-only) -> build\jp_signatures.tsv
rem 2. ApplyJpNames.java on GURUMIN_EN (saved), then ExportAll.java into re\export_new\, swapped in for re\export\game_en_steam.exe
rem Run tools\map_jp_to_en.py first. Takes a long time (the EN export decompiles 24,656 functions).
setlocal
set "EN=%~dp0"
for %%I in ("%~dp0..\re") do set "RE=%%~fI"
set "GHIDRA=E:\Claude Typing of the Dead\ghidra_12.2_PUBLIC"
if defined GURUMIN_GHIDRA set "GHIDRA=%GURUMIN_GHIDRA%"
if not exist "%EN%build" mkdir "%EN%build"
call "%GHIDRA%\support\analyzeHeadless.bat" "%RE%\project" GURUMIN -process APP.dll -noanalysis -readOnly -scriptPath "%EN%ghidra" -postScript ExportJpSignatures.java "%EN%build\jp_signatures.tsv" -log "%EN%build\ghidra_jp_sigs.log"
if exist "%RE%\export_new\game_en_steam.exe" rmdir /s /q "%RE%\export_new\game_en_steam.exe"
if not exist "%RE%\export_new" mkdir "%RE%\export_new"
call "%GHIDRA%\support\analyzeHeadless.bat" "%RE%\project" GURUMIN_EN -process game_en_steam.exe -noanalysis -scriptPath "%EN%ghidra;%RE%\scripts" -postScript ApplyJpNames.java "%EN%jp_en_map.tsv" "%EN%build\jp_signatures.tsv" medium -postScript ExportAll.java "%RE%\export_new" -log "%EN%build\ghidra_apply_en.log"
if exist "%RE%\export_new\game_en_steam.exe\decompiled.c" (
  rmdir /s /q "%RE%\export\game_en_steam.exe"
  move "%RE%\export_new\game_en_steam.exe" "%RE%\export\game_en_steam.exe" >nul
)
