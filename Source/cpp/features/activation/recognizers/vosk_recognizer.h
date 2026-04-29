#pragma once
#include "features/activation/interfaces/i_activation_listener.h"
#include <winsock2.h>
#include <windows.h>
#include <memory>

class VoskRecognizer : public IActivationListener {
public:
    VoskRecognizer();
    ~VoskRecognizer();
    
    bool connect();
    bool isConnected() const override;
    bool waitForWakeWord(std::string& wakeWord) override;
    std::string listenForCommand(int timeoutSec) override;
    
private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;
};