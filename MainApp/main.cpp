#include <QGuiApplication>
#include <QApplication>
#include <QFile>
#include <QFont>
#include <QDebug>

#include <MyWindow/MainWindow.h>
#include <DataCenter/DataCenter.h>

int main(int argc, char *argv[])
{
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    QApplication a(argc, argv);

    QFont font("Microsoft YaHei");
    a.setFont(font);

    // 加载 CSS 样式表
    QFile styleFile(QCoreApplication::applicationDirPath() + "/config/style.css");
    if (styleFile.open(QFile::ReadOnly | QFile::Text))
    {
        QString styleSheet = QString::fromUtf8(styleFile.readAll());
        a.setStyleSheet(styleSheet);
        styleFile.close();
    }
    else
    {
        qDebug() << "样式文件加载失败:" << styleFile.fileName();
    }

    MainWidget w;
    w.show();

    return a.exec();
} 
