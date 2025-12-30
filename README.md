# 二手房信息服务平台

基于Qt6的Linux Web应用，提供二手房信息浏览、筛选、AI智能推荐等服务。

## 功能特性

### 用户功能
- **游客模式**: 无需登录即可浏览房源
- **用户注册**: 支持邮箱验证的用户注册
- **用户登录**: 安全的密码加密存储
- **密码管理**: 修改密码时发送邮件通知
- **收藏功能**: 收藏感兴趣的房源
- **个性化偏好**: 设置购房偏好

### 房源功能
- **房源浏览**: 从MySQL数据库加载真实房源数据
- **多维筛选**: 支持按价格、单价、地区、面积、房型筛选
- **详情查看**: 点击房源查看详细信息
- **百度地图**: 在详情页显示房源地理位置
- **数据统计**: 可视化展示房源统计数据

### AI助手
- **DeepSeek集成**: 接入DeepSeek API提供智能推荐
- **需求分析**: 根据用户需求智能检索数据库
- **购房建议**: 提供个性化的购房推荐

### 管理功能
- **用户管理**: 查看和管理所有用户
- **用户画像**: 按角色统计用户分布
- **数据统计**: 查看平台运营数据

## 技术栈

- **后端**: Qt6 (C++)
- **前端**: Qt WebEngine + HTML/CSS/JavaScript
- **数据库**: MySQL
- **AI**: DeepSeek API
- **地图**: 百度地图API
- **平台**: Ubuntu Linux (阿里云ECS)

## 项目结构

```
House-web-2.0/
├── CMakeLists.txt           # CMake构建配置
├── config.json              # 配置文件（不提交到Git）
├── include/                 # 头文件
│   ├── configmanager.h
│   ├── databasemanager.h
│   ├── deepseekaiservice.h
│   ├── emailservice.h
│   ├── houseinfo.h
│   ├── mainwindow.h
│   ├── usermanager.h
│   └── webserver.h
├── src/                     # 源文件
│   ├── main.cpp
│   ├── configmanager.cpp
│   ├── databasemanager.cpp
│   ├── deepseekaiservice.cpp
│   ├── emailservice.cpp
│   ├── houseinfo.cpp
│   ├── mainwindow.cpp
│   ├── usermanager.cpp
│   └── webserver.cpp
├── web/                     # Web前端资源
│   ├── index.html
│   ├── detail.html
│   ├── login.html
│   ├── statistics.html
│   ├── ai-assistant.html
│   ├── admin.html
│   ├── css/
│   │   └── style.css
│   └── js/
│       ├── common.js
│       ├── index.js
│       ├── detail.js
│       ├── login.js
│       ├── statistics.js
│       ├── ai-assistant.js
│       └── admin.js
└── resources/               # Qt资源文件
    └── resources.qrc
```

## 数据库结构

### 现有表: houseinfo (不可修改)
```sql
CREATE TABLE houseinfo (
    ID INT AUTO_INCREMENT PRIMARY KEY,
    houseTitle VARCHAR(100),
    price DOUBLE,
    area DOUBLE,
    communityNa VARCHAR(255),
    floor VARCHAR(50),
    houseType VARCHAR(50),
    unitPrice DOUBLE,
    houseUrl VARCHAR(1024)
);
```

### 新建表
项目会自动创建以下表：
- `users`: 用户账户信息
- `favorites`: 用户收藏
- `user_preferences`: 用户偏好设置

## 配置说明

编辑 `config.json` 文件配置以下信息：

```json
{
  "database": {
    "host": "localhost",
    "port": 3306,
    "name": "house_db",
    "user": "your_mysql_user",
    "password": "your_mysql_password"
  },
  "deepseek": {
    "api_url": "https://api.deepseek.com/v1/chat/completions",
    "api_key": "your_deepseek_api_key"
  },
  "baidu_map": {
    "api_key": "your_baidu_map_api_key"
  },
  "email": {
    "smtp_host": "smtp.example.com",
    "smtp_port": 587,
    "username": "your_email@example.com",
    "password": "your_email_password",
    "from_name": "House Web Service"
  },
  "server": {
    "port": 8080,
    "web_root": "./web"
  },
  "admin": {
    "default_username": "admin",
    "default_password": "admin123"
  }
}
```

## 本地开发

### 前置要求
- Qt6 (with WebEngine)
- MySQL 5.7+
- CMake 3.16+
- C++17编译器

### 构建步骤
```bash
mkdir build && cd build
cmake ..
make
```

### 运行
```bash
./HouseWeb
```

访问 http://localhost:8080

## 部署到阿里云ECS

详见 [DEPLOYMENT.md](DEPLOYMENT.md)

## API端点

### 认证相关
- `POST /api/login` - 用户登录
- `POST /api/register` - 用户注册
- `POST /api/verify-email` - 验证邮箱
- `POST /api/change-password` - 修改密码

### 房源相关
- `GET /api/houses` - 获取房源列表（支持筛选）
- `GET /api/house/:id` - 获取房源详情
- `GET /api/statistics` - 获取房源统计数据

### 用户功能
- `GET /api/favorites` - 获取收藏列表
- `POST /api/favorites/add` - 添加收藏
- `POST /api/favorites/remove` - 移除收藏
- `POST /api/preferences` - 设置偏好
- `GET /api/preferences` - 获取偏好

### AI助手
- `POST /api/ai/recommend` - 获取AI推荐

### 管理员
- `GET /api/admin/user-stats` - 用户统计
- `GET /api/admin/users` - 所有用户列表

## 安全注意事项

- 密码使用SHA-256哈希存储
- API密钥和数据库密码存储在config.json中（不提交到Git）
- 建议在生产环境中使用HTTPS
- 定期更新依赖包

## 许可证

Copyright © 2025. All rights reserved.
