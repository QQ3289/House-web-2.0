#include "housemanager.h"
#include "database/databasemanager.h"
#include <QDebug>

HouseManager::HouseManager(QObject *parent)
    : QObject(parent)
{
}

QVector<QVariantMap> HouseManager::searchHouses(const QVariantMap& filters)
{
    DatabaseManager* db = DatabaseManager::instance();
    return db->searchHouses(filters);
}

QVariantMap HouseManager::getHouseDetails(int houseId)
{
    DatabaseManager* db = DatabaseManager::instance();
    return db->getHouseById(houseId);
}

QVector<QVariantMap> HouseManager::getHouseStatistics(const QString& type)
{
    DatabaseManager* db = DatabaseManager::instance();
    return db->getHouseStatistics(type);
}

bool HouseManager::addToFavorites(int userId, int houseId)
{
    DatabaseManager* db = DatabaseManager::instance();
    return db->addFavorite(userId, houseId);
}

bool HouseManager::removeFromFavorites(int userId, int houseId)
{
    DatabaseManager* db = DatabaseManager::instance();
    return db->removeFavorite(userId, houseId);
}

QVector<QVariantMap> HouseManager::getUserFavorites(int userId)
{
    DatabaseManager* db = DatabaseManager::instance();
    return db->getUserFavorites(userId);
}

bool HouseManager::setPreference(int userId, const QString& key, const QString& value)
{
    DatabaseManager* db = DatabaseManager::instance();
    return db->setUserPreference(userId, key, value);
}

QVariantMap HouseManager::getPreferences(int userId)
{
    DatabaseManager* db = DatabaseManager::instance();
    return db->getUserPreferences(userId);
}
