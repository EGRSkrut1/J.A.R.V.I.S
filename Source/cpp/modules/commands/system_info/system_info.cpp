#include "system_info.h"
#include "core/logging/logger.h"

struct SystemInfo::Impl {
    std::string getInfo() {
        return "Сэр, доступные команды:\n"
               "- «пошути» — рассказать случайную шутку\n"
               "- «открой музыку» или «запусти спотифай» — открыть Spotify\n"
               "- «любимые треки» — открыть сохранённые треки\n"
               "- «пауза» / «дальше» — управление музыкой\n"
               "- «какие системы работают» — список систем\n"
               "- и любые вопросы — я отвечу с помощью нейросети.";
    }
};

SystemInfo::SystemInfo() : pImpl(std::make_unique<Impl>()) {}
SystemInfo::~SystemInfo() = default;

std::vector<std::string> SystemInfo::getTriggers() const {
    return {"доступные команды", "что ты умеешь", "список команд", "какие команды", "команды"};
}

std::string SystemInfo::execute(const std::string& userInput) {
    return pImpl->getInfo();
}

bool SystemInfo::isAvailable() const { return true; }