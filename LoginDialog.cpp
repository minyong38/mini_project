#include "LoginDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QMessageBox>
#include <QPixmap>
#include <QGraphicsDropShadowEffect>

LoginDialog::LoginDialog(QWidget* parent) : QDialog(parent)
{
    setWindowTitle("로그인");
    setFixedWidth(400);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setStyleSheet("QDialog { background-color: #F5F6FA; }");

    auto* root = new QVBoxLayout(this);
    root->setSpacing(0);
    root->setContentsMargins(0, 0, 0, 0);

    // ── 상단 헤더 패널 (짙은 배경) ──────────────────────
    auto* header = new QFrame(this);
    header->setStyleSheet("QFrame { background: qlineargradient(x1:0,y1:0,x2:1,y2:1,"
                          "stop:0 #1A237E, stop:1 #283593); border:none; }");
    header->setFixedHeight(200);
    auto* headerLayout = new QVBoxLayout(header);
    headerLayout->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    headerLayout->setSpacing(10);

    auto* logoLabel = new QLabel(header);
    QPixmap logoPixmap(":/logo.png");
    if (!logoPixmap.isNull())
        logoLabel->setPixmap(logoPixmap.scaled(120, 120, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    logoLabel->setAlignment(Qt::AlignHCenter);
    headerLayout->addWidget(logoLabel);

    auto* titleLabel = new QLabel("공유 캘린더", header);
    titleLabel->setStyleSheet("font-size:20px; font-weight:700; color:#FFFFFF; background:transparent;");
    titleLabel->setAlignment(Qt::AlignHCenter);
    headerLayout->addWidget(titleLabel);

    root->addWidget(header);

    // ── 본문 카드 ────────────────────────────────────────
    auto* card = new QFrame(this);
    card->setStyleSheet(
        "QFrame { background:#FFFFFF; border:none;"
        " border-radius:0px; }"
    );
    auto* cardLayout = new QVBoxLayout(card);
    cardLayout->setSpacing(0);
    cardLayout->setContentsMargins(36, 32, 36, 32);

    // 서버 IP
    auto* ipLabel = new QLabel("서버 IP 주소", card);
    ipLabel->setStyleSheet("font-size:11px; font-weight:700; color:#5C6BC0; letter-spacing:1px; background:transparent;");
    cardLayout->addWidget(ipLabel);
    cardLayout->addSpacing(6);

    m_ipEdit = new QLineEdit("172.20.35.212", card);
    m_ipEdit->setFixedHeight(44);
    m_ipEdit->setStyleSheet(
        "QLineEdit { padding:0 14px; border:2px solid #E8EAF6; border-radius:8px;"
        " font-size:14px; color:#1A237E; background:#F8F9FF; }"
        "QLineEdit:focus { border-color:#3F51B5; background:#FFFFFF; }"
    );
    cardLayout->addWidget(m_ipEdit);
    cardLayout->addSpacing(20);

    // 구분선
    auto* sep1 = new QFrame(card);
    sep1->setFrameShape(QFrame::HLine);
    sep1->setStyleSheet("color:#E8EAF6;");
    cardLayout->addWidget(sep1);
    cardLayout->addSpacing(20);

    // Google 로그인 버튼
    m_googleBtn = new QPushButton(card);
    m_googleBtn->setText("  Google 계정으로 로그인");
    m_googleBtn->setFixedHeight(48);
    m_googleBtn->setStyleSheet(R"(
        QPushButton {
            background: #FFFFFF;
            border: 2px solid #DADCE0;
            border-radius: 8px;
            font-size: 14px;
            font-weight: 600;
            color: #3C4043;
            padding-left: 8px;
        }
        QPushButton:hover   { background: #F8F9FA; border-color: #3F51B5; }
        QPushButton:pressed { background: #E8EAF6; }
        QPushButton:disabled{ color: #AAAAAA; border-color: #E5E5EA; }
    )");
    auto* gLogo = new QLabel(m_googleBtn);
    gLogo->setText("<b style='font-size:18px; color:#4285F4;'>G</b>");
    gLogo->setStyleSheet("background:transparent;");
    auto* gBtnLayout = new QHBoxLayout(m_googleBtn);
    gBtnLayout->setContentsMargins(14, 0, 14, 0);
    gBtnLayout->addWidget(gLogo);
    gBtnLayout->addStretch();
    cardLayout->addWidget(m_googleBtn);
    cardLayout->addSpacing(8);

    // 상태 레이블
    m_statusLabel = new QLabel(card);
    m_statusLabel->setStyleSheet("font-size:12px; color:#34C759; font-weight:600; background:transparent;");
    m_statusLabel->setAlignment(Qt::AlignHCenter);
    m_statusLabel->setWordWrap(true);
    m_statusLabel->hide();
    cardLayout->addWidget(m_statusLabel);
    cardLayout->addSpacing(16);

    // 또는 구분선
    auto* sepLayout = new QHBoxLayout();
    auto makeLine = [card]() {
        auto* f = new QFrame(card);
        f->setFrameShape(QFrame::HLine);
        f->setStyleSheet("color:#E8EAF6;");
        return f;
    };
    auto* orLabel = new QLabel("또는 직접 입력", card);
    orLabel->setStyleSheet("font-size:11px; color:#9E9E9E; padding:0 10px; background:transparent;");
    orLabel->setAlignment(Qt::AlignHCenter);
    sepLayout->addWidget(makeLine());
    sepLayout->addWidget(orLabel);
    sepLayout->addWidget(makeLine());
    cardLayout->addLayout(sepLayout);
    cardLayout->addSpacing(16);

    // ID 입력
    auto* idLabel = new QLabel("사용자 ID", card);
    idLabel->setStyleSheet("font-size:11px; font-weight:700; color:#5C6BC0; letter-spacing:1px; background:transparent;");
    cardLayout->addWidget(idLabel);
    cardLayout->addSpacing(6);

    m_idEdit = new QLineEdit(card);
    m_idEdit->setPlaceholderText("사용할 ID를 입력하세요");
    m_idEdit->setFixedHeight(44);
    m_idEdit->setStyleSheet(
        "QLineEdit { padding:0 14px; border:2px solid #E8EAF6; border-radius:8px;"
        " font-size:14px; color:#1A237E; background:#F8F9FF; }"
        "QLineEdit:focus { border-color:#3F51B5; background:#FFFFFF; }"
    );
    cardLayout->addWidget(m_idEdit);
    cardLayout->addSpacing(28);

    // 버튼 행
    auto* btnRow = new QHBoxLayout();
    btnRow->setSpacing(12);

    auto* cancelBtn = new QPushButton("취소", card);
    cancelBtn->setFixedHeight(46);
    cancelBtn->setStyleSheet(
        "QPushButton { background:#ECEFF1; border:none; border-radius:8px;"
        " font-size:14px; color:#546E7A; font-weight:600; }"
        "QPushButton:hover { background:#CFD8DC; }"
    );

    m_okBtn = new QPushButton("로그인", card);
    m_okBtn->setFixedHeight(46);
    m_okBtn->setDefault(true);
    m_okBtn->setStyleSheet(
        "QPushButton { background: qlineargradient(x1:0,y1:0,x2:1,y2:0,"
        "stop:0 #3F51B5, stop:1 #5C6BC0);"
        " border:none; border-radius:8px;"
        " font-size:14px; color:#FFFFFF; font-weight:700; }"
        "QPushButton:hover   { background: qlineargradient(x1:0,y1:0,x2:1,y2:0,"
        "stop:0 #303F9F, stop:1 #3F51B5); }"
        "QPushButton:pressed { background:#283593; }"
    );

    btnRow->addWidget(cancelBtn, 1);
    btnRow->addWidget(m_okBtn,   2);
    cardLayout->addLayout(btnRow);

    root->addWidget(card);

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
        "QPushButton { background:#E8F5E9; border:2px solid #43A047; border-radius:8px;"
        " font-size:14px; font-weight:600; color:#2E7D32; padding-left:8px; }"
    );

    m_statusLabel->setStyleSheet("font-size:12px; color:#2E7D32; font-weight:600;");
    m_statusLabel->setText("✓  " + user.email);
    m_statusLabel->show();

    m_idEdit->clear();
    m_idEdit->setPlaceholderText(user.name + " (Google 로그인됨)");
}

void LoginDialog::onLoginFailed(const QString& error)
{
    setGoogleButtonState(false);
    m_statusLabel->setStyleSheet("font-size:12px; color:#D32F2F; font-weight:500;");
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
