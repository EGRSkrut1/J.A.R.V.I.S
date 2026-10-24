#pragma once
#include <string>
#include <vector>

/**
 * @brief Интерфейс для голосовых команд (паттерн Command).
 * 
 * Любой класс, реализующий этот интерфейс, может быть зарегистрирован
 * в ModuleManager и вызываться по голосовому триггеру.
 */
class ICommand {
public:
    virtual ~ICommand() = default;

    /**
     * @brief Возвращает список фраз-триггеров, на которые реагирует команда.
     * @return Вектор строк (например, {"включи свет", "зажги свет"}).
     */
    virtual std::vector<std::string> getTriggers() const = 0;

    /**
     * @brief Выполняет действие команды.
     * @param userInput Полная распознанная фраза пользователя.
     * @return Текстовый ответ для пользователя (озвучивается или пишется в чат).
     */
    virtual std::string execute(const std::string& userInput) = 0;

    /**
     * @brief Проверяет, доступна ли команда в текущей конфигурации.
     * @return true, если команда готова к работе.
     */
    virtual bool isAvailable() const = 0;
};