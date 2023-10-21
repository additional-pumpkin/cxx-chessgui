#include <QApplication>

#include "mainwindow.hxx"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    // w.setMinimumSize(800, 888);
    w.show();
    return a.exec();
}
