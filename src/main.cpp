#include <QApplication>
#include <QFontDatabase>
#include <QFile>
#include <QDir>
#include <QResource>
#include "MainWindow.h"

void listResources(const QString &path = ":/"){
    QDir dir(path);
    QStringList entries = dir.entryList(QDir::AllEntries | QDir::NoDotAndDotDot);
    for (const QString &entry : entries) {
        QString fullPath = path + entry;
        qDebug() << fullPath;
        // If this is a "directory", recurse
        QDir subDir(fullPath);
        if (!subDir.entryList(QDir::NoDotAndDotDot | QDir::AllEntries).isEmpty()) {
            listResources(fullPath + "/");
        }
    }
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    listResources();

    int id = QFontDatabase::addApplicationFont(":/fonts/MontserratLight.ttf");
    if (id == -1){
        qWarning() << "No Montserrat font found!";
    }
    else{
        QString family = QFontDatabase::applicationFontFamilies(id).at(0);
        QFont appFont(family, 10);
        QApplication::setFont(appFont);
    }

    MainWindow window;
    window.show();

    return app.exec();
}

