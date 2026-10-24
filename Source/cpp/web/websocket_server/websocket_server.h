#pragma once
#include <string>
#include <memory>

/** @brief Запускает WebSocket сервер для связи с терминалами (ESP). */
void startWebSocketServer();

/** @brief Останавливает WebSocket сервер. */
void stopWebSocketServer();

/** @brief Отправляет сообщение конкретному устройству по его ID. */
void sendToDevice(const std::string& deviceId, const std::string& message);

/** @brief Обновляет статус системы (idle/processing/speaking). */
void broadcastStatus(const std::string& status);

void broadcastUserMessage(const std::string& text);
void broadcastJarvisMessage(const std::string& text);
void broadcastError(const std::string& error);