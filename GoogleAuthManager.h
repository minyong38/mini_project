#ifndef GOOGLEAUTHMANAGER_H
#define GOOGLEAUTHMANAGER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QNetworkAccessManager>

struct GoogleUserInfo {
    QString sub;        // Google 고유 ID
    QString name;       // 표시 이름
    QString email;
    QString pictureUrl;
};

class GoogleAuthManager : public QObject {
    Q_OBJECT
public:
    explicit GoogleAuthManager(QObject* parent = nullptr);
    void startLogin();

signals:
    void loginSuccess(const GoogleUserInfo& user);
    void loginFailed(const QString& error);

private:
    void exchangeToken(const QString& code);
    void fetchUserInfo(const QString& accessToken);
    void sendBrowserResponse(QTcpSocket* socket, bool success);

    QTcpServer*            m_server;
    QNetworkAccessManager* m_nam;
    QString                m_state;
    QString                m_redirectUri;

    static const QString CLIENT_ID;
    static const QString CLIENT_SECRET;
};

#endif
