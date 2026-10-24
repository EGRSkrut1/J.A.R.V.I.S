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

    // Сгенерировать .wav в указанный файл (для отдачи клиенту через /tts/<id>.wav)
    bool synthesizeToFile(const std::string& text, const std::string& outPath);

    void setServerUrl(const std::string& url);
    void setTimeout(std::chrono::seconds timeout);

private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;
};