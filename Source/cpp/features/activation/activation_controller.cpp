#include "activation_controller.h"
#include "features/activation/interfaces/i_activation_listener.h"
#include "features/activation/recognizers/vosk_recognizer.h"
#include "core/logging/logger.h"
#include "core/logging/voice_logger.h"
#include <iostream>
#include <windows.h>
#include <chrono>
#include <vector>

struct ActivationController::Impl {
    std::shared_ptr<IActivationListener> listener;
    std::string lastWakeWord;
    std::chrono::steady_clock::time_point lastWakeTime;
    
    Impl() {
        auto vosk = std::make_shared<VoskRecognizer>();
        if (vosk->connect()) {
            listener = vosk;
            Logger::success("Vosk recognizer connected");
        }
    }
    
    static std::string cleanText(const std::string& input) {
        std::string s = input;
        size_t start = s.find_first_not_of(" \t\n\r-—.?!*");
        if (start != std::string::npos) s = s.substr(start);
        while (!s.empty() && (s.back() == '.' || s.back() == '?' || s.back() == '!' || s.back() == ' '))
            s.pop_back();
        return s;
    }
    
    static bool containsJarvis(const std::string& text) {
        std::string lower = text;
        for (char& c : lower) c = tolower(c);
        return (lower.find("джарвис") != std::string::npos ||
                lower.find("жарвис") != std::string::npos ||
                lower.find("jarvis") != std::string::npos ||
                lower.find("джорвис") != std::string::npos);
    }
    
    static std::string removeWakeWord(const std::string& text) {
        std::string lower = text;
        for (char& c : lower) c = tolower(c);
        std::vector<std::string> wakeWords = {"джарвис", "жарвис", "джорвис", "jarvis"};
        for (const auto& w : wakeWords) {
            size_t pos = lower.find(w);
            if (pos != std::string::npos) {
                return text.substr(pos + w.length());
            }
        }
        return text;
    }
    
    bool waitForCommand(std::string& recognizedText) {
        if (!listener || !listener->isConnected()) return false;
        
        Logger::info("Waiting for wake word...");
        
        while (true) {
            std::string wakeWord;
            if (listener->waitForWakeWord(wakeWord)) {
                auto now = std::chrono::steady_clock::now();
                auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - lastWakeTime).count();
                
                if (wakeWord == lastWakeWord && elapsed < 5) {
                    continue;
                }
                
                lastWakeWord = wakeWord;
                lastWakeTime = now;
                
                std::cout << "\n>>> ДЖАРВИС АКТИВИРОВАН <<<" << std::endl;
                VoiceLogger::activation(wakeWord);
                Beep(600, 300);
                
                std::string command = removeWakeWord(wakeWord);
                command = cleanText(command);
                
                if (command.empty() || containsJarvis(command)) {
                    Logger::info("Wake word only, listening for command...");
                    command = listener->listenForCommand(7);
                }
                
                command = cleanText(command);
                
                if (command.empty() || containsJarvis(command)) {
                    Logger::info("Empty command, waiting...");
                    continue;
                }
                
                recognizedText = command;
                Beep(1000, 150);
                Beep(1200, 150);
                
                lastWakeWord = "";
                lastWakeTime = std::chrono::steady_clock::now();
                
                return true;
            }
        }
    }
};

ActivationController::ActivationController() : pImpl(std::make_unique<Impl>()) {}
ActivationController::~ActivationController() = default;
bool ActivationController::waitForCommand(std::string& recognizedText) { return pImpl->waitForCommand(recognizedText); }