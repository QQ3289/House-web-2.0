#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QString>
#include <QVariantMap>
#include <QVector>

class DatabaseManager : public QObject
{
    Q_OBJECT

public:
    static DatabaseManager* instance();
    
    bool initialize(const QString& host, int port, const QString& dbName,
                   const QString& user, const QString& password);
    
    // House operations
    QVector<QVariantMap> searchHouses(const QVariantMap& filters);
    QVariantMap getHouseById(int id);
    QVector<QVariantMap> getHouseStatistics(const QString& groupBy);
    
    // User operations
    bool createUser(const QString& username, const QString& password, 
                   const QString& email, const QString& verifyCode);
    bool verifyUser(const QString& email, const QString& code);
    QVariantMap loginUser(const QString& username, const QString& password);
    bool updatePassword(const QString& email, const QString& newPassword);
    QVariantMap getUserById(int userId);
    QVector<QVariantMap> getAllUsers();
    QVariantMap getUserStatistics();
    
    // Favorites operations
    bool addFavorite(int userId, int houseId);
    bool removeFavorite(int userId, int houseId);
    QVector<QVariantMap> getUserFavorites(int userId);
    
    // Preferences operations
    bool setUserPreference(int userId, const QString& key, const QString& value);
    QVariantMap getUserPreferences(int userId);
    
    // Email verification
    bool saveVerificationCode(const QString& email, const QString& code);
    QString getVerificationCode(const QString& email);

private:
    explicit DatabaseManager(QObject *parent = nullptr);
    ~DatabaseManager();
    
    bool createTables();
    QSqlDatabase db;
    static DatabaseManager* m_instance;
};

#endif // DATABASEMANAGER_H
