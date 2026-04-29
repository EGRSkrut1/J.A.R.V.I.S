#pragma once
#include "modules/interfaces/i_command.h"
#include <string>
#include <vector>
#include <memory>

class BrowserControl : public ICommand {
public:
    BrowserControl();
    ~BrowserControl();
    
    std::vector<std::string> getTriggers() const override;
    std::string execute(const std::string& userInput) override;
    bool isAvailable() const override;
    
private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;
};