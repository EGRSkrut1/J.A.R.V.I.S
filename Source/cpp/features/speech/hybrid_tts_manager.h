#pragma once
#include "features/speech/interfaces/i_speech_engine.h"
#include <memory>
#include <chrono>

class HybridTTSManager : public ISpeechEngine {
public:
    HybridTTSManager();
    ~HybridTTSManager();
    
    bool speak(const std::string& text) override;
    bool isSpeaking() const override;
    void stop() override;
    
    void setPrimaryEngine(std::shared_ptr<ISpeechEngine> engine);
    void setFallbackEngine(std::shared_ptr<ISpeechEngine> engine);
    void setTimeout(std::chrono::seconds timeout);
    
private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;
};  