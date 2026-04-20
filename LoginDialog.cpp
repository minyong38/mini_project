#include "LoginDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QMessageBox>

LoginDialog::LoginDialog(QWidget* parent) : QDialog(parent)
{
    setWindowTitle("캘린더 로그인");
    setFixedWidth(340);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    auto* root = new QVBoxLayout(this);
    root->setSpacing(0);
    root->setContentsMargins(32, 32, 32, 28);

    // ── 타이틀 ──────────────────────────────────────
    auto* titleLabel = new QLabel("📅 캘린더", this);
    titleLabel->setStyleSheet("font-size:22px; font-weight:700; color:#1C1C1E;");
    titleLabel->setAlignment(Qt::AlignHCenter);
    root->addWidget(titleLabel);
    root->addSpacing(6);

    auto* subLabel = new QLabel("계정으로 로그인하세요", this);
    subLabel->setStyleSheet("font-size:13px; color:#8E8E93;");
    subLabel->setAlignment(Qt::AlignHCenter);
    root->addWidget(subLabel);
    root->addSpacing(24);

    // ── 서버 IP ──────────────────────────────────────
    auto* ipLabel = new QLabel("서버 IP", this);
    ipLabel->setStyleSheet("font-size:12px; color:#6E6E73; font-weight:600;");
    root->addWidget(ipLabel);
    root->addSpacing(4);

    m_ipEdit = new QLineEdit("172.20.35.212", this);
    m_ipEdit->setStyleSheet(
        "QLineEdit { padding:8px 12px; border:1.5px solid #E5E5EA; border-radius:10px;"
        " font-size:14px; color:#1C1C1E; background:#FAFAFA; }"
        "QLineEdit:focus { border-color:#007AFF; background:#FFFFFF; }"
    );
    root->addWidget(m_ipEdit);
    root->addSpacing(20);

    // ── Google 로그인 버튼 ────────────────────────────
    m_googleBtn = new QPushButton(this);
    m_googleBtn->setText("  Google 계정으로 로그인");
    m_googleBtn->setFixedHeight(44);
    m_googleBtn->setStyleSheet(R"(
        QPushButton {
            background: #FFFFFF;
            border: 1.5px solid #DADCE0;
            border-radius: 10px;
            font-size: 14px;
            font-weight: 600;
            color: #3C4043;
            padding-left: 8px;
        }
        QPushButton:hover  { background: #F8F9FA; border-color: #BBBFC4; }
        QPushButton:pressed{ background: #F1F3F4; }
        QPushButton:disabled { color: #AAAAAA; border-color: #E5E5EA; }
    )");

    // Google "G" 로고 아이콘 (컬러 텍스트로 표현)
    auto* gLogo = new QLabel(m_googleBtn);
    gLogo->setText("<span style='font-size:18px;'>G</span>");
    gLogo->setStyleSheet(
        "background:transparent; color: #4285F4; font-weight:900; font-size:18px;"
    );
    auto* btnLayout = new QHBoxLayout(m_googleBtn);
    btnLayout->setContentsMargins(12, 0, 12, 0);
    btnLayout->addWidget(gLogo);
    btnLayout->addStretch();
    // (버튼 텍스트는 QPushButton::setText로 우측 공간에 표시됨)

    root->addWidget(m_googleBtn);
    root->addSpacing(8);

    // ── 상태 레이블 (로그인 후 사용자 정보 표시) ──────
    m_statusLabel = new QLabel(this);
    m_statusLabel->setStyleSheet("font-size:12px; color:#34C759; font-weight:600;");
    m_statusLabel->setAlignment(Qt::AlignHCenter);
    m_statusLabel->setWordWrap(true);
    m_statusLabel->hide();
    root->addWidget(m_statusLabel);

    // ── 구분선 ───────────────────────────────────────
    root->addSpacing(16);
    auto* sepLayout = new QHBoxLayout();
    auto makeLine = [this]() {
        auto* f = new QFrame(this);
        f->setFrameShape(QFrame::HLine);
        f->setStyleSheet("color:#E5E5EA;");
        return f;
    };
    auto* orLabel = new QLabel("또는", this);
    orLabel->setStyleSheet("font-size:12px; color:#C7C7CC; padding:0 8px;");
    orLabel->setAlignment(Qt::AlignHCenter);
    sepLayout->addWidget(makeLine());
    sepLayout->addWidget(orLabel);
    sepLayout->addWidget(makeLine());
    root->addLayout(sepLayout);
    root->addSpacing(12);

    // ── 수동 ID 입력 (fallback) ───────────────────────
    auto* idLabel = new QLabel("직접 ID 입력", this);
    idLabel->setStyleSheet("font-size:12px; color:#6E6E73; font-weight:600;");
    root->addWidget(idLabel);
    root->addSpacing(4);

    m_idEdit = new QLineEdit(this);
    m_idEdit->setPlaceholderText("사용할 ID를 입력하세요");
    m_idEdit->setStyleSheet(
        "QLineEdit { padding:8px 12px; border:1.5px solid #E5E5EA; border-radius:10px;"
        " font-size:14px; color:#1C1C1E; background:#FAFAFA; }"
        "QLineEdit:focus { border-color:#007AFF; background:#FFFFFF; }"
    );
    root->addWidget(m_idEdit);
    root->addSpacing(24);

    // ── OK / Cancel ───────────────────────────────────
    auto* btnRow = new QHBoxLayout();
    btnRow->setSpacing(10);

    auto* cancelBtn = new QPushButton("취소", this);
    cancelBtn->setFixedHeight(40);
    cancelBtn->setStyleSheet(
        "QPushButton { background:#F2F2F7; border:none; border-radius:10px;"
        " font-size:14px; color:#1C1C1E; font-weight:500; }"
        "QPushButton:hover { background:#E5E5EA; }"
    );

    m_okBtn = new QPushButton("확인", this);
    m_okBtn->setFixedHeight(40);
    m_okBtn->setDefault(true);
    m_okBtn->setStyleSheet(
        "QPushButton { background:#007AFF; border:none; border-radius:10px;"
        " font-size:14px; color:#FFFFFF; font-weight:600; }"
        "QPushButton:hover { background:#0071E3; }"
        "QPushButton:pressed { background:#0062CC; }"
    );

    btnRow->addWidget(cancelBtn);
    btnRow->addWidget(m_okBtn);
    root->addLayout(btnRow);

    // ── 시그널 연결 ───────────────────────────────────
    connect(m_googleBtn, &QPushButton::clicked, this, &LoginDialog::onGoogleLogin);
    connect(m_okBtn,     &QPushButton::clicked, this, &LoginDialog::accept);
    connect(cancelBtn,   &QPushButton::clicked, this, &LoginDialog::reject);
    connect(m_idEdit,    &QLineEdit::returnPressed, this, &LoginDialog::accept);
}

// ── Google 로그인 시작 ──────────────────────────────────

void LoginDialog::onGoogleLogin()
{
    if (!m_auth) {
        m_auth = new GoogleAuthManager(this);
        connect(m_auth, &GoogleAuthManager::loginSuccess, this, &LoginDialog::onLoginSuccess);
        connect(m_auth, &GoogleAuthManager::loginFailed,  this, &LoginDialog::onLoginFailed);
    }
    setGoogleButtonState(true);
    m_statusLabel->setStyleSheet("font-size:12px; color:#8E8E93;");
    m_statusLabel->setText("브라우저에서 로그인을 완료해주세요...");
    m_statusLabel->show();
    m_auth->startLogin();
}

void LoginDialog::onLoginSuccess(const GoogleUserInfo& user)
{
    m_userInfo       = user;
    m_googleLoggedIn = true;

    setGoogleButtonState(false);
    m_googleBtn->setText("  ✓  " + user.name);
    m_googleBtn->setStyleSheet(
        "QPushButton { background:#F0FFF4; border:1.5px solid #34C759; border-radius:10px;"
        " font-size:14px; font-weight:600; color:#1B7F3A; padding-left:8px; }"
    );

    m_statusLabel->setStyleSheet("font-size:12px; color:#34C759; font-weight:600;");
    m_statusLabel->setText("✓  " + user.email);
    m_statusLabel->show();

    m_idEdit->clear();
    m_idEdit->setPlaceholderText(user.name + " (Google 로그인됨)");
}

void LoginDialog::onLoginFailed(const QString& error)
{
    setGoogleButtonState(false);
    m_statusLabel->setStyleSheet("font-size:12px; color:#FF3B30; font-weight:500;");
    m_statusLabel->setText("⚠ " + error);
    m_statusLabel->show();
}

void LoginDialog::setGoogleButtonState(bool loading)
{
    m_googleBtn->setEnabled(!loading);
    if (loading)
        m_googleBtn->setText("  로그인 중...");
    else if (!m_googleLoggedIn)
        m_googleBtn->setText("  Google 계정으로 로그인");
}

// ── Getter ─────────────────────────────────────────────

QString LoginDialog::getIp() const
{
    return m_ipEdit->text().trimmed();
}

QString LoginDialog::getId() const
{
    if (m_googleLoggedIn && !m_userInfo.name.isEmpty())
        return m_userInfo.name;
    return m_idEdit->text().trimmed();
}

QString LoginDialog::getEmail() const
{
    return m_userInfo.email;
}
