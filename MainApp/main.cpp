#include <QGuiApplication>
#include <QApplication>
#include <MyWindow/MainWindow.h>
#include <DataCenter/DataCenter.h>

#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    QApplication a(argc, argv);

    QFont font("SimHei");
    a.setFont(font);

    MainWidget w;
    w.show();

    return a.exec();
} 
