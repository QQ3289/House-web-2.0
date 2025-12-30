#include "emailservice.h"
#include <QProcess>
#include <QDebug>

EmailService::EmailService(QObject *parent)
    : QObject(parent)
{
    // Configure SMTP settings (should be loaded from config file)
    smtpServer = "smtp.example.com";
    smtpPort = 587;
    smtpUsername = "noreply@example.com";
    smtpPassword = "your_password";
}

bool EmailService::sendVerificationEmail(const QString& toEmail, const QString& code)
{
    QString subject = "二手房平台 - 邮箱验证";
    QString body = QString(R"(
        <html>
        <body style="font-family: Arial, sans-serif;">
            <h2>欢迎注册二手房信息平台</h2>
            <p>您的验证码是：<strong style="font-size: 24px; color: #007bff;">%1</strong></p>
            <p>请在注册页面输入此验证码以完成注册。</p>
            <p>如果您没有注册账号，请忽略此邮件。</p>
            <br>
            <p>此邮件由系统自动发送，请勿回复。</p>
        </body>
        </html>
    )").arg(code);
    
    return sendEmail(toEmail, subject, body);
}

bool EmailService::sendPasswordResetEmail(const QString& toEmail, const QString& code)
{
    QString subject = "二手房平台 - 密码重置";
    QString body = QString(R"(
        <html>
        <body style="font-family: Arial, sans-serif;">
            <h2>密码重置请求</h2>
            <p>您的密码重置验证码是：<strong style="font-size: 24px; color: #007bff;">%1</strong></p>
            <p>请使用此验证码重置您的密码。</p>
            <p>如果您没有请求重置密码，请忽略此邮件。</p>
            <br>
            <p>此邮件由系统自动发送，请勿回复。</p>
        </body>
        </html>
    )").arg(code);
    
    return sendEmail(toEmail, subject, body);
}

bool EmailService::sendPasswordChangedEmail(const QString& toEmail)
{
    QString subject = "二手房平台 - 密码已修改";
    QString body = R"(
        <html>
        <body style="font-family: Arial, sans-serif;">
            <h2>密码修改通知</h2>
            <p>您的账号密码已成功修改。</p>
            <p>如果这不是您本人的操作，请立即联系客服。</p>
            <br>
            <p>此邮件由系统自动发送，请勿回复。</p>
        </body>
        </html>
    )";
    
    return sendEmail(toEmail, subject, body);
}

bool EmailService::sendEmail(const QString& toEmail, const QString& subject, const QString& body)
{
    // In production, you should use a proper email library like Qt's QSmtp or an external service
    // For this demo, we'll use a simple approach with sendmail or SMTP command
    
    qDebug() << "Sending email to:" << toEmail;
    qDebug() << "Subject:" << subject;
    qDebug() << "Body:" << body;
    
    // TODO: Implement actual email sending using SMTP
    // For now, just log the email content
    // In production environment, consider using:
    // 1. Third-party email service API (SendGrid, Mailgun, etc.)
    // 2. System sendmail command
    // 3. Qt SMTP library
    
    // Example using sendmail (Linux):
    /*
    QProcess process;
    QString command = QString("echo '%1' | mail -s '%2' %3")
                        .arg(body)
                        .arg(subject)
                        .arg(toEmail);
    process.start("sh", QStringList() << "-c" << command);
    process.waitForFinished();
    return process.exitCode() == 0;
    */
    
    // For development, always return true
    return true;
}
