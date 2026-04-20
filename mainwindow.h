#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QDate>
#include <QMap>
#include <QSet>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QCloseEvent>
#include "ScheduleDialog.h"
#include "ChatDialog.h"
#include "WeatherManager.h"
#include "SharedCalDialog.h"
#include "CustomCalendarWidget.h"
#include <QTabWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

struct SharedCalInfo {
    int         id;
    QString     name;
    QString     owner;
    QStringList members;
};

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(const QString& ip, const QString& myId, QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onConnectSuccess();
    void onReadyRead();
    void onDateClicked(const QDate& date);
    void onUserChanged(int index);
    void onError(QAbstractSocket::SocketError socketError);
    void reconnect();

    void onDialogAdd(const QString& content);
    void onDialogEdit(qint64 rowid, const QString& newContent);
    void onDialogDelete(qint64 rowid);

    void onChatBtnClicked();
    void onChatMessageSent(const QString& message);

private:
    void requestSchedules(const QDate& date);
    void requestMonthSchedules(int year, int month);
    void requestUsers();
    void showDateDialog();
    void processMessage(const QString& data);
    void updateChatBtnText();

    Ui::MainWindow*  ui;
    QTcpSocket*      m_socket;

    QString          m_myId;
    QString          m_selectedId;
    QString          m_serverIp;

    QDate            m_selectedDate;
    bool             m_pendingModal = false;

    QList<qint64>    m_currentRowids;
    QStringList      m_currentContents;
    QMap<QDate, QStringList> m_monthSchedules;

    ScheduleDialog*  m_activeDialog = nullptr;
    ChatDialog*      m_chatDialog   = nullptr;
    QString          m_buffer;

    int              m_unreadCount  = 0;

    QStringList      m_onlineUsers;
    QStringList      m_allKnownUsers;
    bool             m_initialOnlineReceived = false;

    void showJoinNotification(const QString& userId);
    void updateFriendsList();
    void openDmChat(const QString& peerId);
    void setupTray();
    void updateTrayIcon();
    void applyTheme(bool dark);

    // 공유 캘린더
    void requestSharedCals();
    void refreshSharedCalTabs();
    void requestSharedMonthSchedules(int calId, int year, int month);
    void openSharedChat(int calId);
    void showAddCalendarDialog();
    void showSharedDateDialog(int calId, const QDate& date,
                              const QList<qint64>& rowids,
                              const QStringList& contents);

    void closeEvent(QCloseEvent* event) override;

    QSystemTrayIcon* m_trayIcon   = nullptr;
    QMenu*           m_trayMenu   = nullptr;

    QPushButton*     m_themeBtn   = nullptr;
    bool             m_darkMode   = false;

    WeatherManager*  m_weather    = nullptr;

    QMap<QString, ChatDialog*> m_dmDialogs;
    QMap<QString, int>         m_dmUnreadCounts;
    QString                    m_pendingDmPeer;

    bool          m_groupChatLoaded = false;
    QSet<QString> m_dmLoaded;
    QString       m_lastNotifPeer;

    // 공유 캘린더 상태
    int                                     m_activeCalId    = -1;
    QList<SharedCalInfo>                    m_sharedCals;
    QTabWidget*                             m_calTabWidget   = nullptr;
    QMap<int, CustomCalendarWidget*>        m_sharedCalWidgets;
    QMap<int, QMap<QDate, QStringList>>     m_sharedMonthSchedules;
    QMap<int, ChatDialog*>                  m_sharedChatDialogs;
    QMap<int, bool>                         m_sharedChatLoaded;
    int             m_pendingShCalId  = -1;
    QDate           m_pendingShDate;
    bool            m_pendingShModal  = false;
    ScheduleDialog* m_activeShDialog  = nullptr;
};

#endif
