@echo off
title WebServer
cd /d "%~dp0"
"Services\TTS\VoxCPM2\python\python.exe" -m http.server 3000 --bind 0.0.0.0 --directory "Source\cpp\web\static"
pause