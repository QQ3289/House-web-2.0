#ifndef JSONHELPER_H
#define JSONHELPER_H

#include <QString>
#include <QVariantMap>
#include <QVector>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

class JsonHelper
{
public:
    static QString toJson(const QVariantMap& map);
    static QString toJson(const QVector<QVariantMap>& list);
    static QVariantMap fromJson(const QString& json);
    static QJsonObject variantMapToJson(const QVariantMap& map);
    static QJsonArray vectorToJsonArray(const QVector<QVariantMap>& list);
};

#endif // JSONHELPER_H
