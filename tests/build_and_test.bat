@echo off
echo Building and running UEFI Variable Recovery Tool Tests...

REM Check if we have cl (MSVC) or gcc (MinGW) available
where cl >nul 2>nul
if %ERRORLEVEL% EQU 0 (
    echo Using Microsoft Visual C++ compiler...
    
    REM Check if we're in a Visual Studio Command Prompt
    if not defined VSINSTALLDIR (
        echo Error: Please run this from a "Developer Command Prompt for VS"
        echo You can find this in your Start Menu under Visual Studio tools
        exit /b 1
    )
    
    REM Compile with MSVC
    cl /W4 /EHsc /Fe:test_mock.exe test_mock.c mock_uefi.c /I"%VSINSTALLDIR%\VC\Tools\MSVC\14.29.30133\include" /I"%WindowsSdkDir%\Include\10.0.19041.0\ucrt"
) else (
    where gcc >nul 2>nul
    if %ERRORLEVEL% EQU 0 (
        echo Using GCC compiler...
        
        REM Compile with GCC
        gcc -Wall -Wextra -g -o test_mock.exe test_mock.c mock_uefi.c
    ) else (
        echo Error: No suitable compiler found.
        echo.
        echo To fix this, install MinGW-w64 using one of these methods:
        echo.
        echo 1. Using winget:
        echo    winget install mingw
        echo.
        echo 2. Using chocolatey:
        echo    choco install mingw
        echo.
        echo 3. Download installer from:
        echo    https://github.com/msys2/msys2-installer/releases
        echo.
        echo After installation, add MinGW's bin directory to your PATH
        exit /b 1
    )
)

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
del *.obj *.exe 2>nul 