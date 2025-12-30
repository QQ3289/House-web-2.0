#ifndef EMAILSERVICE_H
#define EMAILSERVICE_H

#include <QObject>
#include <QString>

class EmailService : public QObject {
    Q_OBJECT
    
public:
    static EmailService& instance();
    
    void sendVerificationEmail(const QString& toEmail, const QString& username, const QString& token);
    void sendPasswordChangeNotification(const QString& toEmail, const QString& username);

signals:
    void emailSent(bool success, const QString& message);

private slots:
    void onEmailSent(bool success);

private:
    EmailService() = default;
    ~EmailService() = default;
    EmailService(const EmailService&) = delete;
    EmailService& operator=(const EmailService&) = delete;
    
    void sendEmail(const QString& to, const QString& subject, const QString& body);
};

#endif // EMAILSERVICE_H
