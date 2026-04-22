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

//변경사항이 있을시 친구들에게 알림
inline const QString SHUPDATE   = "SHUPDATE";

//채팅 내역 요청
inline const QString REQSHCHAT  = "REQSHCHAT";

//채팅 응답
inline const QString RESSHCHAT  = "RESSHCHAT";

//채팅 전송
inline const QString SHCHAT     = "SHCHAT";

//채팅 중계
inline const QString SHCHATRES  = "SHCHATRES";

//캘린더 삭제
inline const QString DELCAL     = "DELCAL";      // DELCAL:calId

//삭제 알림
inline const QString CALREMOVED = "CALREMOVED";  // CALREMOVED:calId (서버→전체)


// 닉네임
inline const QString NICK_UPDATE = "NICK_UPDATE"; //변경 요청
inline const QString NICK_OK     = "NICK_OK";   //변경 성공
inline const QString NICK_REQ    = "NICK_REQ";  //닉네임 정보 요청
inline const QString NICK_RES    = "NICK_RES";  //정보 응답

// 프로필 사진 삭제
inline const QString PROFILE_DELETE = "PROFILE_DELETE";  //프로필 삭제

// 일정 검색
inline const QString SEARCH_REQ = "SEARCH_REQ";
inline const QString SEARCH_RES = "SEARCH_RES";
}

#endif
