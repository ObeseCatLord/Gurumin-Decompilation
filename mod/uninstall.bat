@echo off
rem Removes the Gurumin mod DLL: uninstall.bat "C:\path\to\Gurumin".
setlocal
set "GAME=%~1"
if "%GAME%"=="" set /p "GAME=Gurumin folder (with game.exe): "
if not exist "%GAME%\game.exe" (echo No game.exe in "%GAME%". & exit /b 1)
if exist "%GAME%\d3d9.dll" findstr /m "gurumin_mod" "%GAME%\d3d9.dll" >nul && del "%GAME%\d3d9.dll"
if exist "%GAME%\d3d9.dll.orig" if not exist "%GAME%\d3d9.dll" ren "%GAME%\d3d9.dll.orig" d3d9.dll
del "%GAME%\gurumin_mod.ini" "%GAME%\gurumin_mod.log" 2>nul
echo Removed.
