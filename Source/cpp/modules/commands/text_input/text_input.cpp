#include "text_input.h"
#include "core/logging/logger.h"
#include <windows.h>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>

struct TextInput::Impl {
    
    void typeText(const std::string& text) {
        std::wstring wideText;
        int len = MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, NULL, 0);
        wideText.resize(len);
        MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, &wideText[0], len);
        
        if (!wideText.empty() && wideText.back() == L'\0')
            wideText.pop_back();
        
        for (wchar_t ch : wideText) {
            if (ch < 0x20 && ch != L'\r' && ch != L'\n') continue;
            
            if (ch == L'\n') {
                // Enter
                keybd_event(VK_RETURN, 0, 0, 0);
                keybd_event(VK_RETURN, 0, KEYEVENTF_KEYUP, 0);
                Sleep(20);
                continue;
            }
            
            SHORT vk = VkKeyScanW(ch);
            if (vk == -1) {
                continue;
            }
            
            BYTE vkCode = LOBYTE(vk);
            BYTE shiftState = HIBYTE(vk);
            
            if (shiftState & 1) keybd_event(VK_SHIFT, 0, 0, 0);      // Shift
            if (shiftState & 2) keybd_event(VK_CONTROL, 0, 0, 0);    // Ctrl
            if (shiftState & 4) keybd_event(VK_MENU, 0, 0, 0);        // Alt
            
            keybd_event(vkCode, 0, 0, 0);
            keybd_event(vkCode, 0, KEYEVENTF_KEYUP, 0);
            
            if (shiftState & 4) keybd_event(VK_MENU, 0, KEYEVENTF_KEYUP, 0);
            if (shiftState & 2) keybd_event(VK_CONTROL, 0, KEYEVENTF_KEYUP, 0);
            if (shiftState & 1) keybd_event(VK_SHIFT, 0, KEYEVENTF_KEYUP, 0);
            
            Sleep(10);
        }
    }
    
    void pressEnter() {
        keybd_event(VK_RETURN, 0, 0, 0);
        keybd_event(VK_RETURN, 0, KEYEVENTF_KEYUP, 0);
    }
    
    void pressTab() {
        keybd_event(VK_TAB, 0, 0, 0);
        keybd_event(VK_TAB, 0, KEYEVENTF_KEYUP, 0);
    }
    
    std::string extractTextAfterCommand(const std::string& userInput, const std::string& command) {
        std::string lowerInput = userInput;
        std::string lowerCmd = command;
        for (char& c : lowerInput) c = tolower(c);
        for (char& c : lowerCmd) c = tolower(c);
        
        size_t pos = lowerInput.find(lowerCmd);
        if (pos == std::string::npos) return "";
        
        std::string result = userInput.substr(pos + command.length());
        while (!result.empty() && result[0] == ' ') result.erase(0, 1);
        return result;
    }
};

TextInput::TextInput() : pImpl(std::make_unique<Impl>()) {}
TextInput::~TextInput() = default;

std::vector<std::string> TextInput::getTriggers() const {
    return {"напиши", "напечатай", "введи", "enter", "энтер"};
}

std::string TextInput::execute(const std::string& userInput) {
    std::string lower = userInput;
    for (char& c : lower) c = tolower(c);
    
    if (lower == "enter" || lower == "энтер" || lower.find("нажми enter") != std::string::npos) {
        pImpl->pressEnter();
        return "Enter нажат, сэр.";
    }
    
    if (lower.find("tab") != std::string::npos || lower.find("таб") != std::string::npos) {
        pImpl->pressTab();
        return "Tab нажат, сэр.";
    }
    
    std::string text = pImpl->extractTextAfterCommand(userInput, "напиши");
    if (text.empty()) text = pImpl->extractTextAfterCommand(userInput, "напечатай");
    if (text.empty()) text = pImpl->extractTextAfterCommand(userInput, "введи");
    
    if (!text.empty()) {
        if (text == "стоп" || text == "отмена") {
            return "Команда ввода отменена, сэр.";
        }
        
        pImpl->typeText(text);
        return "Ввожу: " + text;
    }
    
    return "";
}

bool TextInput::isAvailable() const { return true; }