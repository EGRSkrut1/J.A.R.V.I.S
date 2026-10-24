#include "windows_tts.h"
#include "core/logging/logger.h"
#include <windows.h>
#include <sapi.h>
#include <sphelper.h>
#include <thread>
#include <atomic>
#include <mutex>

#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "sapi.lib")

struct WindowsTTS::Impl {
    std::atomic<bool> isPlaying{false};
    std::mutex mtx;
    ISpVoice* pVoice = nullptr;

    bool initVoice() {
        if (pVoice) return true;
        if (FAILED(::CoInitializeEx(NULL, COINIT_MULTITHREADED))) return false;
        HRESULT hr = CoCreateInstance(CLSID_SpVoice, NULL, CLSCTX_ALL, IID_ISpVoice, (void**)&pVoice);
        if (FAILED(hr)) return false;

        IEnumSpObjectTokens* pEnum = NULL;
        SpEnumTokens(SPCAT_VOICES, L"Language=419", NULL, &pEnum);
        ISpObjectToken* pToken = NULL;
        bool found = false;
        while (pEnum && SUCCEEDED(pEnum->Next(1, &pToken, NULL))) {
            WCHAR* name = NULL;
            SpGetDescription(pToken, &name);
            if (name) {
                std::wstring wname(name);
                if (wname.find(L"Irina") != std::wstring::npos ||
                    wname.find(L"Pavel") != std::wstring::npos) {
                    pVoice->SetVoice(pToken);
                    found = true;
                    ::CoTaskMemFree(name);
                    break;
                }
                ::CoTaskMemFree(name);
            }
            pToken->Release();
        }
        if (!found && pEnum) {
            pEnum->Reset();
            if (SUCCEEDED(pEnum->Next(1, &pToken, NULL))) {
                pVoice->SetVoice(pToken);
                pToken->Release();
            }
        }
        if (pEnum) pEnum->Release();
        pVoice->SetVolume(100);
        return true;
    }

    bool speakAsync(const std::string& text) {
        stop();
        isPlaying = true;
        std::thread([this, text]() {
            std::lock_guard<std::mutex> lock(mtx);
            if (!initVoice()) { isPlaying = false; return; }
            int wlen = MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, NULL, 0);
            std::wstring wtext(wlen, 0);
            MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, &wtext[0], wlen);
            pVoice->Speak(wtext.c_str(), SPF_DEFAULT, NULL);
            isPlaying = false;
        }).detach();
        return true;
    }

    void stop() {
        if (pVoice) {
            std::lock_guard<std::mutex> lock(mtx);
            pVoice->Speak(NULL, SPF_PURGEBEFORESPEAK, NULL);
        }
        isPlaying = false;
    }
};

WindowsTTS::WindowsTTS() : pImpl(std::make_unique<Impl>()) {}
WindowsTTS::~WindowsTTS() { pImpl->stop(); }
bool WindowsTTS::speak(const std::string& text) { return pImpl->speakAsync(text); }
bool WindowsTTS::isSpeaking() const { return pImpl->isPlaying.load(); }
void WindowsTTS::stop() { pImpl->stop(); }