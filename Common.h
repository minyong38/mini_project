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

    // 공유 캘린더
    // CREATECAL  : CREATECAL:owner:calName:member1~member2~...
    // CALID      : CALID:id  (서버→클라, 새 캘린더 ID 알림)
    // REQCALS    : REQCALS:userId
    // RESCALS    : RESCALS:id~name~owner~m1,m2|...
    // REQSHMONTH : REQSHMONTH:calId:yyyy-MM
    // RESSHMONTH : RESSHMONTH:calId:date@rowid@content|...
    // REQSHDAY   : REQSHDAY:calId:date
    // RESSHDAY   : RESSHDAY:calId:rowid:content|rowid:content|...
    // ADDSH      : ADDSH:calId:userId:date:content
    // DELSH      : DELSH:calId:rowid
    // MODSH      : MODSH:calId:rowid:content
    // SHUPDATE   : SHUPDATE:calId  (서버→전체 멤버, 일정 변경 알림)
    // REQSHCHAT  : REQSHCHAT:calId:userId
    // RESSHCHAT  : RESSHCHAT:calId:sender:HHmm:msg|...
    // SHCHAT     : SHCHAT:calId:sender:message
    // SHCHATRES  : SHCHATRES:calId:rowid:sender:HHmm:message
    inline const QString CREATECAL  = "CREATECAL";
    inline const QString CALID      = "CALID";
    inline const QString REQCALS    = "REQCALS";
    inline const QString RESCALS    = "RESCALS";
    inline const QString REQSHMONTH = "REQSHMONTH";
    inline const QString RESSHMONTH = "RESSHMONTH";
    inline const QString REQSHDAY   = "REQSHDAY";
    inline const QString RESSHDAY   = "RESSHDAY";
    inline const QString ADDSH      = "ADDSH";
    inline const QString DELSH      = "DELSH";
    inline const QString MODSH      = "MODSH";
    inline const QString SHUPDATE   = "SHUPDATE";
    inline const QString REQSHCHAT  = "REQSHCHAT";
    inline const QString RESSHCHAT  = "RESSHCHAT";
    inline const QString SHCHAT     = "SHCHAT";
    inline const QString SHCHATRES  = "SHCHATRES";
}

#endif
