#pragma once
#include <string>

class ISpeechEngine {
public:
    virtual ~ISpeechEngine() = default;
    virtual bool speak(const std::string& text) = 0;
    virtual bool isSpeaking() const = 0;
    virtual void stop() = 0;
};