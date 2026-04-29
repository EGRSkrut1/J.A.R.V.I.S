#include "context_manager.h"
#include <deque>

struct ContextManager::Impl {
    std::deque<std::pair<std::string, std::string>> history;
    const size_t maxSize = 10;
    
    void addUserMessage(const std::string& text) {
        history.push_back({"user", text});
        if (history.size() > maxSize) history.pop_front();
    }
    
    void addAssistantMessage(const std::string& text) {
        if (!history.empty() && history.back().first == "user") {
            history.back().second = text;
        }
    }
    
    std::string getHistoryText() const {
        std::string result;
        for (const auto& [role, content] : history) {
            result += (role == "user" ? "Пользователь: " : "Джарвис: ") + content + "\n";
        }
        return result;
    }
};

ContextManager::ContextManager() : pImpl(std::make_unique<Impl>()) {}
ContextManager::~ContextManager() = default;
void ContextManager::addUserMessage(const std::string& text) { pImpl->addUserMessage(text); }
void ContextManager::addAssistantMessage(const std::string& text) { pImpl->addAssistantMessage(text); }
std::string ContextManager::getHistoryText() const { return pImpl->getHistoryText(); }
void ContextManager::clear() { pImpl->history.clear(); }
size_t ContextManager::size() const { return pImpl->history.size(); }