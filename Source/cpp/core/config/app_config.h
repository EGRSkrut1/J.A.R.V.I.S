#pragma once
#include <string>
#include <fstream>
#include <nlohmann/json.hpp>
#include "core/config/paths.h"
#include "core/logging/logger.h"

/**
 * @class AppConfig
 * @brief Класс для чтения настроек приложений и API-ключей из JSON-конфигураций.
 */
class AppConfig {
public:
    /**
     * @brief Возвращает путь к приложению по его идентификатору из apps.json.
     * @param appId Идентификатор приложения (например, "ds4", "zapret").
     * @return std::string Путь к приложению или пустая строка, если не найдено.
     */
    static std::string getPath(const std::string& appId) {
        std::string path = Paths::getConfigsDir() + "/apps.json";
        std::ifstream file(path);
        if (!file.is_open()) {
            Logger::error("[AppConfig] Failed to open: " + path);
            return "";
        }
        try {
            nlohmann::json j;
            file >> j;
            if (j.contains(appId)) return j[appId].get<std::string>();
            Logger::warn("[AppConfig] Key not found in apps.json: " + appId);
        } catch (const std::exception& e) {
            Logger::error("[AppConfig] JSON parse error in apps.json: " + std::string(e.what()));
        }
        return "";
    }

    /**
     * @brief Возвращает настройку Steam из steam_api.json.
     * @param key Ключ настройки (например, "api_key").
     * @return std::string Значение настройки или пустая строка.
     */
    static std::string getSteamSetting(const std::string& key) {
        std::string path = Paths::getConfigsDir() + "/steam_api.json";
        std::ifstream file(path);
        if (!file.is_open()) return "";
        try {
            nlohmann::json j;
            file >> j;
            if (j.contains(key)) return j[key].get<std::string>();
        } catch (...) {}
        return "";
    }

    /**
     * @brief Возвращает настройку Spotify из spotify_api.json.
     * @param key Ключ настройки (например, "client_id").
     * @return std::string Значение настройки или пустая строка.
     */
    static std::string getSpotifySetting(const std::string& key) {
        std::string path = Paths::getConfigsDir() + "/spotify_api.json";
        std::ifstream file(path);
        if (!file.is_open()) return "";
        try {
            nlohmann::json j;
            file >> j;
            if (j.contains(key)) return j[key].get<std::string>();
        } catch (...) {}
        return "";
    }
};