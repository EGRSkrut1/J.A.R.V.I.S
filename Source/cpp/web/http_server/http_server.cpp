#include "http_server.h"
#include "core/config/paths.h"
#include "core/logging/logger.h"
#include "features/llm/clients/ollama_client.h"
#include "features/speech/hybrid_tts_manager.h"
#include "features/speech/engines/windows_tts.h"
#include "features/speech/engines/voxcpm_tts.h"
#include "modules/commands/greetings/greeting_provider.h"
#include "modules/module_manager.h"
#include <httplib.h>
#include "libs/nlohmann/json.hpp"
#include <thread>
#include <deque>

using json = nlohmann::json;

extern std::deque<std::string> chatHistory;
const int MAX_HISTORY_SIZE = 50;
extern OllamaClient* g_llm_instance;
extern std::shared_ptr<GreetingProvider> g_greetingProvider;
extern ModuleManager* g_moduleManager;

std::string g_status = "idle";
std::unique_ptr<HybridTTSManager> g_tts;

void startHttpServer() {
    static httplib::Server svr;
    
    auto windowsTTS = std::make_shared<WindowsTTS>();
    auto voxcpmTTS = std::make_shared<VoxCPMTTS>();
    voxcpmTTS->setServerUrl("http://127.0.0.1:8888/tts");
    
    g_tts = std::make_unique<HybridTTSManager>();
    g_tts->setPrimaryEngine(windowsTTS);
    g_tts->setFallbackEngine(windowsTTS);
    
    svr.set_pre_routing_handler([](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Content-Type");
        if (req.method == "OPTIONS") {
            res.status = 204;
            return httplib::Server::HandlerResponse::Handled;
        }
        return httplib::Server::HandlerResponse::Unhandled;
    });
    
    svr.Get("/", [](const httplib::Request&, httplib::Response& res) {
        res.set_content("Jarvis Core Server is running.", "text/plain; charset=utf-8");
    });
    
    svr.Get("/greeting", [](const httplib::Request&, httplib::Response& res) {
        if (g_greetingProvider) g_greetingProvider->playRandomGreeting();
        res.set_content("{\"status\": \"ok\"}", "application/json");
    });
    
    svr.Get("/status", [](const httplib::Request&, httplib::Response& res) {
        json resp; resp["status"] = g_status;
        res.set_content(resp.dump(), "application/json; charset=utf-8");
    });
    
    svr.Get("/messages", [](const httplib::Request&, httplib::Response& res) {
        json msgs = json::array();
        for (const auto& msg : chatHistory) msgs.push_back(msg);
        res.set_content(msgs.dump(), "application/json; charset=utf-8");
    });
    
    svr.Post("/chat", [](const httplib::Request& req, httplib::Response& res) {
        try {
            auto body = json::parse(req.body);
            std::string userMessage = body.value("message", "");
            if (userMessage.empty()) { res.status = 400; return; }
            
            chatHistory.push_back("Вы: " + userMessage);
            if (chatHistory.size() > MAX_HISTORY_SIZE) chatHistory.pop_front();
            
            std::string reply;
            
            if (g_moduleManager) {
                reply = g_moduleManager->handleCommand(userMessage);
            }
            
            if (reply.empty() && g_llm_instance) {
                reply = g_llm_instance->ask(userMessage);
            }
            
            if (reply.empty()) {
                reply = "Я не понял команду, сэр.";
            }
            
            chatHistory.push_back("Джарвис: " + reply);
            if (chatHistory.size() > MAX_HISTORY_SIZE) chatHistory.pop_front();
            
            json response; response["reply"] = reply;
            res.set_content(response.dump(), "application/json; charset=utf-8");
            
            std::thread([reply]() {
                if (g_tts) g_tts->speak(reply);
            }).detach();
            
        } catch (...) { res.status = 500; }
    });
    
    std::thread([]() { svr.listen("0.0.0.0", 8080); }).detach();
    Logger::success("HTTP server started on port 8080");
}