#include "websocket_server.h"
#include "core/logging/logger.h"

extern std::string g_status;

void startWebSocketServer() {
    Logger::warn("WebSocket server temporarily disabled");
}

void stopWebSocketServer() {}

void broadcastStatus(const std::string& status) {
    g_status = status;
}

void broadcastUserMessage(const std::string& text) {}

void broadcastJarvisMessage(const std::string& text) {}

void broadcastError(const std::string& error) {}    