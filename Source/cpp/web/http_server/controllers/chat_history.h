#pragma once
#include <string>
#include <deque>
#include <map>
#include <mutex>

extern std::mutex g_historyMtx;
extern std::map<std::string, std::deque<std::string>> g_userHistories;
extern const int MAX_HISTORY_SIZE;

void addChatMessage(const std::string& user, const std::string& message);
std::deque<std::string> getChatMessages(const std::string& user);