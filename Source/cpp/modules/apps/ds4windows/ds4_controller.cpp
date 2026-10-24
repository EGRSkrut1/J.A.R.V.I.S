#include "ds4_controller.h"
#include "core/config/app_config.h"
#include "core/utils/process_launcher.h"
#include "core/logging/logger.h"
#include <algorithm>

struct Ds4Controller::Impl {
    bool available = true;

    void launchDs4() {
        std::string path = AppConfig::getPath("ds4");
        if (path.empty()) {
            Logger::error("[Ds4] Path not found in apps.json (key 'ds4')");
            return;
        }
        launchApp(path);
    }
};

Ds4Controller::Ds4Controller() : pImpl(std::make_unique<Impl>()) {}
Ds4Controller::~Ds4Controller() = default;
bool Ds4Controller::initialize() { return true; }

std::string Ds4Controller::executeCommand(const std::string& command, const std::string& params) {
    std::string lower = command;
    for (char& c : lower) c = tolower((unsigned char)c);

    if (lower.find("ds4") != std::string::npos || lower.find("геймпад") != std::string::npos) {
        if (lower.find("открой") != std::string::npos || lower.find("запусти") != std::string::npos) {
            pImpl->launchDs4();
            return "DS4Windows запущен, сэр.";
        }
    }
    return "";
}

bool Ds4Controller::isAvailable() const { return pImpl->available; }