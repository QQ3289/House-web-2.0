#include "houseinfo.h"
#include <QSqlQuery>

HouseInfo HouseInfo::fromDatabase(const QSqlQuery& query) {
    HouseInfo info;
    info.id = query.value("ID").toInt();
    info.houseTitle = query.value("houseTitle").toString();
    info.price = query.value("price").toDouble();
    info.area = query.value("area").toDouble();
    info.communityName = query.value("communityNa").toString();
    info.floor = query.value("floor").toString();
    info.houseType = query.value("houseType").toString();
    info.unitPrice = query.value("unitPrice").toDouble();
    info.houseUrl = query.value("houseUrl").toString();
    return info;
}
