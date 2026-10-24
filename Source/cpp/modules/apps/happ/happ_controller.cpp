#include "happ_controller.h"
#include "core/config/app_config.h"
#include "core/logging/logger.h"
#include <windows.h>
#include <algorithm>

struct HappController::Impl {
    bool available = true;

    void launchHapp() {
        std::string path = AppConfig::getPath("happ");
        if (path.empty()) return;
        std::string cmd = "start \"\" \"" + path + "\"";
        system(cmd.c_str());
    }

    void killHapp() {
        system("taskkill /F /IM Happ.exe > nul 2>&1");
    }
};

HappController::HappController() : pImpl(std::make_unique<Impl>()) {}
HappController::~HappController() = default;
bool HappController::initialize() { return true; }

std::string HappController::executeCommand(const std::string& command, const std::string& params) {
    std::string lower = command;
    for (char& c : lower) c = tolower((unsigned char)c);

    if (lower.find("happ") != std::string::npos || lower.find("впн") != std::string::npos || lower.find("хапп") != std::string::npos) {
        if (lower.find("выключи") != std::string::npos || lower.find("закрой") != std::string::npos) {
            pImpl->killHapp();
            return "Happ VPN выключен, сэр.";
        }
        if (lower.find("открой") != std::string::npos || lower.find("запусти") != std::string::npos || lower.find("включи") != std::string::npos) {
            pImpl->launchHapp();
            return "Happ VPN запущен, сэр.";
        }
    }
    return "";
}

bool HappController::isAvailable() const { return pImpl->available; }