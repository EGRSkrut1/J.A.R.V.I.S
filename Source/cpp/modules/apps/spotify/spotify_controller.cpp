#include "spotify_controller.h"
#include "core/logging/logger.h"
#include <windows.h>
#include <string>
#include <algorithm>

struct SpotifyController::Impl {
    bool available = true;
    std::string spotifyLink = "C:\\Users\\egrsk\\Desktop\\Spotify.lnk";
    std::string spotifyExe = "C:\\Users\\egrsk\\AppData\\Roaming\\Spotify\\Spotify.exe";
    
    void ensureRunning() {
        if (system("tasklist /FI \"IMAGENAME eq Spotify.exe\" 2>NUL | find /I /N \"Spotify.exe\" >NUL") == 0) return;
        std::string cmd = "start \"\" \"" + spotifyLink + "\"";
        if (system(cmd.c_str()) != 0) {
            cmd = "start \"\" \"" + spotifyExe + "\"";
            system(cmd.c_str());
        }
        Sleep(2000);
    }
    
    void sendMediaKey(int key) {
        keybd_event(key, 0, 0, 0);
        Sleep(30);
        keybd_event(key, 0, KEYEVENTF_KEYUP, 0);
    }
    
    void focusSpotify() {
        std::string psCmd = "powershell -command \"(New-Object -ComObject WScript.Shell).AppActivate('Spotify')\"";
        system(psCmd.c_str());
        Sleep(500);
    }
    
    void setVolumePercent(int targetPercent) {
        ensureRunning();
        focusSpotify();
        
        for (int i = 0; i < 15; i++) {
            keybd_event(VK_CONTROL, 0, 0, 0);
            keybd_event(VK_DOWN, 0, 0, 0);
            Sleep(20);
            keybd_event(VK_DOWN, 0, KEYEVENTF_KEYUP, 0);
            keybd_event(VK_CONTROL, 0, KEYEVENTF_KEYUP, 0);
            Sleep(10);
        }
        
        int steps = targetPercent / 5;
        for (int i = 0; i < steps; i++) {
            keybd_event(VK_CONTROL, 0, 0, 0);
            keybd_event(VK_UP, 0, 0, 0);
            Sleep(20);
            keybd_event(VK_UP, 0, KEYEVENTF_KEYUP, 0);
            keybd_event(VK_CONTROL, 0, KEYEVENTF_KEYUP, 0);
            Sleep(10);
        }
    }
    
    int parseVolumeText(const std::string& cmd) {
        if (cmd.find("ноль") != std::string::npos) return 0;
        if (cmd.find("десять") != std::string::npos) return 10;
        if (cmd.find("двадцать") != std::string::npos) return 20;
        if (cmd.find("тридцать") != std::string::npos) return 30;
        if (cmd.find("сорок") != std::string::npos) return 40;
        if (cmd.find("пятьдесят") != std::string::npos) return 50;
        if (cmd.find("шестьдесят") != std::string::npos) return 60;
        if (cmd.find("семьдесят") != std::string::npos) return 70;
        if (cmd.find("восемьдесят") != std::string::npos) return 80;
        if (cmd.find("девяносто") != std::string::npos) return 90;
        if (cmd.find("сто") != std::string::npos) return 100;
        for (size_t i = 0; i < cmd.length(); i++) {
            if (isdigit(cmd[i])) { int p = atoi(&cmd[i]); if (p >= 0 && p <= 100) return p; }
        }
        return 50;
    }
    
    bool isSpotifyRelated(const std::string& lower) {
        if (lower.find("спотифай") != std::string::npos) return true;
        if (lower.find("spotify") != std::string::npos) return true;
        if (lower.find("музык") != std::string::npos) return true;
        if (lower.find("трек") != std::string::npos) return true;
        if (lower.find("любимые") != std::string::npos) return true;
        if (lower.find("сохранен") != std::string::npos) return true;
        if (lower.find("сохранён") != std::string::npos) return true;
        if (lower == "пауза") return true;
        if (lower == "дальше") return true;
        if (lower == "назад") return true;
        if (lower == "стоп") return true;
        if (lower.find("некст") != std::string::npos) return true;
        if (lower.find("скип") != std::string::npos) return true;
        if (lower.find("текст") != std::string::npos) return true;
        if (lower.find("продолж") != std::string::npos) return true;
        if (lower.find("играй") != std::string::npos) return true;
        if (lower.find("громк") != std::string::npos) return true;
        if (lower.find("тише") != std::string::npos) return true;
        if (lower.find("перемешивани") != std::string::npos) return true;
        if (lower.find("перемешать") != std::string::npos) return true;
        if (lower.find("первый") != std::string::npos) return true;
        if (lower.find("первая") != std::string::npos) return true;
        if (lower.find("умное") != std::string::npos) return true;
        if (lower.find("умная") != std::string::npos) return true;
        if (lower.find("начало") != std::string::npos) return true;
        if (lower.find("полная") != std::string::npos) return true;
        if (lower.find("выключи") != std::string::npos) return true;
        if (lower.find("включи") != std::string::npos) return true;
        if (lower.find("отключи") != std::string::npos) return true;
        if (lower.find("поищи") != std::string::npos) return true;
        if (lower.find("найди") != std::string::npos) return true;
        if (lower.find("инфо") != std::string::npos) return true;
        if (lower.find("какой трек") != std::string::npos) return true;
        if (lower.find("сохрани") != std::string::npos) return true;
        if (lower.find("удали") != std::string::npos) return true;
        if (lower.find("повтор") != std::string::npos) return true;
        return false;
    }
    
    void goToFirstTrack() {
        ensureRunning();
        focusSpotify();
        system("start spotify:collection:tracks");
        Sleep(2000);
        keybd_event(VK_HOME, 0, 0, 0);
        Sleep(30);
        keybd_event(VK_HOME, 0, KEYEVENTF_KEYUP, 0);
        Sleep(100);
        keybd_event(VK_RETURN, 0, 0, 0);
        Sleep(30);
        keybd_event(VK_RETURN, 0, KEYEVENTF_KEYUP, 0);
    }
    
    std::string executeCommand(const std::string& command, const std::string& params) {
        std::string lower = command;
        for (char& c : lower) c = tolower(c);
        
        if (!isSpotifyRelated(lower)) return "";
        
        if (lower.find("громк") != std::string::npos || lower.find("тише") != std::string::npos) {
            int percent = parseVolumeText(lower);
            setVolumePercent(percent);
            return "Громкость Spotify установлена на " + std::to_string(percent) + " процентов, сэр.";
        }
        
        if (lower.find("перемешивани") != std::string::npos || lower.find("перемешать") != std::string::npos) {
            ensureRunning();
            focusSpotify();
            bool turnOff = (lower.find("выключи") != std::string::npos || lower.find("отключи") != std::string::npos);
            keybd_event(VK_CONTROL, 0, 0, 0);
            keybd_event('S', 0, 0, 0);
            Sleep(30);
            keybd_event('S', 0, KEYEVENTF_KEYUP, 0);
            keybd_event(VK_CONTROL, 0, KEYEVENTF_KEYUP, 0);
            return turnOff ? "Перемешивание отключено, сэр." : "Перемешивание включено, сэр.";
        }
        
        if (lower.find("первый") != std::string::npos || lower.find("первая") != std::string::npos) {
            goToFirstTrack();
            return "Первый трек, сэр.";
        }
        
        if (lower == "пауза" || lower == "стоп" || lower.find("останови") != std::string::npos) {
            ensureRunning();
            focusSpotify();
            sendMediaKey(0xB3);
            return "Пауза, сэр.";
        }
        
        if (lower.find("продолж") != std::string::npos || lower.find("играй") != std::string::npos) {
            ensureRunning();
            focusSpotify();
            sendMediaKey(0xB3);
            return "Продолжаю воспроизведение, сэр.";
        }
        
        if (lower == "дальше" || lower.find("следующий") != std::string::npos ||
            lower.find("некст") != std::string::npos || lower.find("скип") != std::string::npos) {
            ensureRunning();
            focusSpotify();
            sendMediaKey(0xB0);
            return "Следующий трек, сэр.";
        }
        
        if (lower == "назад" || lower.find("предыдущий") != std::string::npos || lower.find("прошлый") != std::string::npos) {
            ensureRunning();
            focusSpotify();
            sendMediaKey(0xB1);
            return "Предыдущий трек, сэр.";
        }
        
        if (lower.find("открой") != std::string::npos || lower.find("запусти") != std::string::npos || lower.find("включи") != std::string::npos) {
            ensureRunning();
            focusSpotify();
            return "Запускаю Spotify, сэр.";
        }
        
        if (lower.find("любимые") != std::string::npos || lower.find("сохранен") != std::string::npos || lower.find("сохранён") != std::string::npos) {
            ensureRunning();
            focusSpotify();
            system("start spotify:collection:tracks");
            Sleep(1500);
            sendMediaKey(0xB3);
            return "Включаю любимые треки, сэр.";
        }
        
        if (lower.find("поищи") != std::string::npos || lower.find("найди") != std::string::npos) {
            ensureRunning();
            focusSpotify();
            keybd_event(VK_CONTROL, 0, 0, 0);
            keybd_event('L', 0, 0, 0);
            Sleep(30);
            keybd_event('L', 0, KEYEVENTF_KEYUP, 0);
            keybd_event(VK_CONTROL, 0, KEYEVENTF_KEYUP, 0);
            return "Открываю поиск в Spotify, сэр.";
        }
        
        if (lower.find("инфо") != std::string::npos || lower.find("какой трек") != std::string::npos) {
            ensureRunning();
            return "Текущий трек отображается в Spotify, сэр.";
        }
        
        if (lower.find("сохрани") != std::string::npos) {
            ensureRunning();
            focusSpotify();
            keybd_event(VK_CONTROL, 0, 0, 0);
            keybd_event('B', 0, 0, 0);
            Sleep(30);
            keybd_event('B', 0, KEYEVENTF_KEYUP, 0);
            keybd_event(VK_CONTROL, 0, KEYEVENTF_KEYUP, 0);
            return "Трек сохранён в любимые, сэр.";
        }
        
        if (lower.find("повтор") != std::string::npos) {
            ensureRunning();
            focusSpotify();
            bool turnOff = (lower.find("выключи") != std::string::npos || lower.find("отключи") != std::string::npos);
            keybd_event(VK_CONTROL, 0, 0, 0);
            keybd_event('R', 0, 0, 0);
            Sleep(30);
            keybd_event('R', 0, KEYEVENTF_KEYUP, 0);
            keybd_event(VK_CONTROL, 0, KEYEVENTF_KEYUP, 0);
            return turnOff ? "Повтор отключен, сэр." : "Повтор включён, сэр.";
        }
        
        if (lower == "музыка" || lower.find("музыку") != std::string::npos) {
            ensureRunning();
            focusSpotify();
            return "Запускаю Spotify, сэр.";
        }
        
        return "";
    }
};

SpotifyController::SpotifyController() : pImpl(std::make_unique<Impl>()) {}
SpotifyController::~SpotifyController() = default;
bool SpotifyController::initialize() { return true; }
std::string SpotifyController::executeCommand(const std::string& command, const std::string& params) {
    return pImpl->executeCommand(command, params);
}
bool SpotifyController::isAvailable() const { return pImpl->available; }