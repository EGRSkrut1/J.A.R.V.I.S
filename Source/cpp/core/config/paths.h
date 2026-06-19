#pragma once
#include <string>
#include <filesystem>

namespace Paths {
    inline std::string getBaseDir() {
        static std::string base = []() {
            const char* env = std::getenv("JARVIS_BASE");
            if (env) return std::string(env);
            std::filesystem::path exePath = std::filesystem::current_path();
            return exePath.string();
        }();
        return base;
    }

    inline std::string getAppDir() {
        return getBaseDir() + "/App";
    }

    inline std::string getConfigsDir() {
        return getAppDir() + "/configs";
    }

    inline std::string getLogsDir() {
        return getAppDir() + "/logs";
    }

    inline std::string getBinDir() {
        return getAppDir() + "/bin";
    }

    inline std::string getResourcesDir() {
        return getBaseDir() + "/Resources";
    }

    inline std::string getSoundsDir() {
        return getResourcesDir() + "/sounds";
    }

    inline std::string getServicesDir() {
        return getBaseDir() + "/Services";
    }

    inline std::string getVoxCPMDir() {
        return getServicesDir() + "/TTS/VoxCPM2";
    }

    inline std::string getVoskDir() {
        return getServicesDir() + "/Vosk";
    }

    inline std::string getSourceDir() {
        return getBaseDir() + "/Source/cpp";
    }

    inline std::string getJokesJson() {
        return getSourceDir() + "/modules/commands/jokes/jokes.json";
    }
}