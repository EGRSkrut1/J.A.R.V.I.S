#pragma once
#include <string>
#include <memory>

/**
 * @brief Менеджер конфигурации системы.
 * 
 * Загружает системный промпт, список систем и маппинг звуков из файлов
 * в папке App/configs.
 */
class ConfigManager {
public:
    ConfigManager();
    ~ConfigManager();
    
    /**
     * @brief Загружает все конфиги с диска.
     */
    void loadAll();
    
    std::string getSystemPrompt() const;
    std::string getSystemsInfo() const;
    std::string getSoundsMapping() const;
    
private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;
};