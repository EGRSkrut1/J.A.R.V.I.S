#pragma once
#include "features/speech/interfaces/i_speech_engine.h"
#include <memory>
#include <string>
#include <chrono>

class VoxCPMTTS : public ISpeechEngine {
public:
    VoxCPMTTS();
    ~VoxCPMTTS();
    
    bool speak(const std::string& text) override;
    bool isSpeaking() const override;
    void stop() override;
    
    void setServerUrl(const std::string& url);
    void setTimeout(std::chrono::seconds timeout);
    
private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;
};