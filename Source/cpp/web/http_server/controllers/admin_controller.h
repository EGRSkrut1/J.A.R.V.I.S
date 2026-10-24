#pragma once
#include <httplib.h>
#include "auth_controller.h"

class AdminController {
public:
    static void registerRoutes(httplib::Server& svr);
};