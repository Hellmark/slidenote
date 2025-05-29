#ifndef SLIDEWINDOW_H
#define SLIDEWINDOW_H

#include <QWidget>
#include <QMap>
#include <QTextEdit>

class QTabWidget;
class QToolBar;
class QPropertyAnimation;
class QSystemTrayIcon;
class QHotkey;
class QMenuBar;

class SlideWindow : public QWidget {
    Q_OBJECT

public:
    ~SlideWindow();
    enum SlideDirection { Left, Right, Top, Bottom };
    SlideWindow(QWidget *parent = nullptr);

private slots:
    void addNewTab();
    void onTextChanged();
    void saveCurrentNote();
    void saveAllNotes();
    void openNote();
    void closeTab(int index);
    void closeCurrentTab();
    void toggleVisibility();
    void showSettingsDialog();
    void showAboutDialog();

private:
    void loadLastSession();
    void saveLastSession();
    void setupUI();
    void setupTrayIcon();
    void setupHotkey();
    void animateSlide(bool visible);
    void applyGeometryAndPosition();
    void saveSettings();
    void loadSettings();

    void setSlideDirection(SlideDirection direction) { m_direction = direction; }
    void setHeightPercent(double percent) { m_heightPercent = percent; }
    void setWidthPercent(double percent) { m_widthPercent = percent; }
    void setHotkeySequence(const QString &sequence) { m_hotkeySequence = sequence; }
    void setScreenIndex(int index) { m_screenIndex = index; }

    QTabWidget *m_tabWidget;
    QToolBar *m_toolBar;
    QWidget *m_contentWidget;
    QMap<QTextEdit*, QString> m_filePaths;
    QPropertyAnimation *m_animation;
    QSystemTrayIcon *m_trayIcon;
    QHotkey *m_hotkey;
    QTimer *m_autosaveTimer;
    #ifdef Q_OS_MAC
        QMenuBar *m_menuBar = nullptr;
    #endif


    SlideDirection m_direction;
    double m_heightPercent;
    double m_widthPercent;
    QString m_hotkeySequence;
    int m_screenIndex;
    bool m_isSliding;
    bool m_reopenLastSession;
    int m_autosaveInterval = 60;
};

#endif // SLIDEWINDOW_H
