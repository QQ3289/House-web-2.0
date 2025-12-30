#include "databasemanager.h"
#include <QDebug>
#include <QSqlRecord>
#include <QCryptographicHash>
#include <QDateTime>

DatabaseManager* DatabaseManager::m_instance = nullptr;

DatabaseManager* DatabaseManager::instance()
{
    if (!m_instance) {
        m_instance = new DatabaseManager();
    }
    return m_instance;
}

DatabaseManager::DatabaseManager(QObject *parent)
    : QObject(parent)
{
}

DatabaseManager::~DatabaseManager()
{
    if (db.isOpen()) {
        db.close();
    }
}

bool DatabaseManager::initialize(const QString& host, int port, const QString& dbName,
                                const QString& user, const QString& password)
{
    db = QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName(host);
    db.setPort(port);
    db.setDatabaseName(dbName);
    db.setUserName(user);
    db.setPassword(password);
    
    if (!db.open()) {
        qCritical() << "Database connection failed:" << db.lastError().text();
        return false;
    }
    
    qDebug() << "Database connected successfully";
    return createTables();
}

bool DatabaseManager::createTables()
{
    QSqlQuery query(db);
    
    // Create users table
    QString createUsersTable = R"(
        CREATE TABLE IF NOT EXISTS users (
            id INT AUTO_INCREMENT PRIMARY KEY,
            username VARCHAR(50) UNIQUE NOT NULL,
            password_hash VARCHAR(64) NOT NULL,
            email VARCHAR(100) UNIQUE NOT NULL,
            verified BOOLEAN DEFAULT FALSE,
            is_admin BOOLEAN DEFAULT FALSE,
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
            INDEX idx_username (username),
            INDEX idx_email (email)
        ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4
    )";
    
    if (!query.exec(createUsersTable)) {
        qCritical() << "Failed to create users table:" << query.lastError().text();
        return false;
    }
    
    // Create favorites table
    QString createFavoritesTable = R"(
        CREATE TABLE IF NOT EXISTS favorites (
            id INT AUTO_INCREMENT PRIMARY KEY,
            user_id INT NOT NULL,
            house_id INT NOT NULL,
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE,
            FOREIGN KEY (house_id) REFERENCES houseinfo(ID) ON DELETE CASCADE,
            UNIQUE KEY unique_favorite (user_id, house_id)
        ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4
    )";
    
    if (!query.exec(createFavoritesTable)) {
        qCritical() << "Failed to create favorites table:" << query.lastError().text();
        return false;
    }
    
    // Create user preferences table
    QString createPreferencesTable = R"(
        CREATE TABLE IF NOT EXISTS user_preferences (
            id INT AUTO_INCREMENT PRIMARY KEY,
            user_id INT NOT NULL,
            pref_key VARCHAR(50) NOT NULL,
            pref_value TEXT,
            FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE,
            UNIQUE KEY unique_preference (user_id, pref_key)
        ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4
    )";
    
    if (!query.exec(createPreferencesTable)) {
        qCritical() << "Failed to create preferences table:" << query.lastError().text();
        return false;
    }
    
    // Create email verification table
    QString createVerificationTable = R"(
        CREATE TABLE IF NOT EXISTS email_verification (
            id INT AUTO_INCREMENT PRIMARY KEY,
            email VARCHAR(100) NOT NULL,
            code VARCHAR(10) NOT NULL,
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
            INDEX idx_email (email)
        ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4
    )";
    
    if (!query.exec(createVerificationTable)) {
        qCritical() << "Failed to create verification table:" << query.lastError().text();
        return false;
    }
    
    qDebug() << "All tables created successfully";
    return true;
}

QVector<QVariantMap> DatabaseManager::searchHouses(const QVariantMap& filters)
{
    QVector<QVariantMap> results;
    QString queryStr = "SELECT * FROM houseinfo WHERE 1=1";
    
    // Build dynamic query
    if (filters.contains("minPrice")) {
        queryStr += QString(" AND price >= %1").arg(filters["minPrice"].toDouble());
    }
    if (filters.contains("maxPrice")) {
        queryStr += QString(" AND price <= %1").arg(filters["maxPrice"].toDouble());
    }
    if (filters.contains("minArea")) {
        queryStr += QString(" AND area >= %1").arg(filters["minArea"].toDouble());
    }
    if (filters.contains("maxArea")) {
        queryStr += QString(" AND area <= %1").arg(filters["maxArea"].toDouble());
    }
    if (filters.contains("communityName")) {
        queryStr += QString(" AND communityNa LIKE '%%1%'").arg(filters["communityName"].toString());
    }
    if (filters.contains("houseType")) {
        queryStr += QString(" AND houseType LIKE '%%1%'").arg(filters["houseType"].toString());
    }
    if (filters.contains("minUnitPrice")) {
        queryStr += QString(" AND unitPrice >= %1").arg(filters["minUnitPrice"].toDouble());
    }
    if (filters.contains("maxUnitPrice")) {
        queryStr += QString(" AND unitPrice <= %1").arg(filters["maxUnitPrice"].toDouble());
    }
    
    queryStr += " LIMIT 100";
    
    QSqlQuery query(db);
    if (query.exec(queryStr)) {
        while (query.next()) {
            QVariantMap house;
            house["id"] = query.value("ID").toInt();
            house["houseTitle"] = query.value("houseTitle").toString();
            house["price"] = query.value("price").toDouble();
            house["area"] = query.value("area").toDouble();
            house["communityNa"] = query.value("communityNa").toString();
            house["floor"] = query.value("floor").toString();
            house["houseType"] = query.value("houseType").toString();
            house["unitPrice"] = query.value("unitPrice").toDouble();
            house["houseUrl"] = query.value("houseUrl").toString();
            results.append(house);
        }
    } else {
        qWarning() << "Search houses failed:" << query.lastError().text();
    }
    
    return results;
}

QVariantMap DatabaseManager::getHouseById(int id)
{
    QVariantMap house;
    QSqlQuery query(db);
    query.prepare("SELECT * FROM houseinfo WHERE ID = ?");
    query.addBindValue(id);
    
    if (query.exec() && query.next()) {
        house["id"] = query.value("ID").toInt();
        house["houseTitle"] = query.value("houseTitle").toString();
        house["price"] = query.value("price").toDouble();
        house["area"] = query.value("area").toDouble();
        house["communityNa"] = query.value("communityNa").toString();
        house["floor"] = query.value("floor").toString();
        house["houseType"] = query.value("houseType").toString();
        house["unitPrice"] = query.value("unitPrice").toDouble();
        house["houseUrl"] = query.value("houseUrl").toString();
    }
    
    return house;
}

QVector<QVariantMap> DatabaseManager::getHouseStatistics(const QString& groupBy)
{
    QVector<QVariantMap> results;
    QString queryStr;
    
    if (groupBy == "price_range") {
        queryStr = R"(
            SELECT 
                CASE 
                    WHEN price < 1000000 THEN '100万以下'
                    WHEN price < 2000000 THEN '100-200万'
                    WHEN price < 3000000 THEN '200-300万'
                    ELSE '300万以上'
                END as range,
                COUNT(*) as count,
                AVG(price) as avg_price
            FROM houseinfo
            GROUP BY range
        )";
    } else if (groupBy == "area_range") {
        queryStr = R"(
            SELECT 
                CASE 
                    WHEN area < 60 THEN '60平以下'
                    WHEN area < 90 THEN '60-90平'
                    WHEN area < 120 THEN '90-120平'
                    ELSE '120平以上'
                END as range,
                COUNT(*) as count,
                AVG(price) as avg_price
            FROM houseinfo
            GROUP BY range
        )";
    } else {
        queryStr = "SELECT houseType as type, COUNT(*) as count, AVG(price) as avg_price FROM houseinfo GROUP BY houseType";
    }
    
    QSqlQuery query(db);
    if (query.exec(queryStr)) {
        while (query.next()) {
            QVariantMap stat;
            stat["category"] = query.value(0).toString();
            stat["count"] = query.value(1).toInt();
            stat["avgPrice"] = query.value(2).toDouble();
            results.append(stat);
        }
    }
    
    return results;
}

bool DatabaseManager::createUser(const QString& username, const QString& password,
                                const QString& email, const QString& verifyCode)
{
    // Hash password
    QByteArray hash = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256);
    QString passwordHash = hash.toHex();
    
    QSqlQuery query(db);
    query.prepare("INSERT INTO users (username, password_hash, email, verified) VALUES (?, ?, ?, FALSE)");
    query.addBindValue(username);
    query.addBindValue(passwordHash);
    query.addBindValue(email);
    
    if (!query.exec()) {
        qWarning() << "Create user failed:" << query.lastError().text();
        return false;
    }
    
    // Save verification code
    return saveVerificationCode(email, verifyCode);
}

bool DatabaseManager::verifyUser(const QString& email, const QString& code)
{
    QString savedCode = getVerificationCode(email);
    if (savedCode.isEmpty() || savedCode != code) {
        return false;
    }
    
    QSqlQuery query(db);
    query.prepare("UPDATE users SET verified = TRUE WHERE email = ?");
    query.addBindValue(email);
    
    return query.exec();
}

QVariantMap DatabaseManager::loginUser(const QString& username, const QString& password)
{
    QVariantMap user;
    QByteArray hash = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256);
    QString passwordHash = hash.toHex();
    
    QSqlQuery query(db);
    query.prepare("SELECT id, username, email, verified, is_admin FROM users WHERE username = ? AND password_hash = ?");
    query.addBindValue(username);
    query.addBindValue(passwordHash);
    
    if (query.exec() && query.next()) {
        user["id"] = query.value("id").toInt();
        user["username"] = query.value("username").toString();
        user["email"] = query.value("email").toString();
        user["verified"] = query.value("verified").toBool();
        user["isAdmin"] = query.value("is_admin").toBool();
    }
    
    return user;
}

bool DatabaseManager::updatePassword(const QString& email, const QString& newPassword)
{
    QByteArray hash = QCryptographicHash::hash(newPassword.toUtf8(), QCryptographicHash::Sha256);
    QString passwordHash = hash.toHex();
    
    QSqlQuery query(db);
    query.prepare("UPDATE users SET password_hash = ? WHERE email = ?");
    query.addBindValue(passwordHash);
    query.addBindValue(email);
    
    return query.exec();
}

QVariantMap DatabaseManager::getUserById(int userId)
{
    QVariantMap user;
    QSqlQuery query(db);
    query.prepare("SELECT id, username, email, verified, is_admin, created_at FROM users WHERE id = ?");
    query.addBindValue(userId);
    
    if (query.exec() && query.next()) {
        user["id"] = query.value("id").toInt();
        user["username"] = query.value("username").toString();
        user["email"] = query.value("email").toString();
        user["verified"] = query.value("verified").toBool();
        user["isAdmin"] = query.value("is_admin").toBool();
        user["createdAt"] = query.value("created_at").toString();
    }
    
    return user;
}

QVector<QVariantMap> DatabaseManager::getAllUsers()
{
    QVector<QVariantMap> users;
    QSqlQuery query(db);
    
    if (query.exec("SELECT id, username, email, verified, is_admin, created_at FROM users")) {
        while (query.next()) {
            QVariantMap user;
            user["id"] = query.value("id").toInt();
            user["username"] = query.value("username").toString();
            user["email"] = query.value("email").toString();
            user["verified"] = query.value("verified").toBool();
            user["isAdmin"] = query.value("is_admin").toBool();
            user["createdAt"] = query.value("created_at").toString();
            users.append(user);
        }
    }
    
    return users;
}

QVariantMap DatabaseManager::getUserStatistics()
{
    QVariantMap stats;
    QSqlQuery query(db);
    
    if (query.exec("SELECT COUNT(*) as total, SUM(verified) as verified FROM users")) {
        if (query.next()) {
            stats["totalUsers"] = query.value("total").toInt();
            stats["verifiedUsers"] = query.value("verified").toInt();
        }
    }
    
    return stats;
}

bool DatabaseManager::addFavorite(int userId, int houseId)
{
    QSqlQuery query(db);
    query.prepare("INSERT INTO favorites (user_id, house_id) VALUES (?, ?)");
    query.addBindValue(userId);
    query.addBindValue(houseId);
    
    return query.exec();
}

bool DatabaseManager::removeFavorite(int userId, int houseId)
{
    QSqlQuery query(db);
    query.prepare("DELETE FROM favorites WHERE user_id = ? AND house_id = ?");
    query.addBindValue(userId);
    query.addBindValue(houseId);
    
    return query.exec();
}

QVector<QVariantMap> DatabaseManager::getUserFavorites(int userId)
{
    QVector<QVariantMap> favorites;
    QSqlQuery query(db);
    query.prepare(R"(
        SELECT h.* 
        FROM houseinfo h
        INNER JOIN favorites f ON h.ID = f.house_id
        WHERE f.user_id = ?
    )");
    query.addBindValue(userId);
    
    if (query.exec()) {
        while (query.next()) {
            QVariantMap house;
            house["id"] = query.value("ID").toInt();
            house["houseTitle"] = query.value("houseTitle").toString();
            house["price"] = query.value("price").toDouble();
            house["area"] = query.value("area").toDouble();
            house["communityNa"] = query.value("communityNa").toString();
            house["floor"] = query.value("floor").toString();
            house["houseType"] = query.value("houseType").toString();
            house["unitPrice"] = query.value("unitPrice").toDouble();
            house["houseUrl"] = query.value("houseUrl").toString();
            favorites.append(house);
        }
    }
    
    return favorites;
}

bool DatabaseManager::setUserPreference(int userId, const QString& key, const QString& value)
{
    QSqlQuery query(db);
    query.prepare(R"(
        INSERT INTO user_preferences (user_id, pref_key, pref_value) 
        VALUES (?, ?, ?)
        ON DUPLICATE KEY UPDATE pref_value = ?
    )");
    query.addBindValue(userId);
    query.addBindValue(key);
    query.addBindValue(value);
    query.addBindValue(value);
    
    return query.exec();
}

QVariantMap DatabaseManager::getUserPreferences(int userId)
{
    QVariantMap preferences;
    QSqlQuery query(db);
    query.prepare("SELECT pref_key, pref_value FROM user_preferences WHERE user_id = ?");
    query.addBindValue(userId);
    
    if (query.exec()) {
        while (query.next()) {
            preferences[query.value("pref_key").toString()] = query.value("pref_value").toString();
        }
    }
    
    return preferences;
}

bool DatabaseManager::saveVerificationCode(const QString& email, const QString& code)
{
    QSqlQuery query(db);
    query.prepare("INSERT INTO email_verification (email, code) VALUES (?, ?)");
    query.addBindValue(email);
    query.addBindValue(code);
    
    return query.exec();
}

QString DatabaseManager::getVerificationCode(const QString& email)
{
    QSqlQuery query(db);
    query.prepare(R"(
        SELECT code FROM email_verification 
        WHERE email = ? 
        ORDER BY created_at DESC 
        LIMIT 1
    )");
    query.addBindValue(email);
    
    if (query.exec() && query.next()) {
        return query.value("code").toString();
    }
    
    return QString();
}
