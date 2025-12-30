#include "httpserver.h"
#include "auth/usermanager.h"
#include "house/housemanager.h"
#include "ai/aiassistant.h"
#include "utils/jsonhelper.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QDir>
#include <QTextStream>
#include <QDateTime>
#include <QDebug>
#include <QUrlQuery>

HttpServer::HttpServer(QObject *parent)
    : QObject(parent)
{
    userManager = new UserManager(this);
    houseManager = new HouseManager(this);
    aiAssistant = new AIAssistant(this);
}

bool HttpServer::start(quint16 port, const QString& webRootPath)
{
    webRoot = webRootPath;

    connect(&server, &QTcpServer::newConnection, this, &HttpServer::onNewConnection);
    bool ok = server.listen(QHostAddress::Any, port);
    if (!ok) {
        qCritical() << "Failed to start server on port" << port << ":" << server.errorString();
        return false;
    }

    qInfo() << "HTTP server started on port" << port;
    return true;
}

void HttpServer::onNewConnection()
{
    while (server.hasPendingConnections()) {
        QTcpSocket* socket = server.nextPendingConnection();
        connect(socket, &QTcpSocket::readyRead, this, &HttpServer::onReadyRead);
        connect(socket, &QTcpSocket::disconnected, socket, &QTcpSocket::deleteLater);
    }
}

void HttpServer::onReadyRead()
{
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) {
        return;
    }

    QByteArray data = socket->readAll();
    if (data.isEmpty()) {
        return;
    }

    HttpRequest req = parseRequest(data);
    handleRequest(socket, req);
}

HttpRequest HttpServer::parseRequest(const QByteArray& data) const
{
    HttpRequest req;
    QList<QByteArray> parts = data.split('\n');
    if (parts.isEmpty()) {
        return req;
    }

    QString requestLine = QString::fromUtf8(parts.takeFirst()).trimmed();
    const QStringList lineParts = requestLine.split(' ');
    if (lineParts.size() >= 2) {
        req.method = lineParts[0];
        req.url = QUrl(QString::fromUtf8(lineParts[1]));
    }

    // Headers
    int i = 0;
    for (; i < parts.size(); ++i) {
        QString line = QString::fromUtf8(parts[i]).trimmed();
        if (line.isEmpty()) {
            ++i; // move past empty line
            break;
        }
        const int colonPos = line.indexOf(":");
        if (colonPos > 0) {
            const QString key = line.left(colonPos).trimmed();
            const QString value = line.mid(colonPos + 1).trimmed();
            req.headers[key.toLower()] = value;
        }
    }

    // Body
    QByteArray body;
    for (; i < parts.size(); ++i) {
        body += parts[i];
        if (i + 1 < parts.size()) {
            body += '\n';
        }
    }
    req.body = body;

    if (req.headers.value("content-type").contains("application/json", Qt::CaseInsensitive)) {
        QJsonDocument doc = QJsonDocument::fromJson(req.body);
        req.jsonBody = doc.object().toVariantMap();
    }

    return req;
}

void HttpServer::handleRequest(QTcpSocket* socket, const HttpRequest& req)
{
    if (req.method == "OPTIONS") {
        sendOptions(socket);
        return;
    }

    const QString path = req.url.path();

    if (path.startsWith("/api/")) {
        handleApiRequest(socket, req);
        return;
    }

    // Static files
    QString targetPath = path;
    if (targetPath == "/") {
        targetPath = "/index.html";
    }

    QString fullPath = webRoot + targetPath;
    if (QFile::exists(fullPath)) {
        sendFile(socket, fullPath, guessContentType(fullPath));
    } else {
        sendNotFound(socket);
    }
}

void HttpServer::handleApiRequest(QTcpSocket* socket, const HttpRequest& req)
{
    const QString path = req.url.path();
    QVariantMap payload;

    auto sendError = [&](const QString& msg){
        QVariantMap resp; resp["success"] = false; resp["message"] = msg; sendJson(socket, resp, 400); };

    if (path == "/api/register" && req.method == "POST") {
        const auto b = req.jsonBody;
        sendJson(socket, userManager->registerUser(b.value("username").toString(), b.value("password").toString(), b.value("email").toString()));
        return;
    }

    if (path == "/api/login" && req.method == "POST") {
        const auto b = req.jsonBody;
        sendJson(socket, userManager->loginUser(b.value("username").toString(), b.value("password").toString()));
        return;
    }

    if (path == "/api/verify" && req.method == "POST") {
        const auto b = req.jsonBody;
        bool ok = userManager->verifyEmail(b.value("email").toString(), b.value("code").toString());
        payload["success"] = ok; payload["message"] = ok ? "验证成功" : "验证码错误";
        sendJson(socket, payload, ok ? 200 : 400);
        return;
    }

    if (path == "/api/password/request" && req.method == "POST") {
        const auto b = req.jsonBody;
        bool ok = userManager->requestPasswordReset(b.value("email").toString());
        payload["success"] = ok; payload["message"] = ok ? "重置验证码已发送" : "请求失败";
        sendJson(socket, payload, ok ? 200 : 400);
        return;
    }

    if (path == "/api/password/reset" && req.method == "POST") {
        const auto b = req.jsonBody;
        bool ok = userManager->resetPassword(b.value("email").toString(), b.value("code").toString(), b.value("newPassword").toString());
        payload["success"] = ok; payload["message"] = ok ? "密码已更新" : "重置失败";
        sendJson(socket, payload, ok ? 200 : 400);
        return;
    }

    if (path == "/api/houses/search" && req.method == "POST") {
        sendJson(socket, houseManager->searchHouses(req.jsonBody));
        return;
    }

    QUrlQuery query(req.url);

    if (path == "/api/house" && req.method == "GET") {
        bool ok = false;
        int id = query.queryItemValue("id").toInt(&ok);
        if (!ok) { sendBadRequest(socket, "缺少房源ID"); return; }
        sendJson(socket, houseManager->getHouseDetails(id));
        return;
    }

    if (path == "/api/stats" && req.method == "GET") {
        QString type = query.queryItemValue("type");
        sendJson(socket, houseManager->getHouseStatistics(type));
        return;
    }

    if (path == "/api/favorites/add" && req.method == "POST") {
        const auto b = req.jsonBody; bool ok = houseManager->addToFavorites(b.value("userId").toInt(), b.value("houseId").toInt());
        payload["success"] = ok; payload["message"] = ok ? "已收藏" : "收藏失败"; sendJson(socket, payload, ok ? 200 : 400); return;
    }

    if (path == "/api/favorites/remove" && req.method == "POST") {
        const auto b = req.jsonBody; bool ok = houseManager->removeFromFavorites(b.value("userId").toInt(), b.value("houseId").toInt());
        payload["success"] = ok; payload["message"] = ok ? "已取消收藏" : "取消失败"; sendJson(socket, payload, ok ? 200 : 400); return;
    }

    if (path == "/api/favorites" && req.method == "GET") {
        bool ok = false; int uid = query.queryItemValue("userId").toInt(&ok);
        if (!ok) { sendBadRequest(socket, "缺少用户ID"); return; }
        sendJson(socket, houseManager->getUserFavorites(uid)); return;
    }

    if (path == "/api/preferences/set" && req.method == "POST") {
        const auto b = req.jsonBody; bool ok = houseManager->setPreference(b.value("userId").toInt(), b.value("key").toString(), b.value("value").toString());
        payload["success"] = ok; payload["message"] = ok ? "偏好已保存" : "保存失败"; sendJson(socket, payload, ok ? 200 : 400); return;
    }

    if (path == "/api/preferences" && req.method == "GET") {
        bool ok = false; int uid = query.queryItemValue("userId").toInt(&ok);
        if (!ok) { sendBadRequest(socket, "缺少用户ID"); return; }
        sendJson(socket, houseManager->getPreferences(uid)); return;
    }

    if (path == "/api/ai" && req.method == "POST") {
        const auto b = req.jsonBody;
        QString query = b.value("query").toString();
        if (query.trimmed().isEmpty()) { sendBadRequest(socket, "请输入需求"); return; }

        aiAssistant->getHouseRecommendation(query, [this, socket](const QString& content){
            QVariantMap resp; resp["success"] = true; resp["message"] = content; sendJson(socket, resp);
        });
        return;
    }

    sendNotFound(socket);
}

void HttpServer::sendJson(QTcpSocket* socket, const QVariantMap& payload, int statusCode)
{
    QByteArray body = JsonHelper::toJson(payload).toUtf8();
    QByteArray resp = buildHttpResponse(statusCode, "application/json", body);
    socket->write(resp);
    socket->flush();
    socket->disconnectFromHost();
}

void HttpServer::sendJson(QTcpSocket* socket, const QVector<QVariantMap>& payload, int statusCode)
{
    QByteArray body = JsonHelper::toJson(payload).toUtf8();
    QByteArray resp = buildHttpResponse(statusCode, "application/json", body);
    socket->write(resp);
    socket->flush();
    socket->disconnectFromHost();
}

void HttpServer::sendFile(QTcpSocket* socket, const QString& path, const QString& contentType)
{
    QByteArray body = readFileToBytes(path);
    if (body.isEmpty()) {
        sendNotFound(socket);
        return;
    }
    QByteArray resp = buildHttpResponse(200, contentType, body);
    socket->write(resp);
    socket->flush();
    socket->disconnectFromHost();
}

void HttpServer::sendNotFound(QTcpSocket* socket)
{
    QVariantMap resp; resp["success"] = false; resp["message"] = "Not Found";
    sendJson(socket, resp, 404);
}

void HttpServer::sendBadRequest(QTcpSocket* socket, const QString& message)
{
    QVariantMap resp; resp["success"] = false; resp["message"] = message;
    sendJson(socket, resp, 400);
}

void HttpServer::sendOptions(QTcpSocket* socket)
{
    QByteArray resp;
    resp.append("HTTP/1.1 200 OK\r\n");
    resp.append("Access-Control-Allow-Origin: *\r\n");
    resp.append("Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n");
    resp.append("Access-Control-Allow-Headers: Content-Type\r\n");
    resp.append("Content-Length: 0\r\n\r\n");
    socket->write(resp);
    socket->flush();
    socket->disconnectFromHost();
}

QString HttpServer::guessContentType(const QString& path) const
{
    if (path.endsWith(".html")) return "text/html; charset=utf-8";
    if (path.endsWith(".css")) return "text/css";
    if (path.endsWith(".js")) return "application/javascript";
    if (path.endsWith(".png")) return "image/png";
    if (path.endsWith(".jpg") || path.endsWith(".jpeg")) return "image/jpeg";
    if (path.endsWith(".svg")) return "image/svg+xml";
    return "text/plain";
}

QByteArray HttpServer::buildHttpResponse(int statusCode, const QString& contentType, const QByteArray& body) const
{
    QString statusText = "OK";
    if (statusCode == 400) statusText = "Bad Request";
    else if (statusCode == 404) statusText = "Not Found";

    QByteArray resp;
    resp.append(QString("HTTP/1.1 %1 %2\r\n").arg(statusCode).arg(statusText).toUtf8());
    resp.append("Content-Type: " + contentType.toUtf8() + "\r\n");
    resp.append("Access-Control-Allow-Origin: *\r\n");
    resp.append("Access-Control-Allow-Headers: Content-Type\r\n");
    resp.append("Content-Length: " + QByteArray::number(body.size()) + "\r\n");
    resp.append("Connection: close\r\n\r\n");
    resp.append(body);
    return resp;
}

QString HttpServer::readFileToString(const QString& path) const
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        return QString();
    }
    QTextStream in(&file);
    return in.readAll();
}

QByteArray HttpServer::readFileToBytes(const QString& path) const
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        return QByteArray();
    }
    return file.readAll();
}
