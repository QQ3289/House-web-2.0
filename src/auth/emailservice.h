#ifndef EMAILSERVICE_H
#define EMAILSERVICE_H

#include <QObject>
#include <QString>

class EmailService : public QObject
{
    Q_OBJECT

public:
    explicit EmailService(QObject *parent = nullptr);
    
    bool sendVerificationEmail(const QString& toEmail, const QString& code);
    bool sendPasswordResetEmail(const QString& toEmail, const QString& code);
    bool sendPasswordChangedEmail(const QString& toEmail);
    
private:
    bool sendEmail(const QString& toEmail, const QString& subject, const QString& body);
    
    QString smtpServer;
    int smtpPort;
    QString smtpUsername;
    QString smtpPassword;
};

#endif // EMAILSERVICE_H
