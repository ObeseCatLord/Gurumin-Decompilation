@echo off
rem Installs the Gurumin mod DLL (HD movies, display options): install.bat "C:\path\to\Gurumin" (the folder with game.exe).
setlocal
set "GAME=%~1"
if "%GAME%"=="" set /p "GAME=Gurumin folder (with game.exe): "
if not exist "%GAME%\game.exe" (echo No game.exe in "%GAME%". & exit /b 1)
if not exist "%GAME%\bin\APP.dll" (echo No bin\APP.dll in "%GAME%". & exit /b 1)
if not exist "%~dp0build\d3d9.dll" (echo Build the DLL first: mod\build.sh & exit /b 1)
if exist "%GAME%\d3d9.dll" if not exist "%GAME%\d3d9.dll.orig" (
  findstr /m "gurumin_mod" "%GAME%\d3d9.dll" >nul || copy /y "%GAME%\d3d9.dll" "%GAME%\d3d9.dll.orig" >nul
)
copy /y "%~dp0build\d3d9.dll" "%GAME%\d3d9.dll" >nul || exit /b 1
if not exist "%GAME%\gurumin_mod.ini" copy "%~dp0gurumin_mod.ini" "%GAME%\gurumin_mod.ini" >nul
echo Installed. Settings: "%GAME%\gurumin_mod.ini". Remove with uninstall.bat.
