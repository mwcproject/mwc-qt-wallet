@echo off
rem *****************************************
rem Generates a fresh VS solution using CMake
rem *****************************************
echo.

rem Clean env
set CMAKE_BIN=
set _VSVER=
set _TARCH=
set _YEAR=

rem Try to locate cmake
where cmake 1>NUL 2>NUL
if %ERRORLEVEL% equ 0 (
	for /f "tokens=*" %%i in ('where cmake') do set CMAKE_BIN=%%i
) else (
	set CMAKE_BIN=C:\Program Files\CMake\bin\cmake.exe
)
if not defined CMAKE_BIN goto :cmerr

echo Using CMake: %CMAKE_BIN%

rem Visual Studio version as set by vcvarsXXX
set _VSVER=%VisualStudioVersion%
if not defined _VSVER goto :vserr

rem Try to pick an appropriate target arch
if "%1"=="/32" set _TARCH=x86
if not defined _TARCH set _TARCH=%VSCMD_ARG_TGT_ARCH%
if not defined _TARCH set _TARCH=x64

echo Using Visual Studio: %_VSVER%, target architecture: %_TARCH%
if not defined VSCMD_ARG_TGT_ARCH (
	if %_TARCH%==x64 (
		echo.
		echo ====
		echo WARN: Desired architecture cannot be detected on VS2015 and below.
		echo Defaulting to 64-bit. Please re-run this script with the /32 flag
		echo if you need to build for 32-bit.
		echo ====
		echo.
		pause
	)
)

rem Make or clean target directory
mkdir VS\%_TARCH% 2>NUL
cd VS\%_TARCH%
del /f/q/s *.* >NUL
echo.



rem CMakeLists.txt must point to Qt
echo.
echo ====
echo STOP: Did you remember to edit CMakeLists.txt to point to your Qt installation?
echo Do not continue until Qt_DIR is set correctly.
echo ====
echo.
pause



rem Run CMake with the appropriate VS generator
if "%_VSVER%"=="14.0" goto :vs14
if "%_VSVER%"=="15.0" goto :vs15
if "%_VSVER%"=="16.0" goto :vs16

:vs14
set _YEAR=2015
if "%_TARCH%"=="x64" set _ARCH= Win64
"%CMAKE_BIN%" -G"Visual Studio 14 2015%_ARCH%" ..\..
if %ERRORLEVEL% neq 0 goto :generr
goto :success

:vs15
set _YEAR=2017
if "%_TARCH%"=="x64" set _ARCH= Win64
"%CMAKE_BIN%" -G"Visual Studio 15 2017%_ARCH%" ..\..
if %ERRORLEVEL% neq 0 goto :generr
goto :success

:vs16
set _YEAR=2019
if "%_TARCH%"=="x64" set _ARCH= x64
"%CMAKE_BIN%" -G"Visual Studio 16 2019" -A%_ARCH% ..\..
if %ERRORLEVEL% neq 0 goto :generr
goto :success



:success
echo.
echo ====
echo SUCCESS: You can find the generated Visual Studio %_YEAR% solution at
echo %CD%\mwc-qt-wallet.sln
echo ====
cd ..\..
goto :end



:cmerr
echo.
echo ====
echo FATAL: Could not locate CMake. Please add it to your path or edit the CMAKE_BIN
echo variable at the beginning of this script.
echo ====
goto :end

:vserr
echo.
echo ====
echo FATAL: Could not detect Visual Studio version. Please run this script from
echo a Visual Studio developer command prompt or call vcvarsall.bat manually.
echo ====
goto :fail

:generr
echo.
echo ===
echo FATAL: CMake could not generate the Visual Studio %_YEAR% solution. Please check
echo the output above and ensure you have installed all the prerequisites outlined in
for %%a in ("%CD%\..\..") DO echo %%~dpaDOCS\BUILD_WINDOWS.md
echo ===
goto :fail



:fail
rem del /f/q/s *.* >NUL
cd ..
rem rmdir /Q/S %_TARCH%
cd ..

:end
rem Clean env
set CMAKE_BIN=
set _VSVER=
set _TARCH=
set _YEAR=
echo.
pause
