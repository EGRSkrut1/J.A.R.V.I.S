#pragma once
#include "modules/interfaces/i_app_controller.h"
#include <string>
#include <vector>
#include <memory>

class Ds4Controller : public IAppController {
public:
    Ds4Controller();
    ~Ds4Controller();
    bool initialize() override;
    std::string executeCommand(const std::string& command, const std::string& params) override;
    bool isAvailable() const override;
private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;
};