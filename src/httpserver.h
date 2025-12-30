#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QUrl>
#include <QFile>
#include <QMap>
#include <QVariantMap>
#include <functional>

struct HttpRequest
{
    QString method;
    QUrl url;
    QMap<QString, QString> headers;
    QByteArray body;
    QVariantMap jsonBody;
};

class UserManager;
class HouseManager;
class AIAssistant;

class HttpServer : public QObject
{
    Q_OBJECT

public:
    explicit HttpServer(QObject *parent = nullptr);
    bool start(quint16 port, const QString& webRootPath);

private slots:
    void onNewConnection();
    void onReadyRead();

private:
    HttpRequest parseRequest(const QByteArray& data) const;
    void handleRequest(QTcpSocket* socket, const HttpRequest& req);
    void handleApiRequest(QTcpSocket* socket, const HttpRequest& req);
    void sendJson(QTcpSocket* socket, const QVariantMap& payload, int statusCode = 200);
    void sendJson(QTcpSocket* socket, const QVector<QVariantMap>& payload, int statusCode = 200);
    void sendFile(QTcpSocket* socket, const QString& path, const QString& contentType = "text/html");
    void sendNotFound(QTcpSocket* socket);
    void sendBadRequest(QTcpSocket* socket, const QString& message);
    void sendOptions(QTcpSocket* socket);
    QString guessContentType(const QString& path) const;
    QByteArray buildHttpResponse(int statusCode, const QString& contentType, const QByteArray& body) const;
    QString readFileToString(const QString& path) const;
    QByteArray readFileToBytes(const QString& path) const;

    QTcpServer server;
    QString webRoot;
    UserManager* userManager;
    HouseManager* houseManager;
    AIAssistant* aiAssistant;
};

#endif // HTTPSERVER_H
