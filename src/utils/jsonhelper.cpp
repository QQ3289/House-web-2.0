#include "jsonhelper.h"

QString JsonHelper::toJson(const QVariantMap& map)
{
    QJsonObject obj = variantMapToJson(map);
    QJsonDocument doc(obj);
    return QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
}

QString JsonHelper::toJson(const QVector<QVariantMap>& list)
{
    QJsonArray arr = vectorToJsonArray(list);
    QJsonDocument doc(arr);
    return QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
}

QVariantMap JsonHelper::fromJson(const QString& json)
{
    QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8());
    QJsonObject obj = doc.object();
    return obj.toVariantMap();
}

QJsonObject JsonHelper::variantMapToJson(const QVariantMap& map)
{
    QJsonObject obj;
    for (auto it = map.begin(); it != map.end(); ++it) {
        obj[it.key()] = QJsonValue::fromVariant(it.value());
    }
    return obj;
}

QJsonArray JsonHelper::vectorToJsonArray(const QVector<QVariantMap>& list)
{
    QJsonArray arr;
    for (const auto& map : list) {
        arr.append(variantMapToJson(map));
    }
    return arr;
}
