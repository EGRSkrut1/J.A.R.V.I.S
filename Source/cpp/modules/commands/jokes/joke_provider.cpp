#include "joke_provider.h"
#include "core/logging/logger.h"
#include <fstream>
#include <nlohmann/json.hpp>
#include <random>
#include <ctime>
#include <windows.h>

using json = nlohmann::json;

struct JokeProvider::Impl {
    struct Joke { std::string id; std::string text; std::string audio; };
    std::vector<Joke> jokes;
    mutable std::mt19937 rng;
    std::string lastJokeText;
    
    Impl() : rng(std::time(nullptr)) {}
    
    bool loadFromFile(const std::string& path) {
        std::ifstream file(path);
        if (!file.is_open()) return false;
        try {
            json data = json::parse(file);
            for (const auto& item : data["jokes"]) {
                Joke j;
                j.id = item.value("id", "");
                j.text = item.value("text", "");
                j.audio = item.value("audio", "");
                if (!j.id.empty() && !j.text.empty()) jokes.push_back(j);
            }
            Logger::success("JokeProvider: loaded " + std::to_string(jokes.size()) + " jokes");
            return true;
        } catch (...) { return false; }
    }
    
    std::string getRandomJokeAndPlay() {
        if (jokes.empty()) return "Сэр, шутки закончились.";
        std::uniform_int_distribution<size_t> dist(0, jokes.size() - 1);
        const auto& joke = jokes[dist(rng)];
        
        // Играем аудио в фоне
        if (!joke.audio.empty()) {
            std::string path = "C:\\Users\\egrsk\\Desktop\\Jarvis\\Resources\\sounds\\jokes\\" + joke.audio;
            std::string cmd = "start /B C:\\Users\\egrsk\\Desktop\\Jarvis\\Services\\TTS\\VoxCPM2\\ffmpeg\\ffplay.exe -autoexit -nodisp -loglevel quiet \"" + path + "\"";
            system(cmd.c_str());
        }
        
        return joke.text;
    }
};

JokeProvider::JokeProvider() : pImpl(std::make_unique<Impl>()) {}
JokeProvider::~JokeProvider() = default;

std::vector<std::string> JokeProvider::getTriggers() const {
    return {"пошути", "анекдот", "шутка", "расскажи шутку", "рассмеши"};
}

std::string JokeProvider::execute(const std::string& userInput) {
    return pImpl->getRandomJokeAndPlay();
}

bool JokeProvider::isAvailable() const { return !pImpl->jokes.empty(); }
bool JokeProvider::loadFromFile(const std::string& path) { return pImpl->loadFromFile(path); }