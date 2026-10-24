@echo off
title TelegramBot
cd /d "%~dp0"
"Services\TTS\VoxCPM2\python\python.exe" "Services\TelegramBot\bot.py"
pause