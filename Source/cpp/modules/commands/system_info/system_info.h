#pragma once
#include "modules/interfaces/i_command.h"
#include <string>
#include <vector>
#include <memory>

class SystemInfo : public ICommand {
public:
    SystemInfo();
    ~SystemInfo();
    
    std::vector<std::string> getTriggers() const override;
    std::string execute(const std::string& userInput) override;
    bool isAvailable() const override;
    void setAvailableCommands(const std::string& commands);
    
private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;
};