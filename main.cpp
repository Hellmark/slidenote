#include <QApplication>
#include <QSystemTrayIcon>
#include "SlideWindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        qWarning("No system tray available.");
        return -1;
    }

    QApplication::setQuitOnLastWindowClosed(false);

    SlideWindow window;
    if (window.shouldStartVisible()) {
        window.show();
    } else {
        window.hide();
    }
    //window.hide(); // start hidden
    return app.exec();
}
