#include "RegisterDialog.h"
#include "Common.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QVideoWidget>
#include <QBuffer>
#include <QLabel>
#include <QPainter>
#include <QPainterPath>

static QString makeLineEditStyle() {
    return "QLineEdit { padding:0 12px; border:1.5px solid #E0E0E0; border-radius:8px;"
           " font-size:13px; color:#212121; background:#FAFAFA; }"
           "QLineEdit:focus { border-color:#1A237E; background:#FFFFFF; }";
}

static QPixmap circularPixmap(const QImage& img, int size) {
    QPixmap scaled = QPixmap::fromImage(img).scaled(size, size, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    QPixmap result(size, size);
    result.fill(Qt::transparent);
    QPainter p(&result);
    p.setRenderHint(QPainter::Antialiasing);
    QPainterPath path;
    path.addEllipse(0, 0, size, size);
    p.setClipPath(path);
    p.drawPixmap(0, 0, scaled);
    return result;
}

RegisterDialog::RegisterDialog(const QString& serverIp, QWidget* parent)
    : QDialog(parent), m_serverIp(serverIp)
{
    setWindowTitle("회원가입");
    setFixedWidth(400);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setStyleSheet("QDialog { background:#FFFFFF; }");

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    m_stack = new QStackedWidget(this);
    root->addWidget(m_stack);

    // ── Step 1: 카메라 페이지 ────────────────────────────────
    m_cameraPage = new QWidget();
    auto* camLayout = new QVBoxLayout(m_cameraPage);
    camLayout->setContentsMargins(28, 24, 28, 24);
    camLayout->setSpacing(12);

    auto* camTitle = new QLabel("프로필 사진 촬영", m_cameraPage);
    camTitle->setStyleSheet("font-size:17px; font-weight:700; color:#1A237E;");
    auto* camSub = new QLabel("카메라로 프로필 사진을 찍어주세요", m_cameraPage);
    camSub->setStyleSheet("font-size:12px; color:#9E9E9E;");
    camLayout->addWidget(camTitle);
    camLayout->addWidget(camSub);

    // 카메라 미리보기
    m_viewfinder = new QVideoWidget(m_cameraPage);
    m_viewfinder->setFixedSize(344, 258);
    m_viewfinder->setStyleSheet("border-radius:12px; background:#000;");
    camLayout->addWidget(m_viewfinder, 0, Qt::AlignHCenter);

    // 캡처 후 미리보기 (숨김)
    m_photoPreview = new QLabel(m_cameraPage);
    m_photoPreview->setFixedSize(200, 200);
    m_photoPreview->setAlignment(Qt::AlignCenter);
    m_photoPreview->hide();
    camLayout->addWidget(m_photoPreview, 0, Qt::AlignHCenter);

    // 버튼 행
    auto* camBtnRow = new QHBoxLayout();
    camBtnRow->setSpacing(10);

    m_captureBtn = new QPushButton("📷  사진 찍기", m_cameraPage);
    m_captureBtn->setFixedHeight(42);
    m_captureBtn->setStyleSheet(
        "QPushButton { background:#1A237E; border:none; border-radius:8px;"
        " font-size:13px; color:#FFFFFF; font-weight:700; }"
        "QPushButton:hover { background:#283593; }"
    );

    m_retakeBtn = new QPushButton("다시 찍기", m_cameraPage);
    m_retakeBtn->setFixedHeight(42);
    m_retakeBtn->hide();
    m_retakeBtn->setStyleSheet(
        "QPushButton { background:#F5F5F5; border:none; border-radius:8px;"
        " font-size:13px; color:#616161; font-weight:600; }"
        "QPushButton:hover { background:#EEEEEE; }"
    );

    m_nextBtn = new QPushButton("다음 →", m_cameraPage);
    m_nextBtn->setFixedHeight(42);
    m_nextBtn->setEnabled(false);
    m_nextBtn->setStyleSheet(
        "QPushButton { background:#43A047; border:none; border-radius:8px;"
        " font-size:13px; color:#FFFFFF; font-weight:700; }"
        "QPushButton:hover   { background:#388E3C; }"
        "QPushButton:disabled{ background:#BDBDBD; }"
    );

    camBtnRow->addWidget(m_retakeBtn, 1);
    camBtnRow->addWidget(m_captureBtn, 2);
    camBtnRow->addWidget(m_nextBtn, 1);
    camLayout->addLayout(camBtnRow);

    m_stack->addWidget(m_cameraPage);

    // ── Step 2: 가입 폼 페이지 ──────────────────────────────
    m_formPage = new QWidget();
    auto* formLayout = new QVBoxLayout(m_formPage);
    formLayout->setContentsMargins(36, 28, 36, 24);
    formLayout->setSpacing(0);

    auto* formTitle = new QLabel("회원가입", m_formPage);
    formTitle->setStyleSheet("font-size:17px; font-weight:700; color:#1A237E;");
    formLayout->addWidget(formTitle);
    formLayout->addSpacing(4);
    auto* formSub = new QLabel("사용할 ID와 비밀번호를 입력하세요", m_formPage);
    formSub->setStyleSheet("font-size:12px; color:#9E9E9E;");
    formLayout->addWidget(formSub);
    formLayout->addSpacing(18);

    // 프로필 사진 썸네일
    m_photoThumb = new QLabel(m_formPage);
    m_photoThumb->setFixedSize(64, 64);
    m_photoThumb->setAlignment(Qt::AlignCenter);
    formLayout->addWidget(m_photoThumb, 0, Qt::AlignHCenter);
    formLayout->addSpacing(14);

    auto makeField = [&](const QString& label, bool pw = false) -> QLineEdit* {
        auto* lbl = new QLabel(label, m_formPage);
        lbl->setStyleSheet("font-size:11px; font-weight:700; color:#757575; letter-spacing:0.8px;");
        formLayout->addWidget(lbl);
        formLayout->addSpacing(5);
        auto* edit = new QLineEdit(m_formPage);
        edit->setFixedHeight(40);
        if (pw) edit->setEchoMode(QLineEdit::Password);
        edit->setStyleSheet(makeLineEditStyle());
        formLayout->addWidget(edit);
        formLayout->addSpacing(12);
        return edit;
    };

    m_idEdit        = makeField("사용자 ID");
    m_idEdit->setPlaceholderText("영문/숫자로 입력하세요");
    m_pwEdit        = makeField("비밀번호", true);
    m_pwEdit->setPlaceholderText("6자 이상 입력하세요");
    m_pwConfirmEdit = makeField("비밀번호 확인", true);
    m_pwConfirmEdit->setPlaceholderText("비밀번호를 다시 입력하세요");

    m_statusLabel = new QLabel(m_formPage);
    m_statusLabel->setStyleSheet("font-size:11px; color:#E53935; font-weight:500;");
    m_statusLabel->setAlignment(Qt::AlignHCenter);
    m_statusLabel->setWordWrap(true);
    m_statusLabel->hide();
    formLayout->addWidget(m_statusLabel);
    formLayout->addSpacing(6);

    auto* formBtnRow = new QHBoxLayout();
    formBtnRow->setSpacing(10);

    auto* backBtn = new QPushButton("← 뒤로", m_formPage);
    backBtn->setFixedHeight(42);
    backBtn->setStyleSheet(
        "QPushButton { background:#F5F5F5; border:none; border-radius:8px;"
        " font-size:13px; color:#616161; font-weight:600; }"
        "QPushButton:hover { background:#EEEEEE; }"
    );

    m_registerBtn = new QPushButton("가입하기", m_formPage);
    m_registerBtn->setFixedHeight(42);
    m_registerBtn->setDefault(true);
    m_registerBtn->setStyleSheet(
        "QPushButton { background:#1A237E; border:none; border-radius:8px;"
        " font-size:13px; color:#FFFFFF; font-weight:700; }"
        "QPushButton:hover   { background:#283593; }"
        "QPushButton:disabled{ background:#BDBDBD; }"
    );

    formBtnRow->addWidget(backBtn, 1);
    formBtnRow->addWidget(m_registerBtn, 2);
    formLayout->addLayout(formBtnRow);

    m_stack->addWidget(m_formPage);

    // ── 소켓 ────────────────────────────────────────────────
    m_socket = new QTcpSocket(this);
    connect(m_socket, &QTcpSocket::connected,              this, &RegisterDialog::onConnected);
    connect(m_socket, &QTcpSocket::readyRead,              this, &RegisterDialog::onReadyRead);
    connect(m_socket, &QAbstractSocket::errorOccurred,     this, &RegisterDialog::onSocketError);

    // ── 시그널 연결 ──────────────────────────────────────────
    connect(m_captureBtn,  &QPushButton::clicked, this, &RegisterDialog::onCaptureClicked);
    connect(m_retakeBtn,   &QPushButton::clicked, this, &RegisterDialog::onRetakeClicked);
    connect(m_nextBtn,     &QPushButton::clicked, this, [this]() { showFormStep(); });
    connect(backBtn,       &QPushButton::clicked, this, [this]() { showCameraStep(); });
    connect(m_registerBtn, &QPushButton::clicked, this, &RegisterDialog::onRegisterClicked);
    connect(m_idEdit,      &QLineEdit::returnPressed, m_pwEdit,        QOverload<>::of(&QLineEdit::setFocus));
    connect(m_pwEdit,      &QLineEdit::returnPressed, m_pwConfirmEdit, QOverload<>::of(&QLineEdit::setFocus));
    connect(m_pwConfirmEdit, &QLineEdit::returnPressed, this, &RegisterDialog::onRegisterClicked);

    showCameraStep();
}

RegisterDialog::~RegisterDialog() {
    if (m_camera) m_camera->stop();
}

void RegisterDialog::showCameraStep()
{
    m_stack->setCurrentWidget(m_cameraPage);

    if (!m_camera) {
        m_camera  = new QCamera(this);
        m_session = new QMediaCaptureSession(this);
        m_capture = new QImageCapture(this);

        m_session->setCamera(m_camera);
        m_session->setVideoOutput(m_viewfinder);
        m_session->setImageCapture(m_capture);

        connect(m_capture, &QImageCapture::imageCaptured,
                this, &RegisterDialog::onImageCaptured);
    }

    m_capturedImage = QImage();
    m_viewfinder->show();
    m_photoPreview->hide();
    m_captureBtn->show();
    m_retakeBtn->hide();
    m_nextBtn->setEnabled(false);
    m_camera->start();
}

void RegisterDialog::showFormStep()
{
    m_camera->stop();
    m_stack->setCurrentWidget(m_formPage);

    // 프로필 썸네일 표시
    if (!m_capturedImage.isNull())
        m_photoThumb->setPixmap(circularPixmap(m_capturedImage, 64));
}

void RegisterDialog::onCaptureClicked()
{
    m_capture->capture();
}

void RegisterDialog::onImageCaptured(int /*id*/, const QImage& image)
{
    m_capturedImage = image;
    m_viewfinder->hide();
    m_photoPreview->setPixmap(circularPixmap(image, 200));
    m_photoPreview->show();
    m_captureBtn->hide();
    m_retakeBtn->show();
    m_nextBtn->setEnabled(true);
}

void RegisterDialog::onRetakeClicked()
{
    m_capturedImage = QImage();
    m_viewfinder->show();
    m_photoPreview->hide();
    m_captureBtn->show();
    m_retakeBtn->hide();
    m_nextBtn->setEnabled(false);
    m_camera->start();
}

void RegisterDialog::onRegisterClicked()
{
    QString id  = m_idEdit->text().trimmed();
    QString pw  = m_pwEdit->text();
    QString pw2 = m_pwConfirmEdit->text();

    if (id.isEmpty()) {
        m_statusLabel->setText("ID를 입력해주세요."); m_statusLabel->show(); return;
    }
    if (pw.length() < 6) {
        m_statusLabel->setText("비밀번호는 6자 이상이어야 합니다."); m_statusLabel->show(); return;
    }
    if (pw != pw2) {
        m_statusLabel->setText("비밀번호가 일치하지 않습니다."); m_statusLabel->show(); return;
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
    QString resp = QString::fromUtf8(m_socket->readLine()).trimmed();

    if (resp == Protocol::SIGNUP_OK && !m_signupDone) {
        m_signupDone = true;
        m_statusLabel->setText("사진 업로드 중...");

        // 이미지를 128x128로 리사이즈 후 Base64 변환
        QImage thumb = m_capturedImage.scaled(128, 128, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        QBuffer buf;
        buf.open(QIODevice::WriteOnly);
        thumb.save(&buf, "JPEG", 70);
        QString base64 = buf.data().toBase64();

        QString userId = m_idEdit->text().trimmed();
        m_socket->write((Protocol::PROFILE_UPLOAD + Protocol::SEP
                         + userId + Protocol::SEP + base64 + "\n").toUtf8());
        return;
    }

    if (resp == Protocol::PROFILE_OK) {
        m_socket->disconnectFromHost();
        QMessageBox::information(this, "회원가입 완료",
            "회원가입이 완료되었습니다!\n로그인 화면으로 돌아갑니다.");
        accept();
        return;
    }

    if (resp.startsWith(Protocol::SIGNUP_FAIL)) {
        QString reason = resp.section(':', 1);
        m_statusLabel->setStyleSheet("font-size:11px; color:#E53935; font-weight:500;");
        m_statusLabel->setText(reason.isEmpty() ? "회원가입 실패" : reason);
        m_statusLabel->show();
        m_registerBtn->setEnabled(true);
        m_socket->disconnectFromHost();
    }
}

void RegisterDialog::onSocketError()
{
    m_statusLabel->setStyleSheet("font-size:11px; color:#E53935; font-weight:500;");
    m_statusLabel->setText("서버 연결 실패. IP를 확인해주세요.");
    m_statusLabel->show();
    m_registerBtn->setEnabled(true);
    m_signupDone = false;
}
