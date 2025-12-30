async function loadUserStats() {
    try {
        const data = await apiRequest('/admin/user-stats');
        
        document.getElementById('totalUsers').textContent = data.total;
        document.getElementById('adminCount').textContent = data.byRole.admin || 0;
        document.getElementById('userCount').textContent = data.byRole.user || 0;

    } catch (error) {
        console.error('Failed to load user stats:', error);
    }
}

async function loadAllUsers() {
    try {
        const data = await apiRequest('/admin/users');
        const tbody = document.getElementById('usersTableBody');
        
        if (!data.users || data.users.length === 0) {
            tbody.innerHTML = '<tr><td colspan="6">暂无用户</td></tr>';
            return;
        }

        tbody.innerHTML = data.users.map(user => `
            <tr>
                <td>${user.id}</td>
                <td>${user.username}</td>
                <td>${user.email}</td>
                <td>${user.role}</td>
                <td>${user.email_verified === 'true' ? '已验证' : '未验证'}</td>
                <td>${user.created_at}</td>
            </tr>
        `).join('');

    } catch (error) {
        console.error('Failed to load users:', error);
    }
}

document.addEventListener('DOMContentLoaded', () => {
    const user = getCurrentUser();
    
    if (!user || user.role !== 'admin') {
        alert('您没有权限访问此页面');
        window.location.href = 'index.html';
        return;
    }

    loadUserStats();
    loadAllUsers();
});
