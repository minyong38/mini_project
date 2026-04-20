#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QDate>
#include <QMap>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QCloseEvent>
#include "ScheduleDialog.h"
#include "ChatDialog.h"
#include "WeatherManager.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

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
    void setupTray();
    void updateTrayIcon();
    void applyTheme(bool dark);

    void closeEvent(QCloseEvent* event) override;

    QSystemTrayIcon* m_trayIcon   = nullptr;
    QMenu*           m_trayMenu   = nullptr;

    QPushButton*     m_themeBtn   = nullptr;
    bool             m_darkMode   = false;

    WeatherManager*  m_weather    = nullptr;
};

#endif
