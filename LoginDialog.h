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

    QString getIp()       const;
    QString getId()       const;
    QString getPassword() const;
    QString getEmail()    const;
    bool    isGoogleLogin() const;

private slots:
    void onGoogleLogin();
    void onLoginSuccess(const GoogleUserInfo& user);
    void onLoginFailed(const QString& error);
    void setGoogleButtonState(bool loading);
    void onSignupClicked();

private:
    QLineEdit*    m_ipEdit;
    QLineEdit*    m_idEdit;
    QLineEdit*    m_pwEdit;
    QPushButton*  m_googleBtn;
    QPushButton*  m_okBtn;
    QLabel*       m_statusLabel;

    GoogleAuthManager* m_auth = nullptr;
    GoogleUserInfo     m_userInfo;
    bool               m_googleLoggedIn = false;
};

#endif
