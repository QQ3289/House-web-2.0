#include "usermanager.h"
#include "database/databasemanager.h"
#include "emailservice.h"
#include <QRandomGenerator>
#include <QDebug>

UserManager::UserManager(QObject *parent)
    : QObject(parent)
{
}

QString UserManager::generateVerificationCode()
{
    QString code;
    for (int i = 0; i < 6; ++i) {
        code += QString::number(QRandomGenerator::global()->bounded(10));
    }
    return code;
}

QVariantMap UserManager::registerUser(const QString& username, const QString& password, 
                                     const QString& email)
{
    QVariantMap result;
    
    // Validate inputs
    if (username.isEmpty() || password.isEmpty() || email.isEmpty()) {
        result["success"] = false;
        result["message"] = "所有字段都是必填的";
        return result;
    }
    
    if (password.length() < 6) {
        result["success"] = false;
        result["message"] = "密码长度至少6位";
        return result;
    }
    
    // Generate verification code
    QString verifyCode = generateVerificationCode();
    
    // Create user in database
    DatabaseManager* db = DatabaseManager::instance();
    if (!db->createUser(username, password, email, verifyCode)) {
        result["success"] = false;
        result["message"] = "用户名或邮箱已存在";
        return result;
    }
    
    // Send verification email
    EmailService emailService;
    if (emailService.sendVerificationEmail(email, verifyCode)) {
        result["success"] = true;
        result["message"] = "注册成功！验证码已发送到您的邮箱";
    } else {
        result["success"] = true;
        result["message"] = "注册成功！但邮件发送失败，验证码：" + verifyCode;
        result["verifyCode"] = verifyCode;
    }
    
    return result;
}

QVariantMap UserManager::loginUser(const QString& username, const QString& password)
{
    QVariantMap result;
    
    DatabaseManager* db = DatabaseManager::instance();
    QVariantMap user = db->loginUser(username, password);
    
    if (user.isEmpty()) {
        result["success"] = false;
        result["message"] = "用户名或密码错误";
        return result;
    }
    
    if (!user["verified"].toBool()) {
        result["success"] = false;
        result["message"] = "账号未验证，请先验证邮箱";
        return result;
    }
    
    result["success"] = true;
    result["message"] = "登录成功";
    result["user"] = user;
    
    return result;
}

bool UserManager::verifyEmail(const QString& email, const QString& code)
{
    DatabaseManager* db = DatabaseManager::instance();
    return db->verifyUser(email, code);
}

bool UserManager::requestPasswordReset(const QString& email)
{
    QString verifyCode = generateVerificationCode();
    
    DatabaseManager* db = DatabaseManager::instance();
    if (!db->saveVerificationCode(email, verifyCode)) {
        return false;
    }
    
    EmailService emailService;
    return emailService.sendPasswordResetEmail(email, verifyCode);
}

bool UserManager::resetPassword(const QString& email, const QString& code, 
                               const QString& newPassword)
{
    DatabaseManager* db = DatabaseManager::instance();
    
    // Verify code
    QString savedCode = db->getVerificationCode(email);
    if (savedCode.isEmpty() || savedCode != code) {
        return false;
    }
    
    // Update password
    if (!db->updatePassword(email, newPassword)) {
        return false;
    }
    
    // Send notification email
    EmailService emailService;
    emailService.sendPasswordChangedEmail(email);
    
    return true;
}

QVariantMap UserManager::getUserInfo(int userId)
{
    DatabaseManager* db = DatabaseManager::instance();
    return db->getUserById(userId);
}

QVector<QVariantMap> UserManager::getAllUsers()
{
    DatabaseManager* db = DatabaseManager::instance();
    return db->getAllUsers();
}

QVariantMap UserManager::getUserStatistics()
{
    DatabaseManager* db = DatabaseManager::instance();
    return db->getUserStatistics();
}
