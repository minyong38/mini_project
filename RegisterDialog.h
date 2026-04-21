#ifndef REGISTERDIALOG_H
#define REGISTERDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QTcpSocket>

class RegisterDialog : public QDialog {
    Q_OBJECT
public:
    explicit RegisterDialog(const QString& serverIp, QWidget* parent = nullptr);

private slots:
    void onRegisterClicked();
    void onConnected();
    void onReadyRead();
    void onSocketError();

private:
    QString      m_serverIp;
    QTcpSocket*  m_socket;

    QLineEdit*   m_idEdit;
    QLineEdit*   m_pwEdit;
    QLineEdit*   m_pwConfirmEdit;
    QPushButton* m_registerBtn;
    QLabel*      m_statusLabel;
};

#endif
