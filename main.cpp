#include <QApplication>
#include "mainwindow.h"
#include "sigrokworker.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    qRegisterMetaType<CanFrame>("CanFrame");
    MainWindow w;
    w.resize(1100, 700);
    w.show();
    return a.exec();
}
