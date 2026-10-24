#pragma once
#include <string>
#include <filesystem>
#include <cstdlib>
#include <windows.h>

/**
 * @namespace Paths
 * @brief Пространство имен для управления путями файловой системы проекта.
 */
namespace Paths {
    /**
     * @brief Возвращает корневую директорию проекта J.A.R.V.I.S.
     * 
     * Определяет путь на основе расположения исполняемого файла (jarvis_client.exe).
     * Поднимается на 3 уровня вверх: bin -> App -> Корень проекта.
     * 
     * @return std::string Абсолютный путь к корню проекта.
     */
    inline std::string getBaseDir() {
        static std::string base = []() {
            const char* env = std::getenv("JARVIS_BASE");
            if (env) return std::string(env);
            
            wchar_t buf[MAX_PATH];
            DWORD n = GetModuleFileNameW(NULL, buf, MAX_PATH);
            if (n == 0) {
                return std::filesystem::current_path().string();
            }
            
            std::filesystem::path exePath(buf);
            std::filesystem::path basePath = exePath.parent_path().parent_path().parent_path();
            return basePath.string();
        }();
        return base;
    }

    /**
     * @brief Возвращает путь к директории App.
     * @return std::string
     */
    inline std::string getAppDir()       { return getBaseDir() + "/App"; }
    
    /**
     * @brief Возвращает путь к директории конфигураций.
     * @return std::string
     */
    inline std::string getConfigsDir()   { return getAppDir() + "/configs"; }
    
    /**
     * @brief Возвращает путь к директории логов.
     * @return std::string
     */
    inline std::string getLogsDir()      { return getAppDir() + "/logs"; }
    
    /**
     * @brief Возвращает путь к директории с бинарными файлами.
     * @return std::string
     */
    inline std::string getBinDir()       { return getAppDir() + "/bin"; }
    
    /**
     * @brief Возвращает путь к директории временных файлов.
     * @return std::string
     */
    inline std::string getTempDir()      { return getAppDir() + "/temp"; }
    
    /**
     * @brief Возвращает путь к директории ресурсов.
     * @return std::string
     */
    inline std::string getResourcesDir() { return getBaseDir() + "/Resources"; }
    
    /**
     * @brief Возвращает путь к директории звуков.
     * @return std::string
     */
    inline std::string getSoundsDir()    { return getResourcesDir() + "/sounds"; }
    
    /**
     * @brief Возвращает путь к директории сервисов.
     * @return std::string
     */
    inline std::string getServicesDir()  { return getBaseDir() + "/Services"; }
    
    /**
     * @brief Возвращает путь к директории VoxCPM2.
     * @return std::string
     */
    inline std::string getVoxCPMDir()    { return getServicesDir() + "/TTS/VoxCPM2"; }
    
    /**
     * @brief Возвращает путь к директории Vosk.
     * @return std::string
     */
    inline std::string getVoskDir()      { return getServicesDir() + "/Vosk"; }
    
    /**
     * @brief Возвращает путь к директории исходников.
     * @return std::string
     */
    inline std::string getSourceDir()    { return getBaseDir() + "/Source/cpp"; }
    
    /**
     * @brief Возвращает путь к файлу jokes.json.
     * @return std::string
     */
    inline std::string getJokesJson()    { return getConfigsDir() + "/jokes.json"; }

    /**
     * @brief Возвращает путь к ярлыку Spotify.
     * @return std::string
     */
    inline std::string getSpotifyLink() { return getResourcesDir() + "\\Spotify.lnk"; }

    /**
     * @brief Ищет исполняемый файл Spotify в стандартных директориях.
     * @return std::string Путь к Spotify.exe или пустая строка, если не найден.
     */
    inline std::string getSpotifyExe() {
        std::error_code ec;
        if (const char* appdata = std::getenv("APPDATA")) {
            std::string p = std::string(appdata) + "\\Spotify\\Spotify.exe";
            if (std::filesystem::exists(p, ec)) return p;
        }
        if (const char* localappdata = std::getenv("LOCALAPPDATA")) {
            std::string p = std::string(localappdata) + "\\Microsoft\\WindowsApps\\Spotify.exe";
            if (std::filesystem::exists(p, ec)) return p;
        }
        if (const char* programfiles = std::getenv("ProgramFiles")) {
            std::string p = std::string(programfiles) + "\\Spotify\\Spotify.exe";
            if (std::filesystem::exists(p, ec)) return p;
        }
        return "";
    }
}