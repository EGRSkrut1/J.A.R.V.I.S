#pragma once
#include <memory>

class JarvisApp {
public:
    JarvisApp();
    ~JarvisApp();
    
    bool initialize();
    void run();
    void shutdown();
    
private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;
};