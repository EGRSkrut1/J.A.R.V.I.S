#pragma once
#include <string>

void startWebSocketServer();
void stopWebSocketServer();

void broadcastStatus(const std::string& status);
void broadcastUserMessage(const std::string& text);
void broadcastJarvisMessage(const std::string& text);
void broadcastError(const std::string& error);