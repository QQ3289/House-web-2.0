document.addEventListener('DOMContentLoaded', () => {
    const sendBtn = document.getElementById('sendBtn');
    const userInput = document.getElementById('userInput');
    const chatMessages = document.getElementById('chatMessages');

    sendBtn.addEventListener('click', async () => {
        const requirements = userInput.value.trim();
        
        if (!requirements) {
            alert('请输入您的需求');
            return;
        }

        // Add user message
        addMessage(requirements, 'user');
        userInput.value = '';

        // Show loading
        const loadingMsg = addMessage('正在分析您的需求...', 'bot');
        sendBtn.disabled = true;

        try {
            const result = await apiRequest('/ai/recommend', {
                method: 'POST',
                body: JSON.stringify({ requirements })
            });

            // Remove loading message
            loadingMsg.remove();

            if (result.success) {
                addMessage(result.recommendation, 'bot');
            } else {
                addMessage('抱歉，推荐服务暂时不可用：' + result.error, 'bot');
            }
        } catch (error) {
            loadingMsg.remove();
            addMessage('抱歉，推荐服务出错，请稍后重试', 'bot');
        } finally {
            sendBtn.disabled = false;
        }
    });

    userInput.addEventListener('keypress', (e) => {
        if (e.key === 'Enter' && e.ctrlKey) {
            sendBtn.click();
        }
    });
});

function addMessage(text, type) {
    const chatMessages = document.getElementById('chatMessages');
    const messageDiv = document.createElement('div');
    messageDiv.className = `message ${type}-message`;
    messageDiv.textContent = text;
    chatMessages.appendChild(messageDiv);
    chatMessages.scrollTop = chatMessages.scrollHeight;
    return messageDiv;
}
