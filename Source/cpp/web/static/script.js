const launcher = document.getElementById('launcher');
const mainApp = document.getElementById('mainApp');
const activateBtn = document.getElementById('activateJarvis');
const settingsBtn = document.getElementById('settingsBtn');
const chatArea = document.getElementById('chatArea');
const userInput = document.getElementById('userInput');
const sendBtn = document.getElementById('sendBtn');
const statusLabel = document.getElementById('statusLabel');
const settingsPanel = document.getElementById('settingsPanel');
const logPanel = document.getElementById('logPanel');
const showLogsCheckbox = document.getElementById('showLogs');
const voiceSelect = document.getElementById('voiceSelect');
const modelSelect = document.getElementById('modelSelect');

let isProcessing = false;

// Логи
if (logPanel) logPanel.style.display = 'none';

function log(msg, type = 'info') {
    const logArea = document.getElementById('logArea');
    if (!logArea || !showLogsCheckbox.checked) return;
    const entry = document.createElement('div');
    entry.className = `log-entry log-${type}`;
    entry.textContent = `[${new Date().toLocaleTimeString()}] ${msg}`;
    logArea.appendChild(entry);
    logArea.scrollTop = logArea.scrollHeight;
}

// Чекбокс логов
if (showLogsCheckbox) {
    showLogsCheckbox.addEventListener('change', () => {
        if (logPanel) logPanel.style.display = showLogsCheckbox.checked ? 'flex' : 'none';
    });
}

// Настройки
settingsBtn.onclick = () => {
    settingsPanel.classList.toggle('hidden');
};

document.getElementById('settingsClose').onclick = () => {
    settingsPanel.classList.add('hidden');
};

document.getElementById('logClose').onclick = () => {
    if (logPanel) logPanel.style.display = 'none';
    if (showLogsCheckbox) showLogsCheckbox.checked = false;
};

// Печатание текста побуквенно
async function typeText(element, text, speed = 20) {
    let index = 0;
    const bubble = element.querySelector('.bubble') || element;
    bubble.textContent = '';
    
    return new Promise(resolve => {
        const interval = setInterval(() => {
            if (index < text.length) {
                bubble.textContent += text.charAt(index);
                index++;
                chatArea.scrollTop = chatArea.scrollHeight;
            } else {
                clearInterval(interval);
                resolve();
            }
        }, speed);
    });
}

// Добавление сообщения с печатанием
async function appendMessage(sender, text, animate = true) {
    const div = document.createElement('div');
    div.classList.add('message', sender === 'user' ? 'user-message' : 'jarvis-message');
    div.innerHTML = '<div class="bubble"></div>';
    chatArea.appendChild(div);
    chatArea.scrollTop = chatArea.scrollHeight;
    
    if (animate && text.length < 500) {
        await typeText(div, text, 15);
    } else {
        div.querySelector('.bubble').textContent = text;
    }
    return div;
}

// Статус "думает"
function showThinking() {
    const div = document.createElement('div');
    div.classList.add('message', 'jarvis-message', 'thinking');
    div.id = 'thinkingMessage';
    div.innerHTML = '<div class="bubble"><span class="dot"></span><span class="dot"></span><span class="dot"></span></div>';
    chatArea.appendChild(div);
    chatArea.scrollTop = chatArea.scrollHeight;
}

function hideThinking() {
    const thinking = document.getElementById('thinkingMessage');
    if (thinking) thinking.remove();
}

// Отправка сообщения
async function sendMessage(text) {
    if (!text.trim() || isProcessing) return;
    isProcessing = true;
    
    await appendMessage('user', text, true);
    userInput.value = '';
    log(`Отправка: "${text}"`, 'info');
    
    showThinking();
    updateStatus('processing', 'Думает...');
    
    const voice = voiceSelect.value;
    const model = modelSelect.value;
    
    try {
        const res = await fetch('http://localhost:8080/chat', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ message: text, voice: voice, model: model })
        });
        const data = await res.json();
        
        hideThinking();
        await appendMessage('jarvis', data.reply, true);
        log(`Ответ: "${data.reply.substring(0, 80)}..."`, 'success');
        updateStatus('idle', 'Готов');
    } catch (e) {
        hideThinking();
        await appendMessage('jarvis', 'Ошибка подключения, сэр.', false);
        log('Ошибка подключения', 'error');
        updateStatus('error', 'Ошибка');
    }
    
    isProcessing = false;
}

function escapeHtml(str) {
    return str.replace(/[&<>]/g, m => ({ '&':'&amp;', '<':'&lt;', '>':'&gt;' }[m]));
}

function updateStatus(type, text) {
    const led = document.querySelector('.status-led');
    if (led) {
        const colors = { idle: '#00ff00', listening: '#ffaa00', processing: '#ffaa00', speaking: '#00aaff' };
        led.style.backgroundColor = colors[type] || '#00ff00';
    }
    if (statusLabel) statusLabel.textContent = text;
    if (window.reactor) window.reactor.setState(type);
}

// Опрос статуса с сервера
async function pollStatus() {
    try {
        const res = await fetch('http://localhost:8080/status');
        const data = await res.json();
        updateStatus(data.status, getStatusText(data.status));
    } catch (e) {}
}
setInterval(pollStatus, 500);

function getStatusText(type) {
    const texts = { idle: 'Готов', listening: 'Слушаю', processing: 'Думаю', speaking: 'Отвечаю' };
    return texts[type] || 'Активен';
}

// Инициализация
activateBtn.onclick = () => {
    launcher.style.opacity = '0';
    setTimeout(() => {
        launcher.classList.add('hidden');
        mainApp.classList.remove('hidden');
        log('Интерфейс активирован', 'success');
    }, 500);
};

sendBtn.onclick = () => sendMessage(userInput.value);
userInput.addEventListener('keypress', (e) => {
    if (e.key === 'Enter' && !e.shiftKey) {
        e.preventDefault();
        sendMessage(userInput.value);
    }
});

// Обновление сообщений
async function updateMessages() {
    try {
        const res = await fetch('http://localhost:8080/messages');
        const messages = await res.json();
        if (messages.length !== chatArea.querySelectorAll('.message:not(.thinking)').length) {
            chatArea.innerHTML = '';
            for (const msg of messages) {
                if (msg.startsWith('Вы:')) await appendMessage('user', msg.substring(3).trim(), false);
                else if (msg.startsWith('Джарвис:')) await appendMessage('jarvis', msg.substring(8).trim(), false);
            }
        }
    } catch (e) {}
}
setInterval(updateMessages, 1000);  