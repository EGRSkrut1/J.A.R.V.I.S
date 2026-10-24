#include "jarvis_app.h"
#include "core/config/config_manager.h"
#include "core/config/paths.h"
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
#include "modules/commands/devices/device_control.h"
#include "modules/apps/spotify/spotify_controller.h"
#include "modules/apps/steam/steam_controller.h"
#include "modules/apps/discord/discord_controller.h"
#include "modules/apps/ds4windows/ds4_controller.h"
#include "modules/apps/happ/happ_controller.h"
#include "modules/apps/zapret/zapret_controller.h"
#include "web/http_server/http_server.h"
#include "web/http_server/controllers/chat_history.h"
#include <thread>
#include <chrono>
#include <iostream>
#include <windows.h>
#include <iomanip>
#include <ctime>
#include <sstream>

OllamaClient* g_llm_instance = nullptr;
std::shared_ptr<GreetingProvider> g_greetingProvider = nullptr;
ModuleManager* g_moduleManager = nullptr;
std::shared_ptr<JokeProvider> g_jokeProvider = nullptr;
std::shared_ptr<DeviceControl> g_deviceControl = nullptr;

static std::string getCurrentTime() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::tm tm;
    localtime_s(&tm, &time_t);
    std::stringstream ss;
    ss << std::put_time(&tm, "%H:%M:%S");
    return ss.str();
}

struct JarvisApp::Impl {
    std::unique_ptr<ConfigManager> config;
    std::unique_ptr<ActivationController> activation;
    std::unique_ptr<HybridTTSManager> tts;
    std::unique_ptr<OllamaClient> llm;
    std::unique_ptr<ModuleManager> moduleManager;
    std::shared_ptr<GreetingProvider> greetingProvider;
    std::shared_ptr<JokeProvider> jokeProvider;
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
        voxcpmTTS->setServerUrl("http://127.0.0.1:7860/tts");
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
        
        auto jokeProviderLocal = std::make_shared<JokeProvider>();
        if (jokeProviderLocal->loadFromFile(Paths::getJokesJson())) {
            moduleManager->registerCommand(jokeProviderLocal);
            g_jokeProvider = jokeProviderLocal;
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

        auto deviceControlLocal = std::make_shared<DeviceControl>();
        g_deviceControl = deviceControlLocal;
        moduleManager->registerCommand(deviceControlLocal);
        Logger::success("DeviceControl loaded");
        
        auto spotifyController = std::make_shared<SpotifyController>();
        spotifyController->initialize();
        moduleManager->registerApp(spotifyController);
        Logger::success("SpotifyController loaded");

        auto steamController = std::make_shared<SteamController>();
        steamController->initialize();
        moduleManager->registerApp(steamController);
        Logger::success("SteamController loaded");

        auto discordController = std::make_shared<DiscordController>();
        discordController->initialize();
        moduleManager->registerApp(discordController);
        Logger::success("DiscordController loaded");

        auto ds4Controller = std::make_shared<Ds4Controller>();
        ds4Controller->initialize();
        moduleManager->registerApp(ds4Controller);
        Logger::success("Ds4Controller loaded");

        auto happController = std::make_shared<HappController>();
        happController->initialize();
        moduleManager->registerApp(happController);
        Logger::success("HappController loaded");

        auto zapretController = std::make_shared<ZapretController>();
        zapretController->initialize();
        moduleManager->registerApp(zapretController);
        Logger::success("ZapretController loaded");
        std::cout << std::endl;
        
        Logger::info("Starting servers...");
        startHttpServer();
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
        std::string defaultUser = "admin";
        static auto lastClearTime = std::chrono::steady_clock::now();
        
        while (running) {
            std::string userSpeech;
            
            if (activation->waitForCommand(userSpeech)) {
                auto now = std::chrono::steady_clock::now();
                auto elapsed = std::chrono::duration_cast<std::chrono::minutes>(now - lastClearTime).count();
                
                if (elapsed >= 30) {
                    llm->clearHistory();
                    lastClearTime = now;
                    Logger::info("[Voice] Context history cleared (30 min idle)");
                }

                std::string timeStr = getCurrentTime();
                std::cout << "\n[" << timeStr << "] КОМАНДА: " << userSpeech << std::endl;
                VoiceLogger::userSpeech(userSpeech);
                
                addChatMessage(defaultUser, "You:" + timeStr + "|" + userSpeech);
                
                std::string llmInput = "[Пользователь: " + defaultUser + ", пол: male] " + userSpeech;
                std::string moduleResponse = moduleManager->handleCommand(llmInput);
                
                if (!moduleResponse.empty()) {
                    std::string respTime = getCurrentTime();
                    std::cout << "[" << respTime << "] МОДУЛЬ: " << moduleResponse << std::endl;
                    VoiceLogger::jarvisResponse(moduleResponse);
                    
                    addChatMessage(defaultUser, "Jarvis:" + respTime + "|ModuleManager|0|" + moduleResponse);
                    
                    std::string confirmSound = moduleManager->getLastConfirmSound();
                    if (!confirmSound.empty()) {
                        std::string ffplay = Paths::getVoxCPMDir() + "\\ffmpeg\\ffplay.exe";
                        std::string path = Paths::getSoundsDir() + "\\" + confirmSound;
                        std::string cmd = "start \"\" /B \"" + ffplay + "\" -autoexit -nodisp -loglevel quiet \"" + path + "\"";
                        system(cmd.c_str());
                    } else {
                        tts->speak(moduleResponse);
                    }
                } else {
                    std::cout << "[" << getCurrentTime() << "] ЗАПРОС К OLLAMA..." << std::endl;
                    std::string answer = llm->askWithTools(llmInput, moduleManager.get());
                    
                    if (answer == "__TOOL_CALLED__" || answer.find("Вызываю") != std::string::npos || answer.find("execute_system_command") != std::string::npos) {
                        answer = "Выполнено, сэр.";
                    }
                    
                    std::string ansTime = getCurrentTime();
                    std::cout << "[" << ansTime << "] ОТВЕТ: " << answer << std::endl;
                    VoiceLogger::jarvisResponse(answer);
                    
                    addChatMessage(defaultUser, "Jarvis:" + ansTime + "|LLM|0|" + answer);
                    
                    tts->speak(answer);
                }
            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
    
    void shutdown() {
        running = false;
        Logger::info("J.A.R.V.I.S shutting down...");
    }
};

JarvisApp::JarvisApp() : pImpl(std::make_unique<Impl>()) {}
JarvisApp::~JarvisApp() = default;
bool JarvisApp::initialize() { return pImpl->initialize(); }
void JarvisApp::run() { pImpl->run(); }
void JarvisApp::shutdown() { pImpl->shutdown(); }