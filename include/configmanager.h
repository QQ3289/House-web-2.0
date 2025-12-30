#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <QString>
#include <QJsonObject>

class ConfigManager {
public:
    static ConfigManager& instance();
    
    bool loadConfig(const QString& configPath = "config.json");
    
    QString dbHost() const;
    int dbPort() const;
    QString dbName() const;
    QString dbUser() const;
    QString dbPassword() const;
    
    QString deepseekApiUrl() const;
    QString deepseekApiKey() const;
    
    QString baiduMapApiKey() const;
    
    QString smtpHost() const;
    int smtpPort() const;
    QString emailUsername() const;
    QString emailPassword() const;
    QString emailFromName() const;
    
    int serverPort() const;
    QString webRoot() const;
    
    QString adminUsername() const;
    QString adminPassword() const;

private:
    ConfigManager() = default;
    ~ConfigManager() = default;
    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;
    
    QJsonObject m_config;
};

#endif // CONFIGMANAGER_H
