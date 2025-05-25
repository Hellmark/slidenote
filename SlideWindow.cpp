#include "SlideWindow.h"
#include "SettingsDialog.h"

#include <QApplication>
#include <QScreen>
#include <QVBoxLayout>
#include <QTextEdit>
#include <QToolBar>
#include <QAction>
#include <QFileDialog>
#include <QMessageBox>
#include <QMenu>
#include <QHotkey>

SlideWindow::SlideWindow(QWidget *parent)
    : QWidget(parent),
      m_direction(Left),
      m_heightPercent(0.75),
      m_hotkey(nullptr),
      m_trayIcon(nullptr),
      m_animation(new QPropertyAnimation(this, "pos", this))
{
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setupUI();
    setupTrayIcon();
    loadSettings();
    setupHotkey();
    applyGeometryAndPosition();
}

void SlideWindow::setupUI()
{
    m_toolBar = new QToolBar(this);
    m_tabWidget = new QTabWidget(this);
    m_tabWidget->setTabsClosable(true);
    m_tabWidget->setMovable(true);

    connect(m_tabWidget, &QTabWidget::tabCloseRequested, this, &SlideWindow::closeTab);

    QAction *newNote = m_toolBar->addAction(QIcon::fromTheme("document-new"), "New");
    QAction *openNote = m_toolBar->addAction(QIcon::fromTheme("document-open"), "Open");
    QAction *saveNote = m_toolBar->addAction(QIcon::fromTheme("document-save"), "Save");
    QAction *saveAllNote = m_toolBar->addAction(QIcon::fromTheme("document-save-all"), "Save All");
    m_toolBar->addSeparator();
    QAction *settings = m_toolBar->addAction(QIcon::fromTheme("preferences-system"), "Settings");
    QAction *quitApp = m_toolBar->addAction(QIcon::fromTheme("application-exit"),"Quit");

    connect(quitApp, &QAction::triggered, qApp, &QApplication::quit);


    connect(newNote, &QAction::triggered, this, &SlideWindow::addNewTab);
    connect(openNote, &QAction::triggered, this, &SlideWindow::openNote);
    connect(saveNote, &QAction::triggered, this, &SlideWindow::saveCurrentNote);
    connect(saveAllNote, &QAction::triggered, this, &SlideWindow::saveAllNotes);
    connect(settings, &QAction::triggered, this, &SlideWindow::showSettingsDialog);
    connect(quitApp, &QAction::triggered, qApp, &QApplication::quit);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(m_toolBar);
    layout->addWidget(m_tabWidget);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);

    addNewTab();
}

void SlideWindow::addNewTab()
{
    auto *edit = new QTextEdit(this);
    int index = m_tabWidget->addTab(edit, "Untitled");
    m_filePaths[edit] = QString();
    m_tabWidget->setCurrentIndex(index);
    connect(edit, &QTextEdit::textChanged, this, &SlideWindow::onTextChanged);
}

void SlideWindow::onTextChanged()
{
    int index = m_tabWidget->currentIndex();
    QString title = m_tabWidget->tabText(index);
    if (!title.endsWith("*")) {
        m_tabWidget->setTabText(index, title + "*");
    }
}

void SlideWindow::saveCurrentNote()
{
    auto *edit = qobject_cast<QTextEdit *>(m_tabWidget->currentWidget());
    if (!edit) return;
    QString path = QFileDialog::getSaveFileName(this, "Save Note");
    if (path.isEmpty()) return;

    QFile file(path);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << edit->toPlainText();
        file.close();
        m_tabWidget->setTabText(m_tabWidget->currentIndex(), QFileInfo(path).fileName());
    }
}

void SlideWindow::saveAllNotes()
{
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


void SlideWindow::openNote()
{
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


void SlideWindow::closeCurrentTab()
{
    int index = m_tabWidget->currentIndex();
    if (index >= 0) closeTab(index);
}

void SlideWindow::closeTab(int index)
{
    QWidget *w = m_tabWidget->widget(index);
    m_tabWidget->removeTab(index);
    delete w;
}

void SlideWindow::setupTrayIcon()
{
    m_trayIcon = new QSystemTrayIcon(QIcon(":/Slidenote.svg"), this);
    QMenu *menu = new QMenu(this);

    QAction *toggleAction = menu->addAction("Toggle Window");
    QAction *settingsAction = menu->addAction("Settings");
    menu->addSeparator();
    QAction *quitAction = menu->addAction("Quit");

    connect(toggleAction, &QAction::triggered, this, &SlideWindow::toggleVisibility);
    connect(settingsAction, &QAction::triggered, this, &SlideWindow::showSettingsDialog);
    connect(quitAction, &QAction::triggered, qApp, &QApplication::quit);

    m_trayIcon->setContextMenu(menu);
    m_trayIcon->show();
}

void SlideWindow::setupHotkey()
{
    m_hotkey = new QHotkey(QKeySequence(m_hotkeySequence.isEmpty() ? "Alt+F12" : m_hotkeySequence), true, this);
    connect(m_hotkey, &QHotkey::activated, this, &SlideWindow::toggleVisibility);
}

void SlideWindow::toggleVisibility()
{
    if (m_isSliding)
        return;

    m_isSliding = true;

    if (isVisible()) {
        animateSlide(false);
    } else {
        animateSlide(true);
    }
}

void SlideWindow::animateSlide(bool visible)
{
    disconnect(m_animation, nullptr, nullptr, nullptr);  // disconnect all animation signals

    QRect screen = QGuiApplication::screenAt(QCursor::pos())->geometry();
    int w = width();
    int h = screen.height() * m_heightPercent;

    setFixedSize(w, h);
    QPoint start, end;

    if (visible) {
        if (!isVisible()) {
            this->show();  // only show if hidden
            this->raise();
        }
    }

    // Calculate animation positions
    switch (m_direction) {
        case Left:
            start = QPoint(visible ? -w : x(), (screen.height() - h) / 2);
            end = QPoint(visible ? 0 : -w, start.y());
            break;
        case Right:
            start = QPoint(visible ? screen.width() : x(), (screen.height() - h) / 2);
            end = QPoint(visible ? screen.width() - w : screen.width(), start.y());
            break;
        case Top:
            start = QPoint((screen.width() - w) / 2, visible ? -h : y());
            end = QPoint(start.x(), visible ? 0 : -h);
            break;
        case Bottom:
            start = QPoint((screen.width() - w) / 2, visible ? screen.height() : y());
            end = QPoint(start.x(), visible ? screen.height() - h : screen.height());
            break;
    }

    QPoint actualStart = visible ? start : pos();
    QPoint actualEnd = visible ? end : start;

    m_animation->stop();
    m_animation->setStartValue(actualStart);
    m_animation->setEndValue(actualEnd);
    m_animation->setEasingCurve(QEasingCurve::OutCubic);
    m_animation->setDuration(500);
    m_animation->start();

    connect(m_animation, &QPropertyAnimation::finished, this, [this, visible]() {
        if (!visible)
            this->hide();
        m_isSliding = false;
    });
}



void SlideWindow::showSettingsDialog()
{
    SettingsDialog dlg(this);
    dlg.setSlideDirection(static_cast<int>(m_direction));
    dlg.setHeightPercent(m_heightPercent * 100);
    dlg.setWidthPercent(m_widthPercent * 100);
    dlg.setHotkeySequence(m_hotkeySequence);

    if (dlg.exec() == QDialog::Accepted) {
        m_direction = static_cast<SlideDirection>(dlg.slideDirection());
        m_heightPercent = dlg.heightPercent() / 100.0;
        m_widthPercent = dlg.widthPercent() / 100.0;
        m_hotkeySequence = dlg.hotkeySequence();
        setupHotkey();
        applyGeometryAndPosition();
        saveSettings();
    }
}

void SlideWindow::applyGeometryAndPosition()
{
    QRect screen = QGuiApplication::screenAt(QCursor::pos())->geometry();
    int h = screen.height() * m_heightPercent;
    int w = screen.width() * m_widthPercent;
    QPoint pos;
    setFixedSize(w, h);

    if (m_direction == Left) {
        pos = QPoint(-w, (screen.height() - h) / 2);
    } else if (m_direction == Right) {
        pos = QPoint(screen.width(), (screen.height() - h) / 2);
    } else if (m_direction == Top) {
        pos = QPoint((screen.width() - w) / 2, -h);
    } else {
        pos = QPoint((screen.width() - w) / 2, screen.height());
    }

    move(pos);
}

void SlideWindow::saveSettings()
{
    QSettings settings("Hellmark Programming Group", "Slidenote");
    settings.setValue("direction", static_cast<int>(m_direction));
    settings.setValue("height", m_heightPercent);
    settings.setValue("width", m_widthPercent);
    settings.setValue("hotkey", m_hotkeySequence);
}

void SlideWindow::loadSettings()
{
    QSettings settings("Hellmark Programming Group", "Slidenote");
    m_direction = static_cast<SlideDirection>(settings.value("direction", static_cast<int>(Left)).toInt());
    m_heightPercent = settings.value("height", 0.75).toDouble();
    m_widthPercent = settings.value("width", 0.15).toDouble();
    m_hotkeySequence = settings.value("hotkey", "Alt+F12").toString();
}
