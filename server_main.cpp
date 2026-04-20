#include <QCoreApplication>
#include "Server.h"

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);

    ScheduleServer server;
    if (!server.startServer()) {
        return -1;
    }

    return a.exec();
}
