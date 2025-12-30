const API_BASE = '/api';
let currentUser = null;
let pendingVerifyEmail = null;

function showPage(page) {
    document.querySelectorAll('.page').forEach(p => p.style.display = 'none');
    const el = document.getElementById(page + 'Page');
    if (el) el.style.display = 'block';
}

function toggleAuthUI() {
    const userInfo = document.getElementById('userInfo');
    const username = document.getElementById('username');
    const favoritesLink = document.getElementById('favoritesLink');
    const loginBtn = document.getElementById('loginBtn');
    if (currentUser) {
        userInfo.style.display = 'inline-flex';
        username.textContent = currentUser.username;
        favoritesLink.style.display = 'inline-block';
        loginBtn.style.display = 'none';
    } else {
        userInfo.style.display = 'none';
        favoritesLink.style.display = 'none';
        loginBtn.style.display = 'inline-block';
    }
}

async function searchHouses() {
    const houseList = document.getElementById('houseList');
    houseList.innerHTML = '<p class="loading">加载中...</p>';
    const body = {
        minPrice: parseFloat(document.getElementById('minPrice').value) || undefined,
        maxPrice: parseFloat(document.getElementById('maxPrice').value) || undefined,
        minArea: parseFloat(document.getElementById('minArea').value) || undefined,
        maxArea: parseFloat(document.getElementById('maxArea').value) || undefined,
        communityName: document.getElementById('communityName').value || undefined,
        houseType: document.getElementById('houseType').value || undefined
    };
    Object.keys(body).forEach(k => body[k] === undefined && delete body[k]);

    const res = await fetch(API_BASE + '/houses/search', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(body)
    });
    const data = await res.json();
    renderHouseList(data);
}

function renderHouseList(list) {
    const houseList = document.getElementById('houseList');
    if (!list || list.length === 0) {
        houseList.innerHTML = '<p class="loading">没有找到符合条件的房源</p>';
        return;
    }
    houseList.innerHTML = '';
    list.forEach(item => {
        const card = document.createElement('div');
        card.className = 'house-card';
        card.innerHTML = `
            <h3>${item.houseTitle || '未命名房源'}</h3>
            <div class="price">¥${(item.price || 0).toLocaleString()} 万</div>
            <div class="info">面积：${item.area || '-'} ㎡ | 户型：${item.houseType || '-'} | 单价：${item.unitPrice || '-'} 元/㎡</div>
            <div class="info">小区：${item.communityNa || '-'} | 楼层：${item.floor || '-'}</div>
            <div class="actions">
                <button class="btn-detail" onclick="viewDetail(${item.id})">查看详情</button>
                <button class="btn-favorite" onclick="toggleFavorite(${item.id})">收藏</button>
            </div>
        `;
        houseList.appendChild(card);
    });
}

async function viewDetail(id) {
    showPage('detail');
    const detail = document.getElementById('houseDetail');
    detail.innerHTML = '<p class="loading">加载中...</p>';
    const res = await fetch(`${API_BASE}/house?id=${id}`);
    const data = await res.json();
    detail.innerHTML = `
        <h2>${data.houseTitle || '房源详情'}</h2>
        <div class="detail-grid">
            <div class="detail-item"><div class="detail-label">总价</div><div>${data.price || '-'} 万</div></div>
            <div class="detail-item"><div class="detail-label">面积</div><div>${data.area || '-'} ㎡</div></div>
            <div class="detail-item"><div class="detail-label">单价</div><div>${data.unitPrice || '-'} 元/㎡</div></div>
            <div class="detail-item"><div class="detail-label">户型</div><div>${data.houseType || '-'}</div></div>
            <div class="detail-item"><div class="detail-label">楼层</div><div>${data.floor || '-'}</div></div>
            <div class="detail-item"><div class="detail-label">小区</div><div>${data.communityNa || '-'}</div></div>
            <div class="detail-item"><div class="detail-label">来源</div><div><a href="${data.houseUrl || '#'}" target="_blank">${data.houseUrl || '查看链接'}</a></div></div>
        </div>
        <div class="map-container" id="mapContainer">在此处接入百度地图（需要AK）</div>
    `;
}

function resetFilters() {
    ['minPrice','maxPrice','minArea','maxArea','communityName','houseType'].forEach(id => document.getElementById(id).value = '');
    searchHouses();
}

function showLogin() {
    document.getElementById('loginModal').style.display = 'block';
    switchTab('login');
}

function closeModal() {
    document.getElementById('loginModal').style.display = 'none';
}

function switchTab(tab) {
    document.querySelectorAll('.tab-btn').forEach(btn => btn.classList.remove('active'));
    document.querySelectorAll('.form-container').forEach(f => f.style.display = 'none');
    document.getElementById(tab + 'Form').style.display = 'block';
    document.querySelector(`.tab-btn[onclick="switchTab('${tab}')"]`).classList.add('active');
}

async function register() {
    const body = {
        username: document.getElementById('regUsername').value,
        password: document.getElementById('regPassword').value,
        email: document.getElementById('regEmail').value
    };
    const res = await fetch(API_BASE + '/register', { method: 'POST', headers: { 'Content-Type': 'application/json' }, body: JSON.stringify(body)});
    const data = await res.json();
    alert(data.message);
    if (data.success) {
        pendingVerifyEmail = body.email;
        switchTab('verify');
    }
}

async function verifyEmail() {
    const code = document.getElementById('verifyCode').value;
    const res = await fetch(API_BASE + '/verify', { method: 'POST', headers: { 'Content-Type': 'application/json' }, body: JSON.stringify({ email: pendingVerifyEmail, code })});
    const data = await res.json();
    alert(data.message || (data.success ? '验证成功' : '验证失败'));
    if (data.success) {
        switchTab('login');
    }
}

async function login() {
    const body = {
        username: document.getElementById('loginUsername').value,
        password: document.getElementById('loginPassword').value
    };
    const res = await fetch(API_BASE + '/login', { method: 'POST', headers: { 'Content-Type': 'application/json' }, body: JSON.stringify(body)});
    const data = await res.json();
    if (data.success) {
        currentUser = data.user;
        closeModal();
        toggleAuthUI();
        alert('登录成功');
    } else {
        alert(data.message || '登录失败');
    }
}

function logout() {
    currentUser = null;
    toggleAuthUI();
}

async function toggleFavorite(houseId) {
    if (!currentUser) { showLogin(); return; }
    const res = await fetch(API_BASE + '/favorites/add', { method: 'POST', headers: { 'Content-Type': 'application/json' }, body: JSON.stringify({ userId: currentUser.id, houseId })});
    const data = await res.json();
    alert(data.message || '已更新收藏');
    loadFavorites();
}

async function loadFavorites() {
    if (!currentUser) return;
    const res = await fetch(`${API_BASE}/favorites?userId=${currentUser.id}`);
    const data = await res.json();
    const list = document.getElementById('favoritesList');
    list.innerHTML = '';
    if (!data || data.length === 0) {
        list.innerHTML = '<p class="loading">暂无收藏</p>';
        return;
    }
    data.forEach(item => {
        const card = document.createElement('div');
        card.className = 'house-card';
        card.innerHTML = `
            <h3>${item.houseTitle || '未命名房源'}</h3>
            <div class="price">¥${(item.price || 0).toLocaleString()} 万</div>
            <div class="info">面积：${item.area || '-'} ㎡ | 户型：${item.houseType || '-'} | 单价：${item.unitPrice || '-'} 元/㎡</div>
            <div class="actions">
                <button class="btn-detail" onclick="viewDetail(${item.id})">查看详情</button>
                <button class="btn-favorite" onclick="removeFavorite(${item.id})">取消收藏</button>
            </div>
        `;
        list.appendChild(card);
    });
}

async function removeFavorite(houseId) {
    if (!currentUser) return;
    const res = await fetch(API_BASE + '/favorites/remove', { method: 'POST', headers: { 'Content-Type': 'application/json' }, body: JSON.stringify({ userId: currentUser.id, houseId })});
    const data = await res.json();
    alert(data.message || '已取消收藏');
    loadFavorites();
}

async function showStats(type) {
    const res = await fetch(`${API_BASE}/stats?type=${type}`);
    const data = await res.json();
    const chart = document.getElementById('statsChart');
    chart.innerHTML = '';
    data.forEach(item => {
        const bar = document.createElement('div');
        bar.className = 'chart-bar';
        const width = Math.min(100, item.count || 10) * 2; // simple width scaling
        bar.innerHTML = `
            <div class="chart-label">${item.category || item.type}</div>
            <div class="chart-bar-fill" style="width:${width}px">${item.count || 0} 套 / 均价 ${Math.round(item.avgPrice || 0)} 万</div>
        `;
        chart.appendChild(bar);
    });
}

async function sendAIQuery() {
    const input = document.getElementById('aiInput');
    const chat = document.getElementById('aiChat');
    const text = input.value.trim();
    if (!text) return;
    chat.innerHTML += `<div class="ai-message ai-user">${text}</div>`;
    input.value = '';

    chat.innerHTML += `<div class="ai-message ai-assistant">思考中...</div>`;
    chat.scrollTop = chat.scrollHeight;

    try {
        const res = await fetch(API_BASE + '/ai', { method: 'POST', headers: { 'Content-Type': 'application/json' }, body: JSON.stringify({ query: text })});
        const data = await res.json();
        const reply = data.message || 'AI暂不可用';
        chat.innerHTML = chat.innerHTML.replace('思考中...', reply);
    } catch (e) {
        chat.innerHTML = chat.innerHTML.replace('思考中...', '请求失败，请稍后再试');
    }

    chat.scrollTop = chat.scrollHeight;
}

function showPasswordReset() {
    const email = prompt('请输入注册邮箱');
    if (!email) return;
    fetch(API_BASE + '/password/request', { method: 'POST', headers: { 'Content-Type': 'application/json' }, body: JSON.stringify({ email })})
        .then(r => r.json()).then(d => alert(d.message || '已发送验证码'));
}

window.onclick = function(event) {
    const modal = document.getElementById('loginModal');
    if (event.target === modal) {
        modal.style.display = 'none';
    }
};

document.addEventListener('DOMContentLoaded', () => {
    showPage('home');
    toggleAuthUI();
    searchHouses();
});
