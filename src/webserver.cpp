#include "webserver.h"
#include "configmanager.h"
#include "databasemanager.h"
#include "usermanager.h"
#include "deepseekaiservice.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QFileInfo>
#include <QUrlQuery>
#include <QMimeDatabase>
#include <QDebug>

WebServer::WebServer(QObject* parent) : QObject(parent), m_server(new QTcpServer(this)) {
    connect(m_server, &QTcpServer::newConnection, this, &WebServer::onNewConnection);
}

WebServer::~WebServer() {
    stop();
}

bool WebServer::start(int port) {
    if (!m_server->listen(QHostAddress::Any, port)) {
        qCritical() << "Failed to start server:" << m_server->errorString();
        return false;
    }
    
    qInfo() << "Server started on port" << port;
    return true;
}

void WebServer::stop() {
    m_server->close();
}

void WebServer::onNewConnection() {
    QTcpSocket* socket = m_server->nextPendingConnection();
    connect(socket, &QTcpSocket::readyRead, this, &WebServer::onReadyRead);
    connect(socket, &QTcpSocket::disconnected, this, &WebServer::onDisconnected);
}

void WebServer::onReadyRead() {
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;
    
    m_buffers[socket].append(socket->readAll());
    
    if (m_buffers[socket].contains("\r\n\r\n")) {
        handleRequest(socket, m_buffers[socket]);
        m_buffers.remove(socket);
    }
}

void WebServer::onDisconnected() {
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    if (socket) {
        m_buffers.remove(socket);
        socket->deleteLater();
    }
}

void WebServer::handleRequest(QTcpSocket* socket, const QByteArray& request) {
    QString requestStr = QString::fromUtf8(request);
    QStringList lines = requestStr.split("\r\n");
    
    if (lines.isEmpty()) {
        sendResponse(socket, 400, "text/plain", "Bad Request");
        return;
    }
    
    QStringList requestLine = lines[0].split(" ");
    if (requestLine.size() < 3) {
        sendResponse(socket, 400, "text/plain", "Bad Request");
        return;
    }
    
    QString method = requestLine[0];
    QString path = requestLine[1];
    
    // Parse query string and body
    QUrl url("http://localhost" + path);
    QUrlQuery query(url);
    
    QJsonObject params;
    if (method == "POST") {
        int bodyStart = request.indexOf("\r\n\r\n") + 4;
        QByteArray body = request.mid(bodyStart);
        QJsonDocument doc = QJsonDocument::fromJson(body);
        if (doc.isObject()) {
            params = doc.object();
        }
    }
    
    // API Routes
    if (path.startsWith("/api/")) {
        if (path == "/api/login" && method == "POST") {
            handleLogin(socket, params);
        } else if (path == "/api/register" && method == "POST") {
            handleRegister(socket, params);
        } else if (path == "/api/verify-email" && method == "POST") {
            handleVerifyEmail(socket, params);
        } else if (path == "/api/change-password" && method == "POST") {
            handleChangePassword(socket, params);
        } else if (path == "/api/houses" && method == "GET") {
            handleGetHouses(socket, query);
        } else if (path.startsWith("/api/house/") && method == "GET") {
            int houseId = path.mid(11).toInt();
            handleGetHouseDetail(socket, houseId);
        } else if (path == "/api/favorites" && method == "GET") {
            handleGetFavorites(socket, query.queryItemValue("userId").toInt());
        } else if (path == "/api/favorites/add" && method == "POST") {
            handleAddFavorite(socket, params);
        } else if (path == "/api/favorites/remove" && method == "POST") {
            handleRemoveFavorite(socket, params);
        } else if (path == "/api/preferences" && method == "POST") {
            handleSetPreferences(socket, params);
        } else if (path == "/api/preferences" && method == "GET") {
            handleGetPreferences(socket, query.queryItemValue("userId").toInt());
        } else if (path == "/api/statistics" && method == "GET") {
            handleGetStatistics(socket);
        } else if (path == "/api/admin/user-stats" && method == "GET") {
            handleGetUserStats(socket);
        } else if (path == "/api/admin/users" && method == "GET") {
            handleGetAllUsers(socket);
        } else if (path == "/api/ai/recommend" && method == "POST") {
            handleAIRecommendation(socket, params);
        } else {
            sendJsonResponse(socket, 404, QJsonObject{{"error", "API endpoint not found"}});
        }
    } else {
        // Serve static files
        QString filePath = ConfigManager::instance().webRoot() + url.path();
        if (url.path() == "/") {
            filePath += "/index.html";
        }
        sendFile(socket, filePath);
    }
}

void WebServer::sendResponse(QTcpSocket* socket, int statusCode, const QString& contentType, const QByteArray& body) {
    QByteArray response;
    response += QString("HTTP/1.1 %1 OK\r\n").arg(statusCode).toUtf8();
    response += "Content-Type: " + contentType.toUtf8() + "\r\n";
    response += "Content-Length: " + QByteArray::number(body.size()) + "\r\n";
    response += "Access-Control-Allow-Origin: *\r\n";
    response += "\r\n";
    response += body;
    
    socket->write(response);
    socket->flush();
    socket->disconnectFromHost();
}

void WebServer::sendJsonResponse(QTcpSocket* socket, int statusCode, const QJsonObject& json) {
    QJsonDocument doc(json);
    sendResponse(socket, statusCode, "application/json", doc.toJson(QJsonDocument::Compact));
}

void WebServer::sendFile(QTcpSocket* socket, const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        sendResponse(socket, 404, "text/plain", "File Not Found");
        return;
    }
    
    QMimeDatabase mimeDb;
    QString mimeType = mimeDb.mimeTypeForFile(filePath).name();
    
    sendResponse(socket, 200, mimeType, file.readAll());
}

void WebServer::handleLogin(QTcpSocket* socket, const QJsonObject& params) {
    QString username = params["username"].toString();
    QString password = params["password"].toString();
    
    UserManager::LoginResult result = UserManager::instance().loginUser(username, password);
    
    QJsonObject response;
    response["success"] = result.success;
    response["message"] = result.message;
    if (result.success) {
        response["userId"] = result.userId;
        response["username"] = result.username;
        response["role"] = result.role;
    }
    
    sendJsonResponse(socket, 200, response);
}

void WebServer::handleRegister(QTcpSocket* socket, const QJsonObject& params) {
    QString username = params["username"].toString();
    QString password = params["password"].toString();
    QString email = params["email"].toString();
    
    UserManager::RegisterResult result = UserManager::instance().registerUser(username, password, email);
    
    QJsonObject response;
    response["success"] = result.success;
    response["message"] = result.message;
    
    sendJsonResponse(socket, 200, response);
}

void WebServer::handleVerifyEmail(QTcpSocket* socket, const QJsonObject& params) {
    QString username = params["username"].toString();
    QString token = params["token"].toString();
    
    bool success = UserManager::instance().verifyEmailToken(username, token);
    
    QJsonObject response;
    response["success"] = success;
    response["message"] = success ? "Email verified successfully" : "Invalid verification token";
    
    sendJsonResponse(socket, 200, response);
}

void WebServer::handleChangePassword(QTcpSocket* socket, const QJsonObject& params) {
    QString username = params["username"].toString();
    QString oldPassword = params["oldPassword"].toString();
    QString newPassword = params["newPassword"].toString();
    
    bool success = UserManager::instance().changePassword(username, oldPassword, newPassword);
    
    QJsonObject response;
    response["success"] = success;
    response["message"] = success ? "Password changed successfully" : "Failed to change password";
    
    sendJsonResponse(socket, 200, response);
}

void WebServer::handleGetHouses(QTcpSocket* socket, const QUrlQuery& query) {
    HouseFilter filter;
    filter.minPrice = query.queryItemValue("minPrice").toDouble();
    filter.maxPrice = query.queryItemValue("maxPrice").toDouble();
    filter.minUnitPrice = query.queryItemValue("minUnitPrice").toDouble();
    filter.maxUnitPrice = query.queryItemValue("maxUnitPrice").toDouble();
    filter.minArea = query.queryItemValue("minArea").toDouble();
    filter.maxArea = query.queryItemValue("maxArea").toDouble();
    filter.region = query.queryItemValue("region");
    filter.houseType = query.queryItemValue("houseType");
    filter.offset = query.queryItemValue("offset", "0").toInt();
    filter.limit = query.queryItemValue("limit", "20").toInt();
    
    int totalCount = 0;
    QVector<HouseInfo> houses = DatabaseManager::instance().getHouses(filter, totalCount);
    
    QJsonArray housesArray;
    for (const HouseInfo& house : houses) {
        QJsonObject obj;
        obj["id"] = house.id;
        obj["houseTitle"] = house.houseTitle;
        obj["price"] = house.price;
        obj["area"] = house.area;
        obj["communityName"] = house.communityName;
        obj["floor"] = house.floor;
        obj["houseType"] = house.houseType;
        obj["unitPrice"] = house.unitPrice;
        obj["houseUrl"] = house.houseUrl;
        housesArray.append(obj);
    }
    
    QJsonObject response;
    response["houses"] = housesArray;
    response["total"] = totalCount;
    
    sendJsonResponse(socket, 200, response);
}

void WebServer::handleGetHouseDetail(QTcpSocket* socket, int houseId) {
    HouseInfo house = DatabaseManager::instance().getHouseById(houseId);
    
    if (house.id == 0) {
        sendJsonResponse(socket, 404, QJsonObject{{"error", "House not found"}});
        return;
    }
    
    QJsonObject response;
    response["id"] = house.id;
    response["houseTitle"] = house.houseTitle;
    response["price"] = house.price;
    response["area"] = house.area;
    response["communityName"] = house.communityName;
    response["floor"] = house.floor;
    response["houseType"] = house.houseType;
    response["unitPrice"] = house.unitPrice;
    response["houseUrl"] = house.houseUrl;
    response["baiduMapKey"] = ConfigManager::instance().baiduMapApiKey();
    
    sendJsonResponse(socket, 200, response);
}

void WebServer::handleGetFavorites(QTcpSocket* socket, int userId) {
    QVector<HouseInfo> favorites = DatabaseManager::instance().getFavorites(userId);
    
    QJsonArray favoritesArray;
    for (const HouseInfo& house : favorites) {
        QJsonObject obj;
        obj["id"] = house.id;
        obj["houseTitle"] = house.houseTitle;
        obj["price"] = house.price;
        obj["area"] = house.area;
        obj["communityName"] = house.communityName;
        obj["houseType"] = house.houseType;
        obj["unitPrice"] = house.unitPrice;
        favoritesArray.append(obj);
    }
    
    QJsonObject response;
    response["favorites"] = favoritesArray;
    
    sendJsonResponse(socket, 200, response);
}

void WebServer::handleAddFavorite(QTcpSocket* socket, const QJsonObject& params) {
    int userId = params["userId"].toInt();
    int houseId = params["houseId"].toInt();
    
    bool success = DatabaseManager::instance().addFavorite(userId, houseId);
    
    QJsonObject response;
    response["success"] = success;
    
    sendJsonResponse(socket, 200, response);
}

void WebServer::handleRemoveFavorite(QTcpSocket* socket, const QJsonObject& params) {
    int userId = params["userId"].toInt();
    int houseId = params["houseId"].toInt();
    
    bool success = DatabaseManager::instance().removeFavorite(userId, houseId);
    
    QJsonObject response;
    response["success"] = success;
    
    sendJsonResponse(socket, 200, response);
}

void WebServer::handleSetPreferences(QTcpSocket* socket, const QJsonObject& params) {
    int userId = params["userId"].toInt();
    QString preferences = QJsonDocument(params["preferences"].toObject()).toJson();
    
    bool success = DatabaseManager::instance().setPreferences(userId, preferences);
    
    QJsonObject response;
    response["success"] = success;
    
    sendJsonResponse(socket, 200, response);
}

void WebServer::handleGetPreferences(QTcpSocket* socket, int userId) {
    QString preferencesStr = DatabaseManager::instance().getPreferences(userId);
    
    QJsonObject preferences;
    if (!preferencesStr.isEmpty()) {
        QJsonDocument doc = QJsonDocument::fromJson(preferencesStr.toUtf8());
        if (doc.isObject()) {
            preferences = doc.object();
        }
    }
    
    QJsonObject response;
    response["preferences"] = preferences;
    
    sendJsonResponse(socket, 200, response);
}

void WebServer::handleGetStatistics(QTcpSocket* socket) {
    QMap<QString, int> statsByRegion = DatabaseManager::instance().getHouseStatsByRegion();
    QMap<QString, int> statsByType = DatabaseManager::instance().getHouseStatsByType();
    QMap<QString, double> avgPriceByRegion = DatabaseManager::instance().getAveragePriceByRegion();
    
    QJsonObject regionStats;
    for (auto it = statsByRegion.constBegin(); it != statsByRegion.constEnd(); ++it) {
        regionStats[it.key()] = it.value();
    }
    
    QJsonObject typeStats;
    for (auto it = statsByType.constBegin(); it != statsByType.constEnd(); ++it) {
        typeStats[it.key()] = it.value();
    }
    
    QJsonObject avgPriceStats;
    for (auto it = avgPriceByRegion.constBegin(); it != avgPriceByRegion.constEnd(); ++it) {
        avgPriceStats[it.key()] = it.value();
    }
    
    QJsonObject response;
    response["byRegion"] = regionStats;
    response["byType"] = typeStats;
    response["avgPriceByRegion"] = avgPriceStats;
    
    sendJsonResponse(socket, 200, response);
}

void WebServer::handleGetUserStats(QTcpSocket* socket) {
    QMap<QString, int> userStats = DatabaseManager::instance().getUserStatsByRole();
    int totalUsers = DatabaseManager::instance().getTotalUsers();
    
    QJsonObject stats;
    for (auto it = userStats.constBegin(); it != userStats.constEnd(); ++it) {
        stats[it.key()] = it.value();
    }
    
    QJsonObject response;
    response["byRole"] = stats;
    response["total"] = totalUsers;
    
    sendJsonResponse(socket, 200, response);
}

void WebServer::handleGetAllUsers(QTcpSocket* socket) {
    QVector<QMap<QString, QString>> users = DatabaseManager::instance().getAllUsers();
    
    QJsonArray usersArray;
    for (const auto& user : users) {
        QJsonObject obj;
        for (auto it = user.constBegin(); it != user.constEnd(); ++it) {
            obj[it.key()] = it.value();
        }
        usersArray.append(obj);
    }
    
    QJsonObject response;
    response["users"] = usersArray;
    
    sendJsonResponse(socket, 200, response);
}

void WebServer::handleAIRecommendation(QTcpSocket* socket, const QJsonObject& params) {
    QString requirements = params["requirements"].toString();
    
    // Store socket for async response
    DeepSeekAIService& ai = DeepSeekAIService::instance();
    
    connect(&ai, &DeepSeekAIService::recommendationReady, socket, [this, socket](const QString& recommendation) {
        QJsonObject response;
        response["success"] = true;
        response["recommendation"] = recommendation;
        sendJsonResponse(socket, 200, response);
    }, Qt::SingleShotConnection);
    
    connect(&ai, &DeepSeekAIService::error, socket, [this, socket](const QString& error) {
        QJsonObject response;
        response["success"] = false;
        response["error"] = error;
        sendJsonResponse(socket, 200, response);
    }, Qt::SingleShotConnection);
    
    ai.getHouseRecommendation(requirements);
}
