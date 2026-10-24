#include "admin_controller.h"
#include "auth_controller.h"
#include "core/config/paths.h"
#include "libs/nlohmann/json.hpp"
#include <fstream>
#include <sstream>

using json = nlohmann::json;

void AdminController::registerRoutes(httplib::Server& svr) {
    svr.Get("/admin/logs", [](const httplib::Request& req, httplib::Response& res) {
        auto [user, role] = checkAuth(req);
        bool isLocal = (req.remote_addr == "127.0.0.1" || req.remote_addr == "::1");
        
        if (user.empty() && !isLocal) { res.status = 403; return; }
        if (!role.empty() && role != "admin") { res.status = 403; return; }
        
        std::ifstream f(Paths::getLogsDir() + "/console.log");
        if (!f.is_open()) {
            res.set_content("Logs are empty.", "text/plain");
            return;
        }
        std::stringstream ss; ss << f.rdbuf();
        res.set_content(ss.str(), "text/plain; charset=utf-8");
    });
    
    svr.Get("/admin/sessions", [](const httplib::Request& req, httplib::Response& res) {
        auto [user, role] = checkAuth(req);
        bool isLocal = (req.remote_addr == "127.0.0.1" || req.remote_addr == "::1");
        
        if (user.empty() && !isLocal) { res.status = 403; return; }
        if (!role.empty() && role != "admin") { res.status = 403; return; }
        
        json arr = json::array();

        // Веб-сессии
        auto sessions = getSessions();
        for (const auto& [token, session] : sessions) {
            arr.push_back({
                {"user", session.username},
                {"role", session.role},
                {"platform", session.platform},
                {"login_time", session.login_time}
            });
        }

        std::string tg_path = Paths::getConfigsDir() + "/telegram_sessions.json";
        std::ifstream tg_file(tg_path);
        if (tg_file.is_open()) {
            try {
                json tg_data = json::parse(tg_file);
                for (auto& [chat_id, s_data] : tg_data.items()) {
                    if (s_data.is_string()) {
                        arr.push_back({
                            {"user", s_data.get<std::string>()},
                            {"role", "user"},
                            {"platform", "Telegram"},
                            {"login_time", "unknown"}
                        });
                    } else if (s_data.is_object()) {
                        arr.push_back({
                            {"user", s_data.value("username", "?")},
                            {"role", s_data.value("role", "user")},
                            {"platform", "Telegram"},
                            {"login_time", s_data.value("login_time", "unknown")}
                        });
                    }
                }
            } catch (...) {}
        }

        res.set_content(arr.dump(), "application/json");
    });
}