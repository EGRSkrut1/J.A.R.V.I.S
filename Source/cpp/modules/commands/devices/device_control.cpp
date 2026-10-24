#include "device_control.h"
#include "core/config/paths.h"
#include "core/logging/logger.h"
#include <curl/curl.h>
#include <algorithm>
#include <fstream>
#include <nlohmann/json.hpp>
#include <regex>

using json = nlohmann::json;

struct Device {
    std::string id;
    std::string name;
    std::string room;
    std::string ip;
    std::vector<std::string> triggers_on;
    std::vector<std::string> triggers_off;
};

struct Terminal {
    std::string id;
    std::string room;
};

struct DeviceControl::Impl {
    std::vector<Device> devices;
    std::vector<Terminal> terminals;
    std::vector<std::string> allTriggers;

    static size_t writeCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
        size_t total = size * nmemb;
        output->append((char*)contents, total);
        return total;
    }

    void loadDevices() {
        std::string path = Paths::getConfigsDir() + "/devices.json";
        std::ifstream file(path);
        if (!file.is_open()) {
            Logger::error("[DeviceControl] devices.json not found at " + path);
            return;
        }

        try {
            json data = json::parse(file);
            for (const auto& item : data["devices"]) {
                Device dev;
                dev.id = item.value("id", "");
                dev.name = item.value("name", "");
                dev.room = item.value("room", "");
                dev.ip = item.value("ip", "");
                for (const auto& t : item["triggers_on"]) dev.triggers_on.push_back(t.get<std::string>());
                for (const auto& t : item["triggers_off"]) dev.triggers_off.push_back(t.get<std::string>());
                devices.push_back(dev);
                for (const auto& t : dev.triggers_on) allTriggers.push_back(t);
                for (const auto& t : dev.triggers_off) allTriggers.push_back(t);
            }
            
            if (data.contains("terminals")) {
                for (const auto& item : data["terminals"]) {
                    Terminal term;
                    term.id = item.value("id", "");
                    term.room = item.value("room", "");
                    terminals.push_back(term);
                }
            }
            Logger::info("[DeviceControl] Loaded " + std::to_string(devices.size()) + " devices");
        } catch (const std::exception& e) {
            Logger::error("[DeviceControl] JSON parse error: " + std::string(e.what()));
        }
    }

    std::string getRoomByTerminalId(const std::string& terminalId) {
        if (terminalId.empty()) return "";
        for (const auto& t : terminals) {
            if (t.id == terminalId) return t.room;
        }
        return "";
    }

    std::string extractUser(const std::string& input) {
        std::regex user_regex(R"(\[Пользователь:\s*([^,\]]+))");
        std::smatch match;
        if (std::regex_search(input, match, user_regex) && match.size() > 1) {
            return match[1].str();
        }
        return "unknown";
    }

    std::string sendHttp(const std::string& ip, const std::string& endpoint, bool turnOn, const std::string& user) {
        CURL* curl = curl_easy_init();
        if (!curl) {
            Logger::error("[DeviceControl] Failed to init curl");
            return "";
        }

        std::string url = "http://" + ip + endpoint;
        std::string response;
        
        Logger::info("[DeviceControl] Sending request: " + url + " (" + (turnOn ? "ON" : "OFF") + ") from [" + user + "]");

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        
        CURLcode res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        if (res == CURLE_OK) {
            Logger::success("[DeviceControl] ESP response: " + response);
        } else {
            Logger::error("[DeviceControl] CURL error: " + std::string(curl_easy_strerror(res)));
        }

        return response;
    }

    std::string execute(const std::string& input, const std::string& terminalId) {
        std::string lower = input;
        for (char& c : lower) c = tolower((unsigned char)c);

        std::string user = extractUser(input);
        std::string roomContext = getRoomByTerminalId(terminalId);

        for (const auto& dev : devices) {
            bool roomMatches = roomContext.empty() || dev.room == roomContext || lower.find(dev.room) != std::string::npos;
            if (!roomMatches) continue;

            for (const auto& t : dev.triggers_off) {
                if (lower.find(t) != std::string::npos) {
                    sendHttp(dev.ip, "/tree/off", false, user);
                    return dev.name + " выключено, сэр.";
                }
            }
            for (const auto& t : dev.triggers_on) {
                if (lower.find(t) != std::string::npos) {
                    sendHttp(dev.ip, "/tree/on", true, user);
                    return dev.name + " включено, сэр.";
                }
            }
        }
        return "";
    }
};

DeviceControl::DeviceControl() : pImpl(std::make_unique<Impl>()) { pImpl->loadDevices(); }
DeviceControl::~DeviceControl() = default;

std::vector<std::string> DeviceControl::getTriggers() const { return pImpl->allTriggers; }

std::string DeviceControl::execute(const std::string& userInput) { 
    return pImpl->execute(userInput, ""); 
}

std::string DeviceControl::executeWithContext(const std::string& userInput, const std::string& terminalId) {
    return pImpl->execute(userInput, terminalId);
}

bool DeviceControl::isAvailable() const { return !pImpl->devices.empty(); }