#include "http_server.h"
#include "controllers/auth_controller.h"
#include "controllers/chat_controller.h"
#include "controllers/admin_controller.h"
#include "core/config/paths.h"
#include "core/logging/logger.h"
#include <httplib.h>
#include <thread>
#include <fstream>
#include <sstream>
#include <string>
#include "libs/nlohmann/json.hpp"

using json = nlohmann::json;
std::string g_status = "idle";

void startHttpServer() {
    static httplib::Server svr;

    svr.set_pre_routing_handler([](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Content-Type, X-Auth-Token");
        if (req.method == "OPTIONS") { res.status = 204; return httplib::Server::HandlerResponse::Handled; }
        return httplib::Server::HandlerResponse::Unhandled;
    });

    AuthController::registerRoutes(svr);
    ChatController::registerRoutes(svr);
    AdminController::registerRoutes(svr);

    svr.Get("/config/wifi", [](const httplib::Request& req, httplib::Response& res) {
        std::string path = Paths::getConfigsDir() + "/wifi.json";
        std::ifstream file(path, std::ios::binary);
        if (!file.is_open()) { res.status = 404; return; }
        std::stringstream buffer; buffer << file.rdbuf();
        res.set_content(buffer.str(), "application/json");
    });

    svr.Get(R"(/tts/([A-Za-z0-9_\-]+\.wav))", [](const httplib::Request& req, httplib::Response& res) {
        std::string id = req.matches[1];
        std::string path = Paths::getTempDir() + "/" + id;
        std::ifstream file(path, std::ios::binary);
        if (!file.is_open()) { res.status = 404; return; }
        std::stringstream buffer; buffer << file.rdbuf();
        res.set_content(buffer.str(), "audio/wav");
    });

    svr.Get(R"(/sounds/(.+))", [](const httplib::Request& req, httplib::Response& res) {
        std::string subpath = req.matches[1];
        std::string path = Paths::getSoundsDir() + "/" + subpath;
        std::ifstream file(path, std::ios::binary);
        if (!file.is_open()) { res.status = 404; return; }
        std::stringstream buffer; buffer << file.rdbuf();
        res.set_content(buffer.str(), "audio/wav");
    });

    std::thread([]() { svr.listen("0.0.0.0", 8080); }).detach();
    Logger::success("HTTP server started on port 8080");
}