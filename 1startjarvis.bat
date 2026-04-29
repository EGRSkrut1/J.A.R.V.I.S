@echo off
chcp 65001 > nul
setlocal enabledelayedexpansion

set "GREEN=Green"
set "RED=Red"
set "CYAN=Cyan"
set "YELLOW=Yellow"

set "BASE=C:\Users\egrsk\Desktop\Jarvis"
set "PYTHON=%BASE%\Services\TTS\VoxCPM2\python\python.exe"
set "LOG_DIR=%BASE%\App\logs"

if not exist "%LOG_DIR%" mkdir "%LOG_DIR%"
if not exist "%BASE%\App\bin" mkdir "%BASE%\App\bin"

set "LOG_COMPILE=%LOG_DIR%\compile.log"

cls
call :color %CYAN% "J.A.R.V.I.S Launcher"
echo.
echo.

:: [1/7] MSVC
call :color %CYAN% "Настройка окружения MSVC"
echo.
call :progress_bar_fast
call "D:\visual studio\product\VC\Auxiliary\Build\vcvars64.bat" > "%LOG_DIR%\msvc.log" 2>&1
if %errorlevel% neq 0 (
    call :color %RED% " [ERROR]"
    call :color %YELLOW% " (см. %LOG_DIR%\msvc.log)"
    echo.
    pause
    exit /b
)
call :color %GREEN% " [OK]"
echo.

:: [2/7] Python зависимости
call :color %CYAN% "Установка Python-зависимостей"
echo.
call :progress_bar_fast
"%PYTHON%" -m pip install -q vosk pyaudio websockets > "%LOG_DIR%\pip.log" 2>&1
if %errorlevel% neq 0 (
    call :color %RED% " [ERROR]"
    call :color %YELLOW% " (см. %LOG_DIR%\pip.log)"
    echo.
    pause
    exit /b
)
call :color %GREEN% " [OK]"
echo.

:: [3/7] Vosk сервер
call :color %CYAN% "Запуск Vosk сервера"
echo.
taskkill /F /FI "WINDOWTITLE eq Vosk*" > nul 2>&1
start /B "VoskServer" "%PYTHON%" "%BASE%\Services\Vosk\vosk_server.py" > "%LOG_DIR%\vosk.log" 2>&1
call :progress_bar 4
netstat -an | find ":2700" | find "LISTENING" > nul
if %errorlevel% neq 0 (
    call :color %RED% " [ERROR]"
    call :color %YELLOW% " (см. %LOG_DIR%\vosk.log)"
    echo.
    pause
    exit /b
)
call :color %GREEN% " [OK]"
echo.

:: [4/7] TTS сервер
call :color %CYAN% "Запуск TTS сервера"
echo.
start /B "TTSServer" cmd /c "%BASE%\Services\TTS\VoxCPM2\run.bat" > "%LOG_DIR%\tts.log" 2>&1
call :progress_bar 3
call :color %GREEN% " [OK]"
echo.

:: [5/7] Компиляция J.A.R.V.I.S
call :color %CYAN% "Компиляция J.A.R.V.I.S"
echo.
call :progress_bar_fast
cd /d "%BASE%\Source\cpp"

if exist "%BASE%\App\bin\jarvis_client.exe" del "%BASE%\App\bin\jarvis_client.exe" > nul 2>&1

cl /EHsc /std:c++17 ^
    main.cpp ^
    core\application\jarvis_app.cpp ^
    core\config\config_manager.cpp ^
    core\logging\logger.cpp ^
    core\logging\voice_logger.cpp ^
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
    modules\apps\spotify\spotify_controller.cpp ^
    modules\module_manager.cpp ^
    web\http_server\http_server.cpp ^
    web\websocket_server\websocket_server.cpp ^
    /I"C:\dev\vcpkg\installed\x64-windows\include" ^
    /I. ^
    /I"libs\ixwebsocket" ^
    /I"libs\cpp-httplib" ^
    /I"libs\nlohmann" ^
    /link ^
    /LIBPATH:"C:\dev\vcpkg\installed\x64-windows\lib" ^
    libcurl.lib ws2_32.lib winmm.lib ole32.lib uuid.lib sapi.lib ^
    /OUT:"%BASE%\App\bin\jarvis_client.exe" > "%LOG_COMPILE%" 2>&1

if %errorlevel% neq 0 (
    call :color %RED% " [ERROR]"
    call :color %YELLOW% " (см. %LOG_COMPILE%)"
    echo.
    pause
    exit /b
)
call :color %GREEN% " [OK]"
echo.

:: [6/7] Компиляция WebUI
call :color %CYAN% "Компиляция WebUI"
echo.
call :progress_bar_fast

if exist "%BASE%\App\bin\webui.exe" del "%BASE%\App\bin\webui.exe" > nul 2>&1

cl /EHsc /std:c++17 ^
    web\webui\webui.cpp ^
    /I"libs\webview\core\include" ^
    /I"libs\webview\WebView2\include" ^
    /link ^
    /OUT:"%BASE%\App\bin\webui.exe" > "%LOG_DIR%\webui_compile.log" 2>&1

if %errorlevel% equ 0 (
    call :color %GREEN% " [OK]"
) else (
    call :color %YELLOW% " [WARN] (см. %LOG_DIR%\webui_compile.log)"
)
echo.

:: [7/7] Запуск
call :color %CYAN% "Запуск веб-интерфейса"
echo.
start /B "WebServer" "%PYTHON%" -m http.server 3000 --directory "%BASE%\Source\cpp\web\static" > "%LOG_DIR%\web.log" 2>&1
timeout /t 2 /nobreak > nul
call :color %GREEN% " [OK]"
echo.

call :color %CYAN% "Запуск J.A.R.V.I.S"
echo.
start "J.A.R.V.I.S Console" "%BASE%\App\bin\jarvis_client.exe"
timeout /t 1 /nobreak > nul

if exist "%BASE%\App\bin\webui.exe" (
    start "" "%BASE%\App\bin\webui.exe"
)

echo.
call :color %CYAN% "J.A.R.V.I.S готов к работе, сэр."
echo.
call :color %CYAN% "Веб-интерфейс: http://localhost:3000"
echo.

:: [X/X] Компиляция тестов
call :color %CYAN% "Компиляция тестов"
echo.
cd /d "%BASE%\Source\cpp"

cl /EHsc /std:c++17 test_runner.cpp ^
    /I"C:\dev\vcpkg\installed\x64-windows\include" ^
    /I"libs\nlohmann" ^
    /I"libs" ^
    /link ^
    /LIBPATH:"C:\dev\vcpkg\installed\x64-windows\lib" ^
    libcurl.lib ws2_32.lib ^
    /OUT:"%BASE%\App\bin\test_runner.exe" > "%LOG_DIR%\test_compile.log" 2>&1

if %errorlevel% equ 0 (
    call :color %GREEN% " [OK]"
) else (
    call :color %RED% " [ERROR]"
    type "%LOG_DIR%\test_compile.log"
)
echo.
pause
exit /b

:: Функции
:progress_bar_fast
<nul set /p "=100%|████████████████████████████████| "
exit /b

:progress_bar
set "seconds=%~1"
set "width=34"
for /l %%i in (0,1,%width%) do (
    set /a "percent=%%i*100/%width%"
    set "filled="
    for /l %%j in (1,1,%%i) do set "filled=!filled!█"
    set "unfilled="
    for /l %%j in (%%i,1,%width%) do set "unfilled=!unfilled!░"
    <nul set /p "=!percent!%%|!filled!!unfilled!| "
    timeout /t 1 /nobreak > nul
    for /l %%k in (1,1,50) do <nul set /p "="
)
<nul set /p "=100%|████████████████████████████████| "
exit /b

:color
powershell -command "$Host.UI.RawUI.ForegroundColor = '%1'; Write-Host '%2' -NoNewline; $Host.UI.RawUI.ForegroundColor = 'White'"
exit /b

