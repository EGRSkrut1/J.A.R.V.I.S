#pragma once
#include "features/speech/interfaces/i_speech_engine.h"
#include <memory>

class WindowsTTS : public ISpeechEngine {
public:
    WindowsTTS();
    ~WindowsTTS();
    
    bool speak(const std::string& text) override;
    bool isSpeaking() const override;
    void stop() override;
    
private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;
};