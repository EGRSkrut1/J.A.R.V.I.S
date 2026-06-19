#include "module_manager.h"
#include "core/config/paths.h"
#include "core/logging/logger.h"
#include <algorithm>
#include <windows.h>
#include <random>
#include <ctime>
#include <fstream>

struct ModuleManager::Impl {
    std::vector<std::shared_ptr<ICommand>> commands;
    std::vector<std::shared_ptr<IAppController>> apps;
    bool isPlaying = false;
    
    std::vector<std::string> confirmSounds = {
        "ok\\ok1.wav",
        "ok\\ok2.wav",
        "ok\\ok3.wav",
        "ok\\ok4.wav",
        "ok\\reply1.wav",
        "ok\\reply2.wav",
        "ok\\reply3.wav",
        "ok\\reply4.wav",
        "ok\\reply5.wav",
        "ok\\reply6.wav",
        "ok\\Да сэр.wav",
        "ok\\Есть.wav",
        "ok\\Загружаю сэр.wav",
        "ok\\Запрос выполнен сэр.wav",
        "ok\\К вашим услугам сэр.wav",
        "ok\\Как пожелаете .wav"
    };
    std::mt19937 rng;
    
    Impl() : rng(std::time(nullptr)) {}
    
    void playConfirmSound() {
        if (confirmSounds.empty()) return;
        std::uniform_int_distribution<size_t> dist(0, confirmSounds.size() - 1);
        std::string ffplay = Paths::getVoxCPMDir() + "/ffmpeg/ffplay.exe";
        std::string path = Paths::getSoundsDir() + "/" + confirmSounds[dist(rng)];
        std::string cmd = "start /B \"" + ffplay + "\" -autoexit -nodisp -loglevel quiet \"" + path + "\"";
        system(cmd.c_str());
    }
    
    void registerCommand(std::shared_ptr<ICommand> cmd) { commands.push_back(cmd); }
    void registerApp(std::shared_ptr<IAppController> app) { apps.push_back(app); }
    
    std::string handleCommand(const std::string& userInput) {
        if (isPlaying) return "";
        
        std::string lower = userInput;
        for (char& c : lower) c = tolower(c);
        
        for (auto& app : apps) {
            if (!app->isAvailable()) continue;
            std::string result = app->executeCommand(lower, "");
            if (!result.empty()) {
                playConfirmSound();
                return result;
            }
        }
        
        for (auto& cmd : commands) {
            if (!cmd->isAvailable()) continue;
            for (const auto& trigger : cmd->getTriggers()) {
                if (lower.find(trigger) != std::string::npos) {
                    bool isJoke = (trigger == "пошути" || trigger == "анекдот" || trigger == "шутка" || 
                                  trigger == "расскажи шутку" || trigger == "рассмеши");
                    
                    if (!isJoke) isPlaying = true;
                    
                    std::string result = cmd->execute(userInput);
                    
                    if (!isJoke) {
                        playConfirmSound();
                        Sleep(2000);
                        isPlaying = false;
                    }
                    
                    return result;
                }
            }
        }
        
        return "";
    }
};

ModuleManager::ModuleManager() : pImpl(std::make_unique<Impl>()) {}
ModuleManager::~ModuleManager() = default;
void ModuleManager::registerCommand(std::shared_ptr<ICommand> cmd) { pImpl->registerCommand(cmd); }
void ModuleManager::registerApp(std::shared_ptr<IAppController> app) { pImpl->registerApp(app); }
std::string ModuleManager::handleCommand(const std::string& userInput) { return pImpl->handleCommand(userInput); }