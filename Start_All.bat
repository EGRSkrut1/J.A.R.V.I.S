@echo off
chcp 65001 > nul
title J.A.R.V.I.S Launcher
setlocal enabledelayedexpansion

set "GREEN=Green"
set "RED=Red"
set "CYAN=Cyan"
set "YELLOW=Yellow"

set "BASE=%~dp0"
set "BASE=%BASE:~0,-1%"
set "PYTHON=%BASE%\Services\TTS\VoxCPM2\python\python.exe"
set "LOG_DIR=%BASE%\App\logs"

if not exist "%LOG_DIR%" mkdir "%LOG_DIR%"
if not exist "%BASE%\App\bin" mkdir "%BASE%\App\bin"

set "LOG_COMPILE=%LOG_DIR%\compile.log"

cls
call :color %CYAN% "J.A.R.V.I.S Launcher"
echo.
echo.

echo [INFO] Проверка зависимостей...

set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" (
    call :color %RED% " [ERROR] Visual Studio не найдена."
    echo.
    pause
    exit /b 1
)

where ollama >nul 2>&1
if %errorlevel% neq 0 (
    call :color %RED% " [ERROR] Ollama не найдена."
    echo.
    pause
    exit /b 1
)

if not exist "%PYTHON%" (
    call :color %RED% " [ERROR] Python не найден."
    echo.
    pause
    exit /b 1
)

call :color %GREEN% " [OK] Все базовые зависимости на месте."
echo.

call :color %CYAN% "Поиск Visual Studio (vswhere)"
echo.

set "VSPATH="
for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do (
    set "VSPATH=%%i"
)

if "%VSPATH%"=="" (
    call :color %RED% " [ERROR] Visual Studio с C++ tools не найдена."
    echo.
    pause
    exit /b 1
)

call :color %CYAN% "VS: %VSPATH%"
echo.
call "%VSPATH%\VC\Auxiliary\Build\vcvars64.bat" > "%LOG_DIR%\msvc.log" 2>&1
if %errorlevel% neq 0 (
    call :color %RED% " [ERROR] (см. %LOG_DIR%\msvc.log)"
    echo.
    pause
    exit /b 1
)
call :color %GREEN% " [OK]"
echo.

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
    call :color %YELLOW% " [WARN] vcpkg с curl.h не найден."
    echo.
    set "VCPKG_INC="
    set "VCPKG_LIB="
) else (
    call :color %GREEN% " [OK] vcpkg: %VCPKG_DIR%"
    set "VCPKG_INC=%VCPKG_DIR%\installed\x64-windows\include"
    set "VCPKG_LIB=%VCPKG_DIR%\installed\x64-windows\lib"
)
echo.

call :color %CYAN% "Установка Python-зависимостей"
echo.
"%PYTHON%" -m pip install -q vosk pyaudio websockets pyTelegramBotAPI requests > "%LOG_DIR%\pip.log" 2>&1
if %errorlevel% neq 0 (
    call :color %RED% " [ERROR] (см. %LOG_DIR%\pip.log)"
    echo.
    pause
    exit /b 1
)
call :color %GREEN% " [OK]"
echo.

call :color %CYAN% "Компиляция J.A.R.V.I.S"
echo.
cd /d "%BASE%\Source\cpp"

if exist "%BASE%\App\bin\jarvis_client.exe" del "%BASE%\App\bin\jarvis_client.exe" > nul 2>&1

cl /EHsc /std:c++17 ^
    main.cpp ^
    core\application\jarvis_app.cpp ^
    core\config\config_manager.cpp ^
    core\logging\logger.cpp ^
    core\logging\voice_logger.cpp ^
    core\network\http_client.cpp ^
    features\activation\activation_controller.cpp ^
    features\activation\recognizers\vosk_recognizer.cpp ^
    features\speech\engines\windows_tts.cpp ^
    features\speech\engines\voxcpm_tts.cpp ^
    features\speech\hybrid_tts_manager.cpp ^
    features\llm\clients\ollama_client.cpp ^
    features\llm\context_manager.cpp ^
    modules\commands\jokes\joke_provider.cpp ^
    modules\commands\system_info\system_info.cpp ^
    modules\commands\greetings\greeting_provider.cpp ^
    modules\commands\text_input\text_input.cpp ^
    modules\commands\browser\browser_control.cpp ^
    modules\commands\devices\device_control.cpp ^
    modules\apps\spotify\spotify_controller.cpp ^
    modules\apps\steam\steam_controller.cpp ^
    modules\apps\discord\discord_controller.cpp ^
    modules\apps\ds4windows\ds4_controller.cpp ^
    modules\apps\happ\happ_controller.cpp ^
    modules\apps\zapret\zapret_controller.cpp ^
    modules\module_manager.cpp ^
    web\http_server\http_server.cpp ^
    web\http_server\controllers\auth_controller.cpp ^
    web\http_server\controllers\chat_controller.cpp ^
    web\http_server\controllers\chat_history.cpp ^
    web\http_server\controllers\admin_controller.cpp ^
    /I"%VCPKG_INC%" ^
    /I. ^
    /I"libs\ixwebsocket" ^
    /I"libs\cpp-httplib" ^
    /I"libs\nlohmann" ^
    /link ^
    /LIBPATH:"%VCPKG_LIB%" ^
    libcurl.lib ws2_32.lib winmm.lib ole32.lib uuid.lib sapi.lib ^
    /OUT:"%BASE%\App\bin\jarvis_client.exe" > "%LOG_COMPILE%" 2>&1

if %errorlevel% neq 0 (
    call :color %RED% " [ERROR] (см. %LOG_COMPILE%)"
    echo.
    type "%LOG_COMPILE%" | more
    pause
    exit /b 1
)
call :color %GREEN% " [OK]"
echo.

call :color %CYAN% "Запуск Vosk сервера"
echo.
taskkill /F /FI "WINDOWTITLE eq VoskServer*" > nul 2>&1
start "VoskServer" cmd /c "%BASE%\run_vosk.bat"
timeout /t 4 /nobreak > nul
netstat -an | find ":2700" | find "LISTENING" > nul
if %errorlevel% neq 0 (
    call :color %RED% " [ERROR] (см. окно VoskServer)"
    echo.
    pause
    exit /b 1
)
call :color %GREEN% " [OK]"
echo.

call :color %CYAN% "Запуск TTS сервера"
echo.
start "TTSServer" cmd /c "%BASE%\run_tts.bat"
timeout /t 5 /nobreak > nul
call :color %GREEN% " [OK]"
echo.

call :color %CYAN% "Запуск J.A.R.V.I.S Core"
echo.
if not exist "%BASE%\App\bin\jarvis_client.exe" (
    call :color %RED% " [ERROR] jarvis_client.exe не найден."
    echo.
    pause
    exit /b 1
)
set "JARVIS_BASE=%BASE%"
start "CoreServer" cmd /c "%BASE%\run_core.bat"
timeout /t 2 /nobreak > nul
call :color %GREEN% " [OK]"
echo.

call :color %CYAN% "Запуск WebServer"
echo.
start "WebServer" cmd /c "%BASE%\run_web.bat"
timeout /t 2 /nobreak > nul
call :color %GREEN% " [OK]"
echo.

call :color %CYAN% "Запуск TelegramBot"
echo.
if exist "%BASE%\Services\TelegramBot\bot.py" (
    start "TelegramBot" cmd /c "%BASE%\run_bot.bat"
    call :color %GREEN% " [OK]"
    echo.
) else (
    call :color %YELLOW% " [WARN] bot.py не найден."
    echo.
)

echo.
call :color %CYAN% "J.A.R.V.I.S готов к работе, сэр."
echo.

timeout /t 3 /nobreak > nul
start msedge "http://localhost:3000"

pause
exit /b

:color
powershell -command "$Host.UI.RawUI.ForegroundColor = '%1'; Write-Host '%2' -NoNewline; $Host.UI.RawUI.ForegroundColor = 'White'"
exit /b