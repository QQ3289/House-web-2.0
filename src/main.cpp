#include <QCoreApplication>
#include <QDir>
#include <QDebug>
#include <QProcessEnvironment>
#include "httpserver.h"
#include "database/databasemanager.h"

static QString envOrDefault(const char* key, const QString& def)
{
    QByteArray v = qgetenv(key);
    return v.isEmpty() ? def : QString::fromUtf8(v);
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    QString dbHost = envOrDefault("DB_HOST", "127.0.0.1");
    int dbPort = envOrDefault("DB_PORT", "3306").toInt();
    QString dbName = envOrDefault("DB_NAME", "house_db");
    QString dbUser = envOrDefault("DB_USER", "root");
    QString dbPass = envOrDefault("DB_PASS", "");
    quint16 port = static_cast<quint16>(envOrDefault("APP_PORT", "8080").toInt());

    if (!DatabaseManager::instance()->initialize(dbHost, dbPort, dbName, dbUser, dbPass)) {
        qCritical() << "Failed to initialize database. Exiting.";
        return 1;
    }

    HttpServer server;
    QString webRoot = QDir::current().absoluteFilePath("web");
    if (!server.start(port, webRoot)) {
        qCritical() << "Failed to start HTTP server.";
        return 1;
    }

    return app.exec();
}
