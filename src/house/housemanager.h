#ifndef HOUSEMANAGER_H
#define HOUSEMANAGER_H

#include <QObject>
#include <QVariantMap>
#include <QVector>

class HouseManager : public QObject
{
    Q_OBJECT

public:
    explicit HouseManager(QObject *parent = nullptr);
    
    // House browsing and filtering
    QVector<QVariantMap> searchHouses(const QVariantMap& filters);
    QVariantMap getHouseDetails(int houseId);
    QVector<QVariantMap> getHouseStatistics(const QString& type);
    
    // Favorites
    bool addToFavorites(int userId, int houseId);
    bool removeFromFavorites(int userId, int houseId);
    QVector<QVariantMap> getUserFavorites(int userId);
    
    // Preferences
    bool setPreference(int userId, const QString& key, const QString& value);
    QVariantMap getPreferences(int userId);
};

#endif // HOUSEMANAGER_H
