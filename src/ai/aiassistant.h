#ifndef AIASSISTANT_H
#define AIASSISTANT_H

#include <QObject>
#include <QString>
#include <QVariantMap>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class AIAssistant : public QObject
{
    Q_OBJECT

public:
    explicit AIAssistant(QObject *parent = nullptr);
    
    void getHouseRecommendation(const QString& userQuery, 
                               std::function<void(const QString&)> callback);

private slots:
    void onNetworkReply(QNetworkReply* reply);

private:
    QString buildPrompt(const QString& userQuery, const QVector<QVariantMap>& houses);
    void callDeepSeekAPI(const QString& prompt, 
                        std::function<void(const QString&)> callback);
    
    QNetworkAccessManager* networkManager;
    QString apiKey;
    QString apiUrl;
    QMap<QNetworkReply*, std::function<void(const QString&)>> callbacks;
};

#endif // AIASSISTANT_H
