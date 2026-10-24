#include "logger.h"
#include "core/config/paths.h"
#include <fstream>

static void writeToFile(const std::string& msg) {
    static std::ofstream logFile(Paths::getLogsDir() + "/console.log", std::ios::app);
    if (logFile.is_open()) {
        logFile << msg << std::endl;
    }
}

void Logger::info(const std::string& msg) {
    std::cout << "[INFO] " << msg << std::endl;
    writeToFile("[INFO] " + msg);
}

void Logger::warn(const std::string& msg) {
    std::cout << "[WARN] " << msg << std::endl;
    writeToFile("[WARN] " + msg);
}

void Logger::error(const std::string& msg) {
    std::cout << "[ERROR] " << msg << std::endl;
    writeToFile("[ERROR] " + msg);
}

void Logger::success(const std::string& msg) {
    std::cout << "[SUCCESS] " << msg << std::endl;
    writeToFile("[SUCCESS] " + msg);
}

void Logger::debug(const std::string& msg) {
    std::cout << "[DEBUG] " << msg << std::endl;
}