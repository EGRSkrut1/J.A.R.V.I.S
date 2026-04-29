#include "jarvis_app.h"
#include "core/config/config_manager.h"
#include "core/logging/logger.h"
#include "core/logging/voice_logger.h"
#include "features/activation/activation_controller.h"
#include "features/speech/hybrid_tts_manager.h"
#include "features/speech/engines/windows_tts.h"
#include "features/speech/engines/voxcpm_tts.h"
#include "features/llm/clients/ollama_client.h"
#include "modules/module_manager.h"
#include "modules/commands/jokes/joke_provider.h"
#include "modules/commands/system_info/system_info.h"
#include "modules/commands/greetings/greeting_provider.h"
#include "modules/commands/text_input/text_input.h"
#include "modules/commands/browser/browser_control.h"
#include "modules/apps/spotify/spotify_controller.h"
#include "web/http_server/http_server.h"
#include "web/websocket_server/websocket_server.h"
#include <thread>
#include <chrono>
#include <iostream>
#include <windows.h>
#include <deque>

std::deque<std::string> chatHistory;
const int MAX_HISTORY_SIZE = 50;
OllamaClient* g_llm_instance = nullptr;
std::shared_ptr<GreetingProvider> g_greetingProvider = nullptr;
ModuleManager* g_moduleManager = nullptr;

struct JarvisApp::Impl {
    std::unique_ptr<ConfigManager> config;
    std::unique_ptr<ActivationController> activation;
    std::unique_ptr<HybridTTSManager> tts;
    std::unique_ptr<OllamaClient> llm;
    std::unique_ptr<ModuleManager> moduleManager;
    std::shared_ptr<GreetingProvider> greetingProvider;
    bool running = false;
    
    bool initialize() {
        Logger::info("J.A.R.V.I.S starting...");
        std::cout << std::endl;
        
        Logger::info("Loading configuration...");
        config = std::make_unique<ConfigManager>();
        config->loadAll();
        Logger::success("Configuration loaded");
        std::cout << std::endl;
        
        Logger::info("Initializing TTS engines...");
        auto windowsTTS = std::make_shared<WindowsTTS>();
        auto voxcpmTTS = std::make_shared<VoxCPMTTS>();
        voxcpmTTS->setServerUrl("http://127.0.0.1:8888/tts");
        voxcpmTTS->setTimeout(std::chrono::seconds(30));
        
        tts = std::make_unique<HybridTTSManager>();
        tts->setPrimaryEngine(windowsTTS);
        tts->setFallbackEngine(windowsTTS);
        Logger::success("TTS engines initialized");
        std::cout << std::endl;
        
        Logger::info("Initializing LLM client...");
        llm = std::make_unique<OllamaClient>();
        llm->setModel("llama3.1:8b");
        llm->setSystemPrompt(config->getSystemPrompt());
        llm->setSystemsInfo(config->getSystemsInfo());
        g_llm_instance = llm.get();
        Logger::success("LLM client initialized");
        std::cout << std::endl;
        
        Logger::info("Loading modules...");
        moduleManager = std::make_unique<ModuleManager>();
        g_moduleManager = moduleManager.get();
        
        auto jokeProvider = std::make_shared<JokeProvider>();
        if (jokeProvider->loadFromFile("C:\\Users\\egrsk\\Desktop\\Jarvis\\Source\\cpp\\modules\\commands\\jokes\\jokes.json")) {
            moduleManager->registerCommand(jokeProvider);
            Logger::success("JokeProvider loaded");
        }
        
        auto systemInfo = std::make_shared<SystemInfo>();
        moduleManager->registerCommand(systemInfo);
        Logger::success("SystemInfo loaded");
        
        greetingProvider = std::make_shared<GreetingProvider>();
        moduleManager->registerCommand(greetingProvider);
        g_greetingProvider = greetingProvider;
        Logger::success("GreetingProvider loaded");
        
        auto textInput = std::make_shared<TextInput>();
        moduleManager->registerCommand(textInput);
        Logger::success("TextInput loaded");
        
        auto browserControl = std::make_shared<BrowserControl>();
        moduleManager->registerCommand(browserControl);
        Logger::success("BrowserControl loaded");
        
        auto spotifyController = std::make_shared<SpotifyController>();
        spotifyController->initialize();
        moduleManager->registerApp(spotifyController);
        Logger::success("SpotifyController loaded");
        std::cout << std::endl;
        
        Logger::info("Starting servers...");
        startHttpServer();
        startWebSocketServer();
        Logger::success("Servers started");
        std::cout << std::endl;
        
        Logger::info("Initializing voice activation...");
        activation = std::make_unique<ActivationController>();
        Logger::success("Voice activation ready");
        std::cout << std::endl;
        
        running = true;
        Logger::success("J.A.R.V.I.S initialized successfully");
        std::cout << std::endl;
        
        return true;
    }
    
    void run() {
        broadcastStatus("idle");
        
        while (running) {
            std::string userSpeech;
            
            if (activation->waitForCommand(userSpeech)) {
                std::cout << "\n>>> КОМАНДА: " << userSpeech << " <<<" << std::endl;
                VoiceLogger::userSpeech(userSpeech);
                
                chatHistory.push_back("Вы: " + userSpeech);
                if (chatHistory.size() > MAX_HISTORY_SIZE) chatHistory.pop_front();
                
                broadcastUserMessage(userSpeech);
                broadcastStatus("processing");
                
                std::string moduleResponse = moduleManager->handleCommand(userSpeech);
                
                if (!moduleResponse.empty()) {
                    std::cout << "\n>>> МОДУЛЬ: " << moduleResponse << " <<<" << std::endl;
                    VoiceLogger::jarvisResponse(moduleResponse);
                    
                    chatHistory.push_back("Джарвис: " + moduleResponse);
                    if (chatHistory.size() > MAX_HISTORY_SIZE) chatHistory.pop_front();
                    
                    broadcastJarvisMessage(moduleResponse);
                    broadcastStatus("speaking");
                    
                    Beep(1400, 100); Beep(1600, 100); Beep(1800, 100);
                    broadcastStatus("idle");
                } else {
                    std::cout << "\n>>> ЗАПРОС К OLLAMA: " << userSpeech << " <<<" << std::endl;
                    std::string answer = llm->ask(userSpeech);
                    
                    std::cout << "\n>>> ОТВЕТ ДЖАРВИСА: " << answer << " <<<" << std::endl;
                    VoiceLogger::jarvisResponse(answer);
                    
                    chatHistory.push_back("Джарвис: " + answer);
                    if (chatHistory.size() > MAX_HISTORY_SIZE) chatHistory.pop_front();
                    
                    broadcastJarvisMessage(answer);
                    broadcastStatus("speaking");
                    
                    Beep(1400, 100); Beep(1600, 100); Beep(1800, 100);
                    tts->speak(answer);
                    broadcastStatus("idle");
                }
            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
    
    void shutdown() {
        running = false;
        stopWebSocketServer();
        Logger::info("J.A.R.V.I.S shutting down...");
    }
};

JarvisApp::JarvisApp() : pImpl(std::make_unique<Impl>()) {}
JarvisApp::~JarvisApp() = default;
bool JarvisApp::initialize() { return pImpl->initialize(); }
void JarvisApp::run() { pImpl->run(); }
void JarvisApp::shutdown() { pImpl->shutdown(); }