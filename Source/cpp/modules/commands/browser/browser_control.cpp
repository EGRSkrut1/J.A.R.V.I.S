#include "browser_control.h"
#include <windows.h>
#include <string>

struct BrowserControl::Impl {
    void switchToBrowser() {
        std::string psCmd = "powershell -command \"(New-Object -ComObject WScript.Shell).AppActivate('Opera')\"";
        system(psCmd.c_str());
    }
    
    void openNewTab() {
        switchToBrowser();
        Sleep(300);
        keybd_event(VK_CONTROL, 0, 0, 0);
        keybd_event('T', 0, 0, 0);
        Sleep(30);
        keybd_event('T', 0, KEYEVENTF_KEYUP, 0);
        keybd_event(VK_CONTROL, 0, KEYEVENTF_KEYUP, 0);
    }
    
    void openUrl(const std::string& url) {
        std::string cmd = "start \"\" " + url;
        system(cmd.c_str());
    }
    
    void launchBrowser() {
        system("start https://google.com");
        Sleep(3000);
    }
};

BrowserControl::BrowserControl() : pImpl(std::make_unique<Impl>()) {}
BrowserControl::~BrowserControl() = default;

std::vector<std::string> BrowserControl::getTriggers() const {
    return {"браузер", "интернет", "открой браузер", "новая вкладка", "гугл", "найди", "загугли", "поищи"};
}

std::string BrowserControl::execute(const std::string& userInput) {
    std::string lower = userInput;
    for (char& c : lower) c = tolower(c);
    
    if (lower.find("найди") != std::string::npos || lower.find("загугли") != std::string::npos ||
        lower.find("поищи") != std::string::npos) {
        
        std::string query;
        if (lower.find("найди") != std::string::npos) {
            query = lower.substr(lower.find("найди") + 5);
        } else if (lower.find("загугли") != std::string::npos) {
            query = lower.substr(lower.find("загугли") + 7);
        } else {
            query = lower.substr(lower.find("поищи") + 5);
        }
        while (!query.empty() && query[0] == ' ') query = query.substr(1);
        
        if (!query.empty()) {
            std::string url = "https://www.google.com/search?q=" + query;
            pImpl->openUrl(url);
            return "Ищу «" + query + "» в Google, сэр.";
        }
    }
    
    if (lower.find("новая вкладка") != std::string::npos || lower.find("вкладка") != std::string::npos) {
        pImpl->openNewTab();
        return "Новая вкладка открыта, сэр.";
    }
    
    pImpl->launchBrowser();
    return "Браузер открыт, сэр.";
}

bool BrowserControl::isAvailable() const { return true; }