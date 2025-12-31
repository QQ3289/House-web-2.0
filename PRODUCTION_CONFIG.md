# 生产环境配置指南

本文档详细说明在生产环境中部署二手房信息平台所需的配置步骤。

## 一、数据库配置

### 1.1 创建生产数据库账户
```sql
-- 登录MySQL
mysql -u root -p

-- 创建数据库
CREATE DATABASE house_db DEFAULT CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;

-- 创建专用账户（使用强密码）
CREATE USER 'house_user'@'localhost' IDENTIFIED BY 'YOUR_STRONG_PASSWORD_HERE';

-- 授权
GRANT SELECT, INSERT, UPDATE, DELETE, CREATE, INDEX ON house_db.* TO 'house_user'@'localhost';
FLUSH PRIVILEGES;

-- 验证连接
mysql -u house_user -p house_db
```

### 1.2 导入房源数据
确保 `houseinfo` 表已存在并包含数据。如需导入：
```bash
mysql -u house_user -p house_db < /path/to/houseinfo.sql
```

### 1.3 数据库优化配置
在 MySQL 配置文件 `/etc/mysql/mysql.conf.d/mysqld.cnf` 中添加：
```ini
[mysqld]
max_connections = 200
innodb_buffer_pool_size = 1G
character-set-server = utf8mb4
collation-server = utf8mb4_unicode_ci
```

重启 MySQL：
```bash
sudo systemctl restart mysql
```

---

## 二、API密钥配置

### 2.1 DeepSeek AI API
1. 访问 [DeepSeek开放平台](https://platform.deepseek.com/)
2. 注册账号并完成实名认证
3. 创建API密钥
4. 充值账户（建议至少100元）
5. 保存密钥备用

**注意事项：**
- API调用按token计费，监控使用量避免超支
- 设置API调用频率限制（建议每用户每天最多10次）
- 在配置文件中设置 `api.deepseekKey`

### 2.2 百度地图API
1. 访问 [百度地图开放平台](https://lbsyun.baidu.com/)
2. 注册开发者账号
3. 进入控制台 → 应用管理 → 创建应用
4. 选择"浏览器端"应用类型
5. 获取AK（Access Key）
6. 在"IP白名单"中添加服务器IP或设置为 `*`（仅测试）

**配置位置：**
- 在 `config/appconfig.json` 中设置 `api.baiduMapAk`
- 在 `web/index.html` 中将 `YOUR_BAIDU_AK` 替换为实际AK

**配额说明：**
- 个人开发者免费配额：30万次/天
- 超出配额需要付费，建议设置调用限制

---

## 三、邮件服务配置

### 3.1 使用系统sendmail（推荐）

#### 安装sendmail
```bash
sudo apt update
sudo apt install -y sendmail mailutils
sudo systemctl enable sendmail
sudo systemctl start sendmail
```

#### 配置主机名
```bash
# 编辑 /etc/hosts
sudo nano /etc/hosts

# 添加或确认以下行存在
127.0.0.1       localhost
YOUR_SERVER_IP  your.domain.com your
```

#### 测试发送
```bash
echo "测试邮件内容" | mail -s "测试主题" your_email@example.com
```

### 3.2 使用SMTP中继（备选方案）

如果需要使用第三方SMTP服务（如腾讯企业邮箱、阿里云邮件推送等）：

#### 配置示例（腾讯企业邮箱）
在 `config/appconfig.json` 中：
```json
{
  "smtp": {
    "server": "smtp.exmail.qq.com",
    "port": 465,
    "username": "noreply@yourdomain.com",
    "password": "your_email_password",
    "sender": "二手房平台 <noreply@yourdomain.com>"
  }
}
```

#### 配置示例（阿里云邮件推送）
```json
{
  "smtp": {
    "server": "smtpdm.aliyun.com",
    "port": 465,
    "username": "your_aliyun_email@your_domain.com",
    "password": "your_aliyun_smtp_password",
    "sender": "noreply@your_domain.com"
  }
}
```

**注意：** 当前代码使用系统sendmail命令，如需SMTP直连，需要额外实现SMTP客户端或使用第三方库。

---

## 四、应用配置文件

### 4.1 编辑 config/appconfig.json

创建生产环境配置：
```json
{
  "database": {
    "host": "127.0.0.1",
    "port": 3306,
    "name": "house_db",
    "user": "house_user",
    "password": "YOUR_DB_PASSWORD"
  },
  "api": {
    "deepseekKey": "sk-xxxxxxxxxxxxxxxxxxxxx",
    "deepseekUrl": "https://api.deepseek.com/v1/chat/completions",
    "baiduMapAk": "your_baidu_map_ak_here"
  },
  "smtp": {
    "server": "localhost",
    "port": 25,
    "username": "noreply@yourdomain.com",
    "password": "",
    "sender": "二手房信息平台 <noreply@yourdomain.com>"
  },
  "app": {
    "port": 8080,
    "webRoot": "/opt/House-web-2.0/web"
  }
}
```

### 4.2 文件权限设置
```bash
# 设置配置文件仅所有者可读（保护密钥）
chmod 600 /opt/House-web-2.0/config/appconfig.json
chown www-data:www-data /opt/House-web-2.0/config/appconfig.json
```

---

## 五、前端配置

### 5.1 百度地图AK配置
编辑 `web/index.html`，找到：
```html
<script type="text/javascript" src="https://api.map.baidu.com/api?v=3.0&ak=YOUR_BAIDU_AK"></script>
```

替换 `YOUR_BAIDU_AK` 为实际的百度地图AK：
```html
<script type="text/javascript" src="https://api.map.baidu.com/api?v=3.0&ak=abcdefghijklmnopqrstuvwxyz123456"></script>
```

**或者**通过环境变量动态注入（需要在服务端实现模板替换）。

---

## 六、系统服务配置

### 6.1 创建systemd服务文件
```bash
sudo nano /etc/systemd/system/houseweb.service
```

内容：
```ini
[Unit]
Description=House Web Server
After=network.target mysql.service

[Service]
Type=simple
User=www-data
Group=www-data
WorkingDirectory=/opt/House-web-2.0/build
ExecStart=/opt/House-web-2.0/build/HouseWebServer
Restart=on-failure
RestartSec=10

# 环境变量覆盖（可选，优先级高于配置文件）
# Environment=DB_PASS=override_password
# Environment=DEEPSEEK_API_KEY=override_key

# 日志
StandardOutput=journal
StandardError=journal
SyslogIdentifier=houseweb

[Install]
WantedBy=multi-user.target
```

### 6.2 启动并启用服务
```bash
sudo systemctl daemon-reload
sudo systemctl enable houseweb
sudo systemctl start houseweb

# 查看状态
sudo systemctl status houseweb

# 查看日志
sudo journalctl -u houseweb -f
```

---

## 七、Nginx反向代理配置

### 7.1 安装Nginx
```bash
sudo apt install -y nginx
```

### 7.2 创建站点配置
```bash
sudo nano /etc/nginx/sites-available/houseweb
```

内容：
```nginx
server {
    listen 80;
    server_name your.domain.com;

    # 静态文件缓存
    location ~* \.(css|js|png|jpg|jpeg|gif|ico|svg)$ {
        proxy_pass http://127.0.0.1:8080;
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
        proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
        expires 7d;
        add_header Cache-Control "public, immutable";
    }

    # API接口
    location /api/ {
        proxy_pass http://127.0.0.1:8080;
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
        proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
        proxy_set_header X-Forwarded-Proto $scheme;
        
        # 超时设置（AI接口可能需要较长时间）
        proxy_read_timeout 60s;
        proxy_connect_timeout 10s;
    }

    # 其他请求
    location / {
        proxy_pass http://127.0.0.1:8080;
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
        proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
    }

    # 访问日志
    access_log /var/log/nginx/houseweb_access.log;
    error_log /var/log/nginx/houseweb_error.log;
}
```

### 7.3 启用站点
```bash
sudo ln -s /etc/nginx/sites-available/houseweb /etc/nginx/sites-enabled/
sudo nginx -t
sudo systemctl reload nginx
```

### 7.4 配置HTTPS（推荐）
使用Let's Encrypt免费证书：
```bash
sudo apt install -y certbot python3-certbot-nginx
sudo certbot --nginx -d your.domain.com
```

Certbot会自动修改Nginx配置并设置自动续期。

---

## 八、安全加固

### 8.1 防火墙配置
```bash
sudo ufw allow 22/tcp      # SSH
sudo ufw allow 80/tcp      # HTTP
sudo ufw allow 443/tcp     # HTTPS
sudo ufw enable
```

### 8.2 数据库安全
- 禁止root远程登录
- 定期更换数据库密码
- 限制数据库仅本地访问

```sql
-- 删除匿名用户
DELETE FROM mysql.user WHERE User='';

-- 禁止root远程登录
DELETE FROM mysql.user WHERE User='root' AND Host NOT IN ('localhost', '127.0.0.1', '::1');

FLUSH PRIVILEGES;
```

### 8.3 应用安全
- 配置文件权限设为 600
- 日志文件定期轮转
- 实施API速率限制（防止滥用）
- 用户密码强制6位以上

### 8.4 备份策略
```bash
# 创建备份脚本
sudo nano /opt/backup_house_db.sh
```

内容：
```bash
#!/bin/bash
BACKUP_DIR="/var/backups/houseweb"
DATE=$(date +%Y%m%d_%H%M%S)
mkdir -p $BACKUP_DIR

# 备份数据库
mysqldump -u house_user -p'YOUR_DB_PASSWORD' house_db > $BACKUP_DIR/house_db_$DATE.sql

# 备份配置文件
cp /opt/House-web-2.0/config/appconfig.json $BACKUP_DIR/appconfig_$DATE.json

# 删除30天前的备份
find $BACKUP_DIR -name "*.sql" -mtime +30 -delete
find $BACKUP_DIR -name "*.json" -mtime +30 -delete
```

设置定时任务：
```bash
sudo chmod +x /opt/backup_house_db.sh
sudo crontab -e

# 添加：每天凌晨3点备份
0 3 * * * /opt/backup_house_db.sh
```

---

## 九、监控与维护

### 9.1 日志查看
```bash
# 应用日志
sudo journalctl -u houseweb -f

# Nginx访问日志
sudo tail -f /var/log/nginx/houseweb_access.log

# Nginx错误日志
sudo tail -f /var/log/nginx/houseweb_error.log

# MySQL慢查询日志
sudo tail -f /var/log/mysql/mysql-slow.log
```

### 9.2 性能监控
```bash
# 查看服务状态
sudo systemctl status houseweb mysql nginx

# 查看端口占用
sudo netstat -tlnp | grep 8080

# 查看进程资源占用
top -p $(pgrep HouseWebServer)
```

### 9.3 常见问题排查

**问题1：数据库连接失败**
- 检查MySQL服务是否运行：`sudo systemctl status mysql`
- 验证用户密码：`mysql -u house_user -p`
- 查看配置文件是否正确

**问题2：邮件发送失败**
- 检查sendmail服务：`sudo systemctl status sendmail`
- 测试发送：`echo "test" | mail -s "test" your@email.com`
- 查看邮件队列：`mailq`

**问题3：百度地图不显示**
- 检查浏览器控制台是否有JS错误
- 验证AK是否正确配置
- 确认IP白名单设置

**问题4：AI助手不工作**
- 检查DeepSeek API密钥是否有效
- 验证账户余额是否充足
- 查看应用日志中的API调用错误

---

## 十、验收清单

部署完成后，请逐项验证：

- [ ] 数据库连接正常，表结构完整
- [ ] 应用服务正常启动，端口监听正常
- [ ] Nginx反向代理配置正确
- [ ] 百度地图在房源详情页正确显示
- [ ] 用户注册功能正常，邮件验证码能收到
- [ ] 用户登录功能正常
- [ ] 房源浏览、筛选功能正常
- [ ] 收藏功能正常
- [ ] AI助手能返回推荐结果
- [ ] 数据统计页面正常显示
- [ ] HTTPS证书配置正确（如已配置）
- [ ] 防火墙规则正确
- [ ] 日志记录正常
- [ ] 备份脚本测试通过

---

## 十一、技术支持联系方式

部署过程中如遇问题，可参考：
- 项目文档：`README.md` 和 `DEPLOYMENT.md`
- Qt官方文档：https://doc.qt.io/
- MySQL文档：https://dev.mysql.com/doc/
- Nginx文档：https://nginx.org/en/docs/
- 百度地图API文档：https://lbsyun.baidu.com/index.php?title=jspopular3.0
- DeepSeek API文档：https://platform.deepseek.com/docs

---

**最后更新：** 2025年12月31日
