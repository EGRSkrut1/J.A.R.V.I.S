#pragma once
#include "modules/interfaces/i_command.h"
#include "modules/interfaces/i_app_controller.h"
#include <memory>
#include <vector>
#include <string>

/**
 * @brief Менеджер модулей и приложений.
 * 
 * Реализует маршрутизацию входящих команд: сначала проверяет приложения
 * (Spotify, Steam), затем команды (Дерево, Шутки), а если совпадений нет — 
 * возвращает пустую строку (сигнал для отправки запроса в LLM).
 */
class ModuleManager {
public:
    ModuleManager();
    ~ModuleManager();
    
    /**
     * @brief Регистрация голосовой команды.
     */
    void registerCommand(std::shared_ptr<ICommand> cmd);
    
    /**
     * @brief Регистрация контроллера приложения.
     */
    void registerApp(std::shared_ptr<IAppController> app);
    
    /**
     * @brief Главная точка входа для текста от пользователя.
     * @param userInput Текст команды.
     * @return Ответ модуля или пустая строка, если нужно звать LLM.
     */
    std::string handleCommand(const std::string& userInput);

    /**
     * @brief Возвращает путь к последнему проигранному звуку подтверждения.
     * Используется для отдачи audio_url в веб-интерфейс.
     */
    std::string getLastConfirmSound() const;
    
private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;
};