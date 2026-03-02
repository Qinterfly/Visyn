
#include <config.h>
#include <QApplication>

#include "mainwindow.h"

int main(int argc, char* argv[])
{
    // Wayland is still laggy, so use xcb instead
#ifdef Q_OS_LINUX
    qputenv("QT_QPA_PLATFORM", "xcb");
#endif

    // Create the application
    QApplication application(argc, argv);

    // Create the graphical window
    Frontend::MainWindow window;
    window.show();

    return application.exec();
}
