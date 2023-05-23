#include "globals.h"
#include "mainwindow.h"

#include <QApplication>
#include <QTranslator>
#include <QLabel>

int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName(ORGANIZATION_NAME);
    QCoreApplication::setOrganizationDomain(ORGANIZATION_DOMAIN);
    QCoreApplication::setApplicationName(APPLICATION_NAME);

    QApplication a(argc, argv);

    QPalette p = a.palette();
    //p.setBrush(QPalette::Window, QBrush(QPixmap(":/images/bakground.png")));
    a.setPalette(p);

    QTranslator qtLanguageTranslator;
    qtLanguageTranslator.load(QString("QtLanguage_") + QString("ru_RU"));
    a.installTranslator(&qtLanguageTranslator);

    MainWindow *w = new MainWindow();
    w->setAttribute(Qt::WA_DeleteOnClose);
    w->show();

    return a.exec();
}
