#include "voice_logger.h"
#include "core/config/paths.h"
#include <fstream>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <iostream>

static std::string getLogPath() {
    return Paths::getLogsDir() + "/voice.log";
}

void VoiceLogger::userSpeech(const std::string& text) {
    static std::ofstream file(getLogPath(), std::ios::app);
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%H:%M:%S");
    
    file << "[" << ss.str() << "] [VOICE] Вы: " << text << std::endl;
    file.flush();
    std::cout << "[" << ss.str() << "] [VOICE] Вы: " << text << std::endl;
}

void VoiceLogger::jarvisResponse(const std::string& text) {
    static std::ofstream file(getLogPath(), std::ios::app);
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%H:%M:%S");
    
    file << "[" << ss.str() << "] [VOICE] Джарвис: " << text << std::endl;
    file.flush();
    std::cout << "[" << ss.str() << "] [VOICE] Джарвис: " << text << std::endl;
}

void VoiceLogger::activation(const std::string& text) {
    static std::ofstream file(getLogPath(), std::ios::app);
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%H:%M:%S");
    
    file << "[" << ss.str() << "] [VOICE] Wake: " << text << std::endl;
    file.flush();
    std::cout << "[" << ss.str() << "] [VOICE] Wake: " << text << std::endl;
}

void VoiceLogger::error(const std::string& msg) {
    static std::ofstream file(getLogPath(), std::ios::app);
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%H:%M:%S");
    
    file << "[" << ss.str() << "] [VOICE ERROR] " << msg << std::endl;
    file.flush();
    std::cout << "[" << ss.str() << "] [VOICE ERROR] " << msg << std::endl;
}