@echo off
chcp 65001 > nul
setlocal enabledelayedexpansion

set "GREEN=Green"
set "RED=Red"
set "CYAN=Cyan"
set "YELLOW=Yellow"

set "BASE=%~dp0"
set "BASE=%BASE:~0,-1%"
set "LOG_DIR=%BASE%\App\logs"
set "LOG_TEST_COMPILE=%LOG_DIR%\test_compile.log"

cls
call :color %CYAN% "J.A.R.V.I.S Test Runner (Build & Run)"
echo.
echo.

REM Проверяем, запущен ли сервер
netstat -an | find ":8080" | find "LISTENING" > nul
if %errorlevel% neq 0 (
    call :color %RED% " [ERROR] Сервер J.A.R.V.I.S не запущен (порт 8080 закрыт)."
    call :color %YELLOW% " Пожалуйста, сначала запустите 1startjarvis.bat"
    echo.
    pause
    exit /b 1
)

echo [INFO] Сервер Джарвиса запущен. Начинаю сборку тестов...

REM Поиск Visual Studio
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
set "VSPATH="
for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do (
    set "VSPATH=%%i"
)

if "%VSPATH%"=="" (
    call :color %RED% " [ERROR] Visual Studio с C++ tools не найден."
    pause
    exit /b 1
)

call "%VSPATH%\VC\Auxiliary\Build\vcvars64.bat" > nul

REM Поиск vcpkg
set "VCPKG_DIR="
if defined VCPKG_ROOT (
    if exist "%VCPKG_ROOT%\installed\x64-windows\include\curl\curl.h" (
        set "VCPKG_DIR=%VCPKG_ROOT%"
    )
)
if "%VCPKG_DIR%"=="" (
    if exist "C:\dev\vcpkg\installed\x64-windows\include\curl\curl.h" (
        set "VCPKG_DIR=C:\dev\vcpkg"
    )
)
if "%VCPKG_DIR%"=="" (
    if exist "%VSPATH%\VC\vcpkg\installed\x64-windows\include\curl\curl.h" (
        set "VCPKG_DIR=%VSPATH%\VC\vcpkg"
    )
)

if "%VCPKG_DIR%"=="" (
    call :color %RED% " [ERROR] vcpkg с curl.h не найден."
    pause
    exit /b 1
)

set "VCPKG_INC=%VCPKG_DIR%\installed\x64-windows\include"
set "VCPKG_LIB=%VCPKG_DIR%\installed\x64-windows\lib"

REM Компиляция тестов 
cd /d "%BASE%\Source\cpp"

if exist "%BASE%\App\bin\test_runner.exe" del "%BASE%\App\bin\test_runner.exe" > nul 2>&1

cl /EHsc /std:c++17 test_runner.cpp ^
    /I"%VCPKG_INC%" ^
    /I. ^
    /I"libs\nlohmann" ^
    /link ^
    /LIBPATH:"%VCPKG_LIB%" ^
    libcurl.lib ws2_32.lib ^
    /OUT:"%BASE%\App\bin\test_runner.exe" > "%LOG_TEST_COMPILE%" 2>&1

if %errorlevel% neq 0 (
    call :color %RED% " [ERROR] Ошибка компиляции тестов"
    call :color %YELLOW% " (см. %LOG_TEST_COMPILE%)"
    echo.
    type "%LOG_TEST_COMPILE%" | more
    pause
    exit /b 1
)

call :color %GREEN% " [OK] Тесты собраны."
echo.
echo ______________________________________________
echo.

REM Запуск тестов 
"%BASE%\App\bin\test_runner.exe"

echo.
echo ______________________________________________

pause
exit /b

:color
powershell -command "$Host.UI.RawUI.ForegroundColor = '%1'; Write-Host '%2' -NoNewline; $Host.UI.RawUI.ForegroundColor = 'White'"
exit /b