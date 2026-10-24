#pragma once
#include <string>

/**
 * @brief Интерфейс для управления внешними приложениями (паттерн Plugin).
 * 
 * В отличие от ICommand, контроллеры приложений (Spotify, Steam) 
 * регистрируются отдельно и имеют приоритет при обработке.
 */
class IAppController {
public:
    virtual ~IAppController() = default;

    /**
     * @brief Инициализация приложения (поиск путей, проверка доступности).
     */
    virtual bool initialize() = 0;

    /**
     * @brief Обработка команды приложением.
     * @param command Текст команды в нижнем регистре.
     * @param params Дополнительные параметры (зарезервировано).
     * @return Ответ, если команда относится к приложению, иначе пустая строка.
     */
    virtual std::string executeCommand(const std::string& command, const std::string& params) = 0;

    /**
     * @brief Доступно ли приложение (установлено ли на ПК).
     */
    virtual bool isAvailable() const = 0;
};