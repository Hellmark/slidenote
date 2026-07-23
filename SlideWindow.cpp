#include "SlideWindow.h"
#include "SettingsDialog.h"

#include <QApplication>
#include <QScreen>
#include <QCursor>
#include <QPropertyAnimation>
#include <QVBoxLayout>
#include <QTextEdit>
#include <QToolBar>
#include <QAction>
#include <QFileDialog>
#include <QMessageBox>
#include <QMenu>
#include <QMenuBar>
#include <QSystemTrayIcon>
#include <QHotkey>
#include <QSettings>
#include <QFileInfo>
#include <QTabWidget>
#include <QTimer>
#include <QShortcut>

SlideWindow::SlideWindow(QWidget *parent)
    : QWidget(parent),
      m_direction(Left),
      m_heightPercent(0.75),
      m_widthPercent(0.3),
      m_screenIndex(0),
      m_isSliding(false),
      m_hotkey(nullptr),
      m_trayIcon(nullptr),
      m_animation(new QPropertyAnimation(this, "pos", this))
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);

    setupUI();
    setupTrayIcon();
    loadSettings();
    setupHotkey();
    applyGeometryAndPosition();
}

void SlideWindow::setupUI() {
    // Menubar for MacOS
    #ifdef Q_OS_MAC
        m_menuBar = new QMenuBar(this);

        QMenu *fileMenu = m_menuBar->addMenu("File");
        fileMenu->addAction("New Note", QKeySequence("Ctrl+N"), this, SLOT(addNewTab()));
        fileMenu->addAction("Open...", QKeySequence("Ctrl+O"), this, SLOT(openNote()));
        fileMenu->addAction("Save", QKeySequence("Ctrl+S"), this, SLOT(saveCurrentNote()));
        fileMenu->addAction("Save All", QKeySequence("Ctrl+Shift+S"), this, SLOT(saveAllNotes()));
        fileMenu->addSeparator();
        fileMenu->addAction("Exit", QKeySequence("Ctrl+Q"), qApp, SLOT(quit()));

        QMenu *editMenu = m_menuBar->addMenu("Edit");
        editMenu->addAction("Settings", QKeySequence("Ctrl+,"), this, SLOT(showSettingsDialog()));

        QMenu *helpMenu = m_menuBar->addMenu("Help");
        helpMenu->addAction("About", QKeySequence("F1"), this, SLOT(showAboutDialog()));
    #endif

    // Keyboard shortcuts
    new QShortcut(QKeySequence("Ctrl+N"), this, SLOT(addNewTab()));
    new QShortcut(QKeySequence(tr("Ctrl+O","File|Open")), this, SLOT(openNote()));
    new QShortcut(QKeySequence("Ctrl+S"), this, SLOT(saveCurrentNote()));
    new QShortcut(QKeySequence("Ctrl+Shift+S"), this, SLOT(saveAllNotes()));
    new QShortcut(QKeySequence("Ctrl+W"), this, SLOT(closeCurrentTab()));
    new QShortcut(QKeySequence("Ctrl+Q"), this, &QApplication::quit);
    new QShortcut(QKeySequence("Ctrl+,"), this, SLOT(showSettingsDialog()));
    new QShortcut(QKeySequence("F1"), this, SLOT(showAboutDialog()));

    m_contentWidget = new QWidget(this);
    m_contentWidget->setStyleSheet("background-color: palette(window);");
    m_contentWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    m_toolBar = new QToolBar(m_contentWidget);
    m_toolBar->setStyleSheet("border: none;"
                               "background-color: transparent;");
    m_tabWidget = new QTabWidget(m_contentWidget);

    m_tabWidget->setTabsClosable(true);
    m_tabWidget->setMovable(true);
    connect(m_tabWidget, &QTabWidget::tabCloseRequested, this, &SlideWindow::closeTab);

    QAction *newNoteAction = m_toolBar->addAction(QIcon::fromTheme("document-new"), "New Note");
    QAction *openNoteAction = m_toolBar->addAction(QIcon::fromTheme("document-open"), "Open Note");
    QAction *saveAction = m_toolBar->addAction(QIcon::fromTheme("document-save"), "Save");
    QAction *saveAllAction = nullptr;
    if (QIcon::hasThemeIcon("document-save-all")){
        saveAllAction = m_toolBar->addAction(QIcon::fromTheme("document-save-all"), "Save All");
      } else {
        saveAllAction = m_toolBar->addAction(QIcon::fromTheme("edit-copy"), "Save All");
    }
    m_toolBar->addSeparator();
    QAction *settingsAction = nullptr;
    if (QIcon::hasThemeIcon("preferences-system")){
        settingsAction = m_toolBar->addAction(QIcon::fromTheme("preferences-system"), "Settings");
      } else {
        settingsAction = m_toolBar->addAction(QIcon::fromTheme("help-about"), "Settings");
    }
    m_toolBar->addSeparator();
    QWidget* spacer = new QWidget();
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_toolBar->addWidget(spacer);
    QAction *exitAction = m_toolBar->addAction(QIcon::fromTheme("application-exit"), "Exit");

    connect(newNoteAction, &QAction::triggered, this, &SlideWindow::addNewTab);
    connect(openNoteAction, &QAction::triggered, this, &SlideWindow::openNote);
    connect(saveAction, &QAction::triggered, this, &SlideWindow::saveCurrentNote);
    connect(saveAllAction, &QAction::triggered, this, &SlideWindow::saveAllNotes);
    connect(settingsAction, &QAction::triggered, this, &SlideWindow::showSettingsDialog);
    connect(exitAction, &QAction::triggered, qApp, &QApplication::quit);

    QVBoxLayout *innerLayout = new QVBoxLayout(m_contentWidget);
    innerLayout->setContentsMargins(0, 0, 0, 0);
    innerLayout->addWidget(m_toolBar);
    innerLayout->addWidget(m_tabWidget);

    QVBoxLayout *outerLayout = new QVBoxLayout(this);

    loadLastSession();
    outerLayout->setContentsMargins(0, 0, 0, 0);
    outerLayout->addWidget(m_contentWidget);
    setLayout(outerLayout);
    m_autosaveTimer = new QTimer(this);
    connect(m_autosaveTimer, &QTimer::timeout, this, &SlideWindow::saveLastSession);
    m_autosaveTimer->start(m_autosaveInterval * 1000);
}

void SlideWindow::addNewTab() {
    QTextEdit *edit = new QTextEdit(this);
    int index = m_tabWidget->addTab(edit, "Untitled");
    m_tabWidget->setCurrentIndex(index);
    m_filePaths[edit] = QString();
    connect(edit, &QTextEdit::textChanged, this, &SlideWindow::onTextChanged);
}

void SlideWindow::onTextChanged() {
    QTextEdit *edit = qobject_cast<QTextEdit*>(sender());
    int index = m_tabWidget->indexOf(edit);
    QString title = m_tabWidget->tabText(index);
    if (!title.endsWith("*"))
        m_tabWidget->setTabText(index, title + "*");
}

void SlideWindow::applyGeometryAndPosition() {
    QScreen *screen = (m_screenIndex == 0)
        ? QGuiApplication::screenAt(QCursor::pos())
        : QGuiApplication::screens().value(m_screenIndex - 1, QGuiApplication::primaryScreen());

    QRect geometry = screen->geometry();
    int h = geometry.height() * m_heightPercent;
    int w = geometry.width() * m_widthPercent;

    if (w <= 0 || h <= 0) {
        qWarning("Invalid window size: width=%d, height=%d", w, h);
        return;
    }

    setFixedSize(w, h);

    QPoint pos;
    switch (m_direction) {
        case Left: pos = QPoint(geometry.left() - w, geometry.top() + (geometry.height() - h) / 2); break;
        case Right: pos = QPoint(geometry.x() + geometry.width() - w, geometry.top() + (geometry.height() - h) / 2); break;
        case Top: pos = QPoint(geometry.left() + (geometry.width() - w) / 2, geometry.top() - h); break;
        case Bottom: pos = QPoint(geometry.left() + (geometry.width() - w) / 2, geometry.top() + geometry.height()); break;
    }

    qDebug("Applying geometry: size=%dx%d, pos=(%d,%d)", w, h, pos.x(), pos.y());
    move(pos);
}

void SlideWindow::animateSlide(bool visible) {
    qDebug() << "Animated toggle: visible =" << visible;

    QScreen *screen = (m_screenIndex == 0)
        ? QGuiApplication::screenAt(QCursor::pos())
        : QGuiApplication::screens().value(m_screenIndex - 1, QGuiApplication::primaryScreen());

    QRect screenGeometry = screen->geometry();
    int h = screenGeometry.height() * m_heightPercent;
    int w = screenGeometry.width() * m_widthPercent;

    setFixedSize(w, h);
    m_contentWidget->resize(w, h);

    // Position the window itself based on direction
    int windowX = 0, windowY = 0;
    if (m_direction == Left || m_direction == Right) {
        windowY = screenGeometry.top() + (screenGeometry.height() - h) / 2;
        windowX = (m_direction == Left) ? screenGeometry.left() : screenGeometry.right() - w;
    } else {
        windowX = screenGeometry.left() + (screenGeometry.width() - w) / 2;
        windowY = (m_direction == Top) ? screenGeometry.top() : screenGeometry.bottom() - h;
    }
    move(windowX, windowY);

    QPoint start, end;

    if (m_direction == Left) {
        start = visible ? QPoint(-w, 0) : QPoint(0, 0);
        end = visible ? QPoint(0, 0) : QPoint(-w, 0);
    } else if (m_direction == Right) {
        start = visible ? QPoint(w, 0) : QPoint(0, 0);
        end = visible ? QPoint(0, 0) : QPoint(w, 0);
    } else if (m_direction == Top) {
        start = visible ? QPoint(0, -h) : QPoint(0, 0);
        end = visible ? QPoint(0, 0) : QPoint(0, -h);
    } else if (m_direction == Bottom) {
        start = visible ? QPoint(0, h) : QPoint(0, 0);
        end = visible ? QPoint(0, 0) : QPoint(0, h);
    }

    if (visible) {
        m_contentWidget->move(start);
        show();
        raise();
        activateWindow();
    }

    QPropertyAnimation *anim = new QPropertyAnimation(m_contentWidget, "pos");
    anim->setStartValue(start);
    anim->setEndValue(end);
    anim->setDuration(400);
    anim->setEasingCurve(QEasingCurve::OutCubic);

    connect(anim, &QPropertyAnimation::finished, this, [this, visible, anim]() {
        if (!visible) hide();
        anim->deleteLater();
        m_isSliding = false;
        qDebug() << "Animation finished. Visible now:" << isVisible();
    });

    m_isSliding = true;
    anim->start();
}

void SlideWindow::openNote() {
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Note"), "", tr("Text Files (*.txt);;Markdown Files (*.md);;All Files (*)"));
    if (fileName.isEmpty())
        return;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("Error"), tr("Failed to open file."));
        return;
    }

    QTextEdit *edit = new QTextEdit(this);
    edit->setPlainText(file.readAll());
    int index = m_tabWidget->addTab(edit, QFileInfo(fileName).fileName());
    m_tabWidget->setCurrentIndex(index);
    m_filePaths[edit] = fileName;

    connect(edit, &QTextEdit::textChanged, this, &SlideWindow::onTextChanged);
}

void SlideWindow::saveCurrentNote() {
    QTextEdit *edit = qobject_cast<QTextEdit*>(m_tabWidget->currentWidget());
    if (!edit) return;

    QString path = m_filePaths.value(edit);
    if (path.isEmpty()) {
        path = QFileDialog::getSaveFileName(this, tr("Save Note"), "", tr("Text Files (*.txt);;Markdown Files (*.md);;All Files (*)"));
        if (path.isEmpty()) return;
        m_filePaths[edit] = path;
        m_tabWidget->setTabText(m_tabWidget->currentIndex(), QFileInfo(path).fileName());
    }

    QFile file(path);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << edit->toPlainText();
        file.close();
        QString title = m_tabWidget->tabText(m_tabWidget->currentIndex());
        if (title.endsWith("*"))
            m_tabWidget->setTabText(m_tabWidget->currentIndex(), title.left(title.length() - 1));
    }
}

void SlideWindow::saveAllNotes() {
    for (int i = 0; i < m_tabWidget->count(); ++i) {
        QTextEdit *edit = qobject_cast<QTextEdit*>(m_tabWidget->widget(i));
        if (!edit) continue;

        QString path = m_filePaths.value(edit);
        if (path.isEmpty()) {
            path = QFileDialog::getSaveFileName(this, tr("Save Note"), "", tr("Text Files (*.txt);;Markdown Files (*.md);;All Files (*)"));
            if (path.isEmpty()) continue;
            m_filePaths[edit] = path;
            m_tabWidget->setTabText(i, QFileInfo(path).fileName());
        }

        QFile file(path);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out << edit->toPlainText();
            file.close();
            QString title = m_tabWidget->tabText(i);
            if (title.endsWith("*"))
                m_tabWidget->setTabText(i, title.left(title.length() - 1));
        }
    }
}

void SlideWindow::closeTab(int index) {
    QWidget *widget = m_tabWidget->widget(index);
    QTextEdit *edit = qobject_cast<QTextEdit*>(widget);
    if (edit)
        m_filePaths.remove(edit);
    m_tabWidget->removeTab(index);
    widget->deleteLater();
}

void SlideWindow::toggleVisibility() {
    qDebug() << "Toggling to:" << (!isVisible());
    if (m_isSliding) qDebug() << "Blocked: already sliding";
    m_isSliding = true;
    animateSlide(!isVisible());
}

void SlideWindow::setupTrayIcon() {
    QMenu *trayMenu = new QMenu(this);
    QAction *toggleAction = trayMenu->addAction("Toggle Window");
    QAction *settingsAction = trayMenu->addAction("Settings");
    QAction *aboutAction = trayMenu->addAction("About");
    trayMenu->addSeparator();
    QAction *exitAction = trayMenu->addAction("Exit");

    connect(toggleAction, &QAction::triggered, this, &SlideWindow::toggleVisibility);
    connect(settingsAction, &QAction::triggered, this, &SlideWindow::showSettingsDialog);
    connect(aboutAction, &QAction::triggered, this, &SlideWindow::showAboutDialog);
    connect(exitAction, &QAction::triggered, qApp, &QApplication::quit);

    m_trayIcon = new QSystemTrayIcon(this);
    m_trayIcon->setIcon(QIcon(":/resources/Slidenote.svg"));
    m_trayIcon->setContextMenu(trayMenu);
    m_trayIcon->show();
}

void SlideWindow::setupHotkey() {
    if (m_hotkey) {
        m_hotkey->disconnect();
        delete m_hotkey;
    }

    m_hotkey = new QHotkey(QKeySequence(m_hotkeySequence), true, this);
    connect(m_hotkey, &QHotkey::activated, this, &SlideWindow::toggleVisibility);
}

void SlideWindow::showSettingsDialog() {
    SettingsDialog dlg(this);
    QSettings settings("Hellmark Programming Group", "Slidenote");

    dlg.setScreenIndex(m_screenIndex);
    dlg.setSlideDirection(static_cast<int>(m_direction));
    dlg.setHeightPercent(m_heightPercent * 100);
    dlg.setWidthPercent(m_widthPercent * 100);
    dlg.setHotkeySequence(m_hotkeySequence);
    dlg.setAutosaveInterval(m_autosaveInterval);
    dlg.setStartVisible(m_startVisible);
    dlg.setReopenLastSession(settings.value("reopenLastSession", false).toBool());

    if (dlg.exec() == QDialog::Accepted) {
        m_screenIndex = dlg.screenIndex();
        m_direction = static_cast<SlideDirection>(dlg.slideDirection());
        m_heightPercent = dlg.heightPercent() / 100.0;
        m_widthPercent = dlg.widthPercent() / 100.0;
        m_hotkeySequence = dlg.hotkeySequence();
        m_autosaveInterval = dlg.autosaveInterval();
        m_autosaveTimer->start(m_autosaveInterval * 1000);
        m_startVisible = dlg.startVisible();
        m_reopenLastSession = dlg.reopenLastSession();

        setupHotkey();
        applyGeometryAndPosition();
        saveSettings();
    }
}

void SlideWindow::saveSettings() {
    QSettings settings("Hellmark Programming Group", "Slidenote");
    settings.setValue("autosaveInterval", m_autosaveInterval);
    settings.setValue("direction", static_cast<int>(m_direction));
    settings.setValue("height", m_heightPercent);
    settings.setValue("width", m_widthPercent);
    settings.setValue("hotkey", m_hotkeySequence);
    settings.setValue("screenIndex", m_screenIndex);
    settings.setValue("startVisible", m_startVisible);
    settings.setValue("reopenLastSession", m_reopenLastSession);
}

void SlideWindow::loadSettings() {
    QSettings settings("Hellmark Programming Group", "Slidenote");
    m_screenIndex = settings.value("screenIndex", 0).toInt();
    m_direction = static_cast<SlideDirection>(settings.value("direction", static_cast<int>(Left)).toInt());
    m_heightPercent = settings.value("height", 0.75).toDouble();
    m_widthPercent = settings.value("width", 0.3).toDouble();
    m_hotkeySequence = settings.value("hotkey", "Ctrl+Alt+S").toString();
    m_autosaveInterval = settings.value("autosaveInterval", 60).toInt();
    m_startVisible = settings.value("startVisible", true).toBool();
    m_reopenLastSession = settings.value("reopenLastSession", true).toBool();
}


void SlideWindow::loadLastSession() {
    QSettings settings("Hellmark Programming Group", "Slidenote");
    bool reopen = settings.value("reopenLastSession", false).toBool();
    int tabCount = settings.beginReadArray("sessionTabs");

    if (!reopen || tabCount == 0) {
        addNewTab();
        return;
    }

    for (int i = 0; i < tabCount; ++i) {
        settings.setArrayIndex(i);
        QString title = settings.value("title").toString();
        QString content = settings.value("content").toString();

        QTextEdit *edit = new QTextEdit(this);
        edit->setPlainText(content);
        int index = m_tabWidget->addTab(edit, title);
        m_tabWidget->setCurrentIndex(index);
        connect(edit, &QTextEdit::textChanged, this, &SlideWindow::onTextChanged);
    }

    settings.endArray();
}

void SlideWindow::saveLastSession() {
    QSettings settings("Hellmark Programming Group", "Slidenote");
    settings.beginWriteArray("sessionTabs");
    for (int i = 0; i < m_tabWidget->count(); ++i) {
        QTextEdit *edit = qobject_cast<QTextEdit*>(m_tabWidget->widget(i));
        if (!edit) continue;

        settings.setArrayIndex(i);
        settings.setValue("title", m_tabWidget->tabText(i));
        settings.setValue("content", edit->toPlainText());
    }
    settings.endArray();
}

SlideWindow::~SlideWindow() {
    saveLastSession();
}

void SlideWindow::showAboutDialog() {
    QMessageBox::about(this, "About Slidenote",
        "Slidenote v1.0\n"
        "© 2025 Hellmark Programming Group\n\n"
        "A simple note utility that never leaves your side!");
}

void SlideWindow::closeCurrentTab() {
    int index = m_tabWidget->currentIndex();
    if (index >= 0) {
        closeTab(index);
    }
}
