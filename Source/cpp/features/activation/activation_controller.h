#pragma once
#include <string>
#include <memory>

class IActivationListener;

class ActivationController {
public:
    ActivationController();
    ~ActivationController();
    
    bool waitForCommand(std::string& recognizedText);
    void setListener(std::shared_ptr<IActivationListener> listener);
    
private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;
};