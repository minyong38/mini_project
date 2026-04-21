#include "ChatDialog.h"
#include <QFileDialog>
#include <QBuffer>
#include <QImageReader>
#include <QMessageBox>

ChatDialog::ChatDialog(const QString& myId, QWidget* parent)
    : QDialog(parent), m_myId(myId)
{
    setWindowTitle("채팅");
    setMinimumSize(380, 580);
    setAttribute(Qt::WA_DeleteOnClose, false);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    auto* titleBar = new QWidget();
    titleBar->setFixedHeight(52);
    titleBar->setStyleSheet("background: #3C1E1E;");
    auto* titleLayout = new QHBoxLayout(titleBar);
    titleLayout->setContentsMargins(16, 0, 16, 0);
    m_titleLabel = new QLabel("단체 채팅방");
    m_titleLabel->setStyleSheet("color: white; font-size: 16px; font-weight: bold;");
    titleLayout->addWidget(m_titleLabel);
    mainLayout->addWidget(titleBar);

    m_scrollArea = new QScrollArea();
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setStyleSheet(
        "QScrollArea { border: none; background: #B2C7D9; }"
        "QScrollBar:vertical { width: 4px; background: transparent; }"
        "QScrollBar::handle:vertical { background: #90A4AE; border-radius: 2px; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }"
        );

    m_messageContainer = new QWidget();
    m_messageContainer->setStyleSheet("background: #B2C7D9;");
    m_messageLayout = new QVBoxLayout(m_messageContainer);
    m_messageLayout->setSpacing(6);
    m_messageLayout->setContentsMargins(12, 12, 12, 12);
    m_messageLayout->addStretch();

    m_scrollArea->setWidget(m_messageContainer);
    mainLayout->addWidget(m_scrollArea);

    auto* inputBar = new QWidget();
    inputBar->setFixedHeight(60);
    inputBar->setStyleSheet("background: #F5F5F5; border-top: 1px solid #DDDDDD;");
    auto* inputLayout = new QHBoxLayout(inputBar);
    inputLayout->setContentsMargins(10, 8, 10, 8);
    inputLayout->setSpacing(8);

    m_imageBtn = new QPushButton("🖼");
    m_imageBtn->setFixedSize(36, 36);
    m_imageBtn->setToolTip("이미지 전송");
    m_imageBtn->setAutoDefault(false);
    m_imageBtn->setStyleSheet(
        "QPushButton { background:#EEEEEE; border:none; border-radius:18px; font-size:16px; }"
        "QPushButton:hover  { background:#DDDDDD; }"
        "QPushButton:pressed{ background:#CCCCCC; }"
        );

    m_inputEdit = new QLineEdit();
    m_inputEdit->setPlaceholderText("메시지 입력");
    m_inputEdit->setStyleSheet(
        "QLineEdit { background:#FFFFFF; border:1px solid #DDDDDD; border-radius:20px;"
        " padding:6px 16px; font-size:14px; color:#000; }"
        "QLineEdit:focus { border-color:#AAAAAA; }"
        );

    m_sendBtn = new QPushButton("전송");
    m_sendBtn->setFixedSize(60, 36);
    m_sendBtn->setAutoDefault(false);
    m_sendBtn->setStyleSheet(
        "QPushButton { background:#FFCD00; color:#3C1E1E; border:none; border-radius:18px;"
        " font-size:13px; font-weight:bold; }"
        "QPushButton:hover  { background:#FFBA00; }"
        "QPushButton:pressed{ background:#F0A800; }"
        );

    inputLayout->addWidget(m_imageBtn);
    inputLayout->addWidget(m_inputEdit);
    inputLayout->addWidget(m_sendBtn);
    mainLayout->addWidget(inputBar);

    connect(m_sendBtn,   &QPushButton::clicked,    this, &ChatDialog::onSendClicked);
    connect(m_inputEdit, &QLineEdit::returnPressed, this, &ChatDialog::onSendClicked);
    connect(m_imageBtn,  &QPushButton::clicked,    this, &ChatDialog::onImageClicked);

    // 스크롤바 범위가 실제로 늘어난 순간 바닥으로 이동
    connect(m_scrollArea->verticalScrollBar(), &QScrollBar::rangeChanged,
            this, [this](int, int max) {
                m_scrollArea->verticalScrollBar()->setValue(max);
            });
}

void ChatDialog::onSendClicked() {
    QString text = m_inputEdit->text().trimmed();
    if (text.isEmpty()) return;
    m_inputEdit->clear();
    emit messageSent(text);
}

void ChatDialog::onImageClicked() {
    QString path = QFileDialog::getOpenFileName(
        this, "이미지 선택", "",
        "이미지 파일 (*.png *.jpg *.jpeg *.bmp *.gif *.webp)"
        );
    if (path.isEmpty()) return;

    QImageReader reader(path);
    reader.setAutoTransform(true);
    QImage img = reader.read();
    if (img.isNull()) {
        QMessageBox::warning(this, "오류", "이미지를 불러올 수 없습니다.");
        return;
    }

    // 최대 크기로 리사이즈 (비율 유지)
    if (img.width() > IMG_MAX_PX || img.height() > IMG_MAX_PX)
        img = img.scaled(IMG_MAX_PX, IMG_MAX_PX, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    // JPEG로 압축 후 Base64 인코딩
    QByteArray bytes;
    QBuffer buf(&bytes);
    buf.open(QIODevice::WriteOnly);
    img.save(&buf, "JPEG", IMG_QUALITY);

    QString encoded = "IMG:" + QString::fromLatin1(bytes.toBase64());
    emit messageSent(encoded);
}

void ChatDialog::appendMessage(qint64 rowid, int unread,
                               const QString& userId, const QString& message,
                               const QString& time)
{
    bool isMe = (userId == m_myId);

    auto* row = new QWidget();
    row->setStyleSheet("background: transparent;");
    auto* rowLayout = new QHBoxLayout(row);
    rowLayout->setContentsMargins(0, 0, 0, 0);
    rowLayout->setSpacing(4);

    auto* bubble = new QWidget();
    auto* bubbleLayout = new QVBoxLayout(bubble);
    bubbleLayout->setContentsMargins(12, 8, 12, 8);
    bubbleLayout->setSpacing(3);

    if (!isMe) {
        auto* nameLabel = new QLabel(userId);
        nameLabel->setStyleSheet(
            "color:#333; font-size:12px; font-weight:bold; background:transparent;"
            );
        bubbleLayout->addWidget(nameLabel);
    }

    const bool isImage = message.startsWith("IMG:");

    if (isImage) {
        QByteArray bytes = QByteArray::fromBase64(message.mid(4).toLatin1());
        QPixmap pm;
        pm.loadFromData(bytes, "JPEG");

        auto* imgLabel = new QLabel();
        imgLabel->setPixmap(pm.scaledToWidth(220, Qt::SmoothTransformation));
        imgLabel->setFixedWidth(220);
        imgLabel->setCursor(Qt::PointingHandCursor);
        imgLabel->setStyleSheet("background:transparent;");
        bubbleLayout->addWidget(imgLabel);
    } else {
        auto* msgLabel = new QLabel(message);
        msgLabel->setWordWrap(true);
        msgLabel->setMaximumWidth(220);
        msgLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
        msgLabel->setStyleSheet("font-size:14px; color:#000; background:transparent;");
        bubbleLayout->addWidget(msgLabel);
    }

    bubble->setStyleSheet(isMe
                              ? "background:#FFCD00; border-radius:12px; border-top-right-radius:2px;"
                              : "background:#FFFFFF; border-radius:12px; border-top-left-radius:2px;"
                          );

    auto* timeLabel = new QLabel(time);
    timeLabel->setStyleSheet("color:#6E8B9A; font-size:10px; background:transparent;");
    timeLabel->setAlignment(Qt::AlignBottom);

    auto* unreadLabel = new QLabel(unread > 0 ? QString::number(unread) : "");
    unreadLabel->setStyleSheet(
        "color:#34C759; font-size:10px; font-weight:bold; background:transparent;"
        );
    unreadLabel->setAlignment(Qt::AlignBottom);
    unreadLabel->setVisible(unread > 0);
    m_unreadLabels[rowid] = unreadLabel;

    if (isMe) {
        // 오른쪽 정렬: [stretch][미읽음][시간][버블]
        rowLayout->addStretch();
        rowLayout->addWidget(unreadLabel, 0, Qt::AlignBottom);
        if (!time.isEmpty()) rowLayout->addWidget(timeLabel, 0, Qt::AlignBottom);
        rowLayout->addWidget(bubble);
    } else {
        // 왼쪽 정렬: [버블][시간][미읽음][stretch]
        rowLayout->addWidget(bubble);
        if (!time.isEmpty()) rowLayout->addWidget(timeLabel, 0, Qt::AlignBottom);
        rowLayout->addWidget(unreadLabel, 0, Qt::AlignBottom);
        rowLayout->addStretch();
    }

    m_messageLayout->insertWidget(m_messageLayout->count() - 1, row);
}

void ChatDialog::updateUnread(qint64 rowid, int count) {
    auto it = m_unreadLabels.find(rowid);
    if (it == m_unreadLabels.end()) return;
    QLabel* label = it.value();
    if (count <= 0) {
        label->setVisible(false);
    } else {
        label->setText(QString::number(count));
        label->setVisible(true);
    }
}

void ChatDialog::setDarkMode(bool dark)
{
    if (dark) {
        m_scrollArea->setStyleSheet(
            "QScrollArea { border:none; background:#1C1C1E; }"
            "QScrollBar:vertical { width:4px; background:transparent; }"
            "QScrollBar::handle:vertical { background:#48484A; border-radius:2px; }"
            "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height:0px; }");
        m_messageContainer->setStyleSheet("background:#1C1C1E;");
        m_inputEdit->setStyleSheet(
            "QLineEdit { background:#3A3A3C; border:1px solid #48484A; border-radius:20px;"
            " padding:6px 16px; font-size:14px; color:#FFFFFF; }"
            "QLineEdit:focus { border-color:#007AFF; }");
        m_imageBtn->setStyleSheet(
            "QPushButton { background:#3A3A3C; border:none; border-radius:18px; font-size:16px; }"
            "QPushButton:hover { background:#48484A; }");
        parentWidget(); // no-op, just grouping visually
        // inputBar는 직접 접근이 필요하므로 부모를 통해 찾음
        auto* bar = m_sendBtn->parentWidget();
        if (bar) bar->setStyleSheet("background:#2C2C2E; border-top:1px solid #3A3A3C;");
        // titleBar
        if (auto* tb = findChild<QWidget*>("", Qt::FindDirectChildrenOnly)) { (void)tb; }
        // title bar: layout item 0
        if (layout() && layout()->count() > 0) {
            if (auto* item = layout()->itemAt(0))
                if (auto* w = item->widget())
                    w->setStyleSheet("background:#1C1C1E;");
        }
    } else {
        m_scrollArea->setStyleSheet(
            "QScrollArea { border:none; background:#B2C7D9; }"
            "QScrollBar:vertical { width:4px; background:transparent; }"
            "QScrollBar::handle:vertical { background:#90A4AE; border-radius:2px; }"
            "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height:0px; }");
        m_messageContainer->setStyleSheet("background:#B2C7D9;");
        m_inputEdit->setStyleSheet(
            "QLineEdit { background:#FFFFFF; border:1px solid #DDDDDD; border-radius:20px;"
            " padding:6px 16px; font-size:14px; color:#000; }"
            "QLineEdit:focus { border-color:#AAAAAA; }");
        m_imageBtn->setStyleSheet(
            "QPushButton { background:#EEEEEE; border:none; border-radius:18px; font-size:16px; }"
            "QPushButton:hover { background:#DDDDDD; }");
        auto* bar = m_sendBtn->parentWidget();
        if (bar) bar->setStyleSheet("background:#F5F5F5; border-top:1px solid #DDDDDD;");
        if (layout() && layout()->count() > 0) {
            if (auto* item = layout()->itemAt(0))
                if (auto* w = item->widget())
                    w->setStyleSheet("background:#3C1E1E;");
        }
    }
}

void ChatDialog::setTitle(const QString& title) {
    if (m_titleLabel) m_titleLabel->setText(title);
    setWindowTitle(title);
}

void ChatDialog::clearMessages() {
    m_unreadLabels.clear();
    // stretch 아이템(마지막)을 제외하고 모두 제거
    while (m_messageLayout->count() > 1) {
        QLayoutItem* item = m_messageLayout->takeAt(0);
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }
}

