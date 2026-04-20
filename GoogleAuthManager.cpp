
#include "GoogleAuthManager.h"
#include "secrets.h"
#include <QDesktopServices>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrl>
#include <QUrlQuery>
#include <QUuid>

const QString GoogleAuthManager::CLIENT_ID     = GOOGLE_CLIENT_ID;
const QString GoogleAuthManager::CLIENT_SECRET = GOOGLE_CLIENT_SECRET;

GoogleAuthManager::GoogleAuthManager(QObject* parent)
    : QObject(parent)
    , m_server(new QTcpServer(this))
    , m_nam(new QNetworkAccessManager(this))
{}

void GoogleAuthManager::startLogin()
{
    // 포트 0 → OS가 빈 포트 자동 할당 (충돌 방지)
    if (!m_server->listen(QHostAddress::LocalHost, 0)) {
        emit loginFailed("로컬 서버를 시작할 수 없습니다: " + m_server->errorString());
        return;
    }

    int port = m_server->serverPort();
    m_redirectUri = QString("http://127.0.0.1:%1").arg(port);

    // CSRF 방지용 랜덤 state
    m_state = QUuid::createUuid().toString(QUuid::WithoutBraces).left(16);

    // 구글 인증 URL 생성
    QUrl authUrl("https://accounts.google.com/o/oauth2/v2/auth");
    QUrlQuery q;
    q.addQueryItem("client_id",     CLIENT_ID);
    q.addQueryItem("redirect_uri",  m_redirectUri);
    q.addQueryItem("response_type", "code");
    q.addQueryItem("scope",         "openid email profile");
    q.addQueryItem("state",         m_state);
    q.addQueryItem("access_type",   "online");
    authUrl.setQuery(q);

    QDesktopServices::openUrl(authUrl);

    // 브라우저 리다이렉트 수신 대기
    connect(m_server, &QTcpServer::newConnection, this, [this]() {
        QTcpSocket* socket = m_server->nextPendingConnection();
        if (!socket) return;
        m_server->close();

        connect(socket, &QTcpSocket::readyRead, this, [this, socket]() {
            QString req = QString::fromUtf8(socket->readAll());

            // GET /?code=XXX&state=YYY HTTP/1.1  첫 줄에서 파싱
            QString path;
            QStringList lines = req.split("\r\n");
            if (!lines.isEmpty()) {
                QStringList parts = lines[0].split(" ");
                if (parts.size() >= 2) path = parts[1];
            }

            QUrlQuery params(QUrl("http://x" + path).query());
            QString code  = params.queryItemValue("code");
            QString state = params.queryItemValue("state");

            if (state != m_state || code.isEmpty()) {
                sendBrowserResponse(socket, false);
                emit loginFailed("인증 응답이 올바르지 않습니다.");
                return;
            }

            sendBrowserResponse(socket, true);
            exchangeToken(code);
        });
    });
}

void GoogleAuthManager::sendBrowserResponse(QTcpSocket* socket, bool success)
{
    QString body = success
        ? "<html><body style='font-family:sans-serif;text-align:center;padding:60px'>"
          "<h2>✅ 로그인 성공!</h2><p>이 창을 닫고 앱으로 돌아가세요.</p></body></html>"
        : "<html><body style='font-family:sans-serif;text-align:center;padding:60px'>"
          "<h2>❌ 로그인 실패</h2><p>다시 시도해주세요.</p></body></html>";

    QString response = "HTTP/1.1 200 OK\r\n"
                       "Content-Type: text/html; charset=utf-8\r\n"
                       "Connection: close\r\n\r\n" + body;

    socket->write(response.toUtf8());
    socket->flush();
    socket->disconnectFromHost();
    socket->deleteLater();
}

void GoogleAuthManager::exchangeToken(const QString& code)
{
    QNetworkRequest req(QUrl("https://oauth2.googleapis.com/token"));
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QUrlQuery body;
    body.addQueryItem("code",          code);
    body.addQueryItem("client_id",     CLIENT_ID);
    body.addQueryItem("client_secret", CLIENT_SECRET);
    body.addQueryItem("redirect_uri",  m_redirectUri);
    body.addQueryItem("grant_type",    "authorization_code");

    auto* reply = m_nam->post(req, body.toString(QUrl::FullyEncoded).toUtf8());
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            emit loginFailed("토큰 교환 실패: " + reply->errorString());
            return;
        }
        QJsonObject json = QJsonDocument::fromJson(reply->readAll()).object();
        QString accessToken = json["access_token"].toString();
        if (accessToken.isEmpty()) {
            emit loginFailed("액세스 토큰을 받지 못했습니다.");
            return;
        }
        fetchUserInfo(accessToken);
    });
}

void GoogleAuthManager::fetchUserInfo(const QString& accessToken)
{
    QNetworkRequest req(QUrl("https://www.googleapis.com/oauth2/v3/userinfo"));
    req.setRawHeader("Authorization", ("Bearer " + accessToken).toUtf8());

    auto* reply = m_nam->get(req);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            emit loginFailed("사용자 정보 가져오기 실패: " + reply->errorString());
            return;
        }
        QJsonObject json = QJsonDocument::fromJson(reply->readAll()).object();

        GoogleUserInfo info;
        info.sub        = json["sub"].toString();
        info.name       = json["name"].toString();
        info.email      = json["email"].toString();
        info.pictureUrl = json["picture"].toString();

        if (info.name.isEmpty()) info.name = info.email;
        emit loginSuccess(info);
    });
}
