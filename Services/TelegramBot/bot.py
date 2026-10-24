import telebot
import requests
import json
import os
import sys
import subprocess
from telebot import types
from datetime import datetime

BASE_DIR = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))
TELEGRAM_CONFIG_PATH = os.path.join(BASE_DIR, "App", "configs", "telegram.json")
USERS_CONFIG_PATH = os.path.join(BASE_DIR, "App", "configs", "users.json")
SESSIONS_PATH = os.path.join(BASE_DIR, "App", "configs", "telegram_sessions.json")

def load_json(path):
    if not os.path.exists(path): return {}
    try:
        with open(path, "r", encoding="utf-8") as f: return json.load(f)
    except: return {}

def save_json(path, data):
    with open(path, "w", encoding="utf-8") as f:
        json.dump(data, f, ensure_ascii=False, indent=2)

tg_config = load_json(TELEGRAM_CONFIG_PATH)
users_config = load_json(USERS_CONFIG_PATH)
sessions = load_json(SESSIONS_PATH)

for k, v in list(sessions.items()):
    if isinstance(v, str):
        sessions[k] = {
            "username": v,
            "role": "user",
            "name": v,
            "gender": "unknown",
            "platform": "Telegram",
            "login_time": datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        }
save_json(SESSIONS_PATH, sessions)

TOKEN = tg_config.get("bot_token")
if not TOKEN or TOKEN == "YOUR_BOT_TOKEN":
    print("[ERROR] Please set a valid bot_token in telegram.json")
    sys.exit(1)

JARVIS_URL = 'http://127.0.0.1:8080'
bot = telebot.TeleBot(TOKEN)
user_by_password = {u["password"]: u for u in users_config.get("users", [])}

def get_user(chat_id):
    session = sessions.get(str(chat_id))
    if not session or not isinstance(session, dict): return None
    username = session.get("username")
    if not username: return None
    for u in users_config.get("users", []):
        if u["username"] == username: return u
    return None

def send_to_jarvis(text, user_name):
    try:
        res = requests.post(f'{JARVIS_URL}/chat', json={"message": text, "user_name": user_name}, timeout=60)
        if res.status_code == 200:
            return res.json().get('reply', 'Нет ответа.')
        return f"Ошибка сервера: {res.status_code}"
    except Exception as e:
        return f"Ошибка подключения: {e}"

def main_menu(is_admin=False):
    markup = types.InlineKeyboardMarkup(row_width=2)
    markup.add(
        types.InlineKeyboardButton("Приложения", callback_data="menu_apps"),
        types.InlineKeyboardButton("IoT", callback_data="menu_iot")
    )
    markup.add(
        types.InlineKeyboardButton("Статус", callback_data="menu_status"),
        types.InlineKeyboardButton("Шутка", callback_data="cmd_пошути")
    )
    markup.add(
        types.InlineKeyboardButton("Помощь", callback_data="menu_help"),
        types.InlineKeyboardButton("Система", callback_data="menu_system")
    )
    markup.add(types.InlineKeyboardButton("Выйти", callback_data="logout"))
    return markup

def system_menu(is_admin=False):
    markup = types.InlineKeyboardMarkup(row_width=2)
    if is_admin:
        markup.add(
            types.InlineKeyboardButton("Сессии", callback_data="sys_sessions"),
            types.InlineKeyboardButton("Логи", callback_data="sys_logs")
        )
    markup.add(types.InlineKeyboardButton("Назад", callback_data="menu_main"))
    return markup

def apps_menu():
    markup = types.InlineKeyboardMarkup(row_width=2)
    markup.add(
        types.InlineKeyboardButton("Steam", callback_data="menu_steam"),
        types.InlineKeyboardButton("Spotify", callback_data="menu_spotify"),
        types.InlineKeyboardButton("Discord", callback_data="cmd_открой дискорд"),
        types.InlineKeyboardButton("DS4", callback_data="cmd_запусти ds4"),
        types.InlineKeyboardButton("Запрет", callback_data="cmd_включи запрет"),
        types.InlineKeyboardButton("Браузер", callback_data="cmd_открой браузер"),
        types.InlineKeyboardButton("Назад", callback_data="menu_main")
    )
    return markup

def spotify_menu():
    markup = types.InlineKeyboardMarkup(row_width=2)
    markup.add(
        types.InlineKeyboardButton("Включить", callback_data="cmd_включи музыку"),
        types.InlineKeyboardButton("Пауза", callback_data="cmd_пауза"),
        types.InlineKeyboardButton("Следующий", callback_data="cmd_следующий трек"),
        types.InlineKeyboardButton("Предыдущий", callback_data="cmd_предыдущий трек"),
        types.InlineKeyboardButton("Перемешать", callback_data="cmd_включи перемешивание"),
        types.InlineKeyboardButton("Назад", callback_data="menu_apps")
    )
    return markup

def steam_menu():
    markup = types.InlineKeyboardMarkup(row_width=2)
    markup.add(
        types.InlineKeyboardButton("Открыть Steam", callback_data="cmd_открой стим"),
        types.InlineKeyboardButton("Запустить игру", callback_data="ask_game")
    )
    markup.add(types.InlineKeyboardButton("Назад", callback_data="menu_apps"))
    return markup

def iot_menu():
    markup = types.InlineKeyboardMarkup(row_width=2)
    devices_config = load_json(os.path.join(BASE_DIR, "App", "configs", "devices.json"))
    tree_name = "Дерево"
    if "devices" in devices_config and len(devices_config["devices"]) > 0:
        tree_name = devices_config["devices"][0].get("name", "Дерево")
    markup.add(
        types.InlineKeyboardButton(f"{tree_name} Вкл", callback_data="cmd_включи дерево"),
        types.InlineKeyboardButton(f"{tree_name} Выкл", callback_data="cmd_выключи дерево"),
        types.InlineKeyboardButton("Назад", callback_data="menu_main")
    )
    return markup

@bot.message_handler(commands=['start'])
def send_welcome(message):
    user = get_user(message.chat.id)
    if user:
        name = user.get("name", user['username'])
        is_admin = (user.get('role') == 'admin')
        bot.send_message(message.chat.id, f"С возвращением, {name}.", reply_markup=main_menu(is_admin))
    else:
        bot.send_message(message.chat.id, "Здравствуйте. Введите пароль для доступа:")

@bot.message_handler(func=lambda message: str(message.chat.id) not in sessions, content_types=['text'])
def check_password(message):
    text = message.text.strip()
    if text in user_by_password:
        user = user_by_password[text]
        sessions[str(message.chat.id)] = {
            "username": user.get("username", ""),
            "role": user.get("role", "user"),
            "name": user.get("name", user.get("username", "")),
            "gender": user.get("gender", "unknown"),
            "platform": "Telegram",
            "login_time": datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        }
        save_json(SESSIONS_PATH, sessions)
        name = user.get("name", user['username'])
        is_admin = (user.get('role') == 'admin')
        bot.send_message(message.chat.id, f"Авторизация успешна. Добро пожаловать, {name}.", reply_markup=main_menu(is_admin))
    else:
        bot.send_message(message.chat.id, "Неверный пароль. Попробуйте снова.")

@bot.callback_query_handler(func=lambda call: True)
def handle_callback(call):
    user = get_user(call.message.chat.id)
    if not user:
        bot.answer_callback_query(call.id, "Сначала авторизуйтесь через /start")
        return

    data = call.data
    is_admin = (user['role'] == "admin")
    user_name = user.get("name", user['username'])

    if data == "logout":
        if str(call.message.chat.id) in sessions:
            del sessions[str(call.message.chat.id)]
            save_json(SESSIONS_PATH, sessions)
        bot.send_message(call.message.chat.id, "Вы вышли из аккаунта. Введите пароль для входа:")
    elif data == "menu_main":
        bot.edit_message_text("Главное меню:", call.message.chat.id, call.message.message_id, reply_markup=main_menu(is_admin))
    elif data == "menu_apps":
        bot.edit_message_text("Приложения:", call.message.chat.id, call.message.message_id, reply_markup=apps_menu())
    elif data == "menu_spotify":
        bot.edit_message_text("Управление Spotify:", call.message.chat.id, call.message.message_id, reply_markup=spotify_menu())
    elif data == "menu_steam":
        bot.edit_message_text("Управление Steam:", call.message.chat.id, call.message.message_id, reply_markup=steam_menu())
    elif data == "menu_iot":
        bot.edit_message_text("Управление IoT:", call.message.chat.id, call.message.message_id, reply_markup=iot_menu())
    elif data == "menu_system":
        if not is_admin:
            bot.answer_callback_query(call.id, "Только админ.", show_alert=True)
            return
        bot.edit_message_text("Управление системой:", call.message.chat.id, call.message.message_id, reply_markup=system_menu(is_admin))
    elif data == "menu_status":
        try:
            res = requests.get(f'{JARVIS_URL}/status')
            status = res.json().get('status', 'unknown')
            text = f"Статус системы: {status}\n\n(Логи и детали в веб-интерфейсе)"
        except:
            text = "Сервер недоступен (Ядро выключено)."
        bot.edit_message_text(text, call.message.chat.id, call.message.message_id, reply_markup=types.InlineKeyboardMarkup().add(types.InlineKeyboardButton("Назад", callback_data="menu_main")))
    elif data == "menu_help":
        help_text = (
            "Список доступных команд:\n"
            "Приложения: Steam, Spotify, Discord, DS4, Запрет, Браузер.\n"
            "IoT: Управление деревом.\n"
            "Статус: Проверка состояния.\n\n"
            "Также можно писать текстом: 'включи музыку', 'пошути' и т.д."
        )
        bot.edit_message_text(help_text, call.message.chat.id, call.message.message_id, reply_markup=types.InlineKeyboardMarkup().add(types.InlineKeyboardButton("Назад", callback_data="menu_main")))
    elif data == "sys_sessions":
        if not is_admin: return
        text = "Все активные сессии:\n\n"
        
        try:
            res = requests.get(f'{JARVIS_URL}/admin/sessions')
            if res.status_code == 200:
                web_sessions = res.json()
                if web_sessions:
                    text += "--- Web ---\n"
                    for ws in web_sessions:
                        if ws.get('platform') != 'Telegram':
                            text += f"• {ws.get('user')} [{ws.get('platform', 'Web')}] ({ws.get('login_time')})\n"
        except Exception as e:
            text += f"[Web ошибка: {e}]\n"

        tg_sessions = load_json(SESSIONS_PATH)
        if tg_sessions:
            text += "\n--- Telegram ---\n"
            for chat_id, s_data in tg_sessions.items():
                if isinstance(s_data, dict):
                    text += f"• {s_data.get('username', '?')} ({s_data.get('login_time', '?')})\n"
                else:
                    text += f"• {s_data} (старый формат)\n"
        
        bot.edit_message_text(text, call.message.chat.id, call.message.message_id, reply_markup=types.InlineKeyboardMarkup().add(types.InlineKeyboardButton("Назад", callback_data="menu_system")))
    elif data == "sys_logs":
        if not is_admin: return
        try:
            res = requests.get(f'{JARVIS_URL}/admin/logs')
            log_text = res.text if res.status_code == 200 else "Ошибка загрузки логов."
            if len(log_text) > 4000:
                log_text = log_text[-4000:]
            bot.edit_message_text(f"Логи:\n```\n{log_text}\n```", call.message.chat.id, call.message.message_id, reply_markup=types.InlineKeyboardMarkup().add(types.InlineKeyboardButton("Назад", callback_data="menu_system")), parse_mode='Markdown')
        except Exception as e:
            bot.edit_message_text(f"Ошибка: {e}", call.message.chat.id, call.message.message_id, reply_markup=types.InlineKeyboardMarkup().add(types.InlineKeyboardButton("Назад", callback_data="menu_system")))
    elif data == "ask_game":
        msg = bot.send_message(call.message.chat.id, "Введите название игры:")
        bot.register_next_step_handler(msg, process_game_name)
    elif data.startswith("cmd_"):
        command = data[4:]
        bot.answer_callback_query(call.id, f"Выполняю: {command}")
        reply = send_to_jarvis(command, user_name)
        bot.send_message(call.message.chat.id, f"Джарвис: {reply}")

def process_game_name(message):
    user = get_user(message.chat.id)
    if not user: return
    user_name = user.get("name", user['username'])
    reply = send_to_jarvis(f"запусти игру {message.text}", user_name)
    bot.reply_to(message, f"Джарвис: {reply}")

@bot.message_handler(func=lambda message: str(message.chat.id) in sessions, content_types=['text'])
def handle_text(message):
    user = get_user(message.chat.id)
    if not user: return
    user_name = user.get("name", user['username'])
    bot.send_chat_action(message.chat.id, 'typing')
    reply = send_to_jarvis(message.text, user_name)
    bot.reply_to(message, f"Джарвис: {reply}", reply_markup=main_menu(user.get('role') == 'admin'))

if __name__ == '__main__':
    print("Telegram Bot started.")
    bot.infinity_polling()