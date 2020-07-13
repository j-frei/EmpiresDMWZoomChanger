#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication EmpiresDMWZoomChanger(argc, argv);
    MainWindow window;
    window.show();

    return EmpiresDMWZoomChanger.exec();
}
