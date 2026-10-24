@echo off
cd /d "%~dp0"
"Services\TTS\VoxCPM2\python\python.exe" "Services\Vosk\vosk_server.py"
pause