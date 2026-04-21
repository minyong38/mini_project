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
inline const QString LOGIN_FAIL   = "LOGIN_FAIL";   // 비밀번호 불일치
inline const QString ONLINE       = "ONLINE";

// 프로필 사진
// PROFILE_UPLOAD : PROFILE_UPLOAD:userId:base64data
// PROFILE_OK     : PROFILE_OK
// PROFILE_REQ    : PROFILE_REQ:userId
// PROFILE_RES    : PROFILE_RES:userId:base64data
inline const QString PROFILE_UPLOAD = "PROFILE_UPLOAD";
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
inline const QString CREATECAL  = "CREATECAL";  //캘린더 생성

//생성된 캘린더 고유 ID 알림
inline const QString CALID      = "CALID";

//내가 참여중인 공유 캘린더 목록 요청
inline const QString REQCALS    = "REQCALS";

//목록 응답
inline const QString RESCALS    = "RESCALS";

//월별 일정 요청
inline const QString REQSHMONTH = "REQSHMONTH";

//월별 일정 응답
inline const QString RESSHMONTH = "RESSHMONTH";

//월별 상세 요청
inline const QString REQSHDAY   = "REQSHDAY";

//월별 상세 응답
inline const QString RESSHDAY   = "RESSHDAY";

//일정 추가
inline const QString ADDSH      = "ADDSH";

//일정삭제
inline const QString DELSH      = "DELSH";

//일정 수정
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
// SEARCH_RES : SEARCH_RES:date~content|date~content|...  (결과 없으면 SEARCH_RES: 만 전송)
inline const QString SEARCH_REQ = "SEARCH_REQ";
inline const QString SEARCH_RES = "SEARCH_RES";
}

#endif
