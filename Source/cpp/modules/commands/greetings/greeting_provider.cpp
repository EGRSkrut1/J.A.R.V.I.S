#include "greeting_provider.h"
#include "core/logging/logger.h"
#include <random>
#include <ctime>
#include <windows.h>
#include <fstream>

struct GreetingProvider::Impl {
    std::vector<std::string> audioFiles = {
        "Джарвис - приветствие.wav"
    };
    
    std::vector<std::string> textResponses = {
        "Сэр, я готов к работе. Говорите или введите сообщение."
    };
    
    mutable std::mt19937 rng;
    
    Impl() : rng(std::time(nullptr)) {}
    
    std::string getRandomText() const {
        std::uniform_int_distribution<size_t> dist(0, textResponses.size() - 1);
        return textResponses[dist(rng)];
    }
    
    void playRandomAudio() const {
        std::uniform_int_distribution<size_t> dist(0, audioFiles.size() - 1);
        std::string path = "C:\\Users\\egrsk\\Desktop\\Jarvis\\Resources\\sounds\\" + audioFiles[dist(rng)];
        
        std::ifstream test(path);
        if (!test.good()) return;
        test.close();
        
        std::string cmdLine = "C:\\Users\\egrsk\\Desktop\\Jarvis\\Services\\TTS\\VoxCPM2\\ffmpeg\\ffplay.exe -autoexit -nodisp -loglevel quiet \"" + path + "\"";
        
        STARTUPINFO si = { sizeof(STARTUPINFO) };
        PROCESS_INFORMATION pi;
        ZeroMemory(&pi, sizeof(pi));
        
        char* cmd = new char[cmdLine.length() + 1];
        strcpy(cmd, cmdLine.c_str());
        
        if (CreateProcess(NULL, cmd, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
        }
        delete[] cmd;
    }
};

GreetingProvider::GreetingProvider() : pImpl(std::make_unique<Impl>()) {}
GreetingProvider::~GreetingProvider() = default;

std::vector<std::string> GreetingProvider::getTriggers() const {
    return {};
}

std::string GreetingProvider::execute(const std::string& userInput) {
    pImpl->playRandomAudio();
    return pImpl->getRandomText();
}

bool GreetingProvider::isAvailable() const { return true; }

void GreetingProvider::playRandomGreeting() const {
    pImpl->playRandomAudio();
}