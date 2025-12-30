#ifndef USERMANAGER_H
#define USERMANAGER_H

#include <QObject>
#include <QString>
#include <QVariantMap>

class UserManager : public QObject
{
    Q_OBJECT

public:
    explicit UserManager(QObject *parent = nullptr);
    
    // User authentication
    QVariantMap registerUser(const QString& username, const QString& password, 
                            const QString& email);
    QVariantMap loginUser(const QString& username, const QString& password);
    bool verifyEmail(const QString& email, const QString& code);
    bool requestPasswordReset(const QString& email);
    bool resetPassword(const QString& email, const QString& code, 
                      const QString& newPassword);
    
    // User management (admin)
    QVariantMap getUserInfo(int userId);
    QVector<QVariantMap> getAllUsers();
    QVariantMap getUserStatistics();
    
private:
    QString generateVerificationCode();
};

#endif // USERMANAGER_H
