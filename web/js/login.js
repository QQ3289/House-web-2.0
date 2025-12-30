document.addEventListener('DOMContentLoaded', () => {
    const tabs = document.querySelectorAll('.auth-tab');
    const loginForm = document.getElementById('loginForm');
    const registerForm = document.getElementById('registerForm');

    tabs.forEach(tab => {
        tab.addEventListener('click', () => {
            tabs.forEach(t => t.classList.remove('active'));
            tab.classList.add('active');

            const tabName = tab.getAttribute('data-tab');
            if (tabName === 'login') {
                loginForm.style.display = 'block';
                registerForm.style.display = 'none';
            } else {
                loginForm.style.display = 'none';
                registerForm.style.display = 'block';
            }
        });
    });

    // Login handler
    document.getElementById('loginBtn').addEventListener('click', async () => {
        const username = document.getElementById('loginUsername').value;
        const password = document.getElementById('loginPassword').value;
        const messageEl = document.getElementById('loginMessage');

        if (!username || !password) {
            showMessage(messageEl, '请输入用户名和密码', 'error');
            return;
        }

        try {
            const result = await apiRequest('/login', {
                method: 'POST',
                body: JSON.stringify({ username, password })
            });

            if (result.success) {
                setCurrentUser({
                    userId: result.userId,
                    username: result.username,
                    role: result.role
                });
                showMessage(messageEl, '登录成功！', 'success');
                setTimeout(() => {
                    window.location.href = 'index.html';
                }, 1000);
            } else {
                showMessage(messageEl, result.message, 'error');
            }
        } catch (error) {
            showMessage(messageEl, '登录失败，请重试', 'error');
        }
    });

    // Register handler
    document.getElementById('registerBtn').addEventListener('click', async () => {
        const username = document.getElementById('registerUsername').value;
        const email = document.getElementById('registerEmail').value;
        const password = document.getElementById('registerPassword').value;
        const passwordConfirm = document.getElementById('registerPasswordConfirm').value;
        const messageEl = document.getElementById('registerMessage');

        if (!username || !email || !password || !passwordConfirm) {
            showMessage(messageEl, '请填写所有字段', 'error');
            return;
        }

        if (password !== passwordConfirm) {
            showMessage(messageEl, '两次输入的密码不一致', 'error');
            return;
        }

        try {
            const result = await apiRequest('/register', {
                method: 'POST',
                body: JSON.stringify({ username, email, password })
            });

            if (result.success) {
                showMessage(messageEl, result.message, 'success');
                // Clear form
                document.getElementById('registerUsername').value = '';
                document.getElementById('registerEmail').value = '';
                document.getElementById('registerPassword').value = '';
                document.getElementById('registerPasswordConfirm').value = '';
            } else {
                showMessage(messageEl, result.message, 'error');
            }
        } catch (error) {
            showMessage(messageEl, '注册失败，请重试', 'error');
        }
    });
});

function showMessage(element, message, type) {
    element.textContent = message;
    element.className = 'message ' + type;
    element.style.display = 'block';
}
