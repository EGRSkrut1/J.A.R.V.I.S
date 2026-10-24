#pragma once
#include <httplib.h>

class ChatController {
public:
    static void registerRoutes(httplib::Server& svr);
};