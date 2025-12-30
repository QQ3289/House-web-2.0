let currentPage = 1;
const pageSize = 20;
let totalCount = 0;

async function loadHouses() {
    const filters = {
        minPrice: document.getElementById('minPrice').value,
        maxPrice: document.getElementById('maxPrice').value,
        minUnitPrice: document.getElementById('minUnitPrice').value,
        maxUnitPrice: document.getElementById('maxUnitPrice').value,
        minArea: document.getElementById('minArea').value,
        maxArea: document.getElementById('maxArea').value,
        region: document.getElementById('region').value,
        houseType: document.getElementById('houseType').value,
        offset: (currentPage - 1) * pageSize,
        limit: pageSize
    };

    const queryParams = new URLSearchParams();
    for (const [key, value] of Object.entries(filters)) {
        if (value) queryParams.append(key, value);
    }

    try {
        const data = await apiRequest('/houses?' + queryParams.toString());
        displayHouses(data.houses);
        totalCount = data.total;
        updatePagination();
    } catch (error) {
        document.getElementById('housesList').innerHTML = '<p class="error">加载失败，请重试</p>';
    }
}

function displayHouses(houses) {
    const container = document.getElementById('housesList');
    
    if (!houses || houses.length === 0) {
        container.innerHTML = '<p class="loading">暂无房源</p>';
        return;
    }

    container.innerHTML = houses.map(house => `
        <div class="house-card" onclick="viewDetail(${house.id})">
            <h3>${house.houseTitle}</h3>
            <div class="price">${house.price} 万元</div>
            <div class="info">面积: ${house.area} ㎡</div>
            <div class="info">单价: ${house.unitPrice} 元/㎡</div>
            <div class="info">小区: ${house.communityName}</div>
            <div class="info">房型: ${house.houseType}</div>
            ${house.floor ? `<div class="info">楼层: ${house.floor}</div>` : ''}
        </div>
    `).join('');
}

function updatePagination() {
    const totalPages = Math.ceil(totalCount / pageSize);
    document.getElementById('pageInfo').textContent = `第 ${currentPage} 页 / 共 ${totalPages} 页`;
    document.getElementById('prevPage').disabled = currentPage === 1;
    document.getElementById('nextPage').disabled = currentPage >= totalPages;
}

function viewDetail(houseId) {
    window.location.href = `detail.html?id=${houseId}`;
}

document.addEventListener('DOMContentLoaded', () => {
    loadHouses();

    document.getElementById('searchBtn').addEventListener('click', () => {
        currentPage = 1;
        loadHouses();
    });

    document.getElementById('resetBtn').addEventListener('click', () => {
        document.getElementById('minPrice').value = '';
        document.getElementById('maxPrice').value = '';
        document.getElementById('minUnitPrice').value = '';
        document.getElementById('maxUnitPrice').value = '';
        document.getElementById('minArea').value = '';
        document.getElementById('maxArea').value = '';
        document.getElementById('region').value = '';
        document.getElementById('houseType').value = '';
        currentPage = 1;
        loadHouses();
    });

    document.getElementById('prevPage').addEventListener('click', () => {
        if (currentPage > 1) {
            currentPage--;
            loadHouses();
        }
    });

    document.getElementById('nextPage').addEventListener('click', () => {
        const totalPages = Math.ceil(totalCount / pageSize);
        if (currentPage < totalPages) {
            currentPage++;
            loadHouses();
        }
    });
});
