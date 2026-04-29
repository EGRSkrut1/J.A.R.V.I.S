#pragma once
#include <string>

class ILLMClient {
public:
    virtual ~ILLMClient() = default;
    virtual std::string ask(const std::string& prompt) = 0;
    virtual void setModel(const std::string& model) = 0;
    virtual void setSystemPrompt(const std::string& prompt) = 0;
    virtual void setSystemsInfo(const std::string& info) = 0;
};