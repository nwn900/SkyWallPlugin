@echo off
REM WallWalkSKSE Build Script
REM Requires: Visual Studio 2022/2026, CMake, vcpkg

set "VCPKG_ROOT=%~dp0..\vcpkg"

if not exist "%VCPKG_ROOT%\vcpkg.exe" (
    echo vcpkg not found at %VCPKG_ROOT%
    exit /b 1
)

if not exist "%~dp0extern\CommonLibSSE-NG\CMakeLists.txt" (
    echo Cloning CommonLibSSE-NG...
    git clone --depth 1 -b ng https://github.com/alandtse/CommonLibVR.git "%~dp0extern\CommonLibSSE-NG"
)

rmdir /s /q "%~dp0build" 2>nul

cmake -B build -S "." -DCMAKE_TOOLCHAIN_FILE="%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake" -DVCPKG_TARGET_TRIPLET=x64-windows
if %ERRORLEVEL% neq 0 exit /b %ERRORLEVEL%

cmake --build build --config Release --parallel
if %ERRORLEVEL% neq 0 exit /b %ERRORLEVEL%

echo.
echo Build complete: build\Release\WallWalkSKSE.dll
