@echo off
echo Building and running UEFI Variable Recovery Tool Tests...

REM Check for GCC
where gcc >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo Error: GCC not found!
    echo.
    echo Please install MinGW-w64:
    echo 1. Download MSYS2 from https://github.com/msys2/msys2-installer/releases
    echo 2. Run the installer
    echo 3. Open "MSYS2 MINGW64" from Start Menu
    echo 4. Run: pacman -S mingw-w64-x86_64-gcc
    echo 5. Add C:\msys64\mingw64\bin to your PATH
    exit /b 1
)

echo Using GCC compiler...

REM Compile with GCC
gcc -Wall -Wextra -g -o test_mock.exe test_mock.c mock_uefi.c

if %ERRORLEVEL% NEQ 0 (
    echo Build failed!
    exit /b 1
)

echo.
echo Running tests...
echo.
test_mock.exe

echo.
echo Cleaning up...
del test_mock.exe 2>nul 