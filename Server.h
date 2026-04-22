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
    //생성자
    explicit ScheduleServer(QObject *parent = nullptr);

    //서버 시작 함수
    bool startServer();

private slots:
    //클라이언트가 서ㅓ버에 접속하였을때 실행되는 슬롯
    void onNewConnection();

    //클라이언트로부터 데이터를 받았을떄 실행되는 슬롯
    void onReadyRead();

    //클라가 연결을 끊었을떄 실행되는 슬롯
    void onDisconnected();

private:
    //DB연결 및 테이블 초기화
    void initDatabase();
    void handleRequest(QTcpSocket* socket, const QString& data);

    void broadcastOnlineList();
    QStringList getCalMembers(int calId);
    void sendToCalMembers(int calId, const QString& msg, QTcpSocket* exclude = nullptr);

    //멤버변수
    QSqlDatabase               m_db;
    QMap<QTcpSocket*, QString> m_buffers;
    QList<QTcpSocket*>         m_clients;
    QMap<QTcpSocket*, QString> m_clientIds;


};

#endif
