#pragma once
#include "features/llm/interfaces/i_llm_client.h"
#include <memory>
#include <vector>
#include <string>

class OllamaClient : public ILLMClient {
public:
    OllamaClient();
    ~OllamaClient();
    
    std::string ask(const std::string& prompt) override;
    void setModel(const std::string& model) override;
    void setSystemPrompt(const std::string& prompt) override;
    void setSystemsInfo(const std::string& info) override;
    
private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;
};