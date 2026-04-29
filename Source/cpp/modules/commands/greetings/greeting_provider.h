#pragma once
#include "modules/interfaces/i_command.h"
#include <string>
#include <vector>
#include <memory>

class GreetingProvider : public ICommand {
public:
    GreetingProvider();
    ~GreetingProvider();
    
    std::vector<std::string> getTriggers() const override;
    std::string execute(const std::string& userInput) override;
    bool isAvailable() const override;
    void playRandomGreeting() const;
    
private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;
};