#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QMap>

class WebServer : public QObject {
    Q_OBJECT
    
public:
    explicit WebServer(QObject* parent = nullptr);
    ~WebServer();
    
    bool start(int port);
    void stop();

private slots:
    void onNewConnection();
    void onReadyRead();
    void onDisconnected();

private:
    void handleRequest(QTcpSocket* socket, const QByteArray& request);
    void sendResponse(QTcpSocket* socket, int statusCode, const QString& contentType, const QByteArray& body);
    void sendJsonResponse(QTcpSocket* socket, int statusCode, const QJsonObject& json);
    void sendFile(QTcpSocket* socket, const QString& filePath);
    
    // API handlers
    void handleLogin(QTcpSocket* socket, const QJsonObject& params);
    void handleRegister(QTcpSocket* socket, const QJsonObject& params);
    void handleVerifyEmail(QTcpSocket* socket, const QJsonObject& params);
    void handleChangePassword(QTcpSocket* socket, const QJsonObject& params);
    void handleGetHouses(QTcpSocket* socket, const QUrlQuery& query);
    void handleGetHouseDetail(QTcpSocket* socket, int houseId);
    void handleGetFavorites(QTcpSocket* socket, int userId);
    void handleAddFavorite(QTcpSocket* socket, const QJsonObject& params);
    void handleRemoveFavorite(QTcpSocket* socket, const QJsonObject& params);
    void handleSetPreferences(QTcpSocket* socket, const QJsonObject& params);
    void handleGetPreferences(QTcpSocket* socket, int userId);
    void handleGetStatistics(QTcpSocket* socket);
    void handleGetUserStats(QTcpSocket* socket);
    void handleGetAllUsers(QTcpSocket* socket);
    void handleAIRecommendation(QTcpSocket* socket, const QJsonObject& params);
    
    QTcpServer* m_server;
    QMap<QTcpSocket*, QByteArray> m_buffers;
};

#endif // WEBSERVER_H
