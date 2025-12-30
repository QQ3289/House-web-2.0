# 阿里云ECS (Ubuntu) 部署指南

本指南专门针对在阿里云ECS Ubuntu系统上部署二手房信息服务平台。

## 系统要求

- 阿里云ECS实例
- Ubuntu 20.04 LTS 或更高版本
- 至少 2GB RAM
- 至少 20GB 磁盘空间

## 部署步骤

### 1. 准备ECS实例

登录到阿里云ECS控制台，确保：
- 安全组开放端口 8080 (或你配置的端口)
- 如需外网访问，配置公网IP

通过SSH连接到ECS实例：
```bash
ssh root@your_ecs_ip
```

### 2. 更新系统

```bash
apt update
apt upgrade -y
```

### 3. 安装MySQL

```bash
# 安装MySQL Server
apt install -y mysql-server

# 启动MySQL服务
systemctl start mysql
systemctl enable mysql

# 安全配置MySQL
mysql_secure_installation
```

配置MySQL数据库：
```bash
mysql -u root -p

# 在MySQL中执行：
CREATE DATABASE house_db CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
CREATE USER 'houseuser'@'localhost' IDENTIFIED BY 'your_strong_password';
GRANT ALL PRIVILEGES ON house_db.* TO 'houseuser'@'localhost';
FLUSH PRIVILEGES;
EXIT;
```

导入现有房源数据（如果有的话）：
```bash
mysql -u houseuser -p house_db < your_houseinfo_data.sql
```

### 4. 安装Qt6及依赖

```bash
# 安装构建工具
apt install -y build-essential cmake git

# 安装Qt6及相关模块
apt install -y qt6-base-dev qt6-webengine-dev libqt6sql6-mysql

# 安装其他依赖
apt install -y libgl1-mesa-dev libxkbcommon-dev
```

### 5. 克隆项目代码

```bash
# 创建应用目录
mkdir -p /opt/houseweb
cd /opt/houseweb

# 克隆代码（替换为你的实际仓库地址）
git clone <your_repository_url> .

# 或使用scp上传代码
# 在本地执行: scp -r House-web-2.0 root@your_ecs_ip:/opt/houseweb/
```

### 6. 配置应用

```bash
cd /opt/houseweb

# 复制配置文件模板并编辑
cp config.json.example config.json
nano config.json
```

在 `config.json` 中配置：
```json
{
  "database": {
    "host": "localhost",
    "port": 3306,
    "name": "house_db",
    "user": "houseuser",
    "password": "your_strong_password"
  },
  "deepseek": {
    "api_url": "https://api.deepseek.com/v1/chat/completions",
    "api_key": "sk-your-actual-deepseek-api-key"
  },
  "baidu_map": {
    "api_key": "your-actual-baidu-map-api-key"
  },
  "email": {
    "smtp_host": "smtp.aliyun.com",
    "smtp_port": 465,
    "username": "your-email@aliyun.com",
    "password": "your-email-password",
    "from_name": "House Web Service"
  },
  "server": {
    "port": 8080,
    "web_root": "./web"
  },
  "admin": {
    "default_username": "admin",
    "default_password": "change_this_password"
  }
}
```

**重要安全提示**：
- 修改默认管理员密码
- 确保 config.json 权限设置为 600
```bash
chmod 600 config.json
```

### 7. 编译项目

```bash
cd /opt/houseweb

# 创建构建目录
mkdir build
cd build

# 配置CMake
cmake ..

# 编译（使用多核加速）
make -j$(nproc)

# 检查编译结果
ls -lh HouseWeb
```

### 8. 创建systemd服务

创建服务文件：
```bash
nano /etc/systemd/system/houseweb.service
```

内容如下：
```ini
[Unit]
Description=House Web Service
After=network.target mysql.service

[Service]
Type=simple
User=root
WorkingDirectory=/opt/houseweb/build
ExecStart=/opt/houseweb/build/HouseWeb
Restart=on-failure
RestartSec=10
StandardOutput=journal
StandardError=journal

[Install]
WantedBy=multi-user.target
```

启用并启动服务：
```bash
# 重新加载systemd配置
systemctl daemon-reload

# 启动服务
systemctl start houseweb

# 设置开机自启
systemctl enable houseweb

# 查看服务状态
systemctl status houseweb

# 查看日志
journalctl -u houseweb -f
```

### 9. 配置防火墙

如果使用UFW防火墙：
```bash
# 允许SSH
ufw allow 22/tcp

# 允许应用端口
ufw allow 8080/tcp

# 启用防火墙
ufw enable

# 查看状态
ufw status
```

### 10. 配置Nginx反向代理（可选但推荐）

```bash
# 安装Nginx
apt install -y nginx

# 创建配置文件
nano /etc/nginx/sites-available/houseweb
```

配置内容：
```nginx
server {
    listen 80;
    server_name your_domain.com;  # 替换为你的域名或IP

    location / {
        proxy_pass http://localhost:8080;
        proxy_http_version 1.1;
        proxy_set_header Upgrade $http_upgrade;
        proxy_set_header Connection 'upgrade';
        proxy_set_header Host $host;
        proxy_cache_bypass $http_upgrade;
        proxy_set_header X-Real-IP $remote_addr;
        proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
        proxy_set_header X-Forwarded-Proto $scheme;
    }
}
```

启用配置：
```bash
# 创建软链接
ln -s /etc/nginx/sites-available/houseweb /etc/nginx/sites-enabled/

# 测试配置
nginx -t

# 重启Nginx
systemctl restart nginx
systemctl enable nginx
```

### 11. 配置HTTPS（强烈推荐）

使用Let's Encrypt免费SSL证书：
```bash
# 安装Certbot
apt install -y certbot python3-certbot-nginx

# 获取证书（替换为你的域名和邮箱）
certbot --nginx -d your_domain.com -m your_email@example.com --agree-tos

# 测试自动续期
certbot renew --dry-run
```

### 12. 性能优化

#### 数据库优化
编辑MySQL配置：
```bash
nano /etc/mysql/mysql.conf.d/mysqld.cnf
```

添加优化配置：
```ini
[mysqld]
innodb_buffer_pool_size = 512M
innodb_log_file_size = 128M
max_connections = 200
query_cache_size = 32M
```

重启MySQL：
```bash
systemctl restart mysql
```

#### 应用优化
确保Web资源在正确位置：
```bash
cd /opt/houseweb/build
ln -s ../web ./web
```

### 13. 监控和日志

#### 查看应用日志
```bash
# 实时查看日志
journalctl -u houseweb -f

# 查看最近日志
journalctl -u houseweb -n 100

# 查看特定时间日志
journalctl -u houseweb --since "2025-01-01" --until "2025-01-02"
```

#### 设置日志轮转
创建日志轮转配置：
```bash
nano /etc/logrotate.d/houseweb
```

内容：
```
/var/log/houseweb/*.log {
    daily
    rotate 7
    compress
    delaycompress
    notifempty
    create 0640 root root
}
```

### 14. 备份策略

创建备份脚本：
```bash
nano /opt/houseweb/backup.sh
```

内容：
```bash
#!/bin/bash
BACKUP_DIR=/backup/houseweb
DATE=$(date +%Y%m%d_%H%M%S)

mkdir -p $BACKUP_DIR

# 备份数据库
mysqldump -u houseuser -p'your_password' house_db > $BACKUP_DIR/db_$DATE.sql

# 备份配置
cp /opt/houseweb/config.json $BACKUP_DIR/config_$DATE.json

# 删除7天前的备份
find $BACKUP_DIR -name "*.sql" -mtime +7 -delete
find $BACKUP_DIR -name "*.json" -mtime +7 -delete

echo "Backup completed: $DATE"
```

设置权限并添加到cron：
```bash
chmod +x /opt/houseweb/backup.sh

# 添加到crontab（每天凌晨2点备份）
crontab -e
# 添加：0 2 * * * /opt/houseweb/backup.sh >> /var/log/houseweb_backup.log 2>&1
```

### 15. 测试部署

```bash
# 检查服务状态
systemctl status houseweb

# 测试本地访问
curl http://localhost:8080

# 如果配置了Nginx
curl http://localhost

# 从外部访问（替换为你的公网IP或域名）
# 在本地浏览器访问: http://your_ecs_ip
```

### 16. 故障排查

#### 应用无法启动
```bash
# 查看错误日志
journalctl -u houseweb -n 50

# 检查配置文件
cat /opt/houseweb/config.json

# 检查端口占用
netstat -tulpn | grep 8080

# 手动运行查看错误
cd /opt/houseweb/build
./HouseWeb
```

#### 数据库连接失败
```bash
# 测试MySQL连接
mysql -u houseuser -p -h localhost house_db

# 查看MySQL日志
tail -f /var/log/mysql/error.log
```

#### Web界面无法访问
```bash
# 检查防火墙
ufw status

# 检查Nginx
systemctl status nginx
nginx -t

# 查看Nginx日志
tail -f /var/log/nginx/error.log
```

### 17. 更新应用

```bash
# 停止服务
systemctl stop houseweb

# 进入项目目录
cd /opt/houseweb

# 拉取最新代码
git pull

# 重新编译
cd build
cmake ..
make -j$(nproc)

# 启动服务
systemctl start houseweb

# 查看状态
systemctl status houseweb
```

## 获取API密钥

### DeepSeek API
1. 访问 https://platform.deepseek.com/
2. 注册账号并登录
3. 在控制台创建API密钥
4. 充值并确保账户有余额

### 百度地图API
1. 访问 https://lbsyun.baidu.com/
2. 注册开发者账号
3. 创建应用获取AK（API Key）
4. 确保启用JavaScript API服务

## 性能建议

- **ECS配置**: 推荐至少2核4GB内存
- **数据库**: 考虑使用阿里云RDS MySQL获得更好性能
- **CDN**: 使用阿里云CDN加速静态资源
- **负载均衡**: 高流量时使用阿里云SLB

## 安全建议

1. 定期更新系统和依赖包
2. 使用强密码
3. 启用防火墙
4. 配置HTTPS
5. 定期备份数据
6. 限制SSH登录（使用密钥认证）
7. 监控异常访问

## 联系支持

如遇到问题，请查看：
- 应用日志: `journalctl -u houseweb`
- MySQL日志: `/var/log/mysql/error.log`
- Nginx日志: `/var/log/nginx/error.log`

---

部署完成后，访问 `http://your_domain_or_ip` 开始使用！

默认管理员账户：
- 用户名: admin
- 密码: 在config.json中配置的密码（请立即修改）

## 配置文件详细说明

### config.json 配置项说明

#### database - 数据库配置
- **host**: MySQL数据库服务器地址
  - 本地部署使用 `localhost`
  - 使用阿里云RDS时填写RDS内网地址
- **port**: MySQL端口号，默认 `3306`
- **name**: 数据库名称，必须是 `house_db`（包含houseinfo表）
- **user**: 数据库用户名
- **password**: 数据库密码（请使用强密码）

#### deepseek - DeepSeek AI配置
- **api_url**: DeepSeek API接口地址
  - 默认: `https://api.deepseek.com/v1/chat/completions`
  - 如DeepSeek更新接口地址，在此修改
- **api_key**: DeepSeek API密钥
  - 格式: `sk-xxxxxxxxxxxxxxxx`
  - 从DeepSeek平台获取
  - 需要账户有充值余额才能使用
  - **重要**: 不要将此密钥提交到Git仓库

#### baidu_map - 百度地图配置
- **api_key**: 百度地图JavaScript API密钥（AK）
  - 从百度地图开放平台获取
  - 用于在房源详情页显示地理位置
  - 需要在百度地图控制台启用"JavaScript API"服务

#### email - 邮件服务配置
- **smtp_host**: SMTP服务器地址
  - 阿里云企业邮箱: `smtp.mxhichina.com`
  - QQ邮箱: `smtp.qq.com`
  - 网易邮箱: `smtp.163.com`
  - Gmail: `smtp.gmail.com`
- **smtp_port**: SMTP端口
  - SSL加密: `465`
  - TLS加密: `587`
  - 根据邮件服务商要求选择
- **username**: 发件邮箱地址
  - 示例: `service@yourdomain.com`
- **password**: 邮箱密码或授权码
  - QQ邮箱、网易邮箱需要使用"授权码"而非登录密码
  - 在邮箱设置中生成授权码
- **from_name**: 发件人显示名称
  - 显示在邮件"发件人"字段
  - 示例: "二手房信息服务平台"

**注意**: 当前版本邮件功能为占位实现，生产环境需集成真实SMTP库（如VMime）

#### server - Web服务器配置
- **port**: HTTP服务监听端口
  - 默认: `8080`
  - 如需使用80端口需要root权限
  - 建议使用Nginx反向代理，应用监听8080
- **web_root**: Web静态资源根目录
  - 相对路径: `./web`（相对于可执行文件）
  - 绝对路径: `/opt/houseweb/web`
  - 包含HTML、CSS、JS等前端文件

#### admin - 管理员账户配置
- **default_username**: 默认管理员用户名
  - 首次启动时自动创建
  - 建议部署后立即修改
- **default_password**: 默认管理员密码
  - **安全警告**: 必须在部署后立即修改
  - 不要使用弱密码如 `admin123`
  - 建议使用包含大小写字母、数字、特殊字符的强密码

### 配置安全最佳实践

1. **文件权限**: 设置config.json为仅所有者可读写
   ```bash
   chmod 600 /opt/houseweb/config.json
   chown root:root /opt/houseweb/config.json
   ```

2. **密码管理**:
   - 所有密码使用强密码
   - 定期更换数据库密码
   - 不要在代码中硬编码密码

3. **API密钥安全**:
   - 不要将config.json提交到Git仓库
   - 定期检查API密钥使用情况
   - DeepSeek API设置使用额度限制

4. **生产环境建议**:
   - 使用环境变量覆盖敏感配置
   - 考虑使用阿里云密钥管理服务（KMS）
   - 启用API访问频率限制

5. **备份配置**:
   - 备份config.json到安全位置
   - 使用加密存储备份文件
   - 定期验证备份有效性

### 配置示例（生产环境）

```json
{
  "database": {
    "host": "rm-xxx.mysql.rds.aliyuncs.com",
    "port": 3306,
    "name": "house_db",
    "user": "houseapp",
    "password": "StrongP@ssw0rd!2025"
  },
  "deepseek": {
    "api_url": "https://api.deepseek.com/v1/chat/completions",
    "api_key": "sk-1234567890abcdef1234567890abcdef"
  },
  "baidu_map": {
    "api_key": "AbCdEfGhIjKlMnOpQrStUvWxYz123456"
  },
  "email": {
    "smtp_host": "smtp.exmail.qq.com",
    "smtp_port": 465,
    "username": "service@yourcompany.com",
    "password": "EmailAuthCode123",
    "from_name": "二手房服务平台"
  },
  "server": {
    "port": 8080,
    "web_root": "./web"
  },
  "admin": {
    "default_username": "administrator",
    "default_password": "Complex!Pass@2025#Secure"
  }
}
```
