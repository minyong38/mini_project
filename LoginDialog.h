#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QLabel>

class LoginDialog : public QDialog {
    Q_OBJECT
public:
    explicit LoginDialog(QWidget *parent = nullptr) : QDialog(parent) {
        setWindowTitle("로그인");

        auto* layout = new QVBoxLayout(this);

        layout->addWidget(new QLabel("서버 IP:", this));
        m_ipEdit = new QLineEdit("172.20.35.212", this);
        layout->addWidget(m_ipEdit);

        layout->addWidget(new QLabel("사용자 ID:", this));
        m_idEdit = new QLineEdit(this);
        m_idEdit->setPlaceholderText("ID를 입력하세요");
        layout->addWidget(m_idEdit);

        auto* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
        layout->addWidget(buttonBox);

        connect(buttonBox, &QDialogButtonBox::accepted, this, &LoginDialog::accept);
        connect(buttonBox, &QDialogButtonBox::rejected, this, &LoginDialog::reject);
    }

    QString getIp() const { return m_ipEdit->text(); }
    QString getId() const { return m_idEdit->text(); }

private:
    QLineEdit* m_ipEdit;
    QLineEdit* m_idEdit;
};

#endif
