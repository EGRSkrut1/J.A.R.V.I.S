#pragma once
#include <memory>
#include <string>

class OllamaClient;
class ModuleManager;
class IActivationListener;
class HybridTTSManager;

struct ApplicationContext {
    std::shared_ptr<OllamaClient> llm;
    std::shared_ptr<ModuleManager> moduleManager;
    std::shared_ptr<HybridTTSManager> tts;
    std::shared_ptr<IActivationListener> activation;
};