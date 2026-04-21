#ifndef CHATDIALOG_H
#define CHATDIALOG_H

#include <QDialog> // 대화 상자 기본 클래스
#include <QScrollArea>  //스크롤 가능한 영역
#include <QHBoxLayout>  //수평 레이아웃
#include <QVBoxLayout>  //수직 레이아웃

#include <QLineEdit>    //텍스트 입력창
#include <QPushButton>  //클릭 버튼
#include <QWidget>      //최상단 위젯
#include <QScrollBar>   // 스크롤
#include <QPixmap>      //이미지 처리
#include <QMap>         //키-값 자료구조
#include <QLabel>       //텍스트 및 이미지 표시

class ChatDialog : public QDialog
{
private:
    Q_OBJECT //QT 시그널/슬롯 사용을 위한 필수 매크로
public:
    //생성자 : ID와 위젯을 인자로 받음
    explicit ChatDialog(const QString& myId, QWidget* parent = nullptr);

    //채팅방 기능들
    void appendMessage(qint64 rowid, int unread, const QString& userId,const QString& message, const QString& time = "");
\
    //메세지 읽었는지 안읽었는지 업데이트
    void updateUnread(qint64 rowid, int count);

    //채팅방 메세지  삭제 및 초기화
    void clearMessages();

    // 다크/라이트 모드 변경 기능
    void setDarkMode(bool dark);

    // 채팅방 상단에 표시되는 제목
    void setTitle(const QString& title);


    signals:

    //사용자가 메세지 입력하고 전송을 눌렀을때 발생하는 시그널
     void messageSent(const QString& message);


    private slots:

    //전송 버튼 클릭시 실행되는 슬롯
    void onSendClicked();
    // 이미지 첨부 클릭시 실행되는 슬롯
    void onImageClicked();


    private:


    //리사이즈 최대 크기 설정
    static constexpr int IMG_MAX_PX  = 300;
    //압축 품질 설정
    static constexpr int IMG_QUALITY = 75;

    //UI 구성 요소
    QScrollArea* m_scrollArea;
    QWidget*     m_messageContainer;
    QVBoxLayout* m_messageLayout;
    QLineEdit*   m_inputEdit;

    QPushButton* m_sendBtn;
    QPushButton* m_imageBtn;
    QString      m_myId;

    //채팅 숫자 라벨 // 데이터 관리
    QMap<qint64, QLabel*> m_unreadLabels;

    //채팅방 상단 제목 표시 라벨
    QLabel* m_titleLabel = nullptr;

};

#endif