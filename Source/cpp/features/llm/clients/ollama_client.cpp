#include "ollama_client.h"
#include "core/network/http_client.h"
#include "core/logging/logger.h"
#include "modules/module_manager.h"
#include <nlohmann/json.hpp>
#include <chrono>
#include <iomanip>
#include <ctime>
#include <sstream>
#include <regex>
#include <algorithm>

using json = nlohmann::json;

struct OllamaClient::Impl {
    std::string model = "llama3.1:8b";
    std::string systemPrompt;
    std::string systemsInfo;
    std::vector<std::pair<std::string, std::string>> history;
    std::string currentUser = "";
    const int maxHistory = 10;

    std::string getCurrentDateTime() {
        auto now = std::chrono::system_clock::now();
        auto time_t_now = std::chrono::system_clock::to_time_t(now);
        std::tm tm_now;
        localtime_s(&tm_now, &time_t_now);
        std::stringstream ss;
        ss << std::put_time(&tm_now, "%d %B %Y года, %H:%M");
        std::string result = ss.str();
        const std::string monthsEn[] = {"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};
        const std::string monthsRu[] = {"января", "февраля", "марта", "апреля", "мая", "июня", "июля", "августа", "сентября", "октября", "ноября", "декабря"};
        for (int i = 0; i < 12; ++i) {
            size_t pos = result.find(monthsEn[i]);
            if (pos != std::string::npos) { result.replace(pos, monthsEn[i].length(), monthsRu[i]); break; }
        }
        return result;
    }

    std::string cleanUtf8(const std::string& str) {
        std::string result;
        result.reserve(str.size());
        for (size_t i = 0; i < str.size(); ) {
            unsigned char c = str[i];
            if (c < 0x80) { result += c; i++; }
            else if ((c & 0xE0) == 0xC0 && i + 1 < str.size()) { result += str.substr(i, 2); i += 2; }
            else if ((c & 0xF0) == 0xE0 && i + 2 < str.size()) { result += str.substr(i, 3); i += 3; }
            else if ((c & 0xF8) == 0xF0 && i + 3 < str.size()) { result += str.substr(i, 4); i += 4; }
            else { i++; }
        }
        return result;
    }

    std::string extractUser(const std::string& prompt) {
        std::regex user_regex(R"(\[Пользователь:\s*([^,\]]+))");
        std::smatch match;
        if (std::regex_search(prompt, match, user_regex) && match.size() > 1) {
            return match[1].str();
        }
        return "";
    }

    std::string askInternal(const std::string& prompt, bool useInternet, ModuleManager* moduleManager) {
        std::string user = extractUser(prompt);
        if (!user.empty() && user != currentUser) {
            history.clear();
            currentUser = user;
        }

        history.push_back({"user", cleanUtf8(prompt)});
        if (history.size() > maxHistory) history.erase(history.begin());

        json request;
        request["model"] = model;
        request["stream"] = false;
        
        json messages = json::array();
        messages.push_back({{"role", "system"}, {"content", cleanUtf8(systemPrompt + "\n\nТЕКУЩАЯ ДАТА: " + getCurrentDateTime())}});
        for (const auto& [role, content] : history) {
            messages.push_back({{"role", role}, {"content", cleanUtf8(content)}});
        }
        request["messages"] = messages;

        json tools = json::array();
        if (useInternet) {
            tools.push_back({
                {"type", "function"},
                {"function", {
                    {"name", "web_search"},
                    {"description", "Search the internet for current information."},
                    {"parameters", {
                        {"type", "object"},
                        {"properties", {
                            {"query", {{"type", "string"}, {"description", "The search query"}}}
                        }},
                        {"required", json::array({"query"})}
                    }}
                }}
            });
        }

        if (moduleManager) {
            tools.push_back({
                {"type", "function"},
                {"function", {
                    {"name", "execute_system_command"},
                    {"description", "Выполняет одну или несколько системных команд Джарвиса. Если пользователь просит несколько вещей, верни массив команд."},
                    {"parameters", {
                        {"type", "object"},
                        {"properties", {
                            {"commands", {
                                {"type", "array"},
                                {"items", {{"type", "string"}}},
                                {"description", "Массив команд для выполнения, например: [\"включи дерево\", \"открой стим\"]"}
                            }}
                        }},
                        {"required", json::array({"commands"})}
                    }}
                }}
            });
        }

        if (!tools.empty()) request["tools"] = tools;

        std::string reqStr = request.dump(-1, ' ', false, json::error_handler_t::replace);
        auto [code, response] = HttpClient::post("http://localhost:11434/api/chat", reqStr, "", 60L);
        
        if (code == -1) return "ERROR: curl failed";

        try {
            json resp = json::parse(response, nullptr, false);
            if (resp.is_discarded()) return "ERROR: JSON parse error";
            if (resp.contains("error")) return "Ollama error: " + resp["error"].get<std::string>();

            auto msg = resp["message"];
            
            if (msg.contains("tool_calls") && !msg["tool_calls"].empty()) {
                json assistantMsg;
                assistantMsg["role"] = "assistant";
                assistantMsg["content"] = msg.value("content", "");
                assistantMsg["tool_calls"] = msg["tool_calls"];
                messages.push_back(assistantMsg);

                for (const auto& toolCall : msg["tool_calls"]) {
                    std::string funcName = toolCall["function"]["name"];
                    auto args = toolCall["function"]["arguments"];
                    std::string toolResult = "";

                    if (funcName == "execute_system_command") {
                        if (args.contains("commands")) {
                            for (const auto& cmd : args["commands"]) {
                                std::string c = cmd.get<std::string>();
                                c.erase(std::remove(c.begin(), c.end(), '['), c.end());
                                c.erase(std::remove(c.begin(), c.end(), ']'), c.end());
                                c.erase(std::remove(c.begin(), c.end(), '"'), c.end());
                                c.erase(std::remove(c.begin(), c.end(), '\''), c.end());
                                
                                Logger::info("[LLM Tool] Executing: " + c);
                                std::string res = moduleManager->handleCommand(c);
                                toolResult += res + " ";
                            }
                        } else if (args.contains("command")) {
                            std::string c = args["command"].get<std::string>();
                            c.erase(std::remove(c.begin(), c.end(), '['), c.end());
                            c.erase(std::remove(c.begin(), c.end(), ']'), c.end());
                            c.erase(std::remove(c.begin(), c.end(), '"'), c.end());
                            c.erase(std::remove(c.begin(), c.end(), '\''), c.end());
                            
                            Logger::info("[LLM Tool] Executing: " + c);
                            toolResult = moduleManager->handleCommand(c);
                        }
                    } 
                    else if (funcName == "web_search") {
                        toolResult = "Поиск выполнен.";
                    }

                    json toolMsg;
                    toolMsg["role"] = "tool";
                    toolMsg["content"] = toolResult;
                    messages.push_back(toolMsg);
                }

                json followUpReq = request;
                followUpReq["messages"] = messages;
                followUpReq.erase("tools");

                std::string reqStr2 = followUpReq.dump(-1, ' ', false, json::error_handler_t::replace);
                auto [code2, response2] = HttpClient::post("http://localhost:11434/api/chat", reqStr2, "", 60L);
                
                if (code2 == 200) {
                    json resp2 = json::parse(response2, nullptr, false);
                    if (!resp2.is_discarded()) {
                        std::string finalAnswer = resp2["message"]["content"];
                        history.push_back({"assistant", cleanUtf8(finalAnswer)});
                        return finalAnswer;
                    }
                }
                return "Выполнено, сэр.";
            } else {
                std::string answer = msg["content"];
                history.push_back({"assistant", cleanUtf8(answer)});
                return answer;
            }
        } catch (const std::exception& e) {
            return "ERROR: JSON parse error: " + std::string(e.what());
        }
    }
};

OllamaClient::OllamaClient() : pImpl(std::make_unique<Impl>()) {}
OllamaClient::~OllamaClient() = default;
std::string OllamaClient::ask(const std::string& prompt) { return pImpl->askInternal(prompt, false, nullptr); }
std::string OllamaClient::askWithInternet(const std::string& prompt) { return pImpl->askInternal(prompt, true, nullptr); }
std::string OllamaClient::askWithTools(const std::string& prompt, ModuleManager* moduleManager) { return pImpl->askInternal(prompt, false, moduleManager); }
void OllamaClient::clearHistory() { pImpl->history.clear(); pImpl->currentUser = ""; }
void OllamaClient::setModel(const std::string& model) { pImpl->model = model; }
void OllamaClient::setSystemPrompt(const std::string& prompt) { pImpl->systemPrompt = prompt; }
void OllamaClient::setSystemsInfo(const std::string& info) { pImpl->systemsInfo = info; }