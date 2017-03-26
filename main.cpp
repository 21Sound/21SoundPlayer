#include "mainwindow.h"
#include "CWavRW.h"
#include "RtAudio.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    MainWindow mainWin;
    mainWin.setMaximumSize(800,400);
    mainWin.resize(800,400);

    mainWin.show();

    return app.exec();
}
