#pragma once
#include <httplib.h>
#include <string>
#include <map>

struct Session {
    std::string username;
    std::string role;
    std::string platform;
    std::string login_time;
};

std::pair<std::string, std::string> checkAuth(const httplib::Request& req);
std::map<std::string, Session> getSessions();

class AuthController {
public:
    static void registerRoutes(httplib::Server& svr);
};