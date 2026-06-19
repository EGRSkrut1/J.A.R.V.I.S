#include "voxcpm_tts.h"
#include "core/config/paths.h"
#include "core/logging/logger.h"
#include <curl/curl.h>
#include <thread>
#include <chrono>

struct VoxCPMTTS::Impl {
    std::string serverUrl = "http://127.0.0.1:8888/tts";
    std::chrono::seconds timeout{120};
    bool isPlaying = false;
    
    static size_t writeCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
        size_t total = size * nmemb;
        output->append((char*)contents, total);
        return total;
    }
    
    std::string requestAudio(const std::string& text) {
        CURL* curl = curl_easy_init();
        if (!curl) return "";
        
        char* encoded = curl_easy_escape(curl, text.c_str(), static_cast<int>(text.length()));
        std::string url = serverUrl + "?text=" + std::string(encoded);
        curl_free(encoded);
        
        std::string response;
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, static_cast<long>(timeout.count()));
        
        CURLcode res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        
        if (res != CURLE_OK) {
            Logger::error("VoxCPM TTS: CURL error - " + std::string(curl_easy_strerror(res)));
            return "";
        }
        
        while (!response.empty() && (response.back() == '\n' || response.back() == '\r')) {
            response.pop_back();
        }
        
        return response;
    }
    
    void playAudioFile(const std::string& path) {
        if (path.empty()) return;
        
        std::string ffplay = Paths::getVoxCPMDir() + "/ffmpeg/ffplay.exe";
        std::string cmd = "\"" + ffplay + "\" -autoexit -nodisp -loglevel quiet \"" + path + "\"";
        isPlaying = true;
        system(cmd.c_str());
        isPlaying = false;
    }
};

VoxCPMTTS::VoxCPMTTS() : pImpl(std::make_unique<Impl>()) {}
VoxCPMTTS::~VoxCPMTTS() = default;

bool VoxCPMTTS::speak(const std::string& text) {
    Logger::info("VoxCPM TTS: generating speech...");
    
    std::string audioPath = pImpl->requestAudio(text);
    if (audioPath.empty() || audioPath == "ERROR") {
        Logger::error("VoxCPM TTS: failed to generate audio");
        return false;
    }
    
    std::thread([this, audioPath]() {
        pImpl->playAudioFile(audioPath);
    }).detach();
    
    return true;
}

bool VoxCPMTTS::isSpeaking() const { return pImpl->isPlaying; }
void VoxCPMTTS::stop() {}
void VoxCPMTTS::setServerUrl(const std::string& url) { pImpl->serverUrl = url; }
void VoxCPMTTS::setTimeout(std::chrono::seconds timeout) { pImpl->timeout = timeout; }