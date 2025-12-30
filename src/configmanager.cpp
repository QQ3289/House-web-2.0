#include "configmanager.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

ConfigManager& ConfigManager::instance() {
    static ConfigManager instance;
    return instance;
}

bool ConfigManager::loadConfig(const QString& configPath) {
    QFile file(configPath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open config file:" << configPath;
        return false;
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull() || !doc.isObject()) {
        qWarning() << "Invalid JSON in config file";
        return false;
    }
    
    m_config = doc.object();
    return true;
}

QString ConfigManager::dbHost() const {
    return m_config["database"].toObject()["host"].toString();
}

int ConfigManager::dbPort() const {
    return m_config["database"].toObject()["port"].toInt();
}

QString ConfigManager::dbName() const {
    return m_config["database"].toObject()["name"].toString();
}

QString ConfigManager::dbUser() const {
    return m_config["database"].toObject()["user"].toString();
}

QString ConfigManager::dbPassword() const {
    return m_config["database"].toObject()["password"].toString();
}

QString ConfigManager::deepseekApiUrl() const {
    return m_config["deepseek"].toObject()["api_url"].toString();
}

QString ConfigManager::deepseekApiKey() const {
    return m_config["deepseek"].toObject()["api_key"].toString();
}

QString ConfigManager::baiduMapApiKey() const {
    return m_config["baidu_map"].toObject()["api_key"].toString();
}

QString ConfigManager::smtpHost() const {
    return m_config["email"].toObject()["smtp_host"].toString();
}

int ConfigManager::smtpPort() const {
    return m_config["email"].toObject()["smtp_port"].toInt();
}

QString ConfigManager::emailUsername() const {
    return m_config["email"].toObject()["username"].toString();
}

QString ConfigManager::emailPassword() const {
    return m_config["email"].toObject()["password"].toString();
}

QString ConfigManager::emailFromName() const {
    return m_config["email"].toObject()["from_name"].toString();
}

int ConfigManager::serverPort() const {
    return m_config["server"].toObject()["port"].toInt();
}

QString ConfigManager::webRoot() const {
    return m_config["server"].toObject()["web_root"].toString();
}

QString ConfigManager::adminUsername() const {
    return m_config["admin"].toObject()["default_username"].toString();
}

QString ConfigManager::adminPassword() const {
    return m_config["admin"].toObject()["default_password"].toString();
}
