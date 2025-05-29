#include "SettingsDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QGuiApplication>
#include <QScreen>

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Settings");

    QLabel *screenLabel = new QLabel("Target Monitor:");
    screenCombo = new QComboBox;
    screenCombo->addItem("Auto (Follow Cursor)");
    const auto screens = QGuiApplication::screens();
    for (int i = 0; i < screens.size(); ++i) {
        const auto &s = screens[i];
        screenCombo->addItem(QString("Screen %1: %2x%3").arg(i + 1).arg(s->size().width()).arg(s->size().height()));
    }

    QLabel *dirLabel = new QLabel("Slide Direction:");
    directionCombo = new QComboBox;
    directionCombo->addItems({"Left", "Right", "Top", "Bottom"});

    QLabel *heightLabel = new QLabel("Window Height (%):");
    heightSpin = new QSpinBox;
    heightSpin->setRange(10, 100);
    heightSpin->setValue(75);

    QLabel *widthLabel = new QLabel("Window Width (%):");
    widthSpin = new QSpinBox;
    widthSpin->setRange(10, 100);
    widthSpin->setValue(30);

    QLabel *hotkeyLabel = new QLabel("Hotkey:");
    hotkeyEdit = new QKeySequenceEdit;

    QLabel *savedurLabel = new QLabel("Autosave Interval:");
    autosaveSpin = new QSpinBox();
    autosaveSpin->setRange(10, 3600);
    autosaveSpin->setValue(60);
    autosaveSpin->setSuffix(" sec");

    startVisibleCheck = new QCheckBox("Show window on startup");

    reopenCheck = new QCheckBox("Reopen last session");

    QPushButton *okButton = new QPushButton("OK");
    QPushButton *cancelButton = new QPushButton("Cancel");

    connect(okButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(screenLabel);
    layout->addWidget(screenCombo);
    layout->addWidget(dirLabel);
    layout->addWidget(directionCombo);
    layout->addWidget(heightLabel);
    layout->addWidget(heightSpin);
    layout->addWidget(widthLabel);
    layout->addWidget(widthSpin);
    layout->addWidget(hotkeyLabel);
    layout->addWidget(hotkeyEdit);
    layout->addWidget(savedurLabel);
    layout->addWidget(autosaveSpin);
    layout->addWidget(startVisibleCheck);
    layout->addWidget(reopenCheck);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);
    layout->addLayout(buttonLayout);

    setLayout(layout);
}

void SettingsDialog::setScreenIndex(int index) {
    screenCombo->setCurrentIndex(index);
}
int SettingsDialog::screenIndex() const {
    return screenCombo->currentIndex();
}

void SettingsDialog::setSlideDirection(int index) {
    directionCombo->setCurrentIndex(index);
}
int SettingsDialog::slideDirection() const {
    return directionCombo->currentIndex();
}

void SettingsDialog::setHeightPercent(double percent) {
    heightSpin->setValue(static_cast<int>(percent));
}
double SettingsDialog::heightPercent() const {
    return heightSpin->value();
}

void SettingsDialog::setWidthPercent(double percent) {
    widthSpin->setValue(static_cast<int>(percent));
}
double SettingsDialog::widthPercent() const {
    return widthSpin->value();
}

void SettingsDialog::setHotkeySequence(const QString &sequence) {
    hotkeyEdit->setKeySequence(sequence);
}
QString SettingsDialog::hotkeySequence() const {
    return hotkeyEdit->keySequence().toString();
}

void SettingsDialog::setAutosaveInterval(int seconds) {
    autosaveSpin->setValue(seconds);
}
int SettingsDialog::autosaveInterval() const {
    return autosaveSpin->value();
}

void SettingsDialog::setStartVisible(bool visible) {
    startVisibleCheck->setChecked(visible);
}
bool SettingsDialog::startVisible() const {
    return startVisibleCheck->isChecked();
}


void SettingsDialog::setReopenLastSession(bool enabled) {
    reopenCheck->setChecked(enabled);
}
bool SettingsDialog::reopenLastSession() const {
    return reopenCheck->isChecked();
}
