#pragma once
#include <string>
#include <windows.h>
#include "core/logging/logger.h"

/**
 * @brief Запускает внешнее приложение или файл по указанному пути.
 * 
 * Если путь заканчивается на .bat или .cmd, файл запускается через cmd.exe с автоматическим подтверждением.
 * В остальных случаях используется ShellExecuteW.
 * 
 * @param path Путь к исполняемому файлу, батнику или документу.
 */
inline void launchApp(const std::string& path) {
    if (path.empty()) return;

    int len = MultiByteToWideChar(CP_UTF8, 0, path.c_str(), -1, NULL, 0);
    if (len <= 0) {
        Logger::error("[Launcher] Failed to convert path to UTF-16: " + path);
        return;
    }
    std::wstring wpath(len, 0);
    MultiByteToWideChar(CP_UTF8, 0, path.c_str(), -1, &wpath[0], len);

    bool isBatch = false;
    if (wpath.size() > 4) {
        std::wstring ext = wpath.substr(wpath.size() - 4);
        for (auto& c : ext) c = towlower(c);
        if (ext == L".bat" || ext == L".cmd") {
            isBatch = true;
        }
    }

    HINSTANCE result;
    if (isBatch) {
        // Эхо Y | cmd /c "path" - чтобы автоматически принять подтверждение (Y/N) в батнике
        std::wstring cmdLine = L"/c \"echo Y | \"" + wpath + L"\"\"";
        result = ShellExecuteW(NULL, L"open", L"cmd.exe", cmdLine.c_str(), NULL, SW_SHOWNORMAL);
    } else {
        result = ShellExecuteW(NULL, L"open", wpath.c_str(), NULL, NULL, SW_SHOWNORMAL);
    }

    if ((INT_PTR)result <= 32) {
        Logger::error("[Launcher] Failed to launch: " + path + " (Error code: " + std::to_string((INT_PTR)result) + ")");
    } else {
        Logger::success("[Launcher] Launched: " + path);
    }
}