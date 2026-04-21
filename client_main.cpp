#include <QApplication>
#include "MainWindow.h"
#include "LoginDialog.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    LoginDialog login;
    if (login.exec() == QDialog::Accepted) {
        // ID가 비어있으면 종료
        if (login.getId().trimmed().isEmpty()) {
            return 0;
        }

        MainWindow w(login.getIp(), login.getId(), login.getPassword());
        w.show();
        return a.exec();
    }

    return 0; // 취소 버튼 클릭 시 종료
}
