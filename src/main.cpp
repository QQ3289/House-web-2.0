#include <QApplication>
#include "mainwindow.h"
#include "configmanager.h"
#include "databasemanager.h"
#include "webserver.h"
#include <QDebug>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    // Load configuration
    if (!ConfigManager::instance().loadConfig("config.json")) {
        qCritical() << "Failed to load configuration";
        return 1;
    }
    
    // Initialize database
    if (!DatabaseManager::instance().initialize()) {
        qCritical() << "Failed to initialize database";
        return 1;
    }
    
    // Start web server
    WebServer server;
    int port = ConfigManager::instance().serverPort();
    if (!server.start(port)) {
        qCritical() << "Failed to start web server";
        return 1;
    }
    
    qInfo() << "Server running on http://localhost:" << port;
    
    // Show main window with WebEngine
    MainWindow window;
    window.show();
    
    return app.exec();
}
