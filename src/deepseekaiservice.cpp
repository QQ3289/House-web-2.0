#include "deepseekaiservice.h"
#include "configmanager.h"
#include "databasemanager.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

DeepSeekAIService& DeepSeekAIService::instance() {
    static DeepSeekAIService instance;
    return instance;
}

DeepSeekAIService::DeepSeekAIService() {
    m_networkManager = new QNetworkAccessManager(this);
    connect(m_networkManager, &QNetworkAccessManager::finished, this, &DeepSeekAIService::onReplyFinished);
}

void DeepSeekAIService::getHouseRecommendation(const QString& userRequirements) {
    HouseFilter filter;
    filter.limit = 50;
    
    int totalCount = 0;
    QVector<HouseInfo> houses = DatabaseManager::instance().getHouses(filter, totalCount);
    
    QString prompt = preparePrompt(userRequirements, houses);
    
    ConfigManager& config = ConfigManager::instance();
    QNetworkRequest request(config.deepseekApiUrl());
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", QString("Bearer %1").arg(config.deepseekApiKey()).toUtf8());
    
    QJsonObject messageObj;
    messageObj["role"] = "user";
    messageObj["content"] = prompt;
    
    QJsonArray messages;
    messages.append(messageObj);
    
    QJsonObject requestBody;
    requestBody["model"] = "deepseek-chat";
    requestBody["messages"] = messages;
    requestBody["temperature"] = 0.7;
    requestBody["max_tokens"] = 1000;
    
    QJsonDocument doc(requestBody);
    m_networkManager->post(request, doc.toJson());
}

void DeepSeekAIService::onReplyFinished(QNetworkReply* reply) {
    if (reply->error() != QNetworkReply::NoError) {
        emit error(QString("API request failed: %1").arg(reply->errorString()));
        reply->deleteLater();
        return;
    }
    
    QByteArray responseData = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(responseData);
    
    if (!doc.isObject()) {
        emit error("Invalid API response");
        reply->deleteLater();
        return;
    }
    
    QJsonObject responseObj = doc.object();
    QJsonArray choices = responseObj["choices"].toArray();
    
    if (choices.isEmpty()) {
        emit error("No recommendations generated");
        reply->deleteLater();
        return;
    }
    
    QString recommendation = choices[0].toObject()["message"].toObject()["content"].toString();
    emit recommendationReady(recommendation);
    
    reply->deleteLater();
}

QString DeepSeekAIService::preparePrompt(const QString& userRequirements, const QVector<HouseInfo>& houses) {
    QString prompt = QString(
        "You are a professional real estate consultant. Based on the user's requirements and available houses, "
        "provide personalized recommendations.\n\n"
        "User Requirements: %1\n\n"
        "Available Houses:\n"
    ).arg(userRequirements);
    
    for (int i = 0; i < qMin(houses.size(), 20); ++i) {
        const HouseInfo& house = houses[i];
        prompt += QString(
            "%1. %2\n"
            "   Price: %.2f million, Area: %.2f sqm, Unit Price: %.2f/sqm\n"
            "   Community: %3, Type: %4, Floor: %5\n\n"
        ).arg(i + 1)
         .arg(house.houseTitle)
         .arg(house.price)
         .arg(house.area)
         .arg(house.unitPrice)
         .arg(house.communityName)
         .arg(house.houseType)
         .arg(house.floor);
    }
    
    prompt += "\nPlease provide 3-5 best recommendations with reasoning.";
    
    return prompt;
}
