#pragma once
#include "modules/interfaces/i_command.h"
#include <string>
#include <vector>
#include <memory>

class DeviceControl : public ICommand {
public:
    DeviceControl();
    ~DeviceControl();

    std::vector<std::string> getTriggers() const override;
    std::string execute(const std::string& userInput) override;
    bool isAvailable() const override;

    std::string executeWithContext(const std::string& userInput, const std::string& terminalId);

private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;
};