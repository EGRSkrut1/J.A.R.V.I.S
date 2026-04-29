#include "hybrid_tts_manager.h"
#include "core/logging/logger.h"
#include <future>
#include <thread>

struct HybridTTSManager::Impl {
    std::shared_ptr<ISpeechEngine> primary;
    std::shared_ptr<ISpeechEngine> fallback;
    std::chrono::seconds primaryTimeout{30};
    
    bool speak(const std::string& text) {
        if (shouldUsePrimary(text)) {
            Logger::info("Hybrid TTS: trying primary engine");
            
            std::promise<bool> promise;
            auto future = promise.get_future();
            
            std::thread([&]() {
                bool result = primary->speak(text);
                promise.set_value(result);
            }).detach();
            
            if (future.wait_for(primaryTimeout) == std::future_status::ready) {
                if (future.get()) {
                    Logger::success("Hybrid TTS: primary engine succeeded");
                    return true;
                }
            }
            
            Logger::warn("Hybrid TTS: primary engine timeout/failed, falling back to fallback");
        }
        
        Logger::info("Hybrid TTS: using fallback engine");
        return fallback->speak(text);
    }
    
    bool isSpeaking() const {
        return primary->isSpeaking() || fallback->isSpeaking();
    }
    
    void stop() {
        primary->stop();
        fallback->stop();
    }
    
    bool shouldUsePrimary(const std::string& text) {
        return text.length() < 150 ||
               text.find("сэр") != std::string::npos ||
               text.find("Джарвис") != std::string::npos ||
               text.find("Выполнено") != std::string::npos ||
               text.find("Сделано") != std::string::npos;
    }
};

HybridTTSManager::HybridTTSManager() : pImpl(std::make_unique<Impl>()) {}
HybridTTSManager::~HybridTTSManager() = default;

bool HybridTTSManager::speak(const std::string& text) { return pImpl->speak(text); }
bool HybridTTSManager::isSpeaking() const { return pImpl->isSpeaking(); }
void HybridTTSManager::stop() { pImpl->stop(); }

void HybridTTSManager::setPrimaryEngine(std::shared_ptr<ISpeechEngine> engine) { pImpl->primary = engine; }
void HybridTTSManager::setFallbackEngine(std::shared_ptr<ISpeechEngine> engine) { pImpl->fallback = engine; }
void HybridTTSManager::setTimeout(std::chrono::seconds timeout) { pImpl->primaryTimeout = timeout; }