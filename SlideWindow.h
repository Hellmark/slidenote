#ifndef SLIDEWINDOW_H
#define SLIDEWINDOW_H

#include <QWidget>
#include <QSystemTrayIcon>
#include <QPropertyAnimation>
#include <QToolBar>
#include <QTabWidget>
#include <QSettings>
#include <QString>
#include <QMap>
#include <QTextEdit>


class QHotkey;
class QTextEdit;

class SlideWindow : public QWidget
{
    Q_OBJECT
public:
    enum SlideDirection {
        Left,
        Right,
        Top,
        Bottom
    };
    Q_ENUM(SlideDirection)

    explicit SlideWindow(QWidget *parent = nullptr);

    void setSlideDirection(SlideDirection dir);
    void setHeightPercent(double hpercent);
    void setWidthPercent(double wpercent);
    void setHotkeySequence(const QString &seq);

protected:
    void setupUI();
    void setupTrayIcon();
    void setupHotkey();
    void applyGeometryAndPosition();
    void saveSettings();
    void loadSettings();

private slots:
    void toggleVisibility();
    void animateSlide(bool show);
    void addNewTab();
    void openNote();
    void saveCurrentNote();
    void saveAllNotes();
    void closeCurrentTab();
    void closeTab(int index);
    void onTextChanged();
    void showSettingsDialog();

private:
    bool m_isSliding = false;
    SlideDirection m_direction;
    double m_heightPercent;
    double m_widthPercent;
    QString m_hotkeySequence;

    QSystemTrayIcon *m_trayIcon;
    QPropertyAnimation *m_animation;
    QHotkey *m_hotkey;

    QToolBar *m_toolBar;
    QTabWidget *m_tabWidget;
    QMap<QTextEdit*, QString> m_filePaths;
};

#endif // SLIDEWINDOW_H
