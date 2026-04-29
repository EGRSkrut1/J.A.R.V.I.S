#pragma once
#include <string>
#include <iostream>

class Logger {
public:
    static void info(const std::string& msg);
    static void warn(const std::string& msg);
    static void error(const std::string& msg);
    static void success(const std::string& msg);
    static void debug(const std::string& msg);
};