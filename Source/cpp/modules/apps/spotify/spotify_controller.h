#pragma once
#include "modules/interfaces/i_app_controller.h"
#include <memory>
#include <string>

class SpotifyController : public IAppController {
public:
    SpotifyController();
    ~SpotifyController();
    
    bool initialize() override;
    std::string executeCommand(const std::string& command, const std::string& params) override;
    bool isAvailable() const override;
    
private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;
};