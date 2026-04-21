#ifndef REGISTERDIALOG_H
#define REGISTERDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QTcpSocket>
#include <QCamera>
#include <QMediaCaptureSession>
#include <QImageCapture>
#include <QStackedWidget>

class QVideoWidget;

class RegisterDialog : public QDialog {
    Q_OBJECT
public:
    explicit RegisterDialog(const QString& serverIp, QWidget* parent = nullptr);
    ~RegisterDialog();

private slots:
    void onCaptureClicked();
    void onImageCaptured(int id, const QImage& image);
    void onRetakeClicked();
    void onRegisterClicked();
    void onConnected();
    void onReadyRead();
    void onSocketError();

private:
    void showCameraStep();
    void showFormStep();

    QString      m_serverIp;
    QTcpSocket*  m_socket;
    QImage       m_capturedImage;
    bool         m_signupDone = false;

    // 카메라
    QCamera*               m_camera   = nullptr;
    QMediaCaptureSession*  m_session  = nullptr;
    QImageCapture*         m_capture  = nullptr;
    QVideoWidget*          m_viewfinder = nullptr;

    // UI
    QStackedWidget* m_stack;

    // Step 1: 카메라
    QWidget*     m_cameraPage;
    QLabel*      m_photoPreview;
    QPushButton* m_captureBtn;
    QPushButton* m_retakeBtn;
    QPushButton* m_nextBtn;

    // Step 2: 가입 폼
    QWidget*     m_formPage;
    QLineEdit*   m_idEdit;
    QLineEdit*   m_pwEdit;
    QLineEdit*   m_pwConfirmEdit;
    QPushButton* m_registerBtn;
    QLabel*      m_statusLabel;
    QLabel*      m_photoThumb;
};

#endif
