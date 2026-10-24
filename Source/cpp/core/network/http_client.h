#pragma once
#include <string>
#include <utility>

/** 
 * @brief Утилита для выполнения HTTP-запросов (GET/POST).
 */
class HttpClient {
public:
    /** 
     * @brief Выполняет GET-запрос.
     * @param url Адрес запроса.
     * @param authToken Токен авторизации (опционально).
     * @param timeoutSec Таймаут в секундах.
     * @return Пара: HTTP-код и тело ответа.
     */
    static std::pair<int, std::string> get(const std::string& url, const std::string& authToken = "", long timeoutSec = 10L);

    /** 
     * @brief Выполняет POST-запрос.
     * @param url Адрес запроса.
     * @param body Тело запроса (обычно JSON).
     * @param authToken Токен авторизации (опционально).
     * @param timeoutSec Таймаут в секундах.
     * @return Пара: HTTP-код и тело ответа.
     */
    static std::pair<int, std::string> post(const std::string& url, const std::string& body, const std::string& authToken = "", long timeoutSec = 30L);
};