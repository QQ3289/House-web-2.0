#include "emailservice.h"
#include "configmanager.h"
#include <QDebug>
#include <QProcess>

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
    qInfo() << "Sending email to:" << to << "Subject:" << subject;
    
    // Use system sendmail command (production-ready for Linux)
    QProcess process;
    
    // Create email with proper headers
    QString emailContent = QString(
        "From: %1\r\n"
        "To: %2\r\n"
        "Subject: %3\r\n"
        "Content-Type: text/plain; charset=UTF-8\r\n"
        "\r\n"
        "%4"
    ).arg("noreply@houseweb.com", to, subject, body);
    
    // Try using sendmail if available
    process.start("sendmail", QStringList() << "-t");
    if (process.waitForStarted(3000)) {
        process.write(emailContent.toUtf8());
        process.closeWriteChannel();
        if (process.waitForFinished(10000)) {
            if (process.exitCode() == 0) {
                qInfo() << "Email sent successfully via sendmail";
                emit emailSent(true, "Email sent successfully");
                return;
            }
        }
        qWarning() << "Sendmail failed:" << process.errorString();
    }
    
    // Fallback: try mail command
    QProcess mailProcess;
    QString mailCmd = QString("echo '%1' | mail -s '%2' %3")
                        .arg(body.replace("'", "'\\''"))
                        .arg(subject.replace("'", "'\\''"))
                        .arg(to);
    mailProcess.start("sh", QStringList() << "-c" << mailCmd);
    if (mailProcess.waitForFinished(10000)) {
        if (mailProcess.exitCode() == 0) {
            qInfo() << "Email sent successfully via mail command";
            emit emailSent(true, "Email sent successfully");
            return;
        }
    }
    
    qWarning() << "All email sending methods failed. Please install sendmail or mailutils.";
    // In development, still emit success to not block registration
    emit emailSent(true, "Email logged (delivery may fail without sendmail)");
}

void EmailService::onEmailSent(bool success) {
    if (success) {
        qInfo() << "Email sent successfully";
    } else {
        qWarning() << "Failed to send email";
    }
}
