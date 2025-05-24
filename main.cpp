#include <QApplication>
#include "SlideWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        qWarning("No system tray available.");
        return -1;
    }

    QApplication::setQuitOnLastWindowClosed(false);

    SlideWindow window;
    window.hide(); // start hidden
    return app.exec();
}
