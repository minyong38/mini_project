#include "LoginDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QMessageBox>
#include <QPixmap>

LoginDialog::LoginDialog(QWidget* parent) : QDialog(parent)
{
    setWindowTitle("로그인");
    setFixedWidth(380);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setStyleSheet("QDialog { background:#FFFFFF; }");

    auto* root = new QVBoxLayout(this);
    root->setSpacing(0);
    root->setContentsMargins(40, 36, 40, 32);

    // ── 로고 + 타이틀 ────────────────────────────────────
    auto* topRow = new QHBoxLayout();
    topRow->setSpacing(14);
    topRow->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    auto* logoLabel = new QLabel(this);
    QPixmap logoPixmap(":/logo.png");
    if (!logoPixmap.isNull())
        logoLabel->setPixmap(logoPixmap.scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    topRow->addWidget(logoLabel);

    auto* titleCol = new QVBoxLayout();
    titleCol->setSpacing(2);
    auto* titleLabel = new QLabel("공유 캘린더", this);
    titleLabel->setStyleSheet("font-size:18px; font-weight:700; color:#1A237E;");
    auto* subLabel = new QLabel("계정으로 로그인하세요", this);
    subLabel->setStyleSheet("font-size:12px; color:#9E9E9E;");
    titleCol->addWidget(titleLabel);
    titleCol->addWidget(subLabel);
    topRow->addLayout(titleCol);
    root->addLayout(topRow);
    root->addSpacing(24);

    // ── 구분선 ────────────────────────────────────────────
    auto* div0 = new QFrame(this);
    div0->setFrameShape(QFrame::HLine);
    div0->setStyleSheet("color:#EEEEEE;");
    root->addWidget(div0);
    root->addSpacing(20);

    // ── 서버 IP ───────────────────────────────────────────
    auto* ipLabel = new QLabel("서버 IP", this);
    ipLabel->setStyleSheet("font-size:11px; font-weight:700; color:#757575; letter-spacing:0.8px;");
    root->addWidget(ipLabel);
    root->addSpacing(5);

    m_ipEdit = new QLineEdit("172.20.35.212", this);
    m_ipEdit->setFixedHeight(40);
    m_ipEdit->setStyleSheet(
        "QLineEdit { padding:0 12px; border:1.5px solid #E0E0E0; border-radius:8px;"
        " font-size:13px; color:#212121; background:#FAFAFA; }"
        "QLineEdit:focus { border-color:#1A237E; background:#FFFFFF; }"
    );
    root->addWidget(m_ipEdit);
    root->addSpacing(18);

    // ── Google 로그인 ─────────────────────────────────────
    m_googleBtn = new QPushButton(this);
    m_googleBtn->setText("  Google 계정으로 로그인");
    m_googleBtn->setFixedHeight(42);
    m_googleBtn->setStyleSheet(R"(
        QPushButton {
            background:#FFFFFF; border:1.5px solid #DADCE0; border-radius:8px;
            font-size:13px; font-weight:600; color:#3C4043; padding-left:8px;
        }
        QPushButton:hover   { background:#F8F9FA; border-color:#BBBFC4; }
        QPushButton:pressed { background:#F1F3F4; }
        QPushButton:disabled{ color:#AAAAAA; border-color:#EEEEEE; }
    )");
    auto* gLogo = new QLabel(m_googleBtn);
    gLogo->setText("<b style='font-size:16px; color:#4285F4;'>G</b>");
    gLogo->setStyleSheet("background:transparent;");
    auto* gLayout = new QHBoxLayout(m_googleBtn);
    gLayout->setContentsMargins(12, 0, 12, 0);
    gLayout->addWidget(gLogo);
    gLayout->addStretch();
    root->addWidget(m_googleBtn);
    root->addSpacing(6);

    // 상태 레이블
    m_statusLabel = new QLabel(this);
    m_statusLabel->setStyleSheet("font-size:11px; color:#43A047; font-weight:600;");
    m_statusLabel->setAlignment(Qt::AlignHCenter);
    m_statusLabel->setWordWrap(true);
    m_statusLabel->hide();
    root->addWidget(m_statusLabel);
    root->addSpacing(14);

    // ── 또는 ─────────────────────────────────────────────
    auto* sepLayout = new QHBoxLayout();
    auto makeLine = [this]() {
        auto* f = new QFrame(this);
        f->setFrameShape(QFrame::HLine);
        f->setStyleSheet("color:#EEEEEE;");
        return f;
    };
    auto* orLabel = new QLabel("또는", this);
    orLabel->setStyleSheet("font-size:11px; color:#BDBDBD; padding:0 8px;");
    sepLayout->addWidget(makeLine());
    sepLayout->addWidget(orLabel);
    sepLayout->addWidget(makeLine());
    root->addLayout(sepLayout);
    root->addSpacing(14);

    // ── ID 직접 입력 ──────────────────────────────────────
    auto* idLabel = new QLabel("사용자 ID", this);
    idLabel->setStyleSheet("font-size:11px; font-weight:700; color:#757575; letter-spacing:0.8px;");
    root->addWidget(idLabel);
    root->addSpacing(5);

    m_idEdit = new QLineEdit(this);
    m_idEdit->setPlaceholderText("사용할 ID를 입력하세요");
    m_idEdit->setFixedHeight(40);
    m_idEdit->setStyleSheet(
        "QLineEdit { padding:0 12px; border:1.5px solid #E0E0E0; border-radius:8px;"
        " font-size:13px; color:#212121; background:#FAFAFA; }"
        "QLineEdit:focus { border-color:#1A237E; background:#FFFFFF; }"
    );
    root->addWidget(m_idEdit);
    root->addSpacing(22);

    // ── 버튼 행 ───────────────────────────────────────────
    auto* btnRow = new QHBoxLayout();
    btnRow->setSpacing(10);

    auto* cancelBtn = new QPushButton("취소", this);
    cancelBtn->setFixedHeight(42);
    cancelBtn->setStyleSheet(
        "QPushButton { background:#F5F5F5; border:none; border-radius:8px;"
        " font-size:13px; color:#616161; font-weight:600; }"
        "QPushButton:hover { background:#EEEEEE; }"
    );

    m_okBtn = new QPushButton("로그인", this);
    m_okBtn->setFixedHeight(42);
    m_okBtn->setDefault(true);
    m_okBtn->setStyleSheet(
        "QPushButton { background:#1A237E; border:none; border-radius:8px;"
        " font-size:13px; color:#FFFFFF; font-weight:700; }"
        "QPushButton:hover   { background:#283593; }"
        "QPushButton:pressed { background:#0D47A1; }"
    );

    btnRow->addWidget(cancelBtn, 1);
    btnRow->addWidget(m_okBtn,   2);
    root->addLayout(btnRow);

    connect(m_googleBtn, &QPushButton::clicked, this, &LoginDialog::onGoogleLogin);
    connect(m_okBtn,     &QPushButton::clicked, this, &LoginDialog::accept);
    connect(cancelBtn,   &QPushButton::clicked, this, &LoginDialog::reject);
    connect(m_idEdit,    &QLineEdit::returnPressed, this, &LoginDialog::accept);
}

void LoginDialog::onGoogleLogin()
{
    if (!m_auth) {
        m_auth = new GoogleAuthManager(this);
        connect(m_auth, &GoogleAuthManager::loginSuccess, this, &LoginDialog::onLoginSuccess);
        connect(m_auth, &GoogleAuthManager::loginFailed,  this, &LoginDialog::onLoginFailed);
    }
    setGoogleButtonState(true);
    m_statusLabel->setStyleSheet("font-size:11px; color:#9E9E9E;");
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
        "QPushButton { background:#E8F5E9; border:1.5px solid #43A047; border-radius:8px;"
        " font-size:13px; font-weight:600; color:#2E7D32; padding-left:8px; }"
    );
    m_statusLabel->setStyleSheet("font-size:11px; color:#43A047; font-weight:600;");
    m_statusLabel->setText("✓  " + user.email);
    m_statusLabel->show();

    m_idEdit->clear();
    m_idEdit->setPlaceholderText(user.name + " (Google 로그인됨)");
}

void LoginDialog::onLoginFailed(const QString& error)
{
    setGoogleButtonState(false);
    m_statusLabel->setStyleSheet("font-size:11px; color:#E53935; font-weight:500;");
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

QString LoginDialog::getIp()    const { return m_ipEdit->text().trimmed(); }
QString LoginDialog::getEmail() const { return m_userInfo.email; }

QString LoginDialog::getId() const
{
    if (m_googleLoggedIn && !m_userInfo.name.isEmpty())
        return m_userInfo.name;
    return m_idEdit->text().trimmed();
}
