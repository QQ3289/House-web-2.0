#include "usermanager.h"
#include "databasemanager.h"
#include "emailservice.h"
#include <QCryptographicHash>
#include <QDateTime>
#include <QRandomGenerator>

UserManager& UserManager::instance() {
    static UserManager instance;
    return instance;
}

UserManager::RegisterResult UserManager::registerUser(const QString& username, const QString& password, const QString& email) {
    RegisterResult result;
    
    if (username.isEmpty() || password.isEmpty() || email.isEmpty()) {
        result.success = false;
        result.message = "Username, password, and email are required";
        return result;
    }
    
    DatabaseManager& db = DatabaseManager::instance();
    
    if (db.userExists(username)) {
        result.success = false;
        result.message = "Username already exists";
        return result;
    }
    
    if (db.emailExists(email)) {
        result.success = false;
        result.message = "Email already registered";
        return result;
    }
    
    QString hashedPassword = hashPassword(password);
    
    if (db.createUser(username, hashedPassword, email)) {
        QString token = generateToken();
        db.setEmailVerifyToken(username, token);
        
        EmailService::instance().sendVerificationEmail(email, username, token);
        
        result.success = true;
        result.message = "Registration successful. Please check your email for verification.";
    } else {
        result.success = false;
        result.message = "Failed to create user";
    }
    
    return result;
}

UserManager::LoginResult UserManager::loginUser(const QString& username, const QString& password) {
    LoginResult result;
    result.success = false;
    
    if (username.isEmpty() || password.isEmpty()) {
        result.message = "Username and password are required";
        return result;
    }
    
    DatabaseManager& db = DatabaseManager::instance();
    QString hashedPassword = hashPassword(password);
    
    if (db.verifyUser(username, hashedPassword)) {
        result.success = true;
        result.message = "Login successful";
        result.userId = db.getUserId(username);
        result.username = username;
        result.role = db.getUserRole(username);
    } else {
        result.message = "Invalid username or password";
    }
    
    return result;
}

bool UserManager::changePassword(const QString& username, const QString& oldPassword, const QString& newPassword) {
    DatabaseManager& db = DatabaseManager::instance();
    
    QString hashedOldPassword = hashPassword(oldPassword);
    if (!db.verifyUser(username, hashedOldPassword)) {
        return false;
    }
    
    QString hashedNewPassword = hashPassword(newPassword);
    if (db.updatePassword(username, hashedNewPassword)) {
        QSqlQuery query;
        query.prepare("SELECT email FROM users WHERE username = ?");
        query.addBindValue(username);
        
        if (query.exec() && query.next()) {
            QString email = query.value(0).toString();
            EmailService::instance().sendPasswordChangeNotification(email, username);
        }
        
        emit passwordChanged(username);
        return true;
    }
    
    return false;
}

bool UserManager::verifyEmailToken(const QString& username, const QString& token) {
    return DatabaseManager::instance().verifyEmail(username, token);
}

QString UserManager::hashPassword(const QString& password) {
    QByteArray hash = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256);
    return hash.toHex();
}

QString UserManager::generateToken() {
    QString token;
    for (int i = 0; i < 32; ++i) {
        token += QString::number(QRandomGenerator::global()->bounded(16), 16);
    }
    return token;
}
