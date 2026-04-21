#include "RegisterDialog.h"
#include "Common.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>

RegisterDialog::RegisterDialog(const QString& serverIp, QWidget* parent)
    : QDialog(parent), m_serverIp(serverIp)
{
    setWindowTitle("회원가입");
    setFixedWidth(360);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setStyleSheet("QDialog { background:#FFFFFF; }");

    auto* root = new QVBoxLayout(this);
    root->setSpacing(0);
    root->setContentsMargins(36, 32, 36, 28);

    auto* titleLabel = new QLabel("회원가입", this);
    titleLabel->setStyleSheet("font-size:18px; font-weight:700; color:#1A237E;");
    root->addWidget(titleLabel);
    root->addSpacing(6);

    auto* subLabel = new QLabel("사용할 ID와 비밀번호를 입력하세요", this);
    subLabel->setStyleSheet("font-size:12px; color:#9E9E9E;");
    root->addWidget(subLabel);
    root->addSpacing(22);

    auto makeField = [&](const QString& label, bool pw = false) -> QLineEdit* {
        auto* lbl = new QLabel(label, this);
        lbl->setStyleSheet("font-size:11px; font-weight:700; color:#757575; letter-spacing:0.8px;");
        root->addWidget(lbl);
        root->addSpacing(5);
        auto* edit = new QLineEdit(this);
        edit->setFixedHeight(40);
        if (pw) edit->setEchoMode(QLineEdit::Password);
        edit->setStyleSheet(
            "QLineEdit { padding:0 12px; border:1.5px solid #E0E0E0; border-radius:8px;"
            " font-size:13px; color:#212121; background:#FAFAFA; }"
            "QLineEdit:focus { border-color:#1A237E; background:#FFFFFF; }"
        );
        root->addWidget(edit);
        root->addSpacing(14);
        return edit;
    };

    m_idEdit        = makeField("사용자 ID");
    m_idEdit->setPlaceholderText("영문/숫자로 입력하세요");
    m_pwEdit        = makeField("비밀번호", true);
    m_pwEdit->setPlaceholderText("6자 이상 입력하세요");
    m_pwConfirmEdit = makeField("비밀번호 확인", true);
    m_pwConfirmEdit->setPlaceholderText("비밀번호를 다시 입력하세요");

    m_statusLabel = new QLabel(this);
    m_statusLabel->setStyleSheet("font-size:11px; color:#E53935; font-weight:500;");
    m_statusLabel->setAlignment(Qt::AlignHCenter);
    m_statusLabel->setWordWrap(true);
    m_statusLabel->hide();
    root->addWidget(m_statusLabel);
    root->addSpacing(8);

    auto* btnRow = new QHBoxLayout();
    btnRow->setSpacing(10);

    auto* cancelBtn = new QPushButton("취소", this);
    cancelBtn->setFixedHeight(42);
    cancelBtn->setStyleSheet(
        "QPushButton { background:#F5F5F5; border:none; border-radius:8px;"
        " font-size:13px; color:#616161; font-weight:600; }"
        "QPushButton:hover { background:#EEEEEE; }"
    );

    m_registerBtn = new QPushButton("가입하기", this);
    m_registerBtn->setFixedHeight(42);
    m_registerBtn->setDefault(true);
    m_registerBtn->setStyleSheet(
        "QPushButton { background:#1A237E; border:none; border-radius:8px;"
        " font-size:13px; color:#FFFFFF; font-weight:700; }"
        "QPushButton:hover   { background:#283593; }"
        "QPushButton:pressed { background:#0D47A1; }"
        "QPushButton:disabled{ background:#BDBDBD; }"
    );

    btnRow->addWidget(cancelBtn, 1);
    btnRow->addWidget(m_registerBtn, 2);
    root->addLayout(btnRow);

    m_socket = new QTcpSocket(this);
    connect(m_socket, &QTcpSocket::connected,    this, &RegisterDialog::onConnected);
    connect(m_socket, &QTcpSocket::readyRead,    this, &RegisterDialog::onReadyRead);
    connect(m_socket, &QAbstractSocket::errorOccurred, this, &RegisterDialog::onSocketError);

    connect(m_registerBtn, &QPushButton::clicked, this, &RegisterDialog::onRegisterClicked);
    connect(cancelBtn,     &QPushButton::clicked, this, &RegisterDialog::reject);
}

void RegisterDialog::onRegisterClicked()
{
    QString id  = m_idEdit->text().trimmed();
    QString pw  = m_pwEdit->text();
    QString pw2 = m_pwConfirmEdit->text();

    if (id.isEmpty()) {
        m_statusLabel->setText("ID를 입력해주세요.");
        m_statusLabel->show(); return;
    }
    if (pw.length() < 6) {
        m_statusLabel->setText("비밀번호는 6자 이상이어야 합니다.");
        m_statusLabel->show(); return;
    }
    if (pw != pw2) {
        m_statusLabel->setText("비밀번호가 일치하지 않습니다.");
        m_statusLabel->show(); return;
    }

    m_registerBtn->setEnabled(false);
    m_statusLabel->setStyleSheet("font-size:11px; color:#9E9E9E;");
    m_statusLabel->setText("서버에 연결 중...");
    m_statusLabel->show();

    m_socket->connectToHost(m_serverIp, Protocol::PORT);
}

void RegisterDialog::onConnected()
{
    QString id = m_idEdit->text().trimmed();
    QString pw = m_pwEdit->text();
    m_socket->write((Protocol::SIGNUP + Protocol::SEP + id + Protocol::SEP + pw + "\n").toUtf8());
}

void RegisterDialog::onReadyRead()
{
    QString resp = QString::fromUtf8(m_socket->readAll()).trimmed();
    m_socket->disconnectFromHost();

    if (resp == Protocol::SIGNUP_OK) {
        QMessageBox::information(this, "회원가입 완료",
            "회원가입이 완료되었습니다!\n로그인 화면으로 돌아갑니다.");
        accept();
    } else {
        QString reason = resp.section(':', 1);
        m_statusLabel->setStyleSheet("font-size:11px; color:#E53935; font-weight:500;");
        m_statusLabel->setText(reason.isEmpty() ? "회원가입 실패" : reason);
        m_statusLabel->show();
        m_registerBtn->setEnabled(true);
    }
}

void RegisterDialog::onSocketError()
{
    m_statusLabel->setStyleSheet("font-size:11px; color:#E53935; font-weight:500;");
    m_statusLabel->setText("서버 연결 실패. IP를 확인해주세요.");
    m_statusLabel->show();
    m_registerBtn->setEnabled(true);
}
