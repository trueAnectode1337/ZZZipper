#include "mainwindow.h"

#include <QApplication>
#include <QStyleFactory>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QPixmap iconPixmap(":/ico/ico/app_icon.png");
    if (iconPixmap.isNull()) {
        qDebug() << "Failed to load pixmap!";
    }

    QIcon appIcon(iconPixmap);
    if (appIcon.isNull()) {
        qDebug() << "Failed to load icon!";
    }

    a.setWindowIcon(appIcon);
    a.setStyle(QStyleFactory::create("Fusion"));
    MainWindow w;
    w.show();
    return a.exec();
}
