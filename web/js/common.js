// Common utility functions and authentication state

const API_BASE = 'http://localhost:8080/api';

// Get current user from localStorage
function getCurrentUser() {
    const userStr = localStorage.getItem('currentUser');
    return userStr ? JSON.parse(userStr) : null;
}

// Set current user
function setCurrentUser(user) {
    if (user) {
        localStorage.setItem('currentUser', JSON.stringify(user));
    } else {
        localStorage.removeItem('currentUser');
    }
    updateNavigation();
}

// Update navigation based on login state
function updateNavigation() {
    const user = getCurrentUser();
    const loginLink = document.getElementById('loginLink');
    const logoutLink = document.getElementById('logoutLink');
    const favoritesLink = document.getElementById('favoritesLink');
    const adminLink = document.getElementById('adminLink');

    if (user) {
        if (loginLink) loginLink.style.display = 'none';
        if (logoutLink) {
            logoutLink.style.display = 'block';
            logoutLink.textContent = `退出 (${user.username})`;
        }
        if (favoritesLink) favoritesLink.style.display = 'block';
        if (adminLink && user.role === 'admin') adminLink.style.display = 'block';
    } else {
        if (loginLink) loginLink.style.display = 'block';
        if (logoutLink) logoutLink.style.display = 'none';
        if (favoritesLink) favoritesLink.style.display = 'none';
        if (adminLink) adminLink.style.display = 'none';
    }
}

// Logout function
function logout() {
    setCurrentUser(null);
    window.location.href = 'index.html';
}

// API request helper
async function apiRequest(endpoint, options = {}) {
    try {
        const response = await fetch(API_BASE + endpoint, {
            ...options,
            headers: {
                'Content-Type': 'application/json',
                ...options.headers
            }
        });
        return await response.json();
    } catch (error) {
        console.error('API request failed:', error);
        throw error;
    }
}

// Initialize navigation on page load
document.addEventListener('DOMContentLoaded', () => {
    updateNavigation();
    
    const logoutLink = document.getElementById('logoutLink');
    if (logoutLink) {
        logoutLink.addEventListener('click', (e) => {
            e.preventDefault();
            logout();
        });
    }
});
