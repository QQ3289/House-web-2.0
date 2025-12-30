#include "aiassistant.h"
#include "database/databasemanager.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkRequest>
#include <QDebug>
#include <QProcessEnvironment>

AIAssistant::AIAssistant(QObject *parent)
    : QObject(parent)
{
    networkManager = new QNetworkAccessManager(this);
    connect(networkManager, &QNetworkAccessManager::finished,
            this, &AIAssistant::onNetworkReply);
    
    // Configure DeepSeek API (can be overridden by env DEEPSEEK_API_KEY)
    apiKey = "your_deepseek_api_key";
    apiUrl = "https://api.deepseek.com/v1/chat/completions";
    QByteArray keyEnv = qgetenv("DEEPSEEK_API_KEY");
    if (!keyEnv.isEmpty()) {
        apiKey = QString::fromUtf8(keyEnv);
    }
}

void AIAssistant::getHouseRecommendation(const QString& userQuery, 
                                        std::function<void(const QString&)> callback)
{
    // Get all houses from database
    DatabaseManager* db = DatabaseManager::instance();
    QVector<QVariantMap> allHouses = db->searchHouses(QVariantMap());
    
    // Build prompt with house data
    QString prompt = buildPrompt(userQuery, allHouses);
    
    // Call DeepSeek API
    callDeepSeekAPI(prompt, callback);
}

QString AIAssistant::buildPrompt(const QString& userQuery, const QVector<QVariantMap>& houses)
{
    QString houseData;
    for (const auto& house : houses) {
        houseData += QString("ID:%1, 名称:%2, 价格:%3万, 面积:%4㎡, 小区:%5, 户型:%6, 单价:%7元/㎡\n")
            .arg(house["id"].toInt())
            .arg(house["houseTitle"].toString())
            .arg(house["price"].toDouble())
            .arg(house["area"].toDouble())
            .arg(house["communityNa"].toString())
            .arg(house["houseType"].toString())
            .arg(house["unitPrice"].toDouble());
    }
    
    QString prompt = QString(R"(
你是一个专业的房产推荐助手。根据用户的需求，从以下房源数据中推荐最合适的房产。

房源数据：
%1

用户需求：%2

请根据用户需求分析并推荐最合适的3-5套房产，说明推荐理由。请以友好的语气回复，包括：
1. 推荐的房源ID和名称
2. 每套房产的优势
3. 总体建议

请用中文回复。
    )").arg(houseData).arg(userQuery);
    
    return prompt;
}

void AIAssistant::callDeepSeekAPI(const QString& prompt, 
                                 std::function<void(const QString&)> callback)
{
    if (apiKey.startsWith("your_", Qt::CaseInsensitive) || apiKey.isEmpty()) {
        callback("AI助手未配置有效的API密钥，请在服务器上设置环境变量 DEEPSEEK_API_KEY。");
        return;
    }

    QJsonObject json;
    json["model"] = "deepseek-chat";
    
    QJsonArray messages;
    QJsonObject message;
    message["role"] = "user";
    message["content"] = prompt;
    messages.append(message);
    
    json["messages"] = messages;
    json["temperature"] = 0.7;
    json["max_tokens"] = 2000;
    
    QJsonDocument doc(json);
    QByteArray data = doc.toJson();
    
    QNetworkRequest request(QUrl(apiUrl));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", QString("Bearer %1").arg(apiKey).toUtf8());
    
    QNetworkReply* reply = networkManager->post(request, data);
    callbacks[reply] = callback;
}

void AIAssistant::onNetworkReply(QNetworkReply* reply)
{
    reply->deleteLater();
    
    if (!callbacks.contains(reply)) {
        return;
    }
    
    auto callback = callbacks.take(reply);
    
    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "DeepSeek API error:" << reply->errorString();
        callback("抱歉，AI助手暂时无法提供服务，请稍后再试。");
        return;
    }
    
    QByteArray responseData = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(responseData);
    QJsonObject obj = doc.object();
    
    if (obj.contains("choices") && obj["choices"].isArray()) {
        QJsonArray choices = obj["choices"].toArray();
        if (!choices.isEmpty()) {
            QJsonObject firstChoice = choices[0].toObject();
            QJsonObject message = firstChoice["message"].toObject();
            QString content = message["content"].toString();
            callback(content);
            return;
        }
    }
    
    callback("抱歉，无法解析AI响应。");
}
