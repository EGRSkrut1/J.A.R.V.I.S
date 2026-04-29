#pragma once
#include "modules/interfaces/i_command.h"
#include <string>
#include <vector>
#include <memory>

class JokeProvider : public ICommand {
public:
    JokeProvider();
    ~JokeProvider();
    
    std::vector<std::string> getTriggers() const override;
    std::string execute(const std::string& userInput) override;
    bool isAvailable() const override;
    
    bool loadFromFile(const std::string& path);
    
private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;
};