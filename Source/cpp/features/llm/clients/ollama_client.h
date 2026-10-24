#pragma once
#include "features/llm/interfaces/i_llm_client.h"
#include <memory>
#include <vector>
#include <string>

/**
 * @brief Клиент для работы с локальной LLM через Ollama.
 */
class OllamaClient : public ILLMClient {
public:
    OllamaClient();
    ~OllamaClient();
    
    std::string ask(const std::string& prompt) override;
    std::string askWithInternet(const std::string& prompt);
    
    /**
     * @brief Запрос к LLM с поддержкой Tool Calling (Function Calling).
     * @param prompt Текст пользователя.
     * @param moduleManager Указатель на менеджер модулей.
     * @return Ответ LLM.
     */
    std::string askWithTools(const std::string& prompt, class ModuleManager* moduleManager);

    /**
     * @brief Очищает историю контекста LLM.
     */
    void clearHistory();

    void setModel(const std::string& model) override;
    void setSystemPrompt(const std::string& prompt) override;
    void setSystemsInfo(const std::string& info) override;
    
private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;
};