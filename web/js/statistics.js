async function loadStatistics() {
    try {
        const data = await apiRequest('/statistics');

        // Region chart
        createBarChart('regionChart', '地区', Object.keys(data.byRegion), Object.values(data.byRegion));

        // Type chart
        createPieChart('typeChart', Object.keys(data.byType), Object.values(data.byType));

        // Average price chart
        createBarChart('priceChart', '平均价格(万元)', Object.keys(data.avgPriceByRegion), Object.values(data.avgPriceByRegion), '#e74c3c');

    } catch (error) {
        console.error('Failed to load statistics:', error);
    }
}

function createBarChart(canvasId, label, labels, data, color = '#667eea') {
    const ctx = document.getElementById(canvasId).getContext('2d');
    new Chart(ctx, {
        type: 'bar',
        data: {
            labels: labels,
            datasets: [{
                label: label,
                data: data,
                backgroundColor: color,
                borderColor: color,
                borderWidth: 1
            }]
        },
        options: {
            responsive: true,
            maintainAspectRatio: true,
            plugins: {
                legend: {
                    display: false
                }
            },
            scales: {
                y: {
                    beginAtZero: true
                }
            }
        }
    });
}

function createPieChart(canvasId, labels, data) {
    const ctx = document.getElementById(canvasId).getContext('2d');
    const colors = [
        '#667eea', '#764ba2', '#f093fb', '#4facfe',
        '#43e97b', '#fa709a', '#fee140', '#30cfd0'
    ];

    new Chart(ctx, {
        type: 'pie',
        data: {
            labels: labels,
            datasets: [{
                data: data,
                backgroundColor: colors.slice(0, labels.length),
                borderWidth: 2,
                borderColor: '#fff'
            }]
        },
        options: {
            responsive: true,
            maintainAspectRatio: true,
            plugins: {
                legend: {
                    position: 'bottom'
                }
            }
        }
    });
}

document.addEventListener('DOMContentLoaded', loadStatistics);
