#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <vector>
#include <string>
#include <iomanip>
#include <ctime>

using json = nlohmann::json;

std::string g_log;

void log(const std::string& msg) {
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    std::string timestamp = std::ctime(&time_t_now);
    timestamp.pop_back(); // убрать \n
    
    std::string entry = "[" + timestamp + "] " + msg;
    std::cout << entry << std::endl;
    g_log += entry + "\n";
}

size_t writeCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
    size_t total = size * nmemb;
    output->append((char*)contents, total);
    return total;
}

std::string httpPost(const std::string& url, const std::string& body) {
    CURL* curl = curl_easy_init();
    if (!curl) return "";
    
    std::string response;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    
    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);
    
    if (res != CURLE_OK) return "";
    return response;
}

std::string httpGet(const std::string& url) {
    CURL* curl = curl_easy_init();
    if (!curl) return "";
    
    std::string response;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    
    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    
    if (res != CURLE_OK) return "";
    return response;
}

double measureResponseTime(const std::string& message) {
    json req;
    req["message"] = message;
    
    auto start = std::chrono::steady_clock::now();
    std::string response = httpPost("http://localhost:8080/chat", req.dump());
    auto end = std::chrono::steady_clock::now();
    
    double elapsed = std::chrono::duration<double>(end - start).count();
    
    if (!response.empty()) {
        try {
            json resp = json::parse(response);
            log("Ответ: " + resp.value("reply", ""));
        } catch (...) {}
    }
    
    return elapsed;
}

int main() {
    log("JARVIS TEST RUNNER");
    log("Начало тестирования...");
    
    // 1. Проверка доступности сервера
    log("1. Проверка HTTP сервера...");
    std::string status = httpGet("http://localhost:8080/status");
    if (status.empty()) {
        log("ОШИБКА: HTTP сервер не отвечает. Запустите JARVIS сначала.");
        std::ofstream file("test.log");
        file << g_log;
        return 1;
    }
    log("HTTP сервер OK");
    
    // 2. Тест времени ответа LLM
    log("\n2. Тест времени ответа LLM:");
    std::vector<std::pair<std::string, double>> llmTests = {
        {"Как дела?", 0},
        {"Расскажи шутку", 0},
        {"Что ты умеешь?", 0},
        {"Сколько времени?", 0}
    };
    
    double totalLlm = 0;
    for (auto& test : llmTests) {
        log("  Запрос: " + test.first);
        double elapsed = measureResponseTime(test.first);
        test.second = elapsed;
        totalLlm += elapsed;
        log("    Время: " + std::to_string(elapsed) + " с");
    }
    log("  Среднее время LLM: " + std::to_string(totalLlm / llmTests.size()) + " с");
    
    // 3. Тест команд
    log("\n3. Тест команд:");
    std::vector<std::pair<std::string, double>> cmdTests = {
        {"пошути", 0},
        {"музыка", 0},
        {"пауза", 0},
        {"браузер", 0}
    };
    
    double totalCmd = 0;
    for (auto& test : cmdTests) {
        log("  Команда: " + test.first);
        double elapsed = measureResponseTime(test.first);
        test.second = elapsed;
        totalCmd += elapsed;
        log("    Время: " + std::to_string(elapsed) + " с");
    }
    log("  Среднее время команд: " + std::to_string(totalCmd / cmdTests.size()) + " с");
    
    // 4. Тест стабильности
    log("\n4. Тест стабильности (10 запросов 'пошути'):");
    std::vector<double> stabilityTimes;
    for (int i = 0; i < 10; i++) {
        double elapsed = measureResponseTime("пошути");
        stabilityTimes.push_back(elapsed);
        log("  " + std::to_string(i+1) + ": " + std::to_string(elapsed) + " с");
    }
    
    double avg = 0;
    double max = 0;
    double min = 100;
    for (double t : stabilityTimes) {
        avg += t;
        if (t > max) max = t;
        if (t < min) min = t;
    }
    avg /= stabilityTimes.size();
    log("  Среднее: " + std::to_string(avg) + " с");
    log("  Минимум: " + std::to_string(min) + " с");
    log("  Максимум: " + std::to_string(max) + " с");
    
    // 5. Сохранение лога
    log("\nТЕСТ ЗАВЕРШЁН");
    
    std::ofstream file("test.log");
    file << g_log;
    log("Результаты сохранены в test.log");

}