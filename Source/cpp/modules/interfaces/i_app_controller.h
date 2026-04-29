#pragma once
#include <string>

class IAppController {
public:
    virtual ~IAppController() = default;
    virtual bool initialize() = 0;
    virtual std::string executeCommand(const std::string& command, const std::string& params) = 0;
    virtual bool isAvailable() const = 0;
};