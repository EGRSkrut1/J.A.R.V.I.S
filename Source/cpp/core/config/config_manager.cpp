#include "config_manager.h"
#include <fstream>
#include <sstream>
#include <vector>

struct ConfigManager::Impl {
    std::string systemPrompt;
    std::string systemsInfo;
    std::string soundsMapping;
    
    static std::string readFile(const std::string& path) {
        std::ifstream file(path);
        if (!file.is_open()) return "";
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }
    
    static std::string parseSystemsInfo(const std::string& jsonStr) {
        std::string result = "ДОСТУПНЫЕ СИСТЕМЫ:\n";
        size_t pos = 0;
        while ((pos = jsonStr.find("\"name\":", pos)) != std::string::npos) {
            size_t nameStart = jsonStr.find("\"", pos + 7) + 1;
            size_t nameEnd = jsonStr.find("\"", nameStart);
            std::string name = jsonStr.substr(nameStart, nameEnd - nameStart);
            
            size_t availPos = jsonStr.find("\"available\":", nameEnd);
            bool available = (jsonStr.find("true", availPos) < jsonStr.find("false", availPos) && 
                             jsonStr.find("true", availPos) != std::string::npos);
            
            size_t cmdStart = jsonStr.find("\"commands\":", availPos);
            size_t cmdArrayStart = jsonStr.find("[", cmdStart);
            size_t cmdArrayEnd = jsonStr.find("]", cmdArrayStart);
            
            std::vector<std::string> commands;
            size_t cmdPos = cmdArrayStart;
            while ((cmdPos = jsonStr.find("\"", cmdPos)) != std::string::npos && cmdPos < cmdArrayEnd) {
                size_t cmdEnd = jsonStr.find("\"", cmdPos + 1);
                if (cmdEnd == std::string::npos || cmdEnd > cmdArrayEnd) break;
                commands.push_back(jsonStr.substr(cmdPos + 1, cmdEnd - cmdPos - 1));
                cmdPos = cmdEnd + 1;
            }
            
            result += "- " + name + " (" + (available ? "доступно" : "недоступно") + ")";
            if (available && !commands.empty()) {
                result += ". Команды: ";
                for (size_t i = 0; i < commands.size(); ++i) {
                    if (i > 0) result += ", ";
                    result += "\"" + commands[i] + "\"";
                }
            }
            result += ".\n";
            pos = cmdArrayEnd;
        }
        return result;
    }
    
    void loadAll() {
        const std::string configDir = "C:\\Users\\egrsk\\Desktop\\jarvis\\App\\configs\\";
        
        systemPrompt = readFile(configDir + "system_prompt.txt");
        systemsInfo = parseSystemsInfo(readFile(configDir + "systems.json"));
        soundsMapping = readFile(configDir + "sounds_mapping.txt");
        
        size_t placeholder = systemPrompt.find("{systems_info}");
        if (placeholder != std::string::npos) {
            systemPrompt.replace(placeholder, 14, systemsInfo);
        }
    }
};

ConfigManager::ConfigManager() : pImpl(std::make_unique<Impl>()) {}
ConfigManager::~ConfigManager() = default;
void ConfigManager::loadAll() { pImpl->loadAll(); }
std::string ConfigManager::getSystemPrompt() const { return pImpl->systemPrompt; }
std::string ConfigManager::getSystemsInfo() const { return pImpl->systemsInfo; }
std::string ConfigManager::getSoundsMapping() const { return pImpl->soundsMapping; }