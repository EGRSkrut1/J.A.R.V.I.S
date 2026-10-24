#include "module_manager.h"
#include "core/config/paths.h"
#include "core/logging/logger.h"
#include <algorithm>
#include <windows.h>
#include <random>
#include <ctime>

struct ModuleManager::Impl {
    std::vector<std::shared_ptr<ICommand>> commands;
    std::vector<std::shared_ptr<IAppController>> apps;

    std::vector<std::string> confirmSounds = {
        "ok\\ok1.wav", "ok\\ok2.wav", "ok\\ok3.wav", "ok\\ok4.wav",
        "ok\\reply1.wav", "ok\\reply2.wav", "ok\\reply3.wav", "ok\\reply4.wav",
        "ok\\reply5.wav", "ok\\reply6.wav", "ok\\Да сэр.wav", "ok\\Есть.wav",
        "ok\\Загружаю сэр.wav", "ok\\Запрос выполнен сэр.wav",
        "ok\\К вашим услугам сэр.wav", "ok\\Как пожелаете .wav"
    };
    std::mt19937 rng;
    std::string lastConfirmSound;

    Impl() : rng(std::time(nullptr)) {}

    void playConfirmSound() {
        if (confirmSounds.empty()) return;
        std::uniform_int_distribution<size_t> dist(0, confirmSounds.size() - 1);
        lastConfirmSound = confirmSounds[dist(rng)];
    }

    void registerCommand(std::shared_ptr<ICommand> cmd) { commands.push_back(cmd); }
    void registerApp(std::shared_ptr<IAppController> app) { apps.push_back(app); }

    std::string handleCommand(const std::string& userInput) {
        std::string lower = userInput;
        for (char& c : lower) c = tolower((unsigned char)c);
        Logger::info("[ModuleManager] handleCommand: " + userInput);

        for (auto& app : apps) {
            if (!app->isAvailable()) continue;
            std::string result = app->executeCommand(lower, "");
            if (!result.empty()) {
                Logger::info("[ModuleManager] matched app");
                playConfirmSound();
                return result;
            }
        }

        for (auto& cmd : commands) {
            if (!cmd->isAvailable()) continue;
            for (const auto& trigger : cmd->getTriggers()) {
                if (lower.find(trigger) != std::string::npos) {
                    Logger::info("[ModuleManager] matched trigger: " + trigger);
                    std::string result = cmd->execute(userInput);
                    if (!result.empty()) {
                        playConfirmSound();
                        return result;
                    }
                }
            }
        }

        Logger::info("[ModuleManager] no match -> LLM");
        return "";
    }
};

ModuleManager::ModuleManager() : pImpl(std::make_unique<Impl>()) {}
ModuleManager::~ModuleManager() = default;
void ModuleManager::registerCommand(std::shared_ptr<ICommand> cmd) { pImpl->registerCommand(cmd); }
void ModuleManager::registerApp(std::shared_ptr<IAppController> app) { pImpl->registerApp(app); }
std::string ModuleManager::handleCommand(const std::string& userInput) { return pImpl->handleCommand(userInput); }
std::string ModuleManager::getLastConfirmSound() const { return pImpl->lastConfirmSound; }