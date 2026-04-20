#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include "GoogleAuthManager.h"

class LoginDialog : public QDialog {
    Q_OBJECT
public:
    explicit LoginDialog(QWidget* parent = nullptr);

    QString getIp()   const;
    QString getId()   const;  // Google 로그인 시 표시 이름, 수동 입력 시 ID
    QString getEmail()const;

private:
    void onGoogleLogin();
    void onLoginSuccess(const GoogleUserInfo& user);
    void onLoginFailed(const QString& error);
    void setGoogleButtonState(bool loading);

    QLineEdit*    m_ipEdit;
    QLineEdit*    m_idEdit;       // 수동 ID (fallback)
    QPushButton*  m_googleBtn;
    QPushButton*  m_okBtn;
    QLabel*       m_statusLabel;

    GoogleAuthManager* m_auth = nullptr;
    GoogleUserInfo     m_userInfo;
    bool               m_googleLoggedIn = false;
};

#endif
