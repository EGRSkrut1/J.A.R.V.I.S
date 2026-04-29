#pragma once
#include <string>
#include <vector>
#include <memory>

class ContextManager {
public:
    ContextManager();
    ~ContextManager();
    
    void addUserMessage(const std::string& text);
    void addAssistantMessage(const std::string& text);
    std::string getHistoryText() const;
    void clear();
    size_t size() const;
    
private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;
};  