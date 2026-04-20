#ifndef SERVER_H
#define SERVER_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QObject>
#include <QMap>
#include <QList>

class ScheduleServer : public QTcpServer {
    Q_OBJECT
public:
    explicit ScheduleServer(QObject *parent = nullptr);
    bool startServer();

private slots:
    void onNewConnection();
    void onReadyRead();
    void onDisconnected();

private:
    void initDatabase();
    void handleRequest(QTcpSocket* socket, const QString& data);
    void broadcastChat(qint64 rowid, int unread, const QString& userId,
                       const QString& timeStr, const QString& message);
    void broadcastReadRes(qint64 rowid, int count, QTcpSocket* exclude);
    void broadcastOnlineList();

    QSqlDatabase               m_db;
    QMap<QTcpSocket*, QString> m_buffers;
    QList<QTcpSocket*>         m_clients;
    QMap<QTcpSocket*, QString> m_clientIds;

    // 읽음 추적 (메모리, 서버 재시작 시 초기화)
    QMap<qint64, int>     m_unreadCount;    // rowid → 아직 안 읽은 사람 수
    QMap<QString, qint64> m_lastReadRowid;  // userId → 마지막으로 읽은 rowid
};

#endif
