#ifndef COMMON_H
#define COMMON_H

#include <QString>

namespace Protocol {
    inline constexpr int PORT = 25000;
    inline const QString SEP = ":";

    // 일정
    inline const QString REQ      = "REQ";
    inline const QString ADD      = "ADD";
    inline const QString DEL      = "DEL";
    inline const QString MOD      = "MOD";
    inline const QString RES      = "RES";
    inline const QString REQMONTH = "REQMONTH";
    inline const QString RESMONTH = "RESMONTH";
    inline const QString ACK      = "ACK";
    inline const QString REQUSERS = "REQUSERS";
    inline const QString RESUSERS = "RESUSERS";

    // 로그인 / 접속자
    inline const QString LOGIN        = "LOGIN";
    inline const QString LOGIN_OK     = "LOGIN_OK";
    inline const QString LOGIN_REJECT = "LOGIN_REJECT";
    inline const QString ONLINE       = "ONLINE";

    // 단체 채팅
    // CHAT    : CHAT:USER_ID:MESSAGE
    // CHATRES : CHATRES:ROWID:UNREAD:USER_ID:HHmm:MESSAGE
    // REQCHAT : REQCHAT:USER_ID
    // RESCHAT : RESCHAT:ROWID:UNREAD:USER_ID:HHmm:MSG|...
    // READRES : READRES:ROWID:NEW_COUNT
    inline const QString CHAT    = "CHAT";
    inline const QString CHATRES = "CHATRES";
    inline const QString REQCHAT = "REQCHAT";
    inline const QString RESCHAT = "RESCHAT";
    inline const QString READRES = "READRES";

    // 1대1 DM
    // DM     : DM:sender:receiver:message
    // DMRES  : DMRES:sender:receiver:HHmm:message  (sender, receiver 모두에게 전송)
    // REQDM  : REQDM:myId:peerId
    // RESDM  : RESDM:sender:HHmm:msg|sender:HHmm:msg|...
    inline const QString DM    = "DM";
    inline const QString DMRES = "DMRES";
    inline const QString REQDM = "REQDM";
    inline const QString RESDM = "RESDM";
}

#endif
