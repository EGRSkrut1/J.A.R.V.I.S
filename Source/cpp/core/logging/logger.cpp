#include "logger.h"

void Logger::info(const std::string& msg) {
    std::cout << "[INFO] " << msg << std::endl;
}

void Logger::warn(const std::string& msg) {
    std::cout << "[WARN] " << msg << std::endl;
}

void Logger::error(const std::string& msg) {
    std::cout << "[ERROR] " << msg << std::endl;
}

void Logger::success(const std::string& msg) {
    std::cout << "[SUCCESS] " << msg << std::endl;
}

void Logger::debug(const std::string& msg) {
    std::cout << "[DEBUG] " << msg << std::endl;
}