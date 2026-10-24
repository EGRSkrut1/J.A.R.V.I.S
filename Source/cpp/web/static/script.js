const API_BASE = `http://${window.location.hostname}:8080`;

function getToken() { return localStorage.getItem('jarvis_token'); }
function getRole()  { return localStorage.getItem('jarvis_role'); }
function getUser()  { return localStorage.getItem('jarvis_user'); }
function authHeaders() { const t = getToken(); return t ? { 'X-Auth-Token': t } : {}; }

let statusTimer = null;
let messagesTimer = null;
let lastMessageCount = 0;

async function login() {
    const username = document.getElementById('loginUser').value.trim();
    const password = document.getElementById('loginPass').value;
    const errEl = document.getElementById('loginError');
    errEl.textContent = '';
    try {
        const res = await fetch(`${API_BASE}/login`, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ username, password })
        });
        if (!res.ok) { errEl.textContent = 'Invalid login or password'; return; }
        const data = await res.json();
        localStorage.setItem('jarvis_token', data.token);
        localStorage.setItem('jarvis_role', data.role);
        localStorage.setItem('jarvis_user', data.username);
        showMainApp();
    } catch (e) { errEl.textContent = 'Server unavailable'; }
}

async function logout() {
    try { await fetch(`${API_BASE}/logout`, { method: 'POST', headers: authHeaders() }); } catch (e) {}
    showLogin();
}

function showLogin() {
    stopPolling();
    localStorage.removeItem('jarvis_token');
    localStorage.removeItem('jarvis_role');
    localStorage.removeItem('jarvis_user');
    document.getElementById('mainApp').classList.add('hidden');
    document.getElementById('loginScreen').classList.remove('hidden');
}

function showMainApp() {
    document.getElementById('loginScreen').classList.add('hidden');
    document.getElementById('mainApp').classList.remove('hidden');
    document.getElementById('userBadge').textContent = `${getUser()} (${getRole()})`;
    
    const existingBtn = document.querySelector('.admin-btn');
    if (existingBtn) existingBtn.remove();

    if (getRole() === 'admin') {
        const btn = document.createElement('div');
        btn.className = 'settings-btn admin-btn';
        btn.textContent = 'A';
        btn.title = 'Admin panel';
        btn.onclick = () => document.getElementById('adminPanel').classList.toggle('hidden');
        document.querySelector('.status-badge').prepend(btn);
    }
    
    startPolling();
    updateMessages(true);
}

function startPolling() {
    stopPolling();
    statusTimer = setInterval(pollStatus, 1000);
    messagesTimer = setInterval(updateMessages, 1500);
}

function stopPolling() {
    if (statusTimer) { clearInterval(statusTimer); statusTimer = null; }
    if (messagesTimer) { clearInterval(messagesTimer); messagesTimer = null; }
}

async function pollStatus() {
    if (!getToken()) return;
    try {
        const res = await fetch(`${API_BASE}/status`, { headers: authHeaders() });
        if (res.status === 401) { showLogin(); return; }
        const data = await res.json();
        updateStatus(data.status, getStatusText(data.status));
    } catch (e) {}
}

async function updateMessages(force = false) {
    if (!getToken()) return;
    try {
        const res = await fetch(`${API_BASE}/messages`, { headers: authHeaders() });
        if (res.status === 401) { showLogin(); return; }
        const messages = await res.json();
        
        if (messages.length !== lastMessageCount || force) {
            chatArea.innerHTML = '';
            for (const msg of messages) {
                if (msg.startsWith('You:')) {
                    const parts = msg.substring(4).split('|');
                    if (parts.length >= 2) {
                        const timeStr = parts[0].replace(/[\[\]]/g, '');
                        await appendMessage('user', timeStr, parts[1], "", "");
                    }
                } else if (msg.startsWith('Jarvis:')) {
                    const parts = msg.substring(7).split('|');
                    if (parts.length >= 4) {
                        const timeStr = parts[0].replace(/[\[\]]/g, '');
                        await appendMessage('jarvis', timeStr, parts[3], parts[1], parts[2]);
                    }
                }
            }
            lastMessageCount = messages.length;
        }
    } catch (e) {}
}

function speakClient(text) {
    if (!window.speechSynthesis) return;
    window.speechSynthesis.cancel();
    const utter = new SpeechSynthesisUtterance(text);
    utter.lang = 'ru-RU';
    const voices = window.speechSynthesis.getVoices();
    const ru = voices.find(v => v.lang === 'ru-RU');
    if (ru) utter.voice = ru;
    window.speechSynthesis.speak(utter);
}

function playServerAudio(url) {
    const audio = new Audio(`${API_BASE}${url}`);
    audio.play().catch(e => console.warn('audio play failed:', e));
}

const chatArea = document.getElementById('chatArea');
const userInput = document.getElementById('userInput');
const sendBtn = document.getElementById('sendBtn');
const statusLabel = document.getElementById('statusLabel');
const settingsPanel = document.getElementById('settingsPanel');
const voiceSelect = document.getElementById('voiceSelect');
const useInternetCheckbox = document.getElementById('useInternet');

let isProcessing = false;

document.getElementById('settingsBtn').onclick = () => settingsPanel.classList.toggle('hidden');
document.getElementById('settingsClose').onclick = () => settingsPanel.classList.add('hidden');
document.getElementById('logoutBtn').onclick = logout;

const adminClose = document.getElementById('adminClose');
if (adminClose) adminClose.onclick = () => document.getElementById('adminPanel').classList.add('hidden');

const showSessionsBtn = document.getElementById('showSessionsBtn');
if (showSessionsBtn) showSessionsBtn.onclick = async () => {
    const res = await fetch(`${API_BASE}/admin/sessions`, { headers: authHeaders() });
    if (res.status === 403 || res.status === 401) { document.getElementById('adminContent').textContent = 'Access denied'; return; }
    const data = await res.json();
    document.getElementById('adminContent').textContent = JSON.stringify(data, null, 2);
};

const showLogsBtn = document.getElementById('showLogsBtn');
if (showLogsBtn) showLogsBtn.onclick = async () => {
    const res = await fetch(`${API_BASE}/admin/logs`, { headers: authHeaders() });
    if (res.status === 403 || res.status === 401) { document.getElementById('adminContent').textContent = 'Access denied'; return; }
    const data = await res.text();
    document.getElementById('adminContent').textContent = data;
};

async function appendMessage(sender, timeStr, text, moduleName = "", timeMs = "") {
    const div = document.createElement('div');
    div.classList.add('message', sender === 'user' ? 'user-message' : 'jarvis-message');
    
    const bubble = document.createElement('div');
    bubble.className = 'bubble';

    if (sender === 'jarvis' && moduleName) {
        const header = document.createElement('div');
        header.className = 'bubble-header';
        header.textContent = `${moduleName} (${timeMs} ms)`;
        
        const details = document.createElement('div');
        details.className = 'bubble-details';
        details.innerHTML = `<p>Модуль: ${moduleName}</p><p>Время: ${timeMs} ms</p>`;
        
        header.onclick = () => {
            details.classList.toggle('show');
            header.classList.toggle('open');
        };
        
        bubble.appendChild(header);
        bubble.appendChild(details);
    }

    const textNode = document.createElement('div');
    textNode.className = 'bubble-text';
    textNode.textContent = text;
    bubble.appendChild(textNode);

    const footer = document.createElement('div');
    footer.className = 'bubble-footer';
    footer.textContent = timeStr;
    bubble.appendChild(footer);

    div.appendChild(bubble);
    chatArea.appendChild(div);
    chatArea.scrollTop = chatArea.scrollHeight;
}

function showThinking() {
    const t = document.getElementById('thinkingMessage');
    if (t) t.remove();
    
    const div = document.createElement('div');
    div.classList.add('message', 'jarvis-message', 'thinking');
    div.id = 'thinkingMessage';
    div.innerHTML = '<div class="bubble"><span class="dot"></span><span class="dot"></span><span class="dot"></span></div>';
    chatArea.appendChild(div);
    chatArea.scrollTop = chatArea.scrollHeight;
}

async function sendMessage(text) {
    if (!text.trim() || isProcessing) return;
    isProcessing = true;
    
    const now = new Date();
    const userTime = now.toLocaleTimeString('ru-RU', { hour12: false });
    await appendMessage('user', userTime, text);
    userInput.value = '';
    
    showThinking();
    updateStatus('processing', 'Thinking...');

    stopPolling();

    const useInternet = useInternetCheckbox && useInternetCheckbox.checked;

    try {
        const res = await fetch(`${API_BASE}/chat`, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json', ...authHeaders() },
            body: JSON.stringify({ message: text, internet: useInternet })
        });
        if (res.status === 401) { showLogin(); return; }
        const data = await res.json();
        
        if (data.audio_url) {
            playServerAudio(data.audio_url);
        } else if (voiceSelect.value === 'client' && !data.is_module) {
            speakClient(data.reply);
        }
        
        lastMessageCount = -1; 
        await updateMessages(true);
        
        updateStatus('idle', 'Ready');
    } catch (e) {
        updateStatus('error', 'Error');
    }
    
    startPolling();
    isProcessing = false;
    const t = document.getElementById('thinkingMessage');
    if (t) t.remove();
}

function updateStatus(type, text) {
    const led = document.querySelector('.status-led');
    if (led) {
        const colors = { idle: '#00ff00', listening: '#ffaa00', processing: '#ffaa00', speaking: '#00aaff' };
        led.style.backgroundColor = colors[type] || '#00ff00';
    }
    if (statusLabel) statusLabel.textContent = text;
}

function getStatusText(t) {
    return { idle: 'Ready', listening: 'Listening', processing: 'Thinking', speaking: 'Speaking' }[t] || 'Active';
}

document.getElementById('loginBtn').onclick = login;
document.getElementById('loginPass').addEventListener('keypress', e => { if (e.key === 'Enter') login(); });

sendBtn.onclick = () => sendMessage(userInput.value);
userInput.addEventListener('keypress', e => {
    if (e.key === 'Enter' && !e.shiftKey) {
        e.preventDefault();
        sendMessage(userInput.value);
    }
});

(function boot() {
    if (!getToken()) {
        document.getElementById('loginScreen').classList.remove('hidden');
        return;
    }
    fetch(`${API_BASE}/me`, { headers: authHeaders() })
        .then(r => {
            if (r.ok) return r.json();
            throw new Error('unauthorized');
        })
        .then(() => showMainApp())
        .catch(() => showLogin());
})();