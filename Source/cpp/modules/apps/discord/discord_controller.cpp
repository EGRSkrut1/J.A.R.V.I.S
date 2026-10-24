#include "discord_controller.h"
#include "core/config/app_config.h"
#include "core/logging/logger.h"
#include <windows.h>
#include <algorithm>

struct DiscordController::Impl {
    bool available = true;

    void launchDiscord() {
        std::string path = AppConfig::getPath("discord");
        if (path.empty()) path = "discord://";
        std::string cmd = "start \"\" \"" + path + "\"";
        system(cmd.c_str());
    }
};

DiscordController::DiscordController() : pImpl(std::make_unique<Impl>()) {}
DiscordController::~DiscordController() = default;
bool DiscordController::initialize() { return true; }

std::string DiscordController::executeCommand(const std::string& command, const std::string& params) {
    std::string lower = command;
    for (char& c : lower) c = tolower((unsigned char)c);

    if (lower.find("дискорд") != std::string::npos || lower.find("discord") != std::string::npos) {
        if (lower.find("открой") != std::string::npos || lower.find("запусти") != std::string::npos || lower.find("включи") != std::string::npos) {
            pImpl->launchDiscord();
            return "Discord запущен, сэр.";
        }
    }
    return "";
}

bool DiscordController::isAvailable() const { return pImpl->available; }