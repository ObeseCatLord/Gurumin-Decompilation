@echo off
set VC=C:\Program Files (x86)\Microsoft Visual Studio .NET 2003\Vc7
set PATH=%VC%\bin;%VC%\..\Common7\IDE;%PATH%
set INCLUDE=%~dp0include;%VC%\include;%VC%\PlatformSDK\include
set LIB=%VC%\lib;%VC%\PlatformSDK\lib
