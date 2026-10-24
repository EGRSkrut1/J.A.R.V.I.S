#include "steam_controller.h"
#include "core/config/app_config.h"
#include "core/logging/logger.h"
#include <windows.h>
#include <algorithm>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <map>
#include <vector>
#include <cctype>

using json = nlohmann::json;

struct SteamGame {
    std::string name;
    std::string appid;
    std::string name_lower;
};

struct SteamController::Impl {
    bool available = true;
    std::map<std::string, SteamGame> library;

    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
        size_t total = size * nmemb;
        output->append((char*)contents, total);
        return total;
    }

    std::string translit(const std::string& text) {
        std::map<std::string, std::string> tr = {
            {"а","a"},{"б","b"},{"в","v"},{"г","g"},{"д","d"},{"е","e"},{"ё","e"},{"ж","zh"},{"з","z"},
            {"и","i"},{"й","y"},{"к","k"},{"л","l"},{"м","m"},{"н","n"},{"о","o"},{"п","p"},{"р","r"},
            {"с","s"},{"т","t"},{"у","u"},{"ф","f"},{"х","h"},{"ц","ts"},{"ч","ch"},{"ш","sh"},{"щ","sch"},
            {"ъ",""},{"ы","y"},{"ь",""},{"э","e"},{"ю","yu"},{"я","ya"}
        };
        std::string result = "";
        for (char c : text) {
            std::string s(1, tolower((unsigned char)c));
            if (tr.count(s)) result += tr[s];
            else result += s;
        }
        return result;
    }

    std::string normalize(const std::string& text) {
        std::string res;
        for (char c : text) {
            if (isalnum((unsigned char)c) || c == ' ') res += tolower((unsigned char)c);
            else if (c == '-' || c == '_') res += ' ';
        }
        return res;
    }

    void fetchGames() {
        std::string apiKey = AppConfig::getSteamSetting("api_key");
        std::string steamId = AppConfig::getSteamSetting("steam_id");

        if (apiKey.empty() || steamId.empty()) {
            Logger::error("[Steam] API key or Steam ID not found in steam_api.json.");
            available = false;
            return;
        }

        Logger::info("[Steam] Fetching games from Steam API...");
        CURL* curl = curl_easy_init();
        if (!curl) return;

        std::string url = "https://api.steampowered.com/IPlayerService/GetOwnedGames/v1/?key=" 
                        + apiKey + "&steamid=" + steamId + "&include_appinfo=true&include_played_free_games=true";
        
        std::string response;
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);

        CURLcode res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        if (res != CURLE_OK) {
            Logger::error("[Steam] Failed to fetch games from API.");
            available = false;
            return;
        }

        try {
            json j = json::parse(response);
            if (j["response"].contains("games") && !j["response"]["games"].empty()) {
                for (const auto& game : j["response"]["games"]) {
                    SteamGame sg;
                    sg.appid = std::to_string(game["appid"].get<int>());
                    sg.name = game["name"].get<std::string>();
                    sg.name_lower = normalize(sg.name);
                    library[sg.name_lower] = sg;
                }
                Logger::success("[Steam] Loaded " + std::to_string(library.size()) + " games from API.");
            }
        } catch (const std::exception& e) {
            Logger::error("[Steam] JSON parse error: " + std::string(e.what()));
        }
    }

    void launchGame(const std::string& gameName) {
        std::string lower = normalize(gameName);
        std::string trans = translit(lower);

        for (const auto& [key, game] : library) {
            if (key.find(lower) != std::string::npos || 
                key.find(trans) != std::string::npos ||
                lower.find(key) != std::string::npos ||
                trans.find(key) != std::string::npos) {
                std::string cmd = "start \"\" steam://rungameid/" + game.appid;
                system(cmd.c_str());
                Logger::success("[Steam] Launching: " + game.name);
                return;
            }
        }
        Logger::warn("[Steam] Game not found in library: " + gameName);
    }
};

SteamController::SteamController() : pImpl(std::make_unique<Impl>()) {}
SteamController::~SteamController() = default;
bool SteamController::initialize() { pImpl->fetchGames(); return true; }

std::string SteamController::executeCommand(const std::string& command, const std::string& params) {
    if (!pImpl->available) return "";
    
    std::string lower = command;
    for (char& c : lower) c = tolower((unsigned char)c);

    if (lower.find("запусти игру") != std::string::npos || lower.find("включи игру") != std::string::npos) {
        std::string gameName = lower.substr(lower.find("игру") + 4);
        while (!gameName.empty() && gameName[0] == ' ') gameName.erase(0, 1);
        if (!gameName.empty()) {
            pImpl->launchGame(gameName);
            return "Ищу игру, сэр.";
        }
    }

    if (lower.find("стим") != std::string::npos || lower.find("steam") != std::string::npos) {
        if (lower.find("открой") != std::string::npos || lower.find("запусти") != std::string::npos || lower.find("включи") != std::string::npos) {
            std::string path = AppConfig::getPath("steam");
            if (path.empty()) path = "steam://open/main";
            system(("start \"\" \"" + path + "\"").c_str());
            return "Steam запущен, сэр.";
        }
    }
    return "";
}

bool SteamController::isAvailable() const { return pImpl->available; }