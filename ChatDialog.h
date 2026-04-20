#ifndef CHATDIALOG_H
#define CHATDIALOG_H

#include <QDialog>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QWidget>
#include <QScrollBar>
#include <QMap>
#include <QPixmap>

class ChatDialog : public QDialog {
    Q_OBJECT
public:
    explicit ChatDialog(const QString& myId, QWidget* parent = nullptr);

    void appendMessage(qint64 rowid, int unread, const QString& userId,
                       const QString& message, const QString& time = "");
    void updateUnread(qint64 rowid, int count);
    void clearMessages();
    void setDarkMode(bool dark);
    void setTitle(const QString& title);

signals:
    void messageSent(const QString& message);

private slots:
    void onSendClicked();
    void onImageClicked();

private:
    static constexpr int IMG_MAX_PX  = 300;  // 전송 전 리사이즈 최대 크기
    static constexpr int IMG_QUALITY = 75;   // JPEG 압축 품질
    QScrollArea* m_scrollArea;
    QWidget*     m_messageContainer;
    QVBoxLayout* m_messageLayout;
    QLineEdit*   m_inputEdit;
    QPushButton* m_sendBtn;
    QPushButton* m_imageBtn;
    QString      m_myId;

    QMap<qint64, QLabel*> m_unreadLabels; // rowid → 미읽음 숫자 라벨
    QLabel* m_titleLabel = nullptr;

};

#endif
