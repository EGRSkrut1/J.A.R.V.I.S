#include "chat_controller.h"
#include "auth_controller.h"
#include "chat_history.h"
#include "core/config/paths.h"
#include "core/logging/logger.h"
#include "features/llm/clients/ollama_client.h"
#include "features/speech/engines/voxcpm_tts.h"
#include "modules/commands/jokes/joke_provider.h"
#include "modules/module_manager.h"
#include "modules/commands/devices/device_control.h"
#include "libs/nlohmann/json.hpp"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <random>
#include <chrono>
#include <algorithm>
#include <ctime>

using json = nlohmann::json;

extern OllamaClient* g_llm_instance;
extern ModuleManager* g_moduleManager;
extern std::shared_ptr<JokeProvider> g_jokeProvider;
extern std::shared_ptr<DeviceControl> g_deviceControl;
extern std::string g_status;

static std::unique_ptr<VoxCPMTTS> g_voxcpm;

static std::string getCurrentTimeStr() {
    time_t now = time(0);
    tm* ltm = localtime(&now);
    char timeStr[12];
    strftime(timeStr, sizeof(timeStr), "%H:%M:%S", ltm);
    return std::string(timeStr);
}

static std::string generateReplyWav(const std::string& text) {
    if (!g_voxcpm) return "";
    std::filesystem::create_directories(Paths::getTempDir());
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 999999);
    std::string id = std::to_string(std::chrono::steady_clock::now().time_since_epoch().count())
                   + "_" + std::to_string(dis(gen));
    std::string outPath = Paths::getTempDir() + "/reply_" + id + ".wav";
    if (g_voxcpm->synthesizeToFile(text, outPath)) {
        return "reply_" + id + ".wav";
    }
    return "";
}

static bool isJokeCommand(const std::string& msg) {
    std::string lower = msg;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    return lower.find("пошути") != std::string::npos ||
           lower.find("анекдот") != std::string::npos ||
           lower.find("шутка") != std::string::npos;
}

void ChatController::registerRoutes(httplib::Server& svr) {
    g_voxcpm = std::make_unique<VoxCPMTTS>();
    g_voxcpm->setServerUrl("http://127.0.0.1:7860/tts");

    svr.Get("/status", [](const httplib::Request& req, httplib::Response& res) {
        json resp; resp["status"] = g_status;
        res.set_content(resp.dump(), "application/json; charset=utf-8");
    });

    svr.Get("/messages", [](const httplib::Request& req, httplib::Response& res) {
        auto [user, role] = checkAuth(req);
        if (user.empty()) { res.status = 401; return; }
        auto msgs_deque = getChatMessages(user);
        json msgs = json::array();
        for (const auto& msg : msgs_deque) msgs.push_back(msg);
        res.set_content(msgs.dump(), "application/json; charset=utf-8");
    });

    svr.Post("/chat", [](const httplib::Request& req, httplib::Response& res) {
        try {
            std::string user;
            std::string role;
            std::string userName;

            auto [tokenUser, tokenRole] = checkAuth(req);
            if (!tokenUser.empty()) {
                user = tokenUser;
                role = tokenRole;
            }

            auto body = json::parse(req.body, nullptr, false);
            if (body.is_discarded() || !body.contains("message")) { res.status = 400; return; }
            
            std::string userMessage = body.value("message", "");
            if (userMessage.empty()) { res.status = 400; return; }

            if (user.empty()) {
                userName = body.value("user_name", "");
                if (userName.empty()) {
                    res.status = 401;
                    return;
                }
                user = userName;
                role = "user";
            } else {
                userName = user;
            }

            std::string llmInput = userMessage;
            if (!userName.empty()) {
                llmInput = "[Пользователь: " + userName + "] " + userMessage;
            }

            std::string timeStr = getCurrentTimeStr();
            addChatMessage(user, "You:" + timeStr + "|" + userMessage);

            std::string reply;
            bool fromModule = false;
            std::string audioOverride = "";
            std::string moduleName = "";
            long long duration = 0;

            auto start = std::chrono::high_resolution_clock::now();

            if (g_jokeProvider && isJokeCommand(llmInput)) {
                reply = g_jokeProvider->executeSilent(llmInput);
                fromModule = true; moduleName = "JokeProvider";
                std::string lastAudio = g_jokeProvider->getLastAudioFile();
                if (!lastAudio.empty()) audioOverride = "/sounds/jokes/" + lastAudio;
            } 
            else if (g_deviceControl) {
                reply = g_deviceControl->executeWithContext(llmInput, "");
                if (!reply.empty()) { fromModule = true; moduleName = "DeviceControl"; }
            }
            
            if (reply.empty() && g_moduleManager) {
                std::string lower = llmInput;
                std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
                
                size_t andPos = lower.find(" и ");
                if (andPos != std::string::npos) {
                    std::string part1 = llmInput.substr(0, andPos);
                    std::string part2 = llmInput.substr(andPos + 3);
                    
                    std::string res1 = g_moduleManager->handleCommand(part1);
                    std::string res2 = g_moduleManager->handleCommand(part2);
                    
                    if (!res1.empty() || !res2.empty()) {
                        reply = "Выполнено: " + part1 + ", " + part2 + ", сэр.";
                        fromModule = true;
                        moduleName = "ModuleManager";
                        std::string confirmSound = g_moduleManager->getLastConfirmSound();
                        if (!confirmSound.empty()) audioOverride = "/sounds/" + confirmSound;
                    }
                } else {
                    reply = g_moduleManager->handleCommand(llmInput);
                    if (!reply.empty()) {
                        fromModule = true; moduleName = "ModuleManager";
                        std::string confirmSound = g_moduleManager->getLastConfirmSound();
                        if (!confirmSound.empty()) audioOverride = "/sounds/" + confirmSound;
                    }
                }
            }

            if (reply.empty() && g_llm_instance) {
                moduleName = "LLM";
                Logger::info("[Chat] Sending to LLM: " + llmInput);
                reply = g_llm_instance->askWithTools(llmInput, g_moduleManager);
                
                if (reply == "__TOOL_CALLED__" || reply.find("Вызываю") != std::string::npos || reply.find("execute_system_command") != std::string::npos) {
                    reply = "Выполнено, сэр.";
                    fromModule = true;
                    std::string confirmSound = g_moduleManager->getLastConfirmSound();
                    if (!confirmSound.empty()) audioOverride = "/sounds/" + confirmSound;
                } else {
                    Logger::info("[Chat] LLM replied: " + reply);
                    std::string confirmSound = g_moduleManager->getLastConfirmSound();
                    if (!confirmSound.empty()) {
                        audioOverride = "/sounds/" + confirmSound;
                    }
                }
            }
            if (reply.empty()) reply = "I did not understand, sir.";

            auto end = std::chrono::high_resolution_clock::now();
            duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

            addChatMessage(user, "Jarvis:" + timeStr + "|" + moduleName + "|" + std::to_string(duration) + "|" + reply);

            json response;
            response["reply"] = reply;
            response["is_module"] = fromModule;
            response["module"] = moduleName;
            response["time_ms"] = duration;

            if (!audioOverride.empty()) {
                response["audio_url"] = audioOverride;
            } else if (!fromModule) {
                std::string wavName = generateReplyWav(reply);
                if (!wavName.empty()) response["audio_url"] = "/tts/" + wavName;
            }

            res.set_content(response.dump(), "application/json; charset=utf-8");
        } catch (const std::exception& e) {
            Logger::error("[Chat] Exception: " + std::string(e.what()));
            res.status = 500; 
        }
    });
}