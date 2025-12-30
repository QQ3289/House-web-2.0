#include "databasemanager.h"
#include "configmanager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QVariant>

DatabaseManager& DatabaseManager::instance() {
    static DatabaseManager instance;
    return instance;
}

DatabaseManager::~DatabaseManager() {
    if (m_db.isOpen()) {
        m_db.close();
    }
}

bool DatabaseManager::initialize() {
    m_db = QSqlDatabase::addDatabase("QMYSQL");
    
    ConfigManager& config = ConfigManager::instance();
    m_db.setHostName(config.dbHost());
    m_db.setPort(config.dbPort());
    m_db.setDatabaseName(config.dbName());
    m_db.setUserName(config.dbUser());
    m_db.setPassword(config.dbPassword());
    
    if (!m_db.open()) {
        qCritical() << "Database connection failed:" << m_db.lastError().text();
        return false;
    }
    
    qInfo() << "Database connected successfully";
    return createTables();
}

bool DatabaseManager::createTables() {
    QSqlQuery query(m_db);
    
    // Users table
    QString createUsersTable = R"(
        CREATE TABLE IF NOT EXISTS users (
            id INT AUTO_INCREMENT PRIMARY KEY,
            username VARCHAR(50) UNIQUE NOT NULL,
            password_hash VARCHAR(256) NOT NULL,
            email VARCHAR(100) UNIQUE NOT NULL,
            role VARCHAR(20) DEFAULT 'user',
            email_verified BOOLEAN DEFAULT FALSE,
            verify_token VARCHAR(64),
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
        )
    )";
    
    if (!query.exec(createUsersTable)) {
        qCritical() << "Failed to create users table:" << query.lastError().text();
        return false;
    }
    
    // Favorites table
    QString createFavoritesTable = R"(
        CREATE TABLE IF NOT EXISTS favorites (
            id INT AUTO_INCREMENT PRIMARY KEY,
            user_id INT NOT NULL,
            house_id INT NOT NULL,
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
            UNIQUE KEY unique_favorite (user_id, house_id),
            FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE
        )
    )";
    
    if (!query.exec(createFavoritesTable)) {
        qCritical() << "Failed to create favorites table:" << query.lastError().text();
        return false;
    }
    
    // Preferences table
    QString createPreferencesTable = R"(
        CREATE TABLE IF NOT EXISTS user_preferences (
            id INT AUTO_INCREMENT PRIMARY KEY,
            user_id INT UNIQUE NOT NULL,
            preferences TEXT,
            updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
            FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE
        )
    )";
    
    if (!query.exec(createPreferencesTable)) {
        qCritical() << "Failed to create preferences table:" << query.lastError().text();
        return false;
    }
    
    // Create default admin user if not exists
    ConfigManager& config = ConfigManager::instance();
    query.prepare("SELECT id FROM users WHERE username = ?");
    query.addBindValue(config.adminUsername());
    
    if (query.exec() && !query.next()) {
        query.prepare("INSERT INTO users (username, password_hash, email, role, email_verified) VALUES (?, ?, ?, ?, ?)");
        query.addBindValue(config.adminUsername());
        // Simple hash for default admin (should use proper hashing in production)
        query.addBindValue(config.adminPassword());
        query.addBindValue("admin@localhost");
        query.addBindValue("admin");
        query.addBindValue(true);
        
        if (!query.exec()) {
            qWarning() << "Failed to create admin user:" << query.lastError().text();
        }
    }
    
    return true;
}

QVector<HouseInfo> DatabaseManager::getHouses(const HouseFilter& filter, int& totalCount) {
    QVector<HouseInfo> houses;
    QSqlQuery query(m_db);
    
    QString sql = "SELECT * FROM houseinfo WHERE 1=1";
    
    if (filter.minPrice > 0) {
        sql += QString(" AND price >= %1").arg(filter.minPrice);
    }
    if (filter.maxPrice > 0) {
        sql += QString(" AND price <= %1").arg(filter.maxPrice);
    }
    if (filter.minUnitPrice > 0) {
        sql += QString(" AND unitPrice >= %1").arg(filter.minUnitPrice);
    }
    if (filter.maxUnitPrice > 0) {
        sql += QString(" AND unitPrice <= %1").arg(filter.maxUnitPrice);
    }
    if (filter.minArea > 0) {
        sql += QString(" AND area >= %1").arg(filter.minArea);
    }
    if (filter.maxArea > 0) {
        sql += QString(" AND area <= %1").arg(filter.maxArea);
    }
    if (!filter.region.isEmpty()) {
        sql += QString(" AND communityNa LIKE '%%1%'").arg(filter.region);
    }
    if (!filter.houseType.isEmpty()) {
        sql += QString(" AND houseType LIKE '%%1%'").arg(filter.houseType);
    }
    
    // Get total count
    QString countSql = "SELECT COUNT(*) FROM houseinfo WHERE 1=1";
    countSql += sql.mid(sql.indexOf("WHERE") + 5);
    if (query.exec(countSql) && query.next()) {
        totalCount = query.value(0).toInt();
    }
    
    // Add pagination
    sql += QString(" LIMIT %1 OFFSET %2").arg(filter.limit).arg(filter.offset);
    
    if (!query.exec(sql)) {
        qWarning() << "Failed to query houses:" << query.lastError().text();
        return houses;
    }
    
    while (query.next()) {
        houses.append(HouseInfo::fromDatabase(query));
    }
    
    return houses;
}

HouseInfo DatabaseManager::getHouseById(int id) {
    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM houseinfo WHERE ID = ?");
    query.addBindValue(id);
    
    if (query.exec() && query.next()) {
        return HouseInfo::fromDatabase(query);
    }
    
    return HouseInfo();
}

bool DatabaseManager::createUser(const QString& username, const QString& password, const QString& email) {
    QSqlQuery query(m_db);
    query.prepare("INSERT INTO users (username, password_hash, email) VALUES (?, ?, ?)");
    query.addBindValue(username);
    query.addBindValue(password);
    query.addBindValue(email);
    
    if (!query.exec()) {
        qWarning() << "Failed to create user:" << query.lastError().text();
        return false;
    }
    
    return true;
}

bool DatabaseManager::verifyUser(const QString& username, const QString& password) {
    QSqlQuery query(m_db);
    query.prepare("SELECT password_hash FROM users WHERE username = ?");
    query.addBindValue(username);
    
    if (query.exec() && query.next()) {
        return query.value(0).toString() == password;
    }
    
    return false;
}

bool DatabaseManager::userExists(const QString& username) {
    QSqlQuery query(m_db);
    query.prepare("SELECT id FROM users WHERE username = ?");
    query.addBindValue(username);
    
    return query.exec() && query.next();
}

bool DatabaseManager::emailExists(const QString& email) {
    QSqlQuery query(m_db);
    query.prepare("SELECT id FROM users WHERE email = ?");
    query.addBindValue(email);
    
    return query.exec() && query.next();
}

bool DatabaseManager::updatePassword(const QString& username, const QString& newPassword) {
    QSqlQuery query(m_db);
    query.prepare("UPDATE users SET password_hash = ? WHERE username = ?");
    query.addBindValue(newPassword);
    query.addBindValue(username);
    
    return query.exec();
}

bool DatabaseManager::verifyEmail(const QString& username, const QString& verifyToken) {
    QSqlQuery query(m_db);
    query.prepare("SELECT verify_token FROM users WHERE username = ?");
    query.addBindValue(username);
    
    if (query.exec() && query.next()) {
        if (query.value(0).toString() == verifyToken) {
            query.prepare("UPDATE users SET email_verified = TRUE, verify_token = NULL WHERE username = ?");
            query.addBindValue(username);
            return query.exec();
        }
    }
    
    return false;
}

bool DatabaseManager::setEmailVerifyToken(const QString& username, const QString& token) {
    QSqlQuery query(m_db);
    query.prepare("UPDATE users SET verify_token = ? WHERE username = ?");
    query.addBindValue(token);
    query.addBindValue(username);
    
    return query.exec();
}

int DatabaseManager::getUserId(const QString& username) {
    QSqlQuery query(m_db);
    query.prepare("SELECT id FROM users WHERE username = ?");
    query.addBindValue(username);
    
    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }
    
    return -1;
}

QString DatabaseManager::getUserRole(const QString& username) {
    QSqlQuery query(m_db);
    query.prepare("SELECT role FROM users WHERE username = ?");
    query.addBindValue(username);
    
    if (query.exec() && query.next()) {
        return query.value(0).toString();
    }
    
    return "";
}

bool DatabaseManager::addFavorite(int userId, int houseId) {
    QSqlQuery query(m_db);
    query.prepare("INSERT INTO favorites (user_id, house_id) VALUES (?, ?)");
    query.addBindValue(userId);
    query.addBindValue(houseId);
    
    return query.exec();
}

bool DatabaseManager::removeFavorite(int userId, int houseId) {
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM favorites WHERE user_id = ? AND house_id = ?");
    query.addBindValue(userId);
    query.addBindValue(houseId);
    
    return query.exec();
}

QVector<HouseInfo> DatabaseManager::getFavorites(int userId) {
    QVector<HouseInfo> houses;
    QSqlQuery query(m_db);
    query.prepare("SELECT h.* FROM houseinfo h INNER JOIN favorites f ON h.ID = f.house_id WHERE f.user_id = ?");
    query.addBindValue(userId);
    
    if (query.exec()) {
        while (query.next()) {
            houses.append(HouseInfo::fromDatabase(query));
        }
    }
    
    return houses;
}

bool DatabaseManager::isFavorite(int userId, int houseId) {
    QSqlQuery query(m_db);
    query.prepare("SELECT id FROM favorites WHERE user_id = ? AND house_id = ?");
    query.addBindValue(userId);
    query.addBindValue(houseId);
    
    return query.exec() && query.next();
}

bool DatabaseManager::setPreferences(int userId, const QString& preferences) {
    QSqlQuery query(m_db);
    query.prepare("INSERT INTO user_preferences (user_id, preferences) VALUES (?, ?) ON DUPLICATE KEY UPDATE preferences = ?");
    query.addBindValue(userId);
    query.addBindValue(preferences);
    query.addBindValue(preferences);
    
    return query.exec();
}

QString DatabaseManager::getPreferences(int userId) {
    QSqlQuery query(m_db);
    query.prepare("SELECT preferences FROM user_preferences WHERE user_id = ?");
    query.addBindValue(userId);
    
    if (query.exec() && query.next()) {
        return query.value(0).toString();
    }
    
    return "";
}

QMap<QString, int> DatabaseManager::getHouseStatsByRegion() {
    QMap<QString, int> stats;
    QSqlQuery query(m_db);
    
    if (query.exec("SELECT SUBSTRING_INDEX(communityNa, '-', 1) as region, COUNT(*) as count FROM houseinfo GROUP BY region")) {
        while (query.next()) {
            stats[query.value(0).toString()] = query.value(1).toInt();
        }
    }
    
    return stats;
}

QMap<QString, int> DatabaseManager::getHouseStatsByType() {
    QMap<QString, int> stats;
    QSqlQuery query(m_db);
    
    if (query.exec("SELECT houseType, COUNT(*) as count FROM houseinfo GROUP BY houseType")) {
        while (query.next()) {
            stats[query.value(0).toString()] = query.value(1).toInt();
        }
    }
    
    return stats;
}

QMap<QString, double> DatabaseManager::getAveragePriceByRegion() {
    QMap<QString, double> stats;
    QSqlQuery query(m_db);
    
    if (query.exec("SELECT SUBSTRING_INDEX(communityNa, '-', 1) as region, AVG(price) as avg_price FROM houseinfo GROUP BY region")) {
        while (query.next()) {
            stats[query.value(0).toString()] = query.value(1).toDouble();
        }
    }
    
    return stats;
}

QMap<QString, int> DatabaseManager::getUserStatsByRole() {
    QMap<QString, int> stats;
    QSqlQuery query(m_db);
    
    if (query.exec("SELECT role, COUNT(*) as count FROM users GROUP BY role")) {
        while (query.next()) {
            stats[query.value(0).toString()] = query.value(1).toInt();
        }
    }
    
    return stats;
}

int DatabaseManager::getTotalUsers() {
    QSqlQuery query(m_db);
    
    if (query.exec("SELECT COUNT(*) FROM users") && query.next()) {
        return query.value(0).toInt();
    }
    
    return 0;
}

QVector<QMap<QString, QString>> DatabaseManager::getAllUsers() {
    QVector<QMap<QString, QString>> users;
    QSqlQuery query(m_db);
    
    if (query.exec("SELECT id, username, email, role, email_verified, created_at FROM users")) {
        while (query.next()) {
            QMap<QString, QString> user;
            user["id"] = query.value(0).toString();
            user["username"] = query.value(1).toString();
            user["email"] = query.value(2).toString();
            user["role"] = query.value(3).toString();
            user["email_verified"] = query.value(4).toBool() ? "true" : "false";
            user["created_at"] = query.value(5).toString();
            users.append(user);
        }
    }
    
    return users;
}
