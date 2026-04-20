#include "MainWindow.h"
#include "ui_mainwindow.h"
#include "Common.h"
#include <QMessageBox>
#include <QFrame>
#include <QHBoxLayout>
#include <QTimer>
#include <QCloseEvent>
#include <QAction>
#include <QPainter>
#include <QPainterPath>
#include <QStyle>

MainWindow::MainWindow(const QString& ip, const QString& myId, QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow),
    m_myId(myId), m_selectedId(myId), m_serverIp(ip)
{
    ui->setupUi(this);
    setWindowTitle("캘린더 — " + m_myId);

    m_selectedDate = QDate::currentDate();

    ui->userCombo->addItems({m_myId});
    connect(ui->userCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onUserChanged);

    connect(ui->calendarWidget, &QCalendarWidget::clicked,
            this, &MainWindow::onDateClicked);
    connect(ui->calendarWidget, &QCalendarWidget::currentPageChanged,
            this, &MainWindow::requestMonthSchedules);

    // 채팅 다이얼로그 미리 생성
    m_chatDialog = new ChatDialog(m_myId, this);
    connect(m_chatDialog, &ChatDialog::messageSent, this, &MainWindow::onChatMessageSent);
    connect(ui->chatBtn, &QPushButton::clicked, this, &MainWindow::onChatBtnClicked);

    // 다크모드 토글 버튼 (navBar에 삽입)
    m_themeBtn = new QPushButton("🌙", this);
    m_themeBtn->setFixedSize(36, 36);
    m_themeBtn->setToolTip("다크 모드 전환");
    m_themeBtn->setStyleSheet(
        "QPushButton { background:transparent; border:none; font-size:18px; border-radius:18px; }"
        "QPushButton:hover { background:#F2F2F7; }"
    );
    // navLayout에서 spacer 다음 위치(chatBtn 앞)에 삽입
    auto* navLayout = qobject_cast<QHBoxLayout*>(ui->navBar->layout());
    if (navLayout) navLayout->insertWidget(2, m_themeBtn);
    connect(m_themeBtn, &QPushButton::clicked, this, [this]() {
        applyTheme(!m_darkMode);
    });

    setupTray();

    // 소켓
    m_socket = new QTcpSocket(this);
    connect(m_socket, &QTcpSocket::connected,     this, &MainWindow::onConnectSuccess);
    connect(m_socket, &QTcpSocket::readyRead,     this, &MainWindow::onReadyRead);
    connect(m_socket, &QTcpSocket::errorOccurred, this, &MainWindow::onError);

    m_socket->connectToHost(m_serverIp, Protocol::PORT);
}

MainWindow::~MainWindow() { delete ui; }

// ──────────────────────────────────────────────
//  네트워크
// ──────────────────────────────────────────────

void MainWindow::onConnectSuccess() {
    // 접속 후 먼저 로그인
    m_socket->write((Protocol::LOGIN + Protocol::SEP + m_myId + "\n").toUtf8());
}

void MainWindow::requestUsers() {
    if (m_socket->state() != QAbstractSocket::ConnectedState) return;
    m_socket->write((Protocol::REQUSERS + "\n").toUtf8());
}

void MainWindow::requestMonthSchedules(int year, int month) {
    if (m_socket->state() != QAbstractSocket::ConnectedState) return;
    QString ym  = QString("%1-%2").arg(year).arg(month, 2, 10, QChar('0'));
    QString msg = Protocol::REQMONTH + Protocol::SEP + m_selectedId + Protocol::SEP + ym + "\n";
    m_socket->write(msg.toUtf8());
}

void MainWindow::requestSchedules(const QDate& date) {
    if (m_socket->state() != QAbstractSocket::ConnectedState) return;
    QString msg = Protocol::REQ + Protocol::SEP
                  + m_selectedId + Protocol::SEP
                  + date.toString("yyyy-MM-dd") + "\n";
    m_socket->write(msg.toUtf8());
}

void MainWindow::onReadyRead() {
    m_buffer += QString::fromUtf8(m_socket->readAll());
    while (m_buffer.contains('\n')) {
        int idx = m_buffer.indexOf('\n');
        QString data = m_buffer.left(idx).trimmed();
        m_buffer = m_buffer.mid(idx + 1);
        if (data.isEmpty()) continue;
        processMessage(data);
    }
}

void MainWindow::processMessage(const QString& data) {

    // ── 로그인 결과 ──────────────────────────────────────────
    if (data == Protocol::LOGIN_OK) {
        requestUsers();
        requestMonthSchedules(ui->calendarWidget->yearShown(),
                              ui->calendarWidget->monthShown());
    }
    else if (data == Protocol::LOGIN_REJECT) {
        QMessageBox::critical(this, "로그인 실패",
            "'" + m_myId + "' 는 이미 접속 중인 ID입니다.\n"
            "다른 ID로 다시 실행해주세요.");
        close();
    }

    // ── 접속자 목록 ──────────────────────────────────────────
    else if (data.startsWith(Protocol::ONLINE + Protocol::SEP)) {
        QString payload = data.mid(Protocol::ONLINE.length() + Protocol::SEP.length());
        QStringList users = payload.isEmpty() ? QStringList() : payload.split("|");

        // 첫 수신은 현재 목록 저장만 (이미 있던 사람들은 알림 없음)
        if (m_initialOnlineReceived) {
            for (const QString& u : users) {
                if (!m_onlineUsers.contains(u) && u != m_myId)
                    showJoinNotification(u);
            }
        }
        m_onlineUsers = users;
        m_initialOnlineReceived = true;

        QString text = QString("접속 중 (%1명):  ").arg(users.size());
        QStringList dots;
        for (const QString& u : users)
            dots << "● " + u;
        text += dots.join("   ");
        ui->onlineLabel->setText(text);
    }

    // ── 유저 목록 ────────────────────────────────────────────
    else if (data.startsWith(Protocol::RESUSERS + Protocol::SEP)) {
        QString payload = data.mid(Protocol::RESUSERS.length() + Protocol::SEP.length());
        QStringList users = payload.isEmpty() ? QStringList() : payload.split("|");

        if (!users.contains(m_myId))
            users.prepend(m_myId);

        ui->userCombo->blockSignals(true);
        ui->userCombo->clear();
        ui->userCombo->addItems(users);
        int idx = users.indexOf(m_selectedId);
        ui->userCombo->setCurrentIndex(idx >= 0 ? idx : 0);
        ui->userCombo->blockSignals(false);
    }

    // ── 월별 일정 ────────────────────────────────────────────
    else if (data.startsWith(Protocol::RESMONTH + Protocol::SEP)) {
        m_monthSchedules.clear();
        QString payload = data.mid(Protocol::RESMONTH.length() + Protocol::SEP.length());
        if (!payload.isEmpty()) {
            for (const QString& entry : payload.split("|")) {
                QStringList p = entry.split("@");
                if (p.size() < 3) continue;
                QDate d = QDate::fromString(p[0], "yyyy-MM-dd");
                if (d.isValid()) m_monthSchedules[d].append(p[2]);
            }
        }
        ui->calendarWidget->setMonthSchedules(m_monthSchedules);
    }

    // ── 일별 일정 ────────────────────────────────────────────
    else if (data.startsWith(Protocol::RES + Protocol::SEP)) {
        m_currentRowids.clear();
        m_currentContents.clear();
        QString payload = data.mid(Protocol::RES.length() + Protocol::SEP.length());
        if (!payload.isEmpty()) {
            for (const QString& entry : payload.split("|")) {
                int sep = entry.indexOf(Protocol::SEP);
                if (sep == -1) continue;
                m_currentRowids   << entry.left(sep).toLongLong();
                m_currentContents << entry.mid(sep + 1);
            }
        }
        m_monthSchedules[m_selectedDate] = m_currentContents;
        ui->calendarWidget->setMonthSchedules(m_monthSchedules);

        if (m_pendingModal) {
            m_pendingModal = false;
            showDateDialog();
        } else if (m_activeDialog) {
            m_activeDialog->refreshSchedules(m_currentContents, m_currentRowids);
        }
    }

    // ── 채팅 기록 (기존 메시지) ──────────────────────────────
    else if (data.startsWith(Protocol::RESCHAT + Protocol::SEP)) {
        // RESCHAT:ROWID:UNREAD:USER_ID:HHmm:MSG|...
        m_chatDialog->clearMessages();
        QString payload = data.mid(Protocol::RESCHAT.length() + Protocol::SEP.length());
        if (!payload.isEmpty()) {
            for (const QString& entry : payload.split("|")) {
                QStringList p = entry.split(Protocol::SEP);
                if (p.size() < 5) continue;
                qint64  rowid  = p[0].toLongLong();
                int     unread = p[1].toInt();
                QString uid    = p[2];
                QString rawT   = p[3];
                QString msg    = p.mid(4).join(Protocol::SEP);
                QString time   = rawT.length() == 4 ? rawT.left(2) + ":" + rawT.right(2) : "";
                m_chatDialog->appendMessage(rowid, unread, uid, msg, time);
            }
        }
    }

    // ── 채팅 실시간 수신 ─────────────────────────────────────
    else if (data.startsWith(Protocol::CHATRES + Protocol::SEP)) {
        // CHATRES:ROWID:UNREAD:USER_ID:HHmm:MESSAGE
        QString payload = data.mid(Protocol::CHATRES.length() + Protocol::SEP.length());
        QStringList p = payload.split(Protocol::SEP);
        if (p.size() >= 5) {
            qint64  rowid  = p[0].toLongLong();
            int     unread = p[1].toInt();
            QString uid    = p[2];
            QString rawT   = p[3];
            QString msg    = p.mid(4).join(Protocol::SEP);
            QString time   = rawT.length() == 4 ? rawT.left(2) + ":" + rawT.right(2) : "";

            m_chatDialog->appendMessage(rowid, unread, uid, msg, time);

            if (!m_chatDialog->isVisible()) {
                m_unreadCount++;
                updateChatBtnText();
                updateTrayIcon();
                if (m_trayIcon && !isActiveWindow()) {
                    m_trayIcon->showMessage(
                        uid + "님의 메시지",
                        msg,
                        QSystemTrayIcon::NoIcon, 3000
                    );
                }
            }
        }
    }

    // ── 읽음 처리 실시간 업데이트 ────────────────────────────
    else if (data.startsWith(Protocol::READRES + Protocol::SEP)) {
        // READRES:ROWID:COUNT
        QStringList p = data.split(Protocol::SEP);
        if (p.size() >= 3)
            m_chatDialog->updateUnread(p[1].toLongLong(), p[2].toInt());
    }

    // ── ACK ──────────────────────────────────────────────────
    else if (data.startsWith(Protocol::ACK)) {
        if (!data.contains("OK"))
            QMessageBox::critical(this, "오류", "요청 처리에 실패했습니다.");
    }
}

// ──────────────────────────────────────────────
//  채팅
// ──────────────────────────────────────────────

void MainWindow::onChatBtnClicked() {
    if (m_chatDialog->isVisible()) {
        m_chatDialog->hide();
    } else {
        if (m_socket->state() == QAbstractSocket::ConnectedState)
            m_socket->write((Protocol::REQCHAT + Protocol::SEP + m_myId + "\n").toUtf8());
        m_unreadCount = 0;
        updateChatBtnText();
        updateTrayIcon();
        m_chatDialog->show();
        m_chatDialog->raise();
    }
}

void MainWindow::onChatMessageSent(const QString& message) {
    if (m_socket->state() != QAbstractSocket::ConnectedState) return;
    m_socket->write((Protocol::CHAT + Protocol::SEP + m_myId + Protocol::SEP + message + "\n").toUtf8());
}

void MainWindow::updateChatBtnText() {
    if (m_unreadCount > 0)
        ui->chatBtn->setText(QString("💬 채팅 (%1)").arg(m_unreadCount));
    else
        ui->chatBtn->setText("💬 채팅");
}

// ──────────────────────────────────────────────
//  UI 이벤트
// ──────────────────────────────────────────────

void MainWindow::onDateClicked(const QDate& date) {
    m_selectedDate = date;
    m_pendingModal = true;
    requestSchedules(date);
}

void MainWindow::onUserChanged(int) {
    m_selectedId = ui->userCombo->currentText();
    m_monthSchedules.clear();
    ui->calendarWidget->clearSchedules();
    requestMonthSchedules(ui->calendarWidget->yearShown(),
                          ui->calendarWidget->monthShown());
}

void MainWindow::showDateDialog() {
    if (m_activeDialog) {
        m_activeDialog->close();
        return;
    }
    m_activeDialog = new ScheduleDialog(m_selectedDate, this);
    m_activeDialog->refreshSchedules(m_currentContents, m_currentRowids);

    if (m_selectedId != m_myId)
        m_activeDialog->setReadOnly(true);

    connect(m_activeDialog, &ScheduleDialog::addRequested,    this, &MainWindow::onDialogAdd);
    connect(m_activeDialog, &ScheduleDialog::editRequested,   this, &MainWindow::onDialogEdit);
    connect(m_activeDialog, &ScheduleDialog::deleteRequested, this, &MainWindow::onDialogDelete);
    connect(m_activeDialog, &QDialog::finished, this, [this]() {
        m_activeDialog->deleteLater();
        m_activeDialog = nullptr;
    });
    m_activeDialog->open();
}

// ──────────────────────────────────────────────
//  다이얼로그 → 서버 요청
// ──────────────────────────────────────────────

void MainWindow::onDialogAdd(const QString& content) {
    QString msg = Protocol::ADD + Protocol::SEP
                  + m_myId + Protocol::SEP
                  + m_selectedDate.toString("yyyy-MM-dd") + Protocol::SEP
                  + content + "\n";
    m_socket->write(msg.toUtf8());
    requestUsers();
    requestSchedules(m_selectedDate);
    requestMonthSchedules(ui->calendarWidget->yearShown(),
                          ui->calendarWidget->monthShown());
}

void MainWindow::onDialogEdit(qint64 rowid, const QString& newContent) {
    QString msg = Protocol::MOD + Protocol::SEP
                  + QString::number(rowid) + Protocol::SEP
                  + newContent + "\n";
    m_socket->write(msg.toUtf8());
    requestSchedules(m_selectedDate);
    requestMonthSchedules(ui->calendarWidget->yearShown(),
                          ui->calendarWidget->monthShown());
}

void MainWindow::onDialogDelete(qint64 rowid) {
    QString msg = Protocol::DEL + Protocol::SEP + QString::number(rowid) + "\n";
    m_socket->write(msg.toUtf8());
    requestSchedules(m_selectedDate);
    requestMonthSchedules(ui->calendarWidget->yearShown(),
                          ui->calendarWidget->monthShown());
}

// ──────────────────────────────────────────────
//  연결 오류
// ──────────────────────────────────────────────

void MainWindow::showJoinNotification(const QString& userId) {
    auto* popup = new QWidget(nullptr,
        Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    popup->setAttribute(Qt::WA_TranslucentBackground);
    popup->setAttribute(Qt::WA_DeleteOnClose);
    popup->setFixedSize(300, 66);

    auto* bg = new QFrame(popup);
    bg->setGeometry(0, 0, 300, 66);
    bg->setStyleSheet(
        "QFrame {"
        "  background: #1C1C1E;"
        "  border-radius: 14px;"
        "}"
    );

    auto* layout = new QHBoxLayout(bg);
    layout->setContentsMargins(16, 0, 16, 0);
    layout->setSpacing(12);

    auto* iconLabel = new QLabel("👋");
    iconLabel->setStyleSheet("font-size: 24px; background: transparent;");

    auto* textLayout = new QVBoxLayout();
    textLayout->setSpacing(2);

    auto* nameLabel = new QLabel(userId);
    nameLabel->setStyleSheet(
        "color: #FFFFFF; font-size: 14px; font-weight: bold; background: transparent;"
    );
    auto* msgLabel = new QLabel("입장하였습니다");
    msgLabel->setStyleSheet(
        "color: #8E8E93; font-size: 12px; background: transparent;"
    );
    textLayout->addWidget(nameLabel);
    textLayout->addWidget(msgLabel);

    layout->addWidget(iconLabel);
    layout->addLayout(textLayout);
    layout->addStretch();

    // 우측 하단 위치 계산
    QPoint pos = mapToGlobal(QPoint(width() - 320, height() - 86));
    popup->move(pos);
    popup->show();

    // 3초 후 자동 닫힘
    QTimer::singleShot(3000, popup, &QWidget::close);
}

// 캘린더 트레이 아이콘을 QPainter로 그려 반환
// unread > 0 이면 우상단에 빨간 뱃지 추가
static QIcon makeCalendarIcon(int today, int unread = 0) {
    const int S = 32;
    QPixmap pm(S, S);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);

    // ── 흰 카드 배경 (둥근 모서리) ─────────────────────
    p.setPen(Qt::NoPen);
    p.setBrush(QColor("#FFFFFF"));
    p.drawRoundedRect(1, 1, S - 2, S - 2, 5, 5);

    // ── 테두리 ─────────────────────────────────────────
    p.setPen(QPen(QColor("#C7C7CC"), 1));
    p.setBrush(Qt::NoBrush);
    p.drawRoundedRect(1, 1, S - 2, S - 2, 5, 5);

    // ── 빨간 헤더 바 ───────────────────────────────────
    p.setPen(Qt::NoPen);
    p.setBrush(QColor("#FF3B30"));
    // 상단만 둥글게
    QPainterPath header;
    header.addRoundedRect(1, 1, S - 2, 9, 5, 5);
    header.addRect(1, 5, S - 2, 5);   // 하단 직각으로 채움
    p.drawPath(header);

    // ── 격자선 (날짜 칸) ───────────────────────────────
    p.setPen(QPen(QColor("#E5E5EA"), 0.5));
    // 가로선
    p.drawLine(2, 16, S - 2, 16);
    p.drawLine(2, 23, S - 2, 23);
    // 세로선
    for (int x : {12, 22})
        p.drawLine(x, 10, x, S - 2);

    // ── 오늘 날짜 숫자 (중앙 칸에 파란 원) ─────────────
    QRect numCircle(10, 11, 12, 12);
    p.setPen(Qt::NoPen);
    p.setBrush(QColor("#007AFF"));
    p.drawEllipse(numCircle);

    p.setPen(Qt::white);
    QFont nf;
    nf.setPixelSize(7);
    nf.setBold(true);
    p.setFont(nf);
    p.drawText(numCircle, Qt::AlignCenter, QString::number(today));

    // ── 나머지 칸 점 (일정 있음 표시) ─────────────────
    p.setPen(Qt::NoPen);
    p.setBrush(QColor("#C7C7CC"));
    for (QPoint dot : {QPoint(6,19), QPoint(26,19), QPoint(6,26), QPoint(16,26), QPoint(26,26)})
        p.drawEllipse(dot.x() - 1, dot.y() - 1, 3, 3);

    // ── 미읽음 뱃지 ────────────────────────────────────
    if (unread > 0) {
        QString cnt = unread > 99 ? "99+" : QString::number(unread);
        int bW = (unread > 9) ? 14 : 11;
        QRect badge(S - bW - 1, 0, bW, 11);

        p.setPen(Qt::NoPen);
        p.setBrush(QColor("#FF3B30"));
        p.drawRoundedRect(badge, 5, 5);

        p.setPen(Qt::white);
        QFont bf;
        bf.setPixelSize(7);
        bf.setBold(true);
        p.setFont(bf);
        p.drawText(badge, Qt::AlignCenter, cnt);
    }

    return QIcon(pm);
}

void MainWindow::applyTheme(bool dark)
{
    m_darkMode = dark;
    m_themeBtn->setText(dark ? "☀️" : "🌙");

    if (dark) {
        // ── 메인 윈도우 ──────────────────────────────────
        ui->centralwidget->setStyleSheet("QWidget { background:#1C1C1E; }");
        ui->navBar->setStyleSheet(
            "QFrame#navBar { background:#2C2C2E; border-bottom:1px solid #3A3A3C; }");
        ui->titleLabel->setStyleSheet(
            "QLabel { font-size:20px; font-weight:bold; color:#FFFFFF; background:transparent; border:none; }");
        ui->userPrefixLabel->setStyleSheet(
            "QLabel { font-size:13px; color:#8E8E93; background:transparent; border:none; }");
        ui->userCombo->setStyleSheet(
            "QComboBox { padding:4px 12px; border:1.5px solid #3A3A3C; border-radius:18px;"
            " font-size:13px; background:#3A3A3C; color:#FFFFFF; }"
            "QComboBox:hover { border-color:#007AFF; }"
            "QComboBox::drop-down { border:none; width:20px; }"
            "QComboBox QAbstractItemView { background:#2C2C2E; color:#FFFFFF;"
            " selection-background-color:#1A3A5C; selection-color:#FFFFFF; font-size:13px; }");
        ui->onlineBar->setStyleSheet(
            "QFrame#onlineBar { background:#0D2318; border-bottom:1px solid #1A4230; }");
        ui->onlineLabel->setStyleSheet(
            "QLabel { font-size:12px; color:#4CAF7D; background:transparent; border:none; }");
        ui->calendarWrapper->setStyleSheet(
            "QWidget#calendarWrapper { background:#1C1C1E; }");
        ui->calendarCard->setStyleSheet(
            "QFrame#calendarCard { background:#2C2C2E; border-radius:16px; border:none; }");
        ui->calendarWidget->setStyleSheet(R"(
            CustomCalendarWidget { background-color:#2C2C2E; border:none; border-radius:16px; }
            QCalendarWidget QToolButton { color:#FFFFFF; background:transparent; border:none;
                font-size:15px; font-weight:700; padding:8px 14px; border-radius:8px; }
            QCalendarWidget QToolButton:hover { background-color:#3A3A3C; }
            QCalendarWidget QToolButton#qt_calendar_prevmonth,
            QCalendarWidget QToolButton#qt_calendar_nextmonth { color:#007AFF; font-size:18px; font-weight:bold; }
            QCalendarWidget QWidget#qt_calendar_navigationbar { background-color:#2C2C2E;
                padding:10px 8px 6px 8px; border-bottom:1px solid #3A3A3C;
                border-top-left-radius:16px; border-top-right-radius:16px; }
            QCalendarWidget QAbstractItemView { background-color:#2C2C2E;
                selection-background-color:transparent; selection-color:#007AFF;
                font-size:13px; color:#FFFFFF; outline:none; gridline-color:#3A3A3C; }
            QCalendarWidget QAbstractItemView:disabled { color:#48484A; }
            QCalendarWidget QWidget { alternate-background-color:#2C2C2E; }
        )");
        m_themeBtn->setStyleSheet(
            "QPushButton { background:transparent; border:none; font-size:18px; border-radius:18px; }"
            "QPushButton:hover { background:#3A3A3C; }");
    } else {
        // ── 라이트 복원 ───────────────────────────────────
        ui->centralwidget->setStyleSheet("");
        ui->navBar->setStyleSheet(
            "QFrame#navBar { background:#FFFFFF; border-bottom:1px solid #E5E5EA; }");
        ui->titleLabel->setStyleSheet(
            "QLabel { font-size:20px; font-weight:bold; color:#1C1C1E; background:transparent; border:none; }");
        ui->userPrefixLabel->setStyleSheet(
            "QLabel { font-size:13px; color:#8E8E93; background:transparent; border:none; }");
        ui->userCombo->setStyleSheet(
            "QComboBox { padding:4px 12px; border:1.5px solid #E5E5EA; border-radius:18px;"
            " font-size:13px; background:#F2F2F7; color:#1C1C1E; }"
            "QComboBox:hover { border-color:#007AFF; background:#FFFFFF; }"
            "QComboBox::drop-down { border:none; width:20px; }"
            "QComboBox QAbstractItemView { border:1px solid #E5E5EA; border-radius:10px;"
            " background:#FFFFFF; selection-background-color:#E5F0FF;"
            " selection-color:#007AFF; font-size:13px; }");
        ui->onlineBar->setStyleSheet(
            "QFrame#onlineBar { background:#F0FFF4; border-bottom:1px solid #C3E6CB; }");
        ui->onlineLabel->setStyleSheet(
            "QLabel { font-size:12px; color:#2D6A4F; background:transparent; border:none; }");
        ui->calendarWrapper->setStyleSheet(
            "QWidget#calendarWrapper { background:#F2F2F7; }");
        ui->calendarCard->setStyleSheet(
            "QFrame#calendarCard { background:#FFFFFF; border-radius:16px; border:none; }");
        ui->calendarWidget->setStyleSheet(R"(
            CustomCalendarWidget { background-color:#FFFFFF; border:none; border-radius:16px; }
            QCalendarWidget QToolButton { color:#1C1C1E; background:transparent; border:none;
                font-size:15px; font-weight:700; padding:8px 14px; border-radius:8px; }
            QCalendarWidget QToolButton:hover { background-color:#F2F2F7; }
            QCalendarWidget QToolButton#qt_calendar_prevmonth,
            QCalendarWidget QToolButton#qt_calendar_nextmonth { color:#007AFF; font-size:18px; font-weight:bold; }
            QCalendarWidget QWidget#qt_calendar_navigationbar { background-color:#FFFFFF;
                padding:10px 8px 6px 8px; border-bottom:1px solid #F2F2F7;
                border-top-left-radius:16px; border-top-right-radius:16px; }
            QCalendarWidget QAbstractItemView { background-color:#FFFFFF;
                selection-background-color:transparent; selection-color:#007AFF;
                font-size:13px; color:#1C1C1E; outline:none; gridline-color:#F2F2F7; }
            QCalendarWidget QAbstractItemView:disabled { color:#D1D1D6; }
            QCalendarWidget QWidget { alternate-background-color:#FFFFFF; }
        )");
        m_themeBtn->setStyleSheet(
            "QPushButton { background:transparent; border:none; font-size:18px; border-radius:18px; }"
            "QPushButton:hover { background:#F2F2F7; }");
    }

    ui->calendarWidget->setDarkMode(dark);
    m_chatDialog->setDarkMode(dark);
}

void MainWindow::setupTray() {
    if (!QSystemTrayIcon::isSystemTrayAvailable()) return;

    m_trayIcon = new QSystemTrayIcon(this);
    m_trayIcon->setIcon(makeCalendarIcon(QDate::currentDate().day()));
    m_trayIcon->setToolTip("캘린더 — " + m_myId);

    m_trayMenu = new QMenu(this);

    auto* showAction = new QAction("열기", this);
    connect(showAction, &QAction::triggered, this, [this]() {
        showNormal();
        raise();
        activateWindow();
    });

    auto* chatAction = new QAction("채팅 열기", this);
    connect(chatAction, &QAction::triggered, this, [this]() {
        showNormal();
        raise();
        activateWindow();
        onChatBtnClicked();
    });

    auto* quitAction = new QAction("종료", this);
    connect(quitAction, &QAction::triggered, qApp, &QApplication::quit);

    m_trayMenu->addAction(showAction);
    m_trayMenu->addAction(chatAction);
    m_trayMenu->addSeparator();
    m_trayMenu->addAction(quitAction);

    m_trayIcon->setContextMenu(m_trayMenu);

    connect(m_trayIcon, &QSystemTrayIcon::activated, this,
            [this](QSystemTrayIcon::ActivationReason reason) {
        if (reason == QSystemTrayIcon::DoubleClick ||
            reason == QSystemTrayIcon::Trigger) {
            if (isHidden() || isMinimized()) {
                showNormal();
                raise();
                activateWindow();
            } else {
                hide();
            }
        }
    });

    m_trayIcon->show();
}

void MainWindow::updateTrayIcon() {
    if (!m_trayIcon) return;
    int today = QDate::currentDate().day();
    m_trayIcon->setIcon(makeCalendarIcon(today, m_unreadCount));
    if (m_unreadCount > 0)
        m_trayIcon->setToolTip(QString("캘린더 — %1  |  미읽음 %2").arg(m_myId).arg(m_unreadCount));
    else
        m_trayIcon->setToolTip("캘린더 — " + m_myId);
}

void MainWindow::closeEvent(QCloseEvent* event) {
    if (m_trayIcon && m_trayIcon->isVisible()) {
        hide();
        m_trayIcon->showMessage(
            "캘린더",
            "트레이에서 계속 실행 중입니다. 종료하려면 트레이 아이콘을 우클릭하세요.",
            QSystemTrayIcon::Information, 3000
        );
        event->ignore();
    } else {
        event->accept();
    }
}

void MainWindow::onError(QAbstractSocket::SocketError) {
    auto reply = QMessageBox::critical(
        this, "연결 오류",
        "서버와 연결할 수 없습니다.\n" + m_socket->errorString() +
            "\n\nServer.exe가 실행 중인지 확인 후 재시도하세요.",
        QMessageBox::Retry | QMessageBox::Cancel);
    if (reply == QMessageBox::Retry) reconnect();
}

void MainWindow::reconnect() {
    m_socket->abort();
    m_socket->connectToHost(m_serverIp, Protocol::PORT);
}
