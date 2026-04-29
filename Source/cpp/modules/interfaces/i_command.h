#pragma once
#include <string>
#include <vector>

class ICommand {
public:
    virtual ~ICommand() = default;
    virtual std::vector<std::string> getTriggers() const = 0;
    virtual std::string execute(const std::string& userInput) = 0;
    virtual bool isAvailable() const = 0;
};