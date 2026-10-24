#include "zapret_controller.h"
#include "core/config/app_config.h"
#include "core/utils/process_launcher.h"
#include "core/logging/logger.h"
#include <algorithm>

struct ZapretController::Impl {
    bool available = true;

    void launchZapret() {
        std::string path = AppConfig::getPath("zapret");
        if (path.empty()) {
            Logger::error("[Zapret] Path not found in apps.json (key 'zapret')");
            return;
        }
        launchApp(path);
    }
};

ZapretController::ZapretController() : pImpl(std::make_unique<Impl>()) {}
ZapretController::~ZapretController() = default;
bool ZapretController::initialize() { return true; }

std::string ZapretController::executeCommand(const std::string& command, const std::string& params) {
    std::string lower = command;
    for (char& c : lower) c = tolower((unsigned char)c);

    if (lower.find("запрет") != std::string::npos || lower.find("zapret") != std::string::npos) {
        if (lower.find("открой") != std::string::npos || lower.find("запусти") != std::string::npos || lower.find("включи") != std::string::npos) {
            pImpl->launchZapret();
            return "Запрет запущен, сэр.";
        }
    }
    return "";
}

bool ZapretController::isAvailable() const { return pImpl->available; }