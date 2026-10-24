# J.A.R.V.I.S — Local Voice Assistant

J.A.R.V.I.S is a modular client-server voice control system inspired by Iron Man. It runs locally, without clouds or subscriptions. It controls your smart home and applications, and answers questions using neural networks.

## Full Feature List

### 1. Voice Interaction
- Speech recognition (Vosk).
- Wake word activation ("Jarvis").
- Speech synthesis (Windows TTS / VoxCPM2).
- OK sound playback when commands are executed.

### 2. Smart Home Control (IoT)
- Turn the tree on/off (ESP32) over HTTP.
- Context-aware routing (room binding via `terminal_id`).

### 3. Application Control
- Launch Steam, Discord, DS4Windows, Happ VPN, zapret.
- Spotify control (pause, next track, volume, search).
- Launch games from the Steam library by name (via Web API).

### 4. LLM Integration
- Free-form dialogue (Ollama llama3.1:8b).
- Tool Calling (invoking `execute_system_command`).
- System information extraction (date, time).
- Internet search (via tool calling).

### 5. Clients and Interfaces
- Web interface (login, chat, settings, admin panel).
- Telegram bot (admin/user roles, control, system status).

### 6. Security and Architecture
- Token-based authentication.
- Role separation (admin/user).
- Modular architecture (Strategy, Command, Plugin, DI, Facade, Observer).
- Multithreading (TTS, HTTP, WebSocket).

## Technologies
- **Language:** C++17, Python 3.10, Dart (Flutter).
- **Server:** cpp-httplib, libcurl, nlohmann/json.
- **Neural networks:** Ollama (llama3.1:8b), Vosk (STT), VoxCPM2 (TTS).
- **Microcontrollers:** ESP32-C6 (PlatformIO).

## Installation and Launch
1. Install Visual Studio 2022 with the "Desktop development with C++" workload.
2. Install Ollama and download the model: `ollama pull llama3.1:8b`.
3. Install the Python dependencies (the script will do it automatically).
4. Run `Start_All.bat` (as administrator).

## Configuration
Before launching, copy the templates and configure them:
- `App/configs/apps_template.json` -> `apps.json` (application paths).
- `App/configs/network_template.json` -> `network.json` (Wi-Fi for ESP).
- `App/configs/steam_api_template.json` -> `steam_api.json` (Steam API key).
- `App/configs/telegram_template.json` -> `telegram.json` (bot token).
- `App/configs/devices_template.json` -> `devices.json` (smart device IPs).
- `App/configs/users_template.json` -> `users.json` (users and roles).

## License
**PolyForm Noncommercial License 1.0.0**
The project is distributed for educational and noncommercial purposes. Commercial use without the author's permission is prohibited.