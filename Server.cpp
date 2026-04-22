#include "Server.h"
#include "Common.h"
#include <QSqlError>
#include <QDateTime>
#include <QDebug>
#include <QCoreApplication>

ScheduleServer::ScheduleServer(QObject *parent) : QTcpServer(parent) {
    initDatabase();
}

void ScheduleServer::initDatabase() {
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(QCoreApplication::applicationDirPath() + "/schedule.db");
    if (!m_db.open()) { qCritical() << "DB Error:" << m_db.lastError().text(); return; }

    QSqlQuery q;
    q.exec("CREATE TABLE IF NOT EXISTS users "
           "(USER_ID TEXT PRIMARY KEY, PASSWORD TEXT)");
    q.exec("CREATE TABLE IF NOT EXISTS profiles "
           "(USER_ID TEXT PRIMARY KEY, PHOTO TEXT)");
    q.exec("CREATE TABLE IF NOT EXISTS nicknames "
           "(USER_ID TEXT PRIMARY KEY, NICKNAME TEXT)");
    q.exec("CREATE TABLE IF NOT EXISTS schedules (USER_ID TEXT, DATE TEXT, CONTENT TEXT)");
    q.exec("CREATE TABLE IF NOT EXISTS dm_chats "
           "(SENDER TEXT, RECEIVER TEXT, MESSAGE TEXT, SEND_TIME TEXT)");
    q.exec("CREATE TABLE IF NOT EXISTS shared_calendars "
           "(ID INTEGER PRIMARY KEY AUTOINCREMENT, OWNER TEXT, NAME TEXT)");
    q.exec("CREATE TABLE IF NOT EXISTS shared_members "
           "(CAL_ID INTEGER, USER_ID TEXT)");
    q.exec("CREATE TABLE IF NOT EXISTS shared_schedules "
           "(CAL_ID INTEGER, USER_ID TEXT, DATE TEXT, CONTENT TEXT)");
    q.exec("CREATE TABLE IF NOT EXISTS shared_chats "
           "(CAL_ID INTEGER, SENDER TEXT, MESSAGE TEXT, SEND_TIME TEXT)");
    qDebug() << "Database initialized.";
}

bool ScheduleServer::startServer() {
    connect(this, &QTcpServer::newConnection, this, &ScheduleServer::onNewConnection);
    bool ok = listen(QHostAddress::Any, Protocol::PORT);
    if (ok) qDebug() << "Server started on port" << Protocol::PORT;
    return ok;
}

void ScheduleServer::onNewConnection() {
    while (hasPendingConnections()) {
        QTcpSocket* s = nextPendingConnection();
        connect(s, &QTcpSocket::readyRead,    this, &ScheduleServer::onReadyRead);
        connect(s, &QTcpSocket::disconnected, this, &ScheduleServer::onDisconnected);
        m_clients.append(s);
        qDebug() << "New connection:" << s->peerAddress().toString();
    }
}

void ScheduleServer::onReadyRead() {
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;
    // 이미지 같은 큰 메시지는 여러 번에 걸쳐 도착할 수 있어서 버퍼에 쌓아두고 처리
    m_buffers[socket] += QString::fromUtf8(socket->readAll());
    while (m_buffers[socket].contains('\n')) {
        int idx = m_buffers[socket].indexOf('\n');
        QString line = m_buffers[socket].left(idx).trimmed();
        m_buffers[socket] = m_buffers[socket].mid(idx + 1);
        if (!line.isEmpty()) handleRequest(socket, line);
    }
}

void ScheduleServer::handleRequest(QTcpSocket* socket, const QString& data) {
    QStringList parts = data.split(Protocol::SEP);
    if (parts.isEmpty()) return;
    const QString cmd = parts[0];

    if (cmd == Protocol::PROFILE_UPLOAD && parts.size() >= 3) {
        QString userId = parts[1];
        QString base64 = parts.mid(2).join(Protocol::SEP); // base64에 ':' 포함될 수 있어서 join으로 재조립
        QSqlQuery q;
        q.prepare("INSERT OR REPLACE INTO profiles (USER_ID, PHOTO) VALUES(?,?)");
        q.addBindValue(userId); q.addBindValue(base64);
        if (q.exec()) {
            socket->write((Protocol::PROFILE_OK + "\n").toUtf8());
            // 저장 후 다른 접속자들에게 바로 전파해서 실시간으로 프로필 갱신되게 함
            QString broadcast = Protocol::PROFILE_RES + Protocol::SEP
                                + userId + Protocol::SEP + base64 + "\n";
            for (QTcpSocket* s : m_clients) {
                if (s && s != socket && s->state() == QAbstractSocket::ConnectedState)
                    s->write(broadcast.toUtf8());
            }
        } else {
            socket->write(QString("PROFILE_ERR\n").toUtf8());
        }
        return;
    }

    else if (cmd == Protocol::PROFILE_REQ && parts.size() >= 2) {
        QString userId = parts[1];
        QSqlQuery q;
        q.prepare("SELECT PHOTO FROM profiles WHERE USER_ID=?");
        q.addBindValue(userId);
        q.exec();
        if (q.next()) {
            socket->write((Protocol::PROFILE_RES + Protocol::SEP
                           + userId + Protocol::SEP + q.value(0).toString() + "\n").toUtf8());
        }
        return;
    }

    else if (cmd == Protocol::PROFILE_DELETE && parts.size() >= 2) {
        QString userId = parts[1];
        QSqlQuery q;
        q.prepare("DELETE FROM profiles WHERE USER_ID=?");
        q.addBindValue(userId);
        if (q.exec()) {
            socket->write((Protocol::PROFILE_OK + "\n").toUtf8());
            QString broadcast = Protocol::PROFILE_RES + Protocol::SEP
                                + userId + Protocol::SEP + "\n";
            for (QTcpSocket* s : m_clients) {
                if (s && s != socket && s->state() == QAbstractSocket::ConnectedState)
                    s->write(broadcast.toUtf8());
            }
        } else {
            socket->write(QString("PROFILE_ERR\n").toUtf8());
        }
        return;
    }

    else if (cmd == Protocol::NICK_UPDATE && parts.size() >= 3) {
        QString userId   = parts[1];
        QString nickname = parts.mid(2).join(Protocol::SEP);
        QSqlQuery q;
        q.prepare("INSERT OR REPLACE INTO nicknames (USER_ID, NICKNAME) VALUES(?,?)");
        q.addBindValue(userId); q.addBindValue(nickname);
        if (q.exec()) {
            socket->write((Protocol::NICK_OK + Protocol::SEP + nickname + "\n").toUtf8());
            QString broadcast = Protocol::NICK_RES + Protocol::SEP
                                + userId + Protocol::SEP + nickname + "\n";
            for (QTcpSocket* s : m_clients) {
                if (s && s != socket && s->state() == QAbstractSocket::ConnectedState)
                    s->write(broadcast.toUtf8());
            }
        } else {
            socket->write(QString("NICK_FAIL:서버 오류\n").toUtf8());
        }
        return;
    }

    else if (cmd == Protocol::NICK_REQ && parts.size() >= 2) {
        QString userId = parts[1];
        QSqlQuery q;
        q.prepare("SELECT NICKNAME FROM nicknames WHERE USER_ID=?");
        q.addBindValue(userId);
        q.exec();
        QString nick = q.next() ? q.value(0).toString() : userId;
        socket->write((Protocol::NICK_RES + Protocol::SEP
                       + userId + Protocol::SEP + nick + "\n").toUtf8());
        return;
    }

    else if (cmd == Protocol::SEARCH_REQ && parts.size() >= 3) {
        QString userId  = parts[1];
        QString keyword = parts.mid(2).join(Protocol::SEP);
        QStringList entries;

        QSqlQuery q;
        q.prepare("SELECT DATE, CONTENT FROM schedules "
                  "WHERE USER_ID=? AND CONTENT LIKE ? ORDER BY DATE ASC");
        q.addBindValue(userId);
        q.addBindValue("%" + keyword + "%");
        q.exec();
        while (q.next())
            entries << "0\t내 캘린더\t" + q.value(0).toString() + "\t" + q.value(1).toString();

        QSqlQuery q2;
        q2.prepare("SELECT ss.DATE, ss.CONTENT, sc.ID, sc.NAME "
                   "FROM shared_schedules ss "
                   "JOIN shared_calendars sc ON ss.CAL_ID = sc.ID "
                   "WHERE ss.CAL_ID IN (SELECT CAL_ID FROM shared_members WHERE USER_ID=?) "
                   "AND ss.CONTENT LIKE ? ORDER BY ss.DATE ASC");
        q2.addBindValue(userId);
        q2.addBindValue("%" + keyword + "%");
        q2.exec();
        while (q2.next())
            entries << q2.value(2).toString() + "\t" + q2.value(3).toString()
                       + "\t" + q2.value(0).toString() + "\t" + q2.value(1).toString();

        socket->write((Protocol::SEARCH_RES + Protocol::SEP
                       + entries.join("|") + "\n").toUtf8());
        return;
    }

    else if (cmd == Protocol::SIGNUP && parts.size() >= 3) {
        QString userId   = parts[1];
        QString password = parts[2];
        QSqlQuery q;
        q.prepare("SELECT USER_ID FROM users WHERE USER_ID=?");
        q.addBindValue(userId);
        q.exec();
        if (q.next()) {
            socket->write((Protocol::SIGNUP_FAIL + ":이미 존재하는 ID입니다\n").toUtf8());
        } else {
            q.prepare("INSERT INTO users (USER_ID, PASSWORD) VALUES(?,?)");
            q.addBindValue(userId); q.addBindValue(password);
            socket->write(q.exec()
                              ? (Protocol::SIGNUP_OK + "\n").toUtf8()
                              : (Protocol::SIGNUP_FAIL + ":서버 오류\n").toUtf8());
        }
        return;
    }

    else if (cmd == Protocol::LOGIN && parts.size() >= 2) {
        QString userId   = parts[1];
        QString password = (parts.size() >= 3) ? parts[2] : "";

        if (m_clientIds.values().contains(userId)) {
            socket->write((Protocol::LOGIN_REJECT + "\n").toUtf8());
            return;
        }

        if (password.isEmpty()) {
            // Google 로그인: users 테이블에 없으면 자동 등록
            QSqlQuery q;
            q.prepare("INSERT OR IGNORE INTO users (USER_ID, PASSWORD) VALUES(?,?)");
            q.addBindValue(userId); q.addBindValue("");
            q.exec();
        } else {
            // 일반 로그인: 비밀번호 확인
            QSqlQuery q;
            q.prepare("SELECT USER_ID FROM users WHERE USER_ID=? AND PASSWORD=?");
            q.addBindValue(userId); q.addBindValue(password);
            q.exec();
            if (!q.next()) {
                socket->write((Protocol::LOGIN_FAIL + "\n").toUtf8());
                return;
            }
        }

        m_clientIds[socket] = userId;
        socket->write((Protocol::LOGIN_OK + "\n").toUtf8());
        broadcastOnlineList();
    }

    else if (cmd == Protocol::REQ && parts.size() >= 3) {
        QSqlQuery q;
        q.prepare("SELECT rowid, CONTENT FROM schedules WHERE USER_ID=? AND DATE=?");
        q.addBindValue(parts[1]); q.addBindValue(parts[2]);
        if (q.exec()) {
            QStringList res;
            while (q.next()) res << q.value(0).toString() + Protocol::SEP + q.value(1).toString();
            socket->write((Protocol::RES + Protocol::SEP + res.join("|") + "\n").toUtf8());
        }
    }

    else if (cmd == Protocol::ADD && parts.size() >= 4) {
        QSqlQuery q;
        q.prepare("INSERT INTO schedules (USER_ID,DATE,CONTENT) VALUES(?,?,?)");
        q.addBindValue(parts[1]); q.addBindValue(parts[2]);
        q.addBindValue(parts.mid(3).join(Protocol::SEP));
        socket->write(q.exec()
                          ? (Protocol::ACK + ":OK\n").toUtf8()
                          : (Protocol::ACK + ":ERR\n").toUtf8());
    }

    else if (cmd == Protocol::DEL && parts.size() >= 2) {
        QSqlQuery q;
        q.prepare("DELETE FROM schedules WHERE rowid=?");
        q.addBindValue(parts[1].toLongLong());
        socket->write(q.exec()
                          ? (Protocol::ACK + ":OK\n").toUtf8()
                          : (Protocol::ACK + ":ERR\n").toUtf8());
    }

    else if (cmd == Protocol::MOD && parts.size() >= 3) {
        QSqlQuery q;
        q.prepare("UPDATE schedules SET CONTENT=? WHERE rowid=?");
        q.addBindValue(parts.mid(2).join(Protocol::SEP));
        q.addBindValue(parts[1].toLongLong());
        socket->write(q.exec()
                          ? (Protocol::ACK + ":OK\n").toUtf8()
                          : (Protocol::ACK + ":ERR\n").toUtf8());
    }

    else if (cmd == Protocol::REQMONTH && parts.size() >= 3) {
        QSqlQuery q;
        q.prepare("SELECT rowid,DATE,CONTENT FROM schedules WHERE USER_ID=? AND DATE LIKE ?");
        q.addBindValue(parts[1]); q.addBindValue(parts[2] + "%");
        if (q.exec()) {
            QStringList res;
            while (q.next())
                res << q.value(1).toString() + "@" + q.value(0).toString() + "@" + q.value(2).toString();
            socket->write((Protocol::RESMONTH + Protocol::SEP + res.join("|") + "\n").toUtf8());
        }
    }

    else if (cmd == Protocol::REQUSERS) {
        QSqlQuery q;
        q.exec("SELECT DISTINCT USER_ID FROM schedules ORDER BY USER_ID");
        QStringList users;
        while (q.next()) users << q.value(0).toString();
        // 현재 접속 중인 유저도 포함 (일정이 없어도 보기 목록에 표시)
        for (const QString& u : m_clientIds.values())
            if (!users.contains(u)) users << u;
        users.sort();
        socket->write((Protocol::RESUSERS + Protocol::SEP + users.join("|") + "\n").toUtf8());
    }

    else if (cmd == Protocol::DM && parts.size() >= 4) {
        QString sender   = parts[1];
        QString receiver = parts[2];
        QString message  = parts.mid(3).join(Protocol::SEP);
        QString timeStr  = QDateTime::currentDateTime().toString("MM/dd HH.mm");

        QSqlQuery q;
        q.prepare("INSERT INTO dm_chats (SENDER,RECEIVER,MESSAGE,SEND_TIME) VALUES(?,?,?,?)");
        q.addBindValue(sender); q.addBindValue(receiver);
        q.addBindValue(message); q.addBindValue(timeStr);
        if (!q.exec()) { qWarning() << "DM 저장 실패:" << q.lastError().text(); return; }

        // sender 와 receiver 에게만 전달
        QString msg = Protocol::DMRES + Protocol::SEP
                      + sender   + Protocol::SEP
                      + receiver + Protocol::SEP
                      + timeStr  + Protocol::SEP
                      + message  + "\n";
        for (QTcpSocket* s : m_clients) {
            QString uid = m_clientIds.value(s);
            if (uid == sender || uid == receiver)
                s->write(msg.toUtf8());
        }
    }

    else if (cmd == Protocol::REQDM && parts.size() >= 3) {
        QString u1 = parts[1], u2 = parts[2];
        QSqlQuery q;
        // 서브쿼리에 rowid를 포함해야 바깥 ORDER BY rowid ASC가 제대로 동작함
        q.prepare(
            "SELECT SENDER,SEND_TIME,MESSAGE FROM "
            "  (SELECT rowid,SENDER,SEND_TIME,MESSAGE FROM dm_chats "
            "   WHERE (SENDER=? AND RECEIVER=?) OR (SENDER=? AND RECEIVER=?) "
            "   ORDER BY rowid DESC LIMIT 50) "
            "ORDER BY rowid ASC"
            );
        q.addBindValue(u1); q.addBindValue(u2);
        q.addBindValue(u2); q.addBindValue(u1);

        QStringList entries;
        if (q.exec()) {
            while (q.next()) {
                entries << q.value(0).toString()
                               + Protocol::SEP
                               + q.value(1).toString()
                               + Protocol::SEP
                               + q.value(2).toString();
            }
        }
        socket->write((Protocol::RESDM + Protocol::SEP + entries.join("|") + "\n").toUtf8());
    }

    else if (cmd == Protocol::CREATECAL && parts.size() >= 3) {
        QString owner   = parts[1];
        QString calName = parts[2];
        QStringList invited = (parts.size() >= 4 && !parts[3].isEmpty())
                                  ? parts[3].split("~", Qt::SkipEmptyParts)
                                  : QStringList();

        QSqlQuery q;
        q.prepare("INSERT INTO shared_calendars (OWNER, NAME) VALUES(?,?)");
        q.addBindValue(owner); q.addBindValue(calName);
        if (!q.exec()) return;

        int calId = q.lastInsertId().toInt();

        q.prepare("INSERT INTO shared_members (CAL_ID, USER_ID) VALUES(?,?)");
        q.addBindValue(calId); q.addBindValue(owner);
        q.exec();

        for (const QString& m : invited) {
            q.prepare("INSERT INTO shared_members (CAL_ID, USER_ID) VALUES(?,?)");
            q.addBindValue(calId); q.addBindValue(m);
            q.exec();
        }

        QString notif = Protocol::CALID + Protocol::SEP + QString::number(calId) + "\n";
        sendToCalMembers(calId, notif);
    }

    else if (cmd == Protocol::REQCALS && parts.size() >= 2) {
        QString userId = parts[1];
        QSqlQuery q;
        q.prepare("SELECT sc.ID, sc.NAME, sc.OWNER "
                  "FROM shared_calendars sc "
                  "JOIN shared_members sm ON sc.ID = sm.CAL_ID "
                  "WHERE sm.USER_ID = ?");
        q.addBindValue(userId);

        QStringList entries;
        if (q.exec()) {
            while (q.next()) {
                int     calId = q.value(0).toInt();
                QString name  = q.value(1).toString();
                QString owner = q.value(2).toString();

                QSqlQuery mq;
                mq.prepare("SELECT USER_ID FROM shared_members WHERE CAL_ID=?");
                mq.addBindValue(calId);
                QStringList members;
                if (mq.exec()) while (mq.next()) members << mq.value(0).toString();

                entries << QString::number(calId) + "~" + name + "~" + owner
                               + "~" + members.join(",");
            }
        }
        socket->write((Protocol::RESCALS + Protocol::SEP + entries.join("|") + "\n").toUtf8());
    }

    else if (cmd == Protocol::REQSHMONTH && parts.size() >= 3) {
        int     calId = parts[1].toInt();
        QString ym    = parts[2];

        QSqlQuery q;
        q.prepare("SELECT rowid,DATE,CONTENT FROM shared_schedules WHERE CAL_ID=? AND DATE LIKE ?");
        q.addBindValue(calId); q.addBindValue(ym + "%");

        QStringList res;
        if (q.exec()) {
            while (q.next())
                res << q.value(1).toString() + "@"
                           + q.value(0).toString() + "@"
                           + q.value(2).toString();
        }
        socket->write((Protocol::RESSHMONTH + Protocol::SEP
                       + QString::number(calId) + Protocol::SEP
                       + res.join("|") + "\n").toUtf8());
    }

    else if (cmd == Protocol::REQSHDAY && parts.size() >= 3) {
        int     calId = parts[1].toInt();
        QString date  = parts[2];

        QSqlQuery q;
        q.prepare("SELECT rowid, CONTENT FROM shared_schedules WHERE CAL_ID=? AND DATE=?");
        q.addBindValue(calId); q.addBindValue(date);

        QStringList res;
        if (q.exec()) {
            while (q.next())
                res << q.value(0).toString() + Protocol::SEP + q.value(1).toString();
        }
        socket->write((Protocol::RESSHDAY + Protocol::SEP
                       + QString::number(calId) + Protocol::SEP
                       + res.join("|") + "\n").toUtf8());
    }

    else if (cmd == Protocol::ADDSH && parts.size() >= 5) {
        int     calId   = parts[1].toInt();
        QString userId  = parts[2];
        QString date    = parts[3];
        QString content = parts.mid(4).join(Protocol::SEP);

        QSqlQuery q;
        q.prepare("INSERT INTO shared_schedules (CAL_ID,USER_ID,DATE,CONTENT) VALUES(?,?,?,?)");
        q.addBindValue(calId); q.addBindValue(userId);
        q.addBindValue(date);  q.addBindValue(content);

        if (q.exec()) {
            socket->write((Protocol::ACK + ":OK\n").toUtf8());
            QString upd = Protocol::SHUPDATE + Protocol::SEP + QString::number(calId) + "\n";
            sendToCalMembers(calId, upd, socket);
        } else {
            socket->write((Protocol::ACK + ":ERR\n").toUtf8());
        }
    }

    else if (cmd == Protocol::DELSH && parts.size() >= 3) {
        int    calId = parts[1].toInt();
        qint64 rowid = parts[2].toLongLong();

        QSqlQuery q;
        q.prepare("DELETE FROM shared_schedules WHERE rowid=? AND CAL_ID=?");
        q.addBindValue(rowid); q.addBindValue(calId);

        if (q.exec()) {
            socket->write((Protocol::ACK + ":OK\n").toUtf8());
            QString upd = Protocol::SHUPDATE + Protocol::SEP + QString::number(calId) + "\n";
            sendToCalMembers(calId, upd, socket);
        } else {
            socket->write((Protocol::ACK + ":ERR\n").toUtf8());
        }
    }

    else if (cmd == Protocol::MODSH && parts.size() >= 4) {
        int     calId   = parts[1].toInt();
        qint64  rowid   = parts[2].toLongLong();
        QString content = parts.mid(3).join(Protocol::SEP);

        QSqlQuery q;
        q.prepare("UPDATE shared_schedules SET CONTENT=? WHERE rowid=? AND CAL_ID=?");
        q.addBindValue(content); q.addBindValue(rowid); q.addBindValue(calId);

        if (q.exec()) {
            socket->write((Protocol::ACK + ":OK\n").toUtf8());
            QString upd = Protocol::SHUPDATE + Protocol::SEP + QString::number(calId) + "\n";
            sendToCalMembers(calId, upd, socket);
        } else {
            socket->write((Protocol::ACK + ":ERR\n").toUtf8());
        }
    }

    else if (cmd == Protocol::REQSHCHAT && parts.size() >= 2) {
        int calId = parts[1].toInt();

        QSqlQuery q;
        q.prepare("SELECT SENDER,SEND_TIME,MESSAGE FROM "
                  "(SELECT rowid,SENDER,SEND_TIME,MESSAGE FROM shared_chats "
                  " WHERE CAL_ID=? ORDER BY rowid DESC LIMIT 50) "
                  "ORDER BY rowid ASC");
        q.addBindValue(calId);

        QStringList entries;
        if (q.exec()) {
            while (q.next())
                entries << q.value(0).toString() + Protocol::SEP
                               + q.value(1).toString() + Protocol::SEP
                               + q.value(2).toString();
        }
        socket->write((Protocol::RESSHCHAT + Protocol::SEP
                       + QString::number(calId) + Protocol::SEP
                       + entries.join("|") + "\n").toUtf8());
    }

    else if (cmd == Protocol::DELCAL && parts.size() >= 2) {
        int calId = parts[1].toInt();

        // 삭제 전에 모든 온라인 클라이언트에게 알림 (멤버 기록이 지워지기 전에 전송)
        QString notif = Protocol::CALREMOVED + Protocol::SEP + QString::number(calId) + "\n";
        for (QTcpSocket* s : m_clients)
            if (s && s->state() == QAbstractSocket::ConnectedState)
                s->write(notif.toUtf8());

        QSqlQuery q;
        q.prepare("DELETE FROM shared_calendars WHERE ID=?");
        q.addBindValue(calId); q.exec();
        q.prepare("DELETE FROM shared_members WHERE CAL_ID=?");
        q.addBindValue(calId); q.exec();
        q.prepare("DELETE FROM shared_schedules WHERE CAL_ID=?");
        q.addBindValue(calId); q.exec();
        q.prepare("DELETE FROM shared_chats WHERE CAL_ID=?");
        q.addBindValue(calId); q.exec();
    }

    else if (cmd == Protocol::SHCHAT && parts.size() >= 4) {
        int     calId   = parts[1].toInt();
        QString sender  = parts[2];
        QString message = parts.mid(3).join(Protocol::SEP);
        QString timeStr = QDateTime::currentDateTime().toString("MM/dd HH.mm");

        QSqlQuery q;
        q.prepare("INSERT INTO shared_chats (CAL_ID,SENDER,MESSAGE,SEND_TIME) VALUES(?,?,?,?)");
        q.addBindValue(calId); q.addBindValue(sender);
        q.addBindValue(message); q.addBindValue(timeStr);
        if (!q.exec()) { qWarning() << "공유 채팅 저장 실패:" << q.lastError().text(); return; }

        qint64 rowid = q.lastInsertId().toLongLong();

        QString msg = Protocol::SHCHATRES + Protocol::SEP
                      + QString::number(calId) + Protocol::SEP
                      + QString::number(rowid) + Protocol::SEP
                      + sender  + Protocol::SEP
                      + timeStr + Protocol::SEP
                      + message + "\n";
        sendToCalMembers(calId, msg);
    }
}

void ScheduleServer::broadcastOnlineList() {
    QString msg = Protocol::ONLINE + Protocol::SEP + m_clientIds.values().join("|") + "\n";
    for (QTcpSocket* s : m_clients)
        if (s && s->state() == QAbstractSocket::ConnectedState)
            s->write(msg.toUtf8());
}

void ScheduleServer::onDisconnected() {
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;
    qDebug() << "Disconnected:" << m_clientIds.value(socket, "unknown");
    m_buffers.remove(socket);
    m_clients.removeAll(socket);
    m_clientIds.remove(socket);
    broadcastOnlineList();
    socket->deleteLater();
}

QStringList ScheduleServer::getCalMembers(int calId) {
    QSqlQuery q;
    q.prepare("SELECT USER_ID FROM shared_members WHERE CAL_ID=?");
    q.addBindValue(calId);
    QStringList members;
    if (q.exec()) while (q.next()) members << q.value(0).toString();
    return members;
}

void ScheduleServer::sendToCalMembers(int calId, const QString& msg, QTcpSocket* exclude) {
    QStringList members = getCalMembers(calId);
    for (QTcpSocket* s : m_clients) {
        if (s == exclude) continue;
        if (s && s->state() == QAbstractSocket::ConnectedState)
            if (members.contains(m_clientIds.value(s)))
                s->write(msg.toUtf8());
    }
}
