#include "MainWindow.h"
#include "ui_mainwindow.h"
#include "Common.h"
#include "SharedCalDialog.h"
#include "MyPageDialog.h"
#include <QMessageBox>
#include <QFrame>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTimer>
#include <QCloseEvent>
#include <QAction>
#include <QPainter>
#include <QPainterPath>
#include <QTabWidget>
#include <QPixmap>
#include <QBuffer>

MainWindow::MainWindow(const QString& ip, const QString& myId,
                       const QString& password, QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow),
    m_myId(myId), m_password(password), m_selectedId(myId), m_serverIp(ip)
{
    ui->setupUi(this);
    setWindowTitle("캘린더 — " + m_myId);

    // 로고 (navBar 맨 왼쪽)
    auto* logoLabel = new QLabel(this);
    QPixmap logoPixmap(":/logo.png");
    if (!logoPixmap.isNull())
        logoLabel->setPixmap(logoPixmap.scaled(56, 56, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    auto* navLayout0 = qobject_cast<QHBoxLayout*>(ui->navBar->layout());
    if (navLayout0) navLayout0->insertWidget(0, logoLabel);

    m_selectedDate       = QDate::currentDate();
    m_pendingMonthUserId = m_myId;

    // onlineLabel 옆에 클릭 가능한 친구 버튼 컨테이너 추가
    auto* friendContainer = new QWidget(ui->onlineBar);
    friendContainer->setStyleSheet("background: transparent;");
    m_friendsLayout = new QHBoxLayout(friendContainer);
    m_friendsLayout->setContentsMargins(0, 0, 0, 0);
    m_friendsLayout->setSpacing(6);
    auto* onlineBarLayout = qobject_cast<QHBoxLayout*>(ui->onlineBar->layout());
    if (onlineBarLayout) onlineBarLayout->addWidget(friendContainer, 1);

    connect(ui->calendarWidget, &QCalendarWidget::clicked,
            this, &MainWindow::onDateClicked);
    connect(ui->calendarWidget, &QCalendarWidget::currentPageChanged,
            this, &MainWindow::requestMonthSchedules);

    // ── 탭 위젯으로 캘린더 카드 재구성 ──────────────────────
    m_calTabWidget = new QTabWidget(ui->calendarCard);
    m_calTabWidget->setDocumentMode(false);
    m_calTabWidget->setStyleSheet(R"(
        QTabWidget::pane { border: none; }
        QTabBar::tab { padding: 7px 16px; font-size: 13px;
                       border-radius: 8px 8px 0 0; min-width: 80px; }
        QTabBar::tab:selected { background: #007AFF; color: white; font-weight: 600; }
        QTabBar::tab:!selected { background: #F2F2F7; color: #1C1C1E; }
        QTabBar::tab:!selected:hover { background: #E5E5EA; }
    )");

    // 개인 캘린더 탭 (calendarWidget 옮기기)
    auto* personalPage   = new QWidget();
    auto* personalLayout = new QVBoxLayout(personalPage);
    personalLayout->setContentsMargins(0, 0, 0, 0);
    personalLayout->setSpacing(0);
    personalLayout->addWidget(ui->calendarWidget);
    m_calTabWidget->addTab(personalPage, "📅 내 캘린더");

    // cardLayout 에 탭 위젯 추가
    ui->cardLayout->addWidget(m_calTabWidget);

    // 초기 상태: 내 캘린더 탭 → 채팅 버튼 숨김
    ui->chatBtn->setVisible(false);

    m_calTabWidget->setTabsClosable(true);
    m_calTabWidget->tabBar()->setTabButton(0, QTabBar::RightSide, nullptr);


    connect(m_calTabWidget, &QTabWidget::currentChanged, this, [this](int index) {
        QString tabKey = m_calTabWidget->tabBar()->tabData(index).toString();

        if (index == 0) {
            // 내 캘린더
            m_activeCalId = -1;
            m_selectedId  = m_myId;
            ui->chatBtn->setVisible(false);
        } else if (tabKey.startsWith("S:")) {
            // 공유 캘린더
            m_activeCalId = tabKey.mid(2).toInt();
            m_selectedId  = m_myId;
            ui->chatBtn->setVisible(true);
            auto* w = m_sharedCalWidgets.value(m_activeCalId);
            if (w) requestSharedMonthSchedules(m_activeCalId, w->yearShown(), w->monthShown());
        } else if (tabKey.startsWith("F:")) {
            // 친구 캘린더
            QString friendId = tabKey.mid(2);
            m_activeCalId = -1;
            m_selectedId  = friendId;
            ui->chatBtn->setVisible(true);
            auto* w = m_friendCalWidgets.value(friendId);
            if (w && m_socket->state() == QAbstractSocket::ConnectedState) {
                m_pendingMonthUserId = friendId;
                QString ym = QString("%1-%2")
                                 .arg(w->yearShown()).arg(w->monthShown(), 2, 10, QChar('0'));
                m_socket->write((Protocol::REQMONTH + Protocol::SEP
                                 + friendId + Protocol::SEP + ym + "\n").toUtf8());
            }
        }
        updateChatBtnText();
    });

    connect(m_calTabWidget, &QTabWidget::tabCloseRequested, this, [this](int index) {
        QString tabKey = m_calTabWidget->tabBar()->tabData(index).toString();

        if (tabKey.startsWith("F:")) {
            // 친구 탭 — 그냥 닫기
            QString friendId = tabKey.mid(2);
            if (m_friendCalWidgets.contains(friendId)) {
                m_friendCalWidgets[friendId]->deleteLater();
                m_friendCalWidgets.remove(friendId);
            }
            m_friendMonthSchedules.remove(friendId);
            if (m_calTabWidget->currentIndex() == index)
                m_calTabWidget->setCurrentIndex(0);
            m_calTabWidget->removeTab(index);

        } else if (tabKey.startsWith("S:")) {
            // 공유 캘린더 탭 — 확인 후 삭제
            int calId = tabKey.mid(2).toInt();
            QString calName;
            for (const auto& cal : m_sharedCals)
                if (cal.id == calId) { calName = cal.name; break; }

            auto reply = QMessageBox::question(
                this,
                "공유 캘린더 삭제",
                QString("'%1' 공유 캘린더를 삭제 하시겠습니까?\n모든 일정과 채팅이 삭제됩니다.").arg(calName),
                QMessageBox::Yes | QMessageBox::No,
                QMessageBox::No
                );
            if (reply == QMessageBox::Yes && m_socket->state() == QAbstractSocket::ConnectedState)
                m_socket->write((Protocol::DELCAL + Protocol::SEP
                                 + QString::number(calId) + "\n").toUtf8());
        }
    });

    connect(ui->chatBtn,   &QPushButton::clicked, this, &MainWindow::onChatBtnClicked);
    connect(ui->myPageBtn, &QPushButton::clicked, this, &MainWindow::onMyPageBtnClicked);
    connect(ui->searchBtn, &QPushButton::clicked, this, &MainWindow::onSearchBtnClicked);

    // 다크모드 토글 버튼 (navBar에 삽입)
    m_themeBtn = new QPushButton("🌙", this);
    m_themeBtn->setFixedSize(36, 36);
    m_themeBtn->setToolTip("다크 모드 전환");
    m_themeBtn->setStyleSheet(
        "QPushButton { background:transparent; border:none; font-size:18px; border-radius:18px; }"
        "QPushButton:hover { background:#F2F2F7; }"
        );
    auto* navLayout = qobject_cast<QHBoxLayout*>(ui->navBar->layout());
    if (navLayout) navLayout->insertWidget(2, m_themeBtn);
    connect(m_themeBtn, &QPushButton::clicked, this, [this]() {
        applyTheme(!m_darkMode);
    });

    // 캘린더 추가 버튼 (채팅 버튼 왼쪽)
    auto* addCalBtn = new QPushButton("＋ 캘린더 추가", this);
    addCalBtn->setFixedHeight(36);
    addCalBtn->setCursor(Qt::PointingHandCursor);
    addCalBtn->setStyleSheet(
        "QPushButton { background:#34C759; color:white; border:none;"
        " border-radius:18px; padding:0 16px; font-size:13px; font-weight:600; }"
        "QPushButton:hover { background:#2DB84F; }"
        "QPushButton:pressed { background:#28A845; }"
        );
    if (navLayout) navLayout->insertWidget(4, addCalBtn);
    connect(addCalBtn, &QPushButton::clicked, this, &MainWindow::showAddCalendarDialog);

    setupTray();

    // 날씨 매니저 (서울 기준, 30분마다 갱신)
    m_weather = new WeatherManager("84befa7d5f3b34072813213012110a05", this);
    connect(m_weather, &WeatherManager::weatherUpdated, this, [this]() {
        ui->calendarWidget->setWeatherData(m_weather->data());
        for (auto* w : m_sharedCalWidgets)
            w->setWeatherData(m_weather->data());
        for (auto* w : m_friendCalWidgets)
            w->setWeatherData(m_weather->data());
    });
    m_weather->fetchWeather();
    auto* weatherTimer = new QTimer(this);
    connect(weatherTimer, &QTimer::timeout, this, [this]() {
        m_weather->fetchWeather();
    });
    weatherTimer->start(30 * 60 * 1000);

    // 소켓
    m_socket = new QTcpSocket(this);
    connect(m_socket, &QTcpSocket::connected,     this, &MainWindow::onConnectSuccess);
    connect(m_socket, &QTcpSocket::readyRead,     this, &MainWindow::onReadyRead);
    connect(m_socket, &QTcpSocket::errorOccurred, this, &MainWindow::onError);
    connect(m_socket, &QTcpSocket::disconnected,  this, [this]() {
        if (!m_reconnectTimer || !m_reconnectTimer->isActive())
            startReconnectTimer();
    });

    m_socket->connectToHost(m_serverIp, Protocol::PORT);
}

MainWindow::~MainWindow() { delete ui; }

// ──────────────────────────────────────────────
//  네트워크
// ──────────────────────────────────────────────

void MainWindow::onConnectSuccess() {
    stopReconnectTimer();
    setWindowTitle("캘린더 — " + m_myId);
    m_socket->write((Protocol::LOGIN + Protocol::SEP + m_myId
                     + Protocol::SEP + m_password + "\n").toUtf8());
}

void MainWindow::requestUsers() {
    if (m_socket->state() != QAbstractSocket::ConnectedState) return;
    m_socket->write((Protocol::REQUSERS + "\n").toUtf8());
}

void MainWindow::requestMonthSchedules(int year, int month) {
    if (m_socket->state() != QAbstractSocket::ConnectedState) return;
    m_pendingMonthUserId = m_selectedId;
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
        requestSharedCals();
        m_socket->write((Protocol::NICK_REQ    + Protocol::SEP + m_myId + "\n").toUtf8());
        m_socket->write((Protocol::PROFILE_REQ + Protocol::SEP + m_myId + "\n").toUtf8());
    }

    else if (data.startsWith(Protocol::PROFILE_RES + Protocol::SEP)) {
        QStringList p = data.split(Protocol::SEP);
        if (p.size() >= 2) {
            QString userId = p[1];
            QString base64 = p.size() >= 3 ? p.mid(2).join(Protocol::SEP) : QString();
            if (base64.isEmpty()) {
                m_profileCache.remove(userId);
            } else {
                QByteArray bytes = QByteArray::fromBase64(base64.toLatin1());
                QPixmap pix;
                pix.loadFromData(bytes, "JPEG");
                if (!pix.isNull())
                    m_profileCache[userId] = circularPixmap(pix, 36);
            }
            updateFriendsList();
        }
    }
    else if (data.startsWith(Protocol::NICK_RES + Protocol::SEP)) {
        QStringList p = data.split(Protocol::SEP);
        if (p.size() >= 3) {
            QString userId = p[1];
            QString nick   = p.mid(2).join(Protocol::SEP);
            m_nicknameCache[userId] = nick;
            if (userId == m_myId)
                m_nickname = nick;
            // 이미 열린 친구 탭 이름 갱신
            for (int i = 0; i < m_calTabWidget->count(); i++) {
                if (m_calTabWidget->tabBar()->tabData(i).toString() == "F:" + userId) {
                    m_calTabWidget->setTabText(i, "👤 " + nick);
                    break;
                }
            }
            updateFriendsList();
        }
    }
    else if (data.startsWith(Protocol::NICK_OK + Protocol::SEP)) {
        m_nickname = data.mid(Protocol::NICK_OK.length() + 1);
        m_nicknameCache[m_myId] = m_nickname;
    }
    else if (data.startsWith(Protocol::SEARCH_RES + Protocol::SEP)) {
        if (!m_searchDialog) return;
        QString payload = data.mid(Protocol::SEARCH_RES.length() + 1);
        QList<QPair<QString, QString>> results;
        if (!payload.isEmpty()) {
            for (const QString& entry : payload.split("|")) {
                int sep = entry.indexOf('~');
                if (sep > 0)
                    results.append({ entry.left(sep), entry.mid(sep + 1) });
            }
        }
        m_searchDialog->setResults(results);
    }
    else if (data == Protocol::LOGIN_FAIL) {
        QMessageBox::critical(this, "로그인 실패",
                              "ID 또는 비밀번호가 올바르지 않습니다.\n"
                              "회원가입 후 이용해주세요.");
        close();
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
        updateFriendsList();
    }

    // ── 유저 목록 ────────────────────────────────────────────
    else if (data.startsWith(Protocol::RESUSERS + Protocol::SEP)) {
        QString payload = data.mid(Protocol::RESUSERS.length() + Protocol::SEP.length());
        QStringList users = payload.isEmpty() ? QStringList() : payload.split("|");

        users.removeAll(m_myId);

        m_allKnownUsers = users;
        updateFriendsList();
    }

    // ── 월별 일정 ────────────────────────────────────────────
    else if (data.startsWith(Protocol::RESMONTH + Protocol::SEP)) {
        QMap<QDate, QStringList> sched;
        QString payload = data.mid(Protocol::RESMONTH.length() + Protocol::SEP.length());
        if (!payload.isEmpty()) {
            for (const QString& entry : payload.split("|")) {
                QStringList p = entry.split("@");
                if (p.size() < 3) continue;
                QDate d = QDate::fromString(p[0], "yyyy-MM-dd");
                if (d.isValid()) sched[d].append(p[2]);
            }
        }
        if (m_pendingMonthUserId.isEmpty() || m_pendingMonthUserId == m_myId) {
            m_monthSchedules = sched;
            ui->calendarWidget->setMonthSchedules(m_monthSchedules);
        } else {
            m_friendMonthSchedules[m_pendingMonthUserId] = sched;
            if (m_friendCalWidgets.contains(m_pendingMonthUserId))
                m_friendCalWidgets[m_pendingMonthUserId]->setMonthSchedules(sched);
        }
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

    // ── 1:1 DM 기록 수신 ─────────────────────────────────────
    else if (data.startsWith(Protocol::RESDM + Protocol::SEP)) {
        // RESDM:sender:HHmm:msg|sender:HHmm:msg|...
        if (m_pendingDmPeer.isEmpty()) return;
        QString peer = m_pendingDmPeer;
        m_pendingDmPeer.clear();

        if (!m_dmDialogs.contains(peer)) return;
        auto* dlg = m_dmDialogs[peer];

        QString payload = data.mid(Protocol::RESDM.length() + Protocol::SEP.length());
        if (!payload.isEmpty()) {
            for (const QString& entry : payload.split("|")) {
                QStringList p = entry.split(Protocol::SEP);
                if (p.size() < 3) continue;
                QString sender = p[0];
                QString rawT   = p[1];
                QString msg    = p.mid(2).join(Protocol::SEP);
                QString time   = rawT.length() == 4 ? rawT.left(2) + ":" + rawT.right(2) : "";
                dlg->appendMessage(0, 0, sender, msg, time);
            }
        }
    }

    // ── 1:1 DM 실시간 수신 ───────────────────────────────────
    else if (data.startsWith(Protocol::DMRES + Protocol::SEP)) {
        // DMRES:sender:receiver:HHmm:message
        QStringList p = data.split(Protocol::SEP);
        if (p.size() < 5) return;
        QString sender   = p[1];
        QString receiver = p[2];
        QString rawT     = p[3];
        QString msg      = p.mid(4).join(Protocol::SEP);
        QString time     = rawT.length() == 4 ? rawT.left(2) + ":" + rawT.right(2) : "";

        QString peer = (sender == m_myId) ? receiver : sender;

        // 다이얼로그가 없으면 생성 (상대방이 먼저 보낸 경우)
        if (!m_dmDialogs.contains(peer)) {
            auto* dlg = new ChatDialog(m_myId, this);
            dlg->setTitle("💬 " + peer);
            connect(dlg, &ChatDialog::messageSent, this, [this, peer](const QString& m) {
                if (m_socket->state() != QAbstractSocket::ConnectedState) return;
                m_socket->write((Protocol::DM + Protocol::SEP
                                 + m_myId + Protocol::SEP
                                 + peer   + Protocol::SEP
                                 + m + "\n").toUtf8());
            });
            m_dmDialogs[peer] = dlg;
        }

        auto* dlg = m_dmDialogs[peer];
        dlg->appendMessage(0, 0, sender, msg, time);

        if (!dlg->isVisible()) {
            m_dmUnreadCounts[peer] = m_dmUnreadCounts.value(peer, 0) + 1;
            if (m_selectedId == peer) updateChatBtnText();
            if (m_trayIcon && !isActiveWindow()) {
                m_lastNotifPeer  = peer;
                m_lastNotifCalId = -1;
                m_trayIcon->showMessage(
                    sender + "님의 메시지",
                    msg.startsWith("IMG:") ? "[이미지]" : msg,
                    QSystemTrayIcon::NoIcon, 3000
                    );
            }
        }
    }

    // ── 공유 캘린더 새 ID 알림 (CALID) ──────────────────────
    else if (data.startsWith(Protocol::CALID + Protocol::SEP)) {
        requestSharedCals();
    }

    // ── 공유 캘린더 삭제 알림 (CALREMOVED) ───────────────────
    else if (data.startsWith(Protocol::CALREMOVED + Protocol::SEP)) {
        int calId = data.mid(Protocol::CALREMOVED.length() + Protocol::SEP.length()).toInt();
        // 관련 채팅 다이얼로그 정리
        if (m_sharedChatDialogs.contains(calId)) {
            m_sharedChatDialogs[calId]->deleteLater();
            m_sharedChatDialogs.remove(calId);
        }
        m_sharedChatLoaded.remove(calId);
        m_sharedMonthSchedules.remove(calId);
        if (m_activeCalId == calId) {
            m_activeCalId = -1;
            m_calTabWidget->setCurrentIndex(0);
        }
        requestSharedCals();
    }

    // ── 공유 캘린더 목록 수신 ────────────────────────────────
    else if (data.startsWith(Protocol::RESCALS + Protocol::SEP)) {
        QString payload = data.mid(Protocol::RESCALS.length() + Protocol::SEP.length());
        m_sharedCals.clear();
        if (!payload.isEmpty()) {
            for (const QString& entry : payload.split("|")) {
                QStringList f = entry.split("~");
                if (f.size() < 4) continue;
                SharedCalInfo info;
                info.id      = f[0].toInt();
                info.name    = f[1];
                info.owner   = f[2];
                info.members = f[3].split(",", Qt::SkipEmptyParts);
                m_sharedCals.append(info);
            }
        }
        refreshSharedCalTabs();
    }

    // ── 공유 캘린더 월별 일정 수신 ───────────────────────────
    else if (data.startsWith(Protocol::RESSHMONTH + Protocol::SEP)) {
        QString payload = data.mid(Protocol::RESSHMONTH.length() + Protocol::SEP.length());
        int sep = payload.indexOf(Protocol::SEP);
        if (sep < 0) return;
        int calId = payload.left(sep).toInt();
        QString rest = payload.mid(sep + 1);

        auto& ms = m_sharedMonthSchedules[calId];
        ms.clear();
        if (!rest.isEmpty()) {
            for (const QString& e : rest.split("|")) {
                QStringList p = e.split("@");
                if (p.size() < 3) continue;
                QDate d = QDate::fromString(p[0], "yyyy-MM-dd");
                if (d.isValid()) ms[d].append(p[2]);
            }
        }
        if (m_sharedCalWidgets.contains(calId))
            m_sharedCalWidgets[calId]->setMonthSchedules(ms);
    }

    // ── 공유 캘린더 일별 일정 수신 ───────────────────────────
    else if (data.startsWith(Protocol::RESSHDAY + Protocol::SEP)) {
        QString payload = data.mid(Protocol::RESSHDAY.length() + Protocol::SEP.length());
        int sep = payload.indexOf(Protocol::SEP);
        if (sep < 0) return;
        int calId = payload.left(sep).toInt();
        QString rest = payload.mid(sep + 1);

        QList<qint64>  rowids;
        QStringList    contents;
        if (!rest.isEmpty()) {
            for (const QString& e : rest.split("|")) {
                int s = e.indexOf(Protocol::SEP);
                if (s < 0) continue;
                rowids   << e.left(s).toLongLong();
                contents << e.mid(s + 1);
            }
        }

        // 월별 캐시 업데이트
        m_sharedMonthSchedules[calId][m_pendingShDate] = contents;
        if (m_sharedCalWidgets.contains(calId))
            m_sharedCalWidgets[calId]->setMonthSchedules(m_sharedMonthSchedules[calId]);

        if (m_pendingShModal) {
            m_pendingShModal = false;
            showSharedDateDialog(calId, m_pendingShDate, rowids, contents);
        } else if (m_activeShDialog) {
            m_activeShDialog->refreshSchedules(contents, rowids);
        }
        m_pendingShCalId = -1;
    }

    // ── 공유 캘린더 일정 변경 알림 (SHUPDATE) ────────────────
    else if (data.startsWith(Protocol::SHUPDATE + Protocol::SEP)) {
        int calId = data.mid(Protocol::SHUPDATE.length() + Protocol::SEP.length()).toInt();
        if (m_sharedCalWidgets.contains(calId)) {
            auto* w = m_sharedCalWidgets[calId];
            requestSharedMonthSchedules(calId, w->yearShown(), w->monthShown());
        }
        // 다이얼로그가 열려 있으면 날짜 재조회
        if (m_activeShDialog && m_activeCalId == calId) {
            m_socket->write((Protocol::REQSHDAY + Protocol::SEP
                             + QString::number(calId) + Protocol::SEP
                             + m_pendingShDate.toString("yyyy-MM-dd") + "\n").toUtf8());
        }
    }

    // ── 공유 채팅 기록 수신 ──────────────────────────────────
    else if (data.startsWith(Protocol::RESSHCHAT + Protocol::SEP)) {
        QString payload = data.mid(Protocol::RESSHCHAT.length() + Protocol::SEP.length());
        int sep = payload.indexOf(Protocol::SEP);
        if (sep < 0) return;
        int calId = payload.left(sep).toInt();
        QString rest = payload.mid(sep + 1);

        if (!m_sharedChatDialogs.contains(calId)) return;
        auto* dlg = m_sharedChatDialogs[calId];
        dlg->clearMessages();
        if (!rest.isEmpty()) {
            for (const QString& e : rest.split("|")) {
                QStringList p = e.split(Protocol::SEP);
                if (p.size() < 3) continue;
                QString sender = p[0];
                QString rawT   = p[1];
                QString msg    = p.mid(2).join(Protocol::SEP);
                QString time   = rawT.length() == 4 ? rawT.left(2) + ":" + rawT.right(2) : "";
                dlg->appendMessage(0, 0, sender, msg, time);
            }
        }
    }

    // ── 공유 채팅 실시간 수신 ────────────────────────────────
    else if (data.startsWith(Protocol::SHCHATRES + Protocol::SEP)) {
        QString payload = data.mid(Protocol::SHCHATRES.length() + Protocol::SEP.length());
        QStringList p = payload.split(Protocol::SEP);
        if (p.size() < 5) return;
        int     calId  = p[0].toInt();
        // p[1]은 rowid (클라이언트 미사용)
        QString sender = p[2];
        QString rawT   = p[3];
        QString msg    = p.mid(4).join(Protocol::SEP);
        QString time   = rawT.length() == 4 ? rawT.left(2) + ":" + rawT.right(2) : "";

        if (!m_sharedChatDialogs.contains(calId)) {
            QString calName;
            for (const auto& cal : m_sharedCals) if (cal.id == calId) { calName = cal.name; break; }
            auto* dlg = new ChatDialog(m_myId, this);
            dlg->setTitle("🗓 " + calName + " 채팅");
            connect(dlg, &ChatDialog::messageSent, this, [this, calId](const QString& m) {
                if (m_socket->state() != QAbstractSocket::ConnectedState) return;
                m_socket->write((Protocol::SHCHAT + Protocol::SEP
                                 + QString::number(calId) + Protocol::SEP
                                 + m_myId + Protocol::SEP + m + "\n").toUtf8());
            });
            m_sharedChatDialogs[calId] = dlg;
        }

        auto* dlg = m_sharedChatDialogs[calId];
        dlg->appendMessage(0, 0, sender, msg, time);

        if (!dlg->isVisible() && m_trayIcon && !isActiveWindow()) {
            QString calName;
            for (const auto& cal : m_sharedCals) if (cal.id == calId) { calName = cal.name; break; }
            m_lastNotifPeer  = "";
            m_lastNotifCalId = calId;  // 공유 캘린더 알림으로 표시
            m_trayIcon->showMessage(
                sender + " (" + calName + ")",
                msg.startsWith("IMG:") ? "[이미지]" : msg,
                QSystemTrayIcon::NoIcon, 3000
                );
        }
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
    if (m_activeCalId != -1) {
        openSharedChat(m_activeCalId);
    } else if (m_selectedId != m_myId) {
        openDmChat(m_selectedId);
    }
}

void MainWindow::onSearchBtnClicked() {
    if (m_searchDialog) {
        m_searchDialog->raise();
        m_searchDialog->activateWindow();
        return;
    }
    m_searchDialog = new SearchDialog(this);
    m_searchDialog->setDarkMode(m_darkMode);

    connect(m_searchDialog, &SearchDialog::searchRequested, this, [this](const QString& keyword) {
        if (m_socket->state() != QAbstractSocket::ConnectedState) return;
        m_socket->write((Protocol::SEARCH_REQ + Protocol::SEP
                         + m_myId + Protocol::SEP + keyword + "\n").toUtf8());
    });
    connect(m_searchDialog, &SearchDialog::dateSelected, this, [this](const QDate& date) {
        // 내 캘린더 탭으로 이동 후 해당 날짜 선택
        m_calTabWidget->setCurrentIndex(0);
        ui->calendarWidget->setSelectedDate(date);
        ui->calendarWidget->showSelectedDate();
    });
    connect(m_searchDialog, &QDialog::finished, this, [this]() {
        m_searchDialog = nullptr;
    });

    m_searchDialog->show();
}

void MainWindow::onMyPageBtnClicked() {
    QPixmap currentPhoto;
    if (m_profileCache.contains(m_myId))
        currentPhoto = m_profileCache[m_myId];

    QString nickname = m_nickname.isEmpty() ? m_myId : m_nickname;
    MyPageDialog dlg(m_myId, currentPhoto, nickname, this);

    if (dlg.exec() != QDialog::Accepted) return;

    // 닉네임 변경
    QString newNick = dlg.newNickname();
    if (newNick != nickname && !newNick.isEmpty()) {
        m_socket->write((Protocol::NICK_UPDATE + Protocol::SEP
                         + m_myId + Protocol::SEP + newNick + "\n").toUtf8());
    }

    // 프로필 사진 삭제
    if (dlg.photoDeleted()) {
        m_profileCache.remove(m_myId);
        m_socket->write((Protocol::PROFILE_DELETE + Protocol::SEP + m_myId + "\n").toUtf8());
        updateFriendsList();
        return;
    }

    // 프로필 사진 변경
    if (dlg.photoChanged()) {
        QPixmap pix = dlg.newPhoto();
        QByteArray bytes;
        QBuffer buf(&bytes);
        buf.open(QIODevice::WriteOnly);
        pix.scaled(256, 256, Qt::KeepAspectRatio, Qt::SmoothTransformation)
            .save(&buf, "JPEG", 85);
        QString b64 = QString::fromLatin1(bytes.toBase64());
        m_socket->write((Protocol::PROFILE_UPLOAD + Protocol::SEP
                         + m_myId + Protocol::SEP + b64 + "\n").toUtf8());
        m_profileCache[m_myId] = circularPixmap(pix, 36);
        updateFriendsList();
    }
}

void MainWindow::openDmChat(const QString& peerId) {
    // 다이얼로그가 없으면 새로 생성
    if (!m_dmDialogs.contains(peerId)) {
        auto* dlg = new ChatDialog(m_myId, this);
        dlg->setTitle("💬 " + peerId);
        connect(dlg, &ChatDialog::messageSent, this, [this, peerId](const QString& msg) {
            if (m_socket->state() != QAbstractSocket::ConnectedState) return;
            m_socket->write((Protocol::DM + Protocol::SEP
                             + m_myId + Protocol::SEP
                             + peerId + Protocol::SEP
                             + msg + "\n").toUtf8());
        });
        m_dmDialogs[peerId] = dlg;
    }

    auto* dlg = m_dmDialogs[peerId];
    if (dlg->isVisible()) {
        dlg->hide();
    } else {
        // 최초 1회만 기록 요청 (이후엔 실시간 DMRES로 누적)
        if (!m_dmLoaded.contains(peerId) && m_socket->state() == QAbstractSocket::ConnectedState) {
            m_pendingDmPeer = peerId;
            m_socket->write((Protocol::REQDM + Protocol::SEP
                             + m_myId + Protocol::SEP
                             + peerId + "\n").toUtf8());
            m_dmLoaded.insert(peerId);
        }
        m_dmUnreadCounts[peerId] = 0;
        updateChatBtnText();
        dlg->show();
        dlg->raise();
    }
}


void MainWindow::updateChatBtnText() {
    if (m_activeCalId != -1) {
        ui->chatBtn->setText("💬 공유 채팅");
    } else {
        int dmUnread = m_dmUnreadCounts.value(m_selectedId, 0);
        if (dmUnread > 0)
            ui->chatBtn->setText(QString("💬 1:1 채팅 (%1)").arg(dmUnread));
        else
            ui->chatBtn->setText("💬 1:1 채팅");
    }
}

void MainWindow::openFriendTab(const QString& friendId) {
    // 이미 탭이 있으면 그 탭으로 전환
    for (int i = 0; i < m_calTabWidget->count(); i++) {
        if (m_calTabWidget->tabBar()->tabData(i).toString() == "F:" + friendId) {
            m_calTabWidget->setCurrentIndex(i);
            return;
        }
    }

    auto* page       = new QWidget();
    auto* pageLayout = new QVBoxLayout(page);
    pageLayout->setContentsMargins(0, 0, 0, 0);
    pageLayout->setSpacing(0);

    auto* calWidget = new CustomCalendarWidget(page);
    calWidget->setGridVisible(true);
    calWidget->setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);
    calWidget->setStyleSheet(ui->calendarWidget->styleSheet());
    calWidget->setDarkMode(m_darkMode);
    if (m_weather) calWidget->setWeatherData(m_weather->data());

    // 날짜 클릭 → 친구 일정 조회 (읽기 전용)
    connect(calWidget, &QCalendarWidget::clicked,
            this, &MainWindow::onDateClicked);
    // 월 이동 → 친구 월별 일정 요청
    connect(calWidget, &QCalendarWidget::currentPageChanged,
            this, &MainWindow::requestMonthSchedules);

    pageLayout->addWidget(calWidget);
    m_friendCalWidgets[friendId] = calWidget;

    QString displayName = m_nicknameCache.value(friendId, friendId);
    int tabIdx = m_calTabWidget->addTab(page, "👤 " + displayName);
    m_calTabWidget->tabBar()->setTabData(tabIdx, QString("F:") + friendId);

    m_calTabWidget->setCurrentIndex(tabIdx);
    // currentChanged 에서 m_selectedId = friendId 로 설정되고 REQMONTH 전송됨
}

void MainWindow::requestSharedCals() {
    if (m_socket->state() != QAbstractSocket::ConnectedState) return;
    m_socket->write((Protocol::REQCALS + Protocol::SEP + m_myId + "\n").toUtf8());
}

void MainWindow::requestSharedMonthSchedules(int calId, int year, int month) {
    if (m_socket->state() != QAbstractSocket::ConnectedState) return;
    QString ym = QString("%1-%2").arg(year).arg(month, 2, 10, QChar('0'));
    m_socket->write((Protocol::REQSHMONTH + Protocol::SEP
                     + QString::number(calId) + Protocol::SEP + ym + "\n").toUtf8());
}

void MainWindow::refreshSharedCalTabs() {
    // 기존 공유 탭 제거 (개인 탭 0번 제외)
    while (m_calTabWidget->count() > 1)
        m_calTabWidget->removeTab(1);

    for (auto* w : m_sharedCalWidgets) w->deleteLater();
    m_sharedCalWidgets.clear();

    QString calStyle = ui->calendarWidget->styleSheet();

    for (const SharedCalInfo& cal : m_sharedCals) {
        auto* page       = new QWidget();
        auto* pageLayout = new QVBoxLayout(page);
        pageLayout->setContentsMargins(0, 0, 0, 0);
        pageLayout->setSpacing(0);

        auto* calWidget = new CustomCalendarWidget(page);
        calWidget->setGridVisible(true);
        calWidget->setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);
        calWidget->setStyleSheet(calStyle);
        calWidget->setDarkMode(m_darkMode);
        if (m_weather) calWidget->setWeatherData(m_weather->data());

        const int calId = cal.id;
        connect(calWidget, &QCalendarWidget::clicked,
                this, [this, calId](const QDate& date) {
                    if (m_socket->state() != QAbstractSocket::ConnectedState) return;
                    m_selectedDate    = date;
                    m_pendingShCalId  = calId;
                    m_pendingShDate   = date;
                    m_pendingShModal  = true;
                    m_socket->write((Protocol::REQSHDAY + Protocol::SEP
                                     + QString::number(calId) + Protocol::SEP
                                     + date.toString("yyyy-MM-dd") + "\n").toUtf8());
                });

        connect(calWidget, &QCalendarWidget::currentPageChanged,
                this, [this, calId](int year, int month) {
                    requestSharedMonthSchedules(calId, year, month);
                });

        pageLayout->addWidget(calWidget);
        m_sharedCalWidgets[calId] = calWidget;

        int tabIdx = m_calTabWidget->addTab(page, "🗓 " + cal.name);
        m_calTabWidget->tabBar()->setTabData(tabIdx, QString("S:%1").arg(calId));
    }
}

void MainWindow::openSharedChat(int calId) {
    if (!m_sharedChatDialogs.contains(calId)) {
        QString calName;
        for (const auto& cal : m_sharedCals) if (cal.id == calId) { calName = cal.name; break; }
        auto* dlg = new ChatDialog(m_myId, this);
        dlg->setTitle("🗓 " + calName + " 채팅");
        dlg->setDarkMode(m_darkMode);
        connect(dlg, &ChatDialog::messageSent, this, [this, calId](const QString& m) {
            if (m_socket->state() != QAbstractSocket::ConnectedState) return;
            m_socket->write((Protocol::SHCHAT + Protocol::SEP
                             + QString::number(calId) + Protocol::SEP
                             + m_myId + Protocol::SEP + m + "\n").toUtf8());
        });
        m_sharedChatDialogs[calId] = dlg;
    }

    auto* dlg = m_sharedChatDialogs[calId];
    if (dlg->isVisible()) {
        dlg->hide();
    } else {
        if (!m_sharedChatLoaded.value(calId, false)
            && m_socket->state() == QAbstractSocket::ConnectedState) {
            m_socket->write((Protocol::REQSHCHAT + Protocol::SEP
                             + QString::number(calId) + "\n").toUtf8());
            m_sharedChatLoaded[calId] = true;
        }
        dlg->show();
        dlg->raise();
    }
}

void MainWindow::showAddCalendarDialog() {
    QStringList friends = m_allKnownUsers;
    friends.removeAll(m_myId);

    SharedCalDialog dlg(friends, this);
    if (dlg.exec() != QDialog::Accepted) return;

    QString name = dlg.calendarName();
    if (name.isEmpty()) {
        QMessageBox::warning(this, "캘린더 추가", "캘린더 이름을 입력해주세요.");
        return;
    }

    QStringList invited = dlg.selectedFriends();
    QString msg = Protocol::CREATECAL + Protocol::SEP
                  + m_myId + Protocol::SEP
                  + name   + Protocol::SEP
                  + invited.join("~") + "\n";
    m_socket->write(msg.toUtf8());
}

void MainWindow::showSharedDateDialog(int calId, const QDate& date,
                                      const QList<qint64>& rowids,
                                      const QStringList& contents)
{
    if (m_activeShDialog) {
        m_activeShDialog->close();
        return;
    }

    m_activeShDialog = new ScheduleDialog(date, this);
    m_activeShDialog->refreshSchedules(contents, rowids);
    m_pendingShDate = date;

    connect(m_activeShDialog, &ScheduleDialog::addRequested,
            this, [this, calId, date](const QString& content) {
                if (m_socket->state() != QAbstractSocket::ConnectedState) return;
                m_socket->write((Protocol::ADDSH + Protocol::SEP
                                 + QString::number(calId) + Protocol::SEP
                                 + m_myId + Protocol::SEP
                                 + date.toString("yyyy-MM-dd") + Protocol::SEP
                                 + content + "\n").toUtf8());
                m_pendingShCalId = calId;
                m_pendingShDate  = date;
                m_pendingShModal = false;
                m_socket->write((Protocol::REQSHDAY + Protocol::SEP
                                 + QString::number(calId) + Protocol::SEP
                                 + date.toString("yyyy-MM-dd") + "\n").toUtf8());
            });

    connect(m_activeShDialog, &ScheduleDialog::editRequested,
            this, [this, calId, date](qint64 rowid, const QString& newContent) {
                if (m_socket->state() != QAbstractSocket::ConnectedState) return;
                m_socket->write((Protocol::MODSH + Protocol::SEP
                                 + QString::number(calId) + Protocol::SEP
                                 + QString::number(rowid) + Protocol::SEP
                                 + newContent + "\n").toUtf8());
                m_pendingShCalId = calId;
                m_pendingShDate  = date;
                m_pendingShModal = false;
                m_socket->write((Protocol::REQSHDAY + Protocol::SEP
                                 + QString::number(calId) + Protocol::SEP
                                 + date.toString("yyyy-MM-dd") + "\n").toUtf8());
            });

    connect(m_activeShDialog, &ScheduleDialog::deleteRequested,
            this, [this, calId, date](qint64 rowid) {
                if (m_socket->state() != QAbstractSocket::ConnectedState) return;
                m_socket->write((Protocol::DELSH + Protocol::SEP
                                 + QString::number(calId) + Protocol::SEP
                                 + QString::number(rowid) + "\n").toUtf8());
                m_pendingShCalId = calId;
                m_pendingShDate  = date;
                m_pendingShModal = false;
                m_socket->write((Protocol::REQSHDAY + Protocol::SEP
                                 + QString::number(calId) + Protocol::SEP
                                 + date.toString("yyyy-MM-dd") + "\n").toUtf8());
            });

    connect(m_activeShDialog, &QDialog::finished, this, [this]() {
        m_activeShDialog->deleteLater();
        m_activeShDialog = nullptr;
    });

    m_activeShDialog->open();
}

void MainWindow::updateFriendsList() {
    // 기존 버튼 전부 제거
    while (m_friendsLayout->count() > 0) {
        QLayoutItem* item = m_friendsLayout->takeAt(0);
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }

    QStringList known = m_allKnownUsers;
    for (const QString& u : m_onlineUsers)
        if (!known.contains(u)) known << u;

    // 본인 제외, 온라인/오프라인 분리
    QStringList onlineFriends, offlineFriends;
    for (const QString& u : known) {
        if (u == m_myId) continue;
        if (m_onlineUsers.contains(u)) onlineFriends << u;
        else                            offlineFriends << u;
    }

    int friendOnlineCount = onlineFriends.size();
    ui->onlineLabel->setText(QString("👥  %1명 접속 중  |").arg(friendOnlineCount));

    // 프로필 없는 유저는 요청
    QStringList allFriends = onlineFriends + offlineFriends;
    for (const QString& u : allFriends)
        if (!m_profileCache.contains(u))
            requestProfile(u);

    // 친구 위젯 생성 람다 (아바타 + 이름)
    auto addBtn = [this](const QString& userId, bool online) {
        auto* container = new QWidget();
        container->setCursor(Qt::PointingHandCursor);
        auto* hbox = new QHBoxLayout(container);
        hbox->setContentsMargins(6, 2, 10, 2);
        hbox->setSpacing(6);

        // 동그란 프로필 사진
        auto* avatar = new QLabel();
        avatar->setFixedSize(28, 28);
        if (m_profileCache.contains(userId)) {
            avatar->setPixmap(circularPixmap(m_profileCache[userId], 28));
        } else {
            // 기본 아바타: 이니셜
            QPixmap def(28, 28);
            def.fill(Qt::transparent);
            QPainter p(&def);
            p.setRenderHint(QPainter::Antialiasing);
            p.setBrush(online ? QColor("#1A237E") : QColor("#9E9E9E"));
            p.setPen(Qt::NoPen);
            p.drawEllipse(0, 0, 28, 28);
            p.setPen(Qt::white);
            p.setFont(QFont("Arial", 11, QFont::Bold));
            p.drawText(def.rect(), Qt::AlignCenter, userId.left(1).toUpper());
            avatar->setPixmap(def);
        }

        // 닉네임이 있으면 닉네임 표시, 없으면 userId
        if (!m_nicknameCache.contains(userId))
            m_socket->write((Protocol::NICK_REQ + Protocol::SEP + userId + "\n").toUtf8());
        QString displayName = m_nicknameCache.value(userId, userId);
        QString dot = online ? "🟢" : "⚫";
        auto* nameLabel = new QLabel(dot + " " + displayName);
        nameLabel->setStyleSheet(online
                                     ? "font-size:12px; color:#2D6A4F; font-weight:600;"
                                     : "font-size:12px; color:#8E8E93;");

        hbox->addWidget(avatar);
        hbox->addWidget(nameLabel);

        QString borderColor = online ? "#C3E6CB" : "#E0E0E0";
        QString bgColor     = online ? "#E8F5E9" : "#F5F5F5";
        QString hoverColor  = online ? "#C8E6C9" : "#EEEEEE";
        container->setStyleSheet(QString(
                                     "QWidget { background:%1; border:1px solid %2; border-radius:14px; }"
                                     "QWidget:hover { background:%3; }").arg(bgColor, borderColor, hoverColor));

        container->installEventFilter(this);
        container->setProperty("userId", userId);

        // 클릭 이벤트를 위해 마우스 press를 감지
        connect(new QObject(container), &QObject::destroyed, this, []{});
        auto* clickFilter = new QObject(container);
        container->installEventFilter(clickFilter);

        // QPushButton을 투명하게 위에 덮어서 클릭 처리
        auto* clickBtn = new QPushButton(container);
        clickBtn->setFlat(true);
        clickBtn->setStyleSheet("QPushButton{background:transparent;border:none;}");
        clickBtn->setGeometry(0, 0, 300, 40);
        clickBtn->raise();
        connect(clickBtn, &QPushButton::clicked, this, [this, userId]() {
            openFriendTab(userId);
        });

        m_friendsLayout->addWidget(container);
    };

    for (const QString& u : onlineFriends)  addBtn(u, true);
    for (const QString& u : offlineFriends) addBtn(u, false);
    m_friendsLayout->addStretch();
}

// ──────────────────────────────────────────────
//  UI 이벤트
// ──────────────────────────────────────────────

void MainWindow::onDateClicked(const QDate& date) {
    m_selectedDate = date;
    m_pendingModal = true;
    requestSchedules(date);
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
        ui->onlineBar->setStyleSheet(
            "QFrame#onlineBar { background:#0D2318; border-bottom:1px solid #1A4230; }");
        ui->onlineLabel->setStyleSheet(
            "QLabel { font-size:12px; color:#4CAF7D; background:transparent; border:none; }");
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
        ui->onlineBar->setStyleSheet(
            "QFrame#onlineBar { background:#F0FFF4; border-bottom:1px solid #C3E6CB; }");
        ui->onlineLabel->setStyleSheet(
            "QLabel { font-size:12px; color:#2D6A4F; background:transparent; border:none; }");
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

    updateFriendsList();  // 친구 버튼 색상 갱신

    ui->calendarWidget->setDarkMode(dark);
    for (auto* dlg : m_dmDialogs)
        dlg->setDarkMode(dark);
    for (auto* dlg : m_sharedChatDialogs)
        dlg->setDarkMode(dark);

    // 탭 위젯 스타일
    if (m_calTabWidget) {
        if (dark) {
            m_calTabWidget->setStyleSheet(R"(
                QTabWidget::pane { border: none; }
                QTabBar::tab { padding: 7px 16px; font-size: 13px;
                               border-radius: 8px 8px 0 0; min-width: 80px; }
                QTabBar::tab:selected { background: #0A84FF; color: white; font-weight: 600; }
                QTabBar::tab:!selected { background: #3A3A3C; color: #EBEBF5; }
                QTabBar::tab:!selected:hover { background: #48484A; }
            )");
        } else {
            m_calTabWidget->setStyleSheet(R"(
                QTabWidget::pane { border: none; }
                QTabBar::tab { padding: 7px 16px; font-size: 13px;
                               border-radius: 8px 8px 0 0; min-width: 80px; }
                QTabBar::tab:selected { background: #007AFF; color: white; font-weight: 600; }
                QTabBar::tab:!selected { background: #F2F2F7; color: #1C1C1E; }
                QTabBar::tab:!selected:hover { background: #E5E5EA; }
            )");
        }
    }

    // 공유/친구 캘린더 위젯 스타일 적용
    for (auto* w : m_sharedCalWidgets) {
        w->setStyleSheet(ui->calendarWidget->styleSheet());
        w->setDarkMode(dark);
    }
    for (auto* w : m_friendCalWidgets) {
        w->setStyleSheet(ui->calendarWidget->styleSheet());
        w->setDarkMode(dark);
    }
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

    auto* quitAction = new QAction("종료", this);
    connect(quitAction, &QAction::triggered, qApp, &QApplication::quit);

    m_trayMenu->addAction(showAction);
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

    // 알림 클릭 → 해당 채팅 바로 열기
    connect(m_trayIcon, &QSystemTrayIcon::messageClicked, this, [this]() {
        showNormal();
        raise();
        activateWindow();

        if (m_lastNotifCalId != -1) {
            openSharedChat(m_lastNotifCalId);
            m_lastNotifCalId = -1;
        } else if (!m_lastNotifPeer.isEmpty()) {
            openDmChat(m_lastNotifPeer);
        }
    });

    m_trayIcon->show();
}

void MainWindow::updateTrayIcon() {
    if (!m_trayIcon) return;
    m_trayIcon->setIcon(makeCalendarIcon(QDate::currentDate().day()));
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

void MainWindow::onError(QAbstractSocket::SocketError err) {
    // RemoteHostClosed는 정상 종료 — 재연결 시도
    // 이미 재연결 타이머가 돌고 있으면 무시
    if (m_reconnectTimer && m_reconnectTimer->isActive()) return;
    startReconnectTimer();
}

void MainWindow::reconnect() {
    m_socket->abort();
    setWindowTitle(QString("캘린더 — %1  |  재연결 중...").arg(m_myId));
    m_socket->connectToHost(m_serverIp, Protocol::PORT);
}

void MainWindow::startReconnectTimer() {
    if (!m_reconnectTimer) {
        m_reconnectTimer = new QTimer(this);
        m_reconnectTimer->setInterval(5000);
        connect(m_reconnectTimer, &QTimer::timeout, this, [this]() {
            if (m_socket->state() == QAbstractSocket::UnconnectedState)
                reconnect();
        });
    }
    m_reconnectTimer->start();  // reconnect() 전에 먼저 시작해야 abort()의 disconnected 재귀 방지
    reconnect();
}

void MainWindow::stopReconnectTimer() {
    if (m_reconnectTimer) m_reconnectTimer->stop();
}

void MainWindow::requestProfile(const QString& userId) {
    if (m_socket->state() != QAbstractSocket::ConnectedState) return;
    m_socket->write((Protocol::PROFILE_REQ + Protocol::SEP + userId + "\n").toUtf8());
}

QPixmap MainWindow::circularPixmap(const QPixmap& src, int size) {
    QPixmap scaled = src.scaled(size, size, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
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
