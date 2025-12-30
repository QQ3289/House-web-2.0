#ifndef USERMANAGER_H
#define USERMANAGER_H

#include <QString>
#include <QObject>

class UserManager : public QObject {
    Q_OBJECT
    
public:
    static UserManager& instance();
    
    struct RegisterResult {
        bool success;
        QString message;
    };
    
    struct LoginResult {
        bool success;
        QString message;
        int userId;
        QString username;
        QString role;
    };
    
    RegisterResult registerUser(const QString& username, const QString& password, const QString& email);
    LoginResult loginUser(const QString& username, const QString& password);
    bool changePassword(const QString& username, const QString& oldPassword, const QString& newPassword);
    bool verifyEmailToken(const QString& username, const QString& token);
    
signals:
    void emailVerificationSent(const QString& email);
    void passwordChanged(const QString& username);

private:
    UserManager() = default;
    ~UserManager() = default;
    UserManager(const UserManager&) = delete;
    UserManager& operator=(const UserManager&) = delete;
    
    QString hashPassword(const QString& password);
    QString generateToken();
};

#endif // USERMANAGER_H
