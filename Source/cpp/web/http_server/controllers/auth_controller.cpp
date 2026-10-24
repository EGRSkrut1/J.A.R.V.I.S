#include "auth_controller.h"
#include "core/config/paths.h"
#include "core/logging/logger.h"
#include "libs/nlohmann/json.hpp"
#include <fstream>
#include <map>
#include <mutex>
#include <random>
#include <chrono>
#include <iomanip>
#include <sstream>

using json = nlohmann::json;

struct User {
    std::string password;
    std::string role;
};

static std::mutex g_sessionsMtx;
static std::map<std::string, Session> g_sessions;
static std::map<std::string, User> g_users;

static std::string getCurrentTimeStr() {
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    std::tm tm_now;
    localtime_s(&tm_now, &time_t_now);
    std::stringstream ss;
    ss << std::put_time(&tm_now, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

static std::string generateToken() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);
    const char* hex = "0123456789abcdef";
    std::string token;
    for (int i = 0; i < 32; ++i) token += hex[dis(gen)];
    return token;
}

static void loadUsers() {
    std::string path = Paths::getConfigsDir() + "/users.json";
    Logger::info("[Auth] Loading users from: " + path);
    std::ifstream file(path);
    if (!file.is_open()) {
        Logger::error("[Auth] users.json NOT FOUND at " + path + " - login disabled!");
        return;
    }
    try {
        json data = json::parse(file);
        for (const auto& u : data["users"]) {
            g_users[u.value("username", "")] = { u.value("password", ""), u.value("role", "user") };
        }
        Logger::success("[Auth] Loaded " + std::to_string(g_users.size()) + " users");
    } catch (const std::exception& e) {
        Logger::error("[Auth] JSON parse error in users.json: " + std::string(e.what()));
    }
}

std::pair<std::string, std::string> checkAuth(const httplib::Request& req) {
    auto token = req.get_header_value("X-Auth-Token");
    if (token.empty()) return {"", ""};
    std::lock_guard<std::mutex> lock(g_sessionsMtx);
    auto it = g_sessions.find(token);
    if (it == g_sessions.end()) return {"", ""};
    return { it->second.username, it->second.role };
}

std::map<std::string, Session> getSessions() {
    std::lock_guard<std::mutex> lock(g_sessionsMtx);
    return g_sessions;
}

void AuthController::registerRoutes(httplib::Server& svr) {
    loadUsers();

    svr.Post("/login", [](const httplib::Request& req, httplib::Response& res) {
        try {
            auto body = json::parse(req.body);
            std::string user = body.value("username", "");
            std::string pass = body.value("password", "");
            std::string platform = body.value("platform", "Web");

            auto it = g_users.find(user);
            if (it == g_users.end() || it->second.password != pass) {
                res.status = 401;
                res.set_content(R"({"error":"invalid credentials"})", "application/json");
                return;
            }

            std::string token = generateToken();
            {
                std::lock_guard<std::mutex> lock(g_sessionsMtx);
                g_sessions[token] = { user, it->second.role, platform, getCurrentTimeStr() };
            }

            json resp;
            resp["token"] = token;
            resp["username"] = user;
            resp["role"] = it->second.role;
            res.set_content(resp.dump(), "application/json");
        } catch (...) { res.status = 400; }
    });

    svr.Post("/logout", [](const httplib::Request& req, httplib::Response& res) {
        auto token = req.get_header_value("X-Auth-Token");
        std::lock_guard<std::mutex> lock(g_sessionsMtx);
        g_sessions.erase(token);
        res.set_content(R"({"ok":true})", "application/json");
    });

    svr.Get("/me", [](const httplib::Request& req, httplib::Response& res) {
        auto [user, role] = checkAuth(req);
        if (user.empty()) { res.status = 401; return; }
        json resp; resp["username"] = user; resp["role"] = role;
        res.set_content(resp.dump(), "application/json");
    });
}