#pragma once
#include <string>
#include <memory>

class ConfigManager {
public:
    ConfigManager();
    ~ConfigManager();
    
    void loadAll();
    
    std::string getSystemPrompt() const;
    std::string getSystemsInfo() const;
    std::string getSoundsMapping() const;
    
private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;
};