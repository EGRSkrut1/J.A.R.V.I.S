#include "voxcpm_tts.h"
#include "core/config/paths.h"
#include "core/logging/logger.h"
#include <curl/curl.h>
#include <thread>
#include <chrono>
#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

struct VoxCPMTTS::Impl {
    std::string serverUrl = "http://127.0.0.1:7860/tts";
    std::chrono::seconds timeout{120};
    bool isPlaying = false;
    bool serverAvailable = true;

    static size_t writeCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
        size_t total = size * nmemb;
        output->append((char*)contents, total);
        return total;
    }

    std::string requestAudio(const std::string& text) {
        if (!serverAvailable) return "";
        
        CURL* curl = curl_easy_init();
        if (!curl) return "";

        json payload;
        payload["text"] = text;
        std::string jsonStr = payload.dump();

        std::string response;
        curl_easy_setopt(curl, CURLOPT_URL, serverUrl.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonStr.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, static_cast<long>(timeout.count()));

        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        CURLcode res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);

        if (res != CURLE_OK) {
            Logger::warn("VoxCPM TTS: server unavailable, switching to client TTS");
            serverAvailable = false;
            return "";
        }

        if (response.find("detail") != std::string::npos || response.find("error") != std::string::npos) {
            Logger::warn("VoxCPM TTS: API error, switching to client TTS");
            serverAvailable = false;
            return "";
        }

        while (!response.empty() && (response.back() == '\n' || response.back() == '\r' || response.back() == ' ')) {
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
    if (audioPath.empty()) {
        return false;
    }
    std::thread([this, audioPath]() { pImpl->playAudioFile(audioPath); }).detach();
    return true;
}

bool VoxCPMTTS::synthesizeToFile(const std::string& text, const std::string& outPath) {
    std::string srcPath = pImpl->requestAudio(text);
    if (srcPath.empty()) {
        return false;
    }
    try {
        std::filesystem::copy_file(srcPath, outPath, std::filesystem::copy_options::overwrite_existing);
        return true;
    } catch (const std::exception& e) {
        Logger::error("VoxCPM TTS: copy failed: " + std::string(e.what()));
        return false;
    }
}

bool VoxCPMTTS::isSpeaking() const { return pImpl->isPlaying; }
void VoxCPMTTS::stop() {}
void VoxCPMTTS::setServerUrl(const std::string& url) { pImpl->serverUrl = url; }
void VoxCPMTTS::setTimeout(std::chrono::seconds timeout) { pImpl->timeout = timeout; }