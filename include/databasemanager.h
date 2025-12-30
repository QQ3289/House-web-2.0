#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QSqlDatabase>
#include <QVector>
#include "houseinfo.h"

class DatabaseManager {
public:
    static DatabaseManager& instance();
    
    bool initialize();
    bool createTables();
    
    // House operations
    QVector<HouseInfo> getHouses(const HouseFilter& filter, int& totalCount);
    HouseInfo getHouseById(int id);
    QMap<QString, int> getHouseStatsByRegion();
    QMap<QString, int> getHouseStatsByType();
    QMap<QString, double> getAveragePriceByRegion();
    
    // User operations
    bool createUser(const QString& username, const QString& password, const QString& email);
    bool verifyUser(const QString& username, const QString& password);
    bool userExists(const QString& username);
    bool emailExists(const QString& email);
    bool updatePassword(const QString& username, const QString& newPassword);
    bool verifyEmail(const QString& username, const QString& verifyToken);
    bool setEmailVerifyToken(const QString& username, const QString& token);
    int getUserId(const QString& username);
    QString getUserRole(const QString& username);
    
    // Favorites operations
    bool addFavorite(int userId, int houseId);
    bool removeFavorite(int userId, int houseId);
    QVector<HouseInfo> getFavorites(int userId);
    bool isFavorite(int userId, int houseId);
    
    // Preferences operations
    bool setPreferences(int userId, const QString& preferences);
    QString getPreferences(int userId);
    
    // Admin operations
    QMap<QString, int> getUserStatsByRole();
    int getTotalUsers();
    QVector<QMap<QString, QString>> getAllUsers();

private:
    DatabaseManager() = default;
    ~DatabaseManager();
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;
    
    QSqlDatabase m_db;
};

#endif // DATABASEMANAGER_H
