#pragma once
#include <string>

class VoiceLogger {
public:
    static void userSpeech(const std::string& text);
    static void jarvisResponse(const std::string& text);
    static void activation(const std::string& text);
    static void error(const std::string& msg);
};