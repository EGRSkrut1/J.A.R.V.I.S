#pragma once
#include "modules/interfaces/i_command.h"
#include "modules/interfaces/i_app_controller.h"
#include <memory>
#include <vector>
#include <string>

class ModuleManager {
public:
    ModuleManager();
    ~ModuleManager();
    
    void registerCommand(std::shared_ptr<ICommand> cmd);
    void registerApp(std::shared_ptr<IAppController> app);
    std::string handleCommand(const std::string& userInput);
    
private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;
};