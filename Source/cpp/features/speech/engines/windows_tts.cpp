#include "windows_tts.h"
#include "core/logging/logger.h"
#include <windows.h>
#include <sapi.h>
#include <sphelper.h>

#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "sapi.lib")

struct WindowsTTS::Impl {
    bool isPlaying = false;
    
    bool speak(const std::string& text) {
        ISpVoice* pVoice = NULL;
        if (FAILED(::CoInitialize(NULL))) return false;
        
        HRESULT hr = CoCreateInstance(CLSID_SpVoice, NULL, CLSCTX_ALL, IID_ISpVoice, (void**)&pVoice);
        if (SUCCEEDED(hr)) {
            // Ищем русский голос
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
            
            int wlen = MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, NULL, 0);
            std::wstring wtext(wlen, 0);
            MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, &wtext[0], wlen);
            
            isPlaying = true;
            hr = pVoice->Speak(wtext.c_str(), SPF_DEFAULT, NULL);
            isPlaying = false;
            pVoice->Release();
        }
        ::CoUninitialize();
        return SUCCEEDED(hr);
    }
};

WindowsTTS::WindowsTTS() : pImpl(std::make_unique<Impl>()) {}
WindowsTTS::~WindowsTTS() = default;
bool WindowsTTS::speak(const std::string& text) { return pImpl->speak(text); }
bool WindowsTTS::isSpeaking() const { return pImpl->isPlaying; }
void WindowsTTS::stop() {}