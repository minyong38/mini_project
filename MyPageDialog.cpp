#include "MyPageDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QPainter>
#include <QImageReader>
#include <QCameraDevice>
#include <QMediaDevices>

MyPageDialog::MyPageDialog(const QString& userId,
                           const QPixmap& currentPhoto,
                           const QString& nickname,
                           QWidget* parent)
    : QDialog(parent), m_userId(userId), m_currentPhoto(currentPhoto)
{
    setWindowTitle("내 프로필");
    setFixedWidth(360);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    auto* root = new QVBoxLayout(this);
    root->setSpacing(0);
    root->setContentsMargins(0, 0, 0, 0);

    // ── 스택: 0=프로필뷰, 1=카메라뷰 ──────────────────────────
    m_stack = new QStackedWidget(this);
    root->addWidget(m_stack);

    // ── 페이지 0: 프로필 편집 ──────────────────────────────────
    auto* profilePage = new QWidget;
    auto* pLayout = new QVBoxLayout(profilePage);
    pLayout->setSpacing(20);
    pLayout->setContentsMargins(32, 32, 32, 28);

    auto* title = new QLabel("내 프로필");
    title->setStyleSheet("font-size:18px; font-weight:700; color:#1C1C1E;");
    title->setAlignment(Qt::AlignCenter);
    pLayout->addWidget(title);

    // 프로필 사진
    m_photoLabel = new QLabel;
    m_photoLabel->setFixedSize(100, 100);
    m_photoLabel->setAlignment(Qt::AlignCenter);
    m_photoLabel->setStyleSheet("border-radius:50px; background:#E5E5EA;");
    updatePhotoLabel();

    auto* photoRow = new QHBoxLayout;
    photoRow->addStretch();
    photoRow->addWidget(m_photoLabel);
    photoRow->addStretch();
    pLayout->addLayout(photoRow);

    // 사진 버튼 3개
    auto* btnRow = new QHBoxLayout;
    btnRow->setSpacing(8);

    auto makePhotoBtn = [](const QString& text) {
        auto* b = new QPushButton(text);
        b->setFixedHeight(34);
        b->setStyleSheet(
            "QPushButton { background:#F2F2F7; border:none; border-radius:10px;"
            " font-size:12px; color:#1C1C1E; padding:0 10px; }"
            "QPushButton:hover { background:#E5E5EA; }");
        return b;
    };

    auto* retakeBtn = makePhotoBtn("📷 재촬영");
    auto* uploadBtn = makePhotoBtn("🖼 업로드");
    auto* deleteBtn = makePhotoBtn("🗑 삭제");
    deleteBtn->setStyleSheet(
        "QPushButton { background:#FFF0F0; border:none; border-radius:10px;"
        " font-size:12px; color:#FF3B30; padding:0 10px; }"
        "QPushButton:hover { background:#FFE0E0; }");

    btnRow->addWidget(retakeBtn);
    btnRow->addWidget(uploadBtn);
    btnRow->addWidget(deleteBtn);
    pLayout->addLayout(btnRow);

    // 닉네임
    auto* nickLabel = new QLabel("닉네임");
    nickLabel->setStyleSheet("font-size:12px; font-weight:600; color:#6E6E73;");
    pLayout->addWidget(nickLabel);

    m_nicknameEdit = new QLineEdit;
    m_nicknameEdit->setPlaceholderText("닉네임을 입력하세요");
    m_nicknameEdit->setText(nickname);
    m_nicknameEdit->setMaxLength(20);
    m_nicknameEdit->setStyleSheet(
        "QLineEdit { padding:10px 14px; border:1.5px solid #E5E5EA; border-radius:12px;"
        " font-size:14px; color:#1C1C1E; }"
        "QLineEdit:focus { border-color:#007AFF; }");
    pLayout->addWidget(m_nicknameEdit);

    pLayout->addSpacing(4);

    // 하단 버튼
    auto* footRow = new QHBoxLayout;
    footRow->setSpacing(10);
    auto* cancelBtn = new QPushButton("취소");
    cancelBtn->setFixedHeight(44);
    cancelBtn->setStyleSheet(
        "QPushButton { background:#F2F2F7; border:none; border-radius:12px;"
        " font-size:15px; color:#1C1C1E; font-weight:600; }"
        "QPushButton:hover { background:#E5E5EA; }");
    auto* saveBtn = new QPushButton("저장");
    saveBtn->setFixedHeight(44);
    saveBtn->setDefault(true);
    saveBtn->setStyleSheet(
        "QPushButton { background:#007AFF; border:none; border-radius:12px;"
        " font-size:15px; color:#FFFFFF; font-weight:600; }"
        "QPushButton:hover { background:#0071E3; }");
    footRow->addWidget(cancelBtn);
    footRow->addWidget(saveBtn);
    pLayout->addLayout(footRow);

    m_stack->addWidget(profilePage);  // index 0

    // ── 페이지 1: 카메라 촬영 ──────────────────────────────────
    auto* cameraPage = new QWidget;
    auto* cLayout = new QVBoxLayout(cameraPage);
    cLayout->setSpacing(12);
    cLayout->setContentsMargins(16, 16, 16, 16);

    auto* camTitle = new QLabel("📷 사진 촬영");
    camTitle->setStyleSheet("font-size:16px; font-weight:700; color:#1C1C1E;");
    camTitle->setAlignment(Qt::AlignCenter);
    cLayout->addWidget(camTitle);

    m_videoWidget = new QVideoWidget;
    m_videoWidget->setMinimumHeight(220);
    m_videoWidget->setStyleSheet("background:#000; border-radius:12px;");
    cLayout->addWidget(m_videoWidget);

    auto* camBtnRow = new QHBoxLayout;
    camBtnRow->setSpacing(10);
    auto* cancelCamBtn = new QPushButton("취소");
    cancelCamBtn->setFixedHeight(40);
    cancelCamBtn->setStyleSheet(
        "QPushButton { background:#F2F2F7; border:none; border-radius:10px;"
        " font-size:14px; color:#1C1C1E; }"
        "QPushButton:hover { background:#E5E5EA; }");
    m_captureBtn = new QPushButton("촬영");
    m_captureBtn->setFixedHeight(40);
    m_captureBtn->setStyleSheet(
        "QPushButton { background:#007AFF; border:none; border-radius:10px;"
        " font-size:14px; color:#FFFFFF; font-weight:600; }"
        "QPushButton:hover { background:#0071E3; }");
    camBtnRow->addWidget(cancelCamBtn);
    camBtnRow->addWidget(m_captureBtn);
    cLayout->addLayout(camBtnRow);

    m_stack->addWidget(cameraPage);   // index 1

    // ── 시그널 연결 ────────────────────────────────────────────
    connect(retakeBtn,    &QPushButton::clicked, this, &MyPageDialog::onRetake);
    connect(uploadBtn,    &QPushButton::clicked, this, &MyPageDialog::onUpload);
    connect(deleteBtn,    &QPushButton::clicked, this, &MyPageDialog::onDelete);
    connect(cancelBtn,    &QPushButton::clicked, this, &QDialog::reject);
    connect(saveBtn,      &QPushButton::clicked, this, &MyPageDialog::onSave);
    connect(m_captureBtn, &QPushButton::clicked, this, &MyPageDialog::onCaptureClicked);
    connect(cancelCamBtn, &QPushButton::clicked, this, &MyPageDialog::onCancelCamera);
}

QString MyPageDialog::newNickname() const {
    return m_nicknameEdit->text().trimmed();
}

QPixmap MyPageDialog::makeCircular(const QPixmap& src, int size) {
    QPixmap scaled = src.scaled(size, size, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    QPixmap result(size, size);
    result.fill(Qt::transparent);
    QPainter p(&result);
    p.setRenderHint(QPainter::Antialiasing);
    p.setClipRegion(QRegion(0, 0, size, size, QRegion::Ellipse));
    int ox = (scaled.width()  - size) / 2;
    int oy = (scaled.height() - size) / 2;
    p.drawPixmap(0, 0, scaled, ox, oy, size, size);
    return result;
}

void MyPageDialog::updatePhotoLabel() {
    const QPixmap& base = m_photoChanged ? m_newPhoto : m_currentPhoto;
    if (base.isNull()) {
        m_photoLabel->setText("👤");
        m_photoLabel->setStyleSheet(
            "font-size:40px; border-radius:50px; background:#E5E5EA;");
    } else {
        m_photoLabel->setPixmap(makeCircular(base, 100));
        m_photoLabel->setStyleSheet("border-radius:50px;");
    }
}

void MyPageDialog::onUpload() {
    QString path = QFileDialog::getOpenFileName(
        this, "이미지 선택", "",
        "이미지 파일 (*.png *.jpg *.jpeg *.bmp *.gif)");
    if (path.isEmpty()) return;

    QPixmap pix(path);
    if (pix.isNull()) {
        QMessageBox::warning(this, "오류", "이미지를 불러올 수 없습니다.");
        return;
    }
    m_newPhoto      = pix;
    m_photoChanged  = true;
    m_photoDeleted  = false;
    updatePhotoLabel();
}

void MyPageDialog::onDelete() {
    m_newPhoto      = QPixmap();
    m_photoChanged  = false;
    m_photoDeleted  = true;
    updatePhotoLabel();
}

void MyPageDialog::onRetake() {
    const auto cameras = QMediaDevices::videoInputs();
    if (cameras.isEmpty()) {
        QMessageBox::information(this, "카메라 없음",
            "연결된 카메라가 없습니다.\n이미지 업로드를 이용해주세요.");
        return;
    }

    m_session    = new QMediaCaptureSession(this);
    m_camera     = new QCamera(cameras.first(), this);
    m_imgCapture = new QImageCapture(this);

    m_session->setCamera(m_camera);
    m_session->setVideoOutput(m_videoWidget);
    m_session->setImageCapture(m_imgCapture);

    connect(m_imgCapture, &QImageCapture::imageCaptured,
            this, &MyPageDialog::onImageCaptured);

    m_camera->start();
    m_stack->setCurrentIndex(1);
    setFixedHeight(420);
}

void MyPageDialog::onCaptureClicked() {
    if (m_imgCapture) m_imgCapture->capture();
}

void MyPageDialog::onImageCaptured(int, const QImage& img) {
    m_camera->stop();
    m_newPhoto     = QPixmap::fromImage(img);
    m_photoChanged = true;
    m_photoDeleted = false;
    updatePhotoLabel();
    m_stack->setCurrentIndex(0);
    setFixedHeight(sizeHint().height());
}

void MyPageDialog::onCancelCamera() {
    if (m_camera) m_camera->stop();
    m_stack->setCurrentIndex(0);
    setFixedHeight(sizeHint().height());
}

void MyPageDialog::onSave() {
    if (newNickname().isEmpty()) {
        QMessageBox::warning(this, "입력 오류", "닉네임을 입력해주세요.");
        return;
    }
    accept();
}
