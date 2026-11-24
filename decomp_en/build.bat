@echo off
rem usage: build.bat <file.cpp> [extra cl flags]   (file in src\, or ..\src_xx\file.cpp)
rem Compiles one file with the EN game flags and byte-compares every function in it with game_en_steam.exe.
call "%~dp0env.bat"
if not exist "%~dp0build" mkdir "%~dp0build"
set SRC=%~1
rem Flags found by matching EN functions (NOTES.md "EN toolchain"); experiments go in GURUMIN_EN_CFLAGS.
set "ENFLAGS=/O2 /Oy- /GS- /EHsc /MT /Gy /D WIN32 /D NDEBUG /D _WINDOWS"
%CLRUN% cl /nologo /c %ENFLAGS% /Fo"%~dp0build\\" %GURUMIN_EN_CFLAGS% %2 %3 %4 %5 "%~dp0src\%SRC%"
if errorlevel 1 exit /b 1
for %%F in ("%SRC%") do "%GURUMIN_PY%" "%~dp0tools\compare_obj.py" "%~dp0build\%%~nF.obj" %6
