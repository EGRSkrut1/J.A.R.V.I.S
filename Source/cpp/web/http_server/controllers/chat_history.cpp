#include "chat_history.h"

std::mutex g_historyMtx;
std::map<std::string, std::deque<std::string>> g_userHistories;
const int MAX_HISTORY_SIZE = 50;

void addChatMessage(const std::string& user, const std::string& message) {
    std::lock_guard<std::mutex> lock(g_historyMtx);
    g_userHistories[user].push_back(message);
    if (g_userHistories[user].size() > MAX_HISTORY_SIZE) {
        g_userHistories[user].pop_front();
    }
}

std::deque<std::string> getChatMessages(const std::string& user) {
    std::lock_guard<std::mutex> lock(g_historyMtx);
    return g_userHistories[user];
}