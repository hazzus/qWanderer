#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    //Indexer ind(QFile("/home/haze/study/university/DM/matroidlab/check.py"));
    //ind.process();
    return a.exec();
}
