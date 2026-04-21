#ifndef MYPAGEDIALOG_H
#define MYPAGEDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QPixmap>
#include <QStackedWidget>
#include <QCamera>
#include <QMediaCaptureSession>
#include <QImageCapture>
#include <QVideoWidget>

class MyPageDialog : public QDialog {
    Q_OBJECT
public:
    explicit MyPageDialog(const QString& userId,
                          const QPixmap& currentPhoto,
                          const QString& nickname,
                          QWidget* parent = nullptr);

    QPixmap  newPhoto()      const { return m_newPhoto; }
    QString  newNickname()   const;
    bool     photoChanged()  const { return m_photoChanged; }
    bool     photoDeleted()  const { return m_photoDeleted; }

private slots:
    void onUpload();
    void onDelete();
    void onRetake();
    void onCaptureClicked();
    void onImageCaptured(int id, const QImage& img);
    void onCancelCamera();
    void onSave();

private:
    QPixmap makeCircular(const QPixmap& src, int size);
    void    updatePhotoLabel();

    QString  m_userId;
    QPixmap  m_currentPhoto;
    QPixmap  m_newPhoto;
    bool     m_photoChanged = false;
    bool     m_photoDeleted = false;

    QStackedWidget*       m_stack;
    QLabel*               m_photoLabel;
    QLineEdit*            m_nicknameEdit;

    QVideoWidget*         m_videoWidget;
    QCamera*              m_camera      = nullptr;
    QMediaCaptureSession* m_session     = nullptr;
    QImageCapture*        m_imgCapture  = nullptr;
    QPushButton*          m_captureBtn;
};

#endif
