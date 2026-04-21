#ifndef COMMON_H
#define COMMON_H

#include <QString>

namespace Protocol  //서버와 클라이언트 간의 통신 규약 정의
{
//통신에 사용하는 포트 고정
inline constexpr int PORT = 25000;
//데이터 구분자 (EX : LOGIN : id:pw)
inline const QString SEP = ":";

// 일정관련 프로토콜
inline const QString REQ      = "REQ";  //특정 날짜 일정 요청

//일정 추가
inline const QString ADD      = "ADD";

//일정 삭제
inline const QString DEL      = "DEL";

//일정 수정
inline const QString MOD      = "MOD";

//일정 응답  (server -> client)
inline const QString RES      = "RES";

//한달치 일정 요청
inline const QString REQMONTH = "REQMONTH";

//한달치 일정 응답
inline const QString RESMONTH = "RESMONTH";

//명령 수신 확인 응답
inline const QString ACK      = "ACK";

//사용자 목록 요청
inline const QString REQUSERS = "REQUSERS";

//사용자 목록 응답
inline const QString RESUSERS = "RESUSERS";


// 로그인 및 접속자 상태 관련
inline const QString LOGIN        = "LOGIN";    //로그인 시도

//로그인 성공
inline const QString LOGIN_OK     = "LOGIN_OK";

//로그인 거부
inline const QString LOGIN_REJECT = "LOGIN_REJECT";

//로그인 실패
inline const QString LOGIN_FAIL   = "LOGIN_FAIL";

//현재 접속자 정보
inline const QString ONLINE       = "ONLINE";

//프로필 사진 관련
inline const QString PROFILE_UPLOAD = "PROFILE_UPLOAD"; // 프로필 업로
inline const QString PROFILE_OK     = "PROFILE_OK";
inline const QString PROFILE_REQ    = "PROFILE_REQ";
inline const QString PROFILE_RES    = "PROFILE_RES";

// 회원가입
// SIGNUP      : SIGNUP:userId:password
// SIGNUP_OK   : SIGNUP_OK
// SIGNUP_FAIL : SIGNUP_FAIL:reason
inline const QString SIGNUP      = "SIGNUP";
inline const QString SIGNUP_OK   = "SIGNUP_OK";
inline const QString SIGNUP_FAIL = "SIGNUP_FAIL";

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
inline const QString DELCAL     = "DELCAL";      // DELCAL:calId
inline const QString CALREMOVED = "CALREMOVED";  // CALREMOVED:calId (서버→전체)

// 닉네임
// NICK_UPDATE : NICK_UPDATE:userId:nickname
// NICK_OK     : NICK_OK:nickname
// NICK_REQ    : NICK_REQ:userId
// NICK_RES    : NICK_RES:userId:nickname
inline const QString NICK_UPDATE = "NICK_UPDATE";
inline const QString NICK_OK     = "NICK_OK";
inline const QString NICK_REQ    = "NICK_REQ";
inline const QString NICK_RES    = "NICK_RES";

// 프로필 사진 삭제
inline const QString PROFILE_DELETE = "PROFILE_DELETE";  // PROFILE_DELETE:userId

// 일정 검색
// SEARCH_REQ : SEARCH_REQ:userId:keyword
// SEARCH_RES : SEARCH_RES:calId\tcalName\tdate\tcontent|...
inline const QString SEARCH_REQ = "SEARCH_REQ";
inline const QString SEARCH_RES = "SEARCH_RES";
}

#endif
