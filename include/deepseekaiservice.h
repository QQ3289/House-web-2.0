#ifndef DEEPSEEKAISERVICE_H
#define DEEPSEEKAISERVICE_H

#include <QObject>
#include <QString>
#include <QNetworkAccessManager>
#include <QVector>
#include "houseinfo.h"

class DeepSeekAIService : public QObject {
    Q_OBJECT
    
public:
    static DeepSeekAIService& instance();
    
    void getHouseRecommendation(const QString& userRequirements);

signals:
    void recommendationReady(const QString& recommendation);
    void error(const QString& errorMessage);

private slots:
    void onReplyFinished(QNetworkReply* reply);

private:
    DeepSeekAIService();
    ~DeepSeekAIService() = default;
    DeepSeekAIService(const DeepSeekAIService&) = delete;
    DeepSeekAIService& operator=(const DeepSeekAIService&) = delete;
    
    QString preparePrompt(const QString& userRequirements, const QVector<HouseInfo>& houses);
    
    QNetworkAccessManager* m_networkManager;
};

#endif // DEEPSEEKAISERVICE_H
