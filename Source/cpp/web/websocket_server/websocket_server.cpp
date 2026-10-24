#include "websocket_server.h"
#include "core/logging/logger.h"
#include <ixwebsocket/IXWebSocketServer.h>
#include <mutex>
#include <map>
#include <thread>
#include <chrono>

extern std::string g_status;

std::unique_ptr<ix::WebSocketServer> g_wsServer;
std::map<std::string, std::shared_ptr<ix::WebSocket>> g_connectedDevices;
std::mutex g_wsMtx;

void startWebSocketServer() {
    g_wsServer = std::make_unique<ix::WebSocketServer>(8081, "0.0.0.0");
    
    g_wsServer->setOnClientMessageCallback(
        [](std::shared_ptr<ix::ConnectionState> connectionState, 
           ix::WebSocket& webSocket, 
           const ix::WebSocketMessagePtr& msg) 
        {
            if (msg->type == ix::WebSocketMessageType::Message) {
                std::string payload = msg->str;
                Logger::info("[WS] Received from " + connectionState->getRemoteIp() + ": " + payload);
                
                if (payload.rfind("ID:", 0) == 0) {
                    std::string deviceId = payload.substr(3);
                    std::lock_guard<std::mutex> lock(g_wsMtx);
                    g_connectedDevices[deviceId] = std::make_shared<ix::WebSocket>(webSocket);
                    Logger::success("[WS] Device registered: " + deviceId);
                }
            }
            else if (msg->type == ix::WebSocketMessageType::Open) {
                Logger::info("[WS] New connection from " + connectionState->getRemoteIp());
            }
            else if (msg->type == ix::WebSocketMessageType::Close) {
                Logger::info("[WS] Connection closed");
            }
        }
    );

    g_wsServer->listen();
    g_wsServer->start();
    
    std::thread([]() {
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }).detach();
    
    Logger::success("WebSocket server started on port 8081");
}

void stopWebSocketServer() {
    if (g_wsServer) {
        g_wsServer->stop();
        Logger::info("WebSocket server stopped");
    }
}

void sendToDevice(const std::string& deviceId, const std::string& message) {
    std::lock_guard<std::mutex> lock(g_wsMtx);
    if (g_connectedDevices.find(deviceId) != g_connectedDevices.end()) {
        auto ws = g_connectedDevices[deviceId];
        if (ws->getReadyState() == ix::ReadyState::Open) {
            ws->send(message);
            Logger::info("[WS] Sent to " + deviceId + ": " + message);
        }
    } else {
        Logger::warn("[WS] Device not connected: " + deviceId);
    }
}

void broadcastStatus(const std::string& status) {
    g_status = status;
}

void broadcastUserMessage(const std::string& text) {}
void broadcastJarvisMessage(const std::string& text) {}
void broadcastError(const std::string& error) {}