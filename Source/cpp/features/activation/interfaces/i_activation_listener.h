#pragma once
#include <string>

class IActivationListener {
public:
    virtual ~IActivationListener() = default;
    virtual bool waitForWakeWord(std::string& wakeWord) = 0;
    virtual std::string listenForCommand(int timeoutSec) = 0;
    virtual bool isConnected() const = 0;
};