#ifndef HOUSEINFO_H
#define HOUSEINFO_H

#include <QString>
#include <QVector>

class HouseInfo {
public:
    HouseInfo() = default;
    
    int id;
    QString houseTitle;
    double price;
    double area;
    QString communityName;
    QString floor;
    QString houseType;
    double unitPrice;
    QString houseUrl;
    
    static HouseInfo fromDatabase(const QSqlQuery& query);
};

struct HouseFilter {
    double minPrice = 0.0;
    double maxPrice = 0.0;
    double minUnitPrice = 0.0;
    double maxUnitPrice = 0.0;
    double minArea = 0.0;
    double maxArea = 0.0;
    QString region;
    QString houseType;
    int offset = 0;
    int limit = 20;
};

#endif // HOUSEINFO_H
