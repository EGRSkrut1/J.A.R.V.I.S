#include "ollama_client.h"
#include "core/logging/logger.h"
#include "core/logging/voice_logger.h"
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <fstream>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <ctime>

using json = nlohmann::json;

struct OllamaClient::Impl {
    std::string model = "llama3.1:8b";
    std::string systemPrompt;
    std::string systemsInfo;
    std::vector<std::pair<std::string, std::string>> history;
    const int maxHistory = 10;
    
    static size_t writeCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
        size_t total = size * nmemb;
        output->append((char*)contents, total);
        return total;
    }
    
    std::string getCurrentDateTime() {
        auto now = std::chrono::system_clock::now();
        auto time_t_now = std::chrono::system_clock::to_time_t(now);
        std::tm tm_now;
        localtime_s(&tm_now, &time_t_now);
        
        std::stringstream ss;
        ss << std::put_time(&tm_now, "%d %B %Y года, %H:%M");
        
        std::string result = ss.str();
        const std::string monthsEn[] = {"January", "February", "March", "April", "May", "June",
                                        "July", "August", "September", "October", "November", "December"};
        const std::string monthsRu[] = {"января", "февраля", "марта", "апреля", "мая", "июня",
                                        "июля", "августа", "сентября", "октября", "ноября", "декабря"};
        
        for (int i = 0; i < 12; ++i) {
            size_t pos = result.find(monthsEn[i]);
            if (pos != std::string::npos) {
                result.replace(pos, monthsEn[i].length(), monthsRu[i]);
                break;
            }
        }
        return result;
    }
    
    std::string buildFullPrompt(const std::string& userPrompt) {
        std::string historyText;
        for (const auto& [role, content] : history) {
            historyText += (role == "user" ? "Пользователь: " : "Джарвис: ") + content + "\n";
        }
        
        std::string fullPrompt = systemPrompt;
        std::string dynamicContext = "\n\nТЕКУЩАЯ ДАТА И ВРЕМЯ:\nСейчас " + getCurrentDateTime() + ".\n";
        fullPrompt += dynamicContext;
        fullPrompt += "\nИСТОРИЯ ДИАЛОГА:\n" + historyText;
        fullPrompt += "Джарвис, ответь на последнее сообщение пользователя.";
        
        return fullPrompt;
    }
    
    std::string ask(const std::string& prompt) {
        Logger::debug("Ollama: sending request...");
        
        history.push_back({"user", prompt});
        if (history.size() > maxHistory) history.erase(history.begin());
        
        CURL* curl = curl_easy_init();
        if (!curl) return "ERROR: curl init failed";
        
        json request;
        request["model"] = model;
        request["prompt"] = buildFullPrompt(prompt);
        request["stream"] = false;
        request["options"]["temperature"] = 0.5;
        
        std::string jsonStr = request.dump();
        std::string response;
        
        curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:11434/api/generate");
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonStr.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        
        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        
        CURLcode res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
        
        if (res != CURLE_OK) return "ERROR: curl failed";
        
        try {
            json resp = json::parse(response);
            std::string answer = resp.value("response", "ERROR");
            history.push_back({"assistant", answer});
            Logger::debug("Ollama: response received (" + std::to_string(answer.length()) + " chars)");
            return answer;
        } catch (...) {
            return "ERROR: failed to parse JSON";
        }
    }
};

OllamaClient::OllamaClient() : pImpl(std::make_unique<Impl>()) {}
OllamaClient::~OllamaClient() = default;

std::string OllamaClient::ask(const std::string& prompt) { return pImpl->ask(prompt); }
void OllamaClient::setModel(const std::string& model) { pImpl->model = model; }
void OllamaClient::setSystemPrompt(const std::string& prompt) { pImpl->systemPrompt = prompt; }
void OllamaClient::setSystemsInfo(const std::string& info) { pImpl->systemsInfo = info; }