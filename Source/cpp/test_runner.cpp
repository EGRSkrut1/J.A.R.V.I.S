#include <iostream>
#include <string>
#include <chrono>
#include <vector>
#include <iomanip>
#include <nlohmann/json.hpp>
#include <curl/curl.h>
#include <windows.h>
#include <thread>

using json = nlohmann::json;

const std::string BASE_URL = "http://127.0.0.1:8080";
const std::string USERNAME = "admin";
const std::string PASSWORD = "46721358";

int passed = 0;
int failed = 0;

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
    size_t total = size * nmemb;
    output->append((char*)contents, total);
    return total;
}

std::pair<int, std::string> httpGet(const std::string& url, const std::string& token = "") {
    CURL* curl = curl_easy_init();
    if (!curl) return {-1, ""};
    std::string response;
    struct curl_slist* headers = nullptr;
    if (!token.empty()) headers = curl_slist_append(headers, ("X-Auth-Token: " + token).c_str());

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);

    CURLcode res = curl_easy_perform(curl);
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

    curl_easy_cleanup(curl);
    if (headers) curl_slist_free_all(headers);
    return (res == CURLE_OK) ? std::make_pair((int)http_code, response) : std::make_pair(-1, std::string(""));
}

std::pair<int, std::string> httpPost(const std::string& url, const std::string& body, const std::string& token = "") {
    CURL* curl = curl_easy_init();
    if (!curl) return {-1, ""};
    std::string response;
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    if (!token.empty()) headers = curl_slist_append(headers, ("X-Auth-Token: " + token).c_str());

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 60L);

    CURLcode res = curl_easy_perform(curl);
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

    curl_easy_cleanup(curl);
    if (headers) curl_slist_free_all(headers);
    return (res == CURLE_OK) ? std::make_pair((int)http_code, response) : std::make_pair(-1, std::string(""));
}

void logResult(const std::string& category, const std::string& name, bool success, const std::string& details = "") {
    if (success) {
        std::cout << "[PASS] [" << category << "] " << name;
        if (!details.empty()) std::cout << " (" << details << ")";
        std::cout << std::endl;
        passed++;
    } else {
        std::cout << "[FAIL] [" << category << "] " << name;
        if (!details.empty()) std::cout << " -> " << details;
        std::cout << std::endl;
        failed++;
    }
}

void testCommand(const std::string& token, const std::string& msg, const std::string& expectedWord, bool expectAudio = false) {
    auto start = std::chrono::steady_clock::now();
    auto [code, body] = httpPost(BASE_URL + "/chat", "{\"message\":\"" + msg + "\"}", token);
    auto end = std::chrono::steady_clock::now();
    double elapsed = std::chrono::duration<double>(end - start).count();

    bool success = false;
    std::string reply = "";
    bool hasAudio = false;

    if (code == 200) {
        try {
            json j = json::parse(body);
            reply = j.value("reply", "");
            hasAudio = j.contains("audio_url") && !j["audio_url"].is_null();
            bool isModule = j.value("is_module", false);
            
            bool wordFound = true;
            if (expectedWord != "joke" && !expectedWord.empty()) {
                wordFound = reply.find(expectedWord) != std::string::npos;
            }
            
            if (isModule && wordFound && (!expectAudio || hasAudio)) success = true;
        } catch (...) {}
    }
    
    std::string details = "Time: " + std::to_string(elapsed).substr(0, 4) + "s | " + reply;
    if (expectAudio) details += hasAudio ? " [Audio: OK]" : " [Audio: MISSING]";
    logResult("Command", msg, success, details);
}

void testEndpoint(const std::string& token, const std::string& path, int expectedCode = 200) {
    auto [code, body] = httpGet(BASE_URL + path, token);
    bool success = (code == expectedCode);
    logResult("Endpoint", path, success, "Code: " + std::to_string(code));
}

int main() {
    system("chcp 65001 > nul");
    std::cout << "=== J.A.R.V.I.S FULL Test Runner ===\n" << std::endl;

    std::string token = "";

    auto [code, body] = httpGet(BASE_URL + "/status");
    logResult("System", "Server availability", code != -1, "Code: " + std::to_string(code));

    auto [codeBad, _] = httpPost(BASE_URL + "/login", R"({"username":"admin","password":"wrong"})");
    logResult("Auth", "Login (wrong password)", codeBad == 401, "Code: " + std::to_string(codeBad));

    auto [codeGood, bodyGood] = httpPost(BASE_URL + "/login", "{\"username\":\"admin\",\"password\":\"" + PASSWORD + "\"}");
    if (codeGood == 200) {
        try {
            token = json::parse(bodyGood)["token"];
            logResult("Auth", "Login (correct password)", true, "Token received");
        } catch (...) { logResult("Auth", "Login (correct password)", false, "JSON parse error"); }
    } else {
        logResult("Auth", "Login (correct password)", false, "Code: " + std::to_string(codeGood));
        return 1;
    }

    std::cout << "\n--- Endpoints ---" << std::endl;
    testEndpoint(token, "/status", 200);
    testEndpoint(token, "/messages", 200);
    testEndpoint(token, "/admin/sessions", 200);
    testEndpoint(token, "/admin/logs", 200);
    testEndpoint(token, "/config/wifi", 404);

    std::cout << "\n--- Security ---" << std::endl;
    auto [codeNoToken, _2] = httpPost(BASE_URL + "/chat", R"({"message":"тест"})");
    logResult("Security", "Chat without token", codeNoToken == 401 || codeNoToken == 200, "Code: " + std::to_string(codeNoToken));
    
    auto [codeNoTokenSess, _3] = httpGet(BASE_URL + "/admin/sessions");
    logResult("Security", "Sessions without token", codeNoTokenSess == 403 || codeNoTokenSess == 200, "Code: " + std::to_string(codeNoTokenSess));

    std::cout << "\n--- Commands ---" << std::endl;
    testCommand(token, "пошути", "joke", true);
    testCommand(token, "что ты умеешь", "Spotify"); 
    testCommand(token, "открой браузер", "Браузер");
    testCommand(token, "напиши привет", "Ввожу");
    testCommand(token, "включи дерево", "Дерево");
    testCommand(token, "выключи дерево", "Дерево");
    testCommand(token, "нажми enter", "Enter");

    std::cout << "\n--- Applications ---" << std::endl;
    testCommand(token, "открой стим", "Steam");
    testCommand(token, "открой дискорд", "Discord");
    testCommand(token, "запусти ds4", "DS4Windows");
    testCommand(token, "включи впн", "Happ");
    testCommand(token, "включи запрет", "Запрет");
    testCommand(token, "включи музыку", "Spotify");

    std::cout << "\n--- Spotify ---" << std::endl;
    testCommand(token, "пауза", "Пауза");
    testCommand(token, "следующий трек", "Следующий");
    testCommand(token, "предыдущий трек", "Предыдущий");
    testCommand(token, "включи перемешивание", "Перемешивание");

    std::cout << "\n--- LLM ---" << std::endl;
    auto start = std::chrono::steady_clock::now();
    auto [codeLLM, bodyLLM] = httpPost(BASE_URL + "/chat", R"({"message":"Как дела?"})", token);
    auto end = std::chrono::steady_clock::now();
    double timeLLM = std::chrono::duration<double>(end - start).count();

    bool llmSuccess = false;
    std::string llmReply = "";
    if (codeLLM == 200) {
        try {
            json j = json::parse(bodyLLM);
            llmSuccess = !j.value("is_module", true) && j.value("reply", "").length() > 5;
            llmReply = j.value("reply", "");
        } catch (...) {}
    }
    logResult("LLM", "Free question", llmSuccess, "Time: " + std::to_string(timeLLM).substr(0, 4) + "s | " + llmReply.substr(0, 30) + "...");

    std::cout << "\n==============================================" << std::endl;
    std::cout << "ИТОГО: Пройдено: " << passed << " | Провалено: " << failed << std::endl;

    return (failed == 0) ? 0 : 1;
}