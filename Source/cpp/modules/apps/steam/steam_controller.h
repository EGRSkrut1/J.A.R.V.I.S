#pragma once
#include "modules/interfaces/i_app_controller.h"
#include <string>
#include <vector>
#include <memory>

class SteamController : public IAppController {
public:
    SteamController();
    ~SteamController();

    bool initialize() override;
    std::string executeCommand(const std::string& command, const std::string& params) override;
    bool isAvailable() const override;

private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;
};