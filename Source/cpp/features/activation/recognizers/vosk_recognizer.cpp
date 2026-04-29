#include "vosk_recognizer.h"
#include "core/logging/logger.h"
#include <thread>
#include <chrono>
#include <vector>

#pragma comment(lib, "ws2_32.lib")

struct VoskRecognizer::Impl {
    SOCKET sock = INVALID_SOCKET;
    
    ~Impl() {
        if (sock != INVALID_SOCKET) closesocket(sock);
    }
    
    bool connect() {
        sock = ::socket(AF_INET, SOCK_STREAM, 0);
        if (sock == INVALID_SOCKET) return false;
        
        sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(2700);
        addr.sin_addr.s_addr = inet_addr("127.0.0.1");
        
        if (::connect(sock, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
            closesocket(sock);
            sock = INVALID_SOCKET;
            return false;
        }
        
        u_long mode = 1;
        ioctlsocket(sock, FIONBIO, &mode);
        return true;
    }
    
    std::string readLine(int timeoutMs = 1000) {
        static std::string accumulated;
        char buffer[1024];
        auto start = std::chrono::steady_clock::now();
        
        while (std::chrono::duration_cast<std::chrono::milliseconds>(
                   std::chrono::steady_clock::now() - start).count() < timeoutMs) {
            int bytes = ::recv(sock, buffer, sizeof(buffer) - 1, 0);
            if (bytes > 0) {
                buffer[bytes] = '\0';
                accumulated += buffer;
                size_t newlinePos;
                if ((newlinePos = accumulated.find('\n')) != std::string::npos) {
                    std::string line = accumulated.substr(0, newlinePos);
                    accumulated = accumulated.substr(newlinePos + 1);
                    return cleanText(line);
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        return "";
    }
    
    static std::string cleanText(const std::string& input) {
        std::string s = input;
        size_t start = s.find_first_not_of(" \t\n\r-—.?!*");
        if (start != std::string::npos) s = s.substr(start);
        while (!s.empty() && (s.back() == '.' || s.back() == '?' || s.back() == '!' || s.back() == ' '))
            s.pop_back();
        return s;
    }
    
    static bool containsJarvis(const std::string& text) {
        std::string lower = text;
        for (char& c : lower) c = tolower(c);
        return (lower.find("джарвис") != std::string::npos ||
                lower.find("жарвис") != std::string::npos ||
                lower.find("jarvis") != std::string::npos ||
                lower.find("джорвис") != std::string::npos);
    }
    
    static std::string removeWakeWord(const std::string& text) {
        std::string lower = text;
        for (char& c : lower) c = tolower(c);
        
        std::vector<std::string> wakeWords = {"джарвис", "жарвис", "джорвис", "jarvis"};
        for (const auto& w : wakeWords) {
            size_t pos = lower.find(w);
            if (pos != std::string::npos) {
                return text.substr(pos + w.length());
            }
        }
        return text;
    }
};

VoskRecognizer::VoskRecognizer() : pImpl(std::make_unique<Impl>()) {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
}

VoskRecognizer::~VoskRecognizer() = default;

bool VoskRecognizer::connect() { return pImpl->connect(); }
bool VoskRecognizer::isConnected() const { return pImpl->sock != INVALID_SOCKET; }

bool VoskRecognizer::waitForWakeWord(std::string& wakeWord) {
    while (true) {
        std::string line = pImpl->readLine(1000);
        if (!line.empty() && Impl::containsJarvis(line)) {
            wakeWord = line;
            return true;
        }
    }
}

std::string VoskRecognizer::listenForCommand(int timeoutSec) {
    std::string fullCommand;
    auto start = std::chrono::steady_clock::now();
    
    while (std::chrono::duration_cast<std::chrono::seconds>(
               std::chrono::steady_clock::now() - start).count() < timeoutSec) {
        std::string line = pImpl->readLine(500);
        if (!line.empty()) {
            if (!fullCommand.empty()) fullCommand += " ";
            fullCommand += line;
        }
    }
    
    return fullCommand;
}