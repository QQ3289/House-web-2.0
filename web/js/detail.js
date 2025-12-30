async function loadHouseDetail() {
    const urlParams = new URLSearchParams(window.location.search);
    const houseId = urlParams.get('id');

    if (!houseId) {
        document.getElementById('houseDetail').innerHTML = '<p class="error">无效的房源ID</p>';
        return;
    }

    try {
        const house = await apiRequest(`/house/${houseId}`);
        
        const detailHTML = `
            <div class="house-detail">
                <h2>${house.houseTitle}</h2>
                
                <div class="price" style="font-size: 2rem; color: #e74c3c; margin: 1rem 0;">
                    ${house.price} 万元
                </div>

                <div class="detail-info">
                    <div class="detail-item">
                        <label>小区名称</label>
                        <div>${house.communityName}</div>
                    </div>
                    <div class="detail-item">
                        <label>房型</label>
                        <div>${house.houseType}</div>
                    </div>
                    <div class="detail-item">
                        <label>面积</label>
                        <div>${house.area} ㎡</div>
                    </div>
                    <div class="detail-item">
                        <label>单价</label>
                        <div>${house.unitPrice} 元/㎡</div>
                    </div>
                    ${house.floor ? `
                    <div class="detail-item">
                        <label>楼层</label>
                        <div>${house.floor}</div>
                    </div>
                    ` : ''}
                    <div class="detail-item">
                        <label>来源链接</label>
                        <div><a href="${house.houseUrl}" target="_blank">查看原网页</a></div>
                    </div>
                </div>

                <div style="margin-top: 2rem;">
                    <h3 style="color: #667eea; margin-bottom: 1rem;">位置地图</h3>
                    <div id="map"></div>
                </div>
            </div>
        `;

        document.getElementById('houseDetail').innerHTML = detailHTML;

        // Initialize Baidu Map
        initMap(house.communityName, house.baiduMapKey);

    } catch (error) {
        document.getElementById('houseDetail').innerHTML = '<p class="error">加载失败，请重试</p>';
    }
}

function initMap(location, apiKey) {
    // Load Baidu Map API
    const script = document.createElement('script');
    script.src = `https://api.map.baidu.com/api?v=3.0&ak=${apiKey}&callback=initBaiduMap`;
    document.body.appendChild(script);

    window.initBaiduMap = function() {
        const map = new BMap.Map('map');
        const geocoder = new BMap.Geocoder();

        geocoder.getPoint(location, function(point) {
            if (point) {
                map.centerAndZoom(point, 15);
                const marker = new BMap.Marker(point);
                map.addOverlay(marker);
                const label = new BMap.Label(location, {offset: new BMap.Size(20, -10)});
                marker.setLabel(label);
            } else {
                document.getElementById('map').innerHTML = '<p style="padding: 2rem; text-align: center;">地址解析失败</p>';
            }
        });

        map.enableScrollWheelZoom(true);
    };
}

document.addEventListener('DOMContentLoaded', loadHouseDetail);
