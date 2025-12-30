#include "emailservice.h"
#include "configmanager.h"
#include <QDebug>

EmailService& EmailService::instance() {
    static EmailService instance;
    return instance;
}

void EmailService::sendVerificationEmail(const QString& toEmail, const QString& username, const QString& token) {
    QString subject = "Email Verification - House Web Service";
    QString body = QString(
        "Dear %1,\n\n"
        "Thank you for registering with House Web Service.\n\n"
        "Please verify your email by using this token: %2\n\n"
        "Or visit: http://localhost:8080/verify?username=%1&token=%2\n\n"
        "Best regards,\n"
        "House Web Team"
    ).arg(username, token);
    
    sendEmail(toEmail, subject, body);
}

void EmailService::sendPasswordChangeNotification(const QString& toEmail, const QString& username) {
    QString subject = "Password Changed - House Web Service";
    QString body = QString(
        "Dear %1,\n\n"
        "Your password has been successfully changed.\n\n"
        "If you did not make this change, please contact us immediately.\n\n"
        "Best regards,\n"
        "House Web Team"
    ).arg(username);
    
    sendEmail(toEmail, subject, body);
}

void EmailService::sendEmail(const QString& to, const QString& subject, const QString& body) {
    // Note: This is a placeholder implementation
    // In production, use a proper SMTP client library like VMime or SmtpClient-for-Qt
    
    qInfo() << "Sending email to:" << to;
    qInfo() << "Subject:" << subject;
    qInfo() << "Body:" << body;
    
    // Simulate successful email sending
    // In production, implement actual SMTP sending logic
    emit emailSent(true, "Email sent successfully");
}

void EmailService::onEmailSent(bool success) {
    if (success) {
        qInfo() << "Email sent successfully";
    } else {
        qWarning() << "Failed to send email";
    }
}
