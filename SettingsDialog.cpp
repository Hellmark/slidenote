#include "SettingsDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Settings");

    QLabel *dirLabel = new QLabel("Slide Direction:");
    directionCombo = new QComboBox;
    directionCombo->addItems({"Left", "Right", "Top", "Bottom"});

    QLabel *heightLabel = new QLabel("Window Height (%):");
    heightSpin = new QSpinBox;
    heightSpin->setRange(10, 100);
    heightSpin->setValue(75);

    QLabel *hotkeyLabel = new QLabel("Hotkey:");
    hotkeyEdit = new QLineEdit;

    QPushButton *okButton = new QPushButton("OK");
    QPushButton *cancelButton = new QPushButton("Cancel");

    connect(okButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(dirLabel);
    layout->addWidget(directionCombo);
    layout->addWidget(heightLabel);
    layout->addWidget(heightSpin);
    layout->addWidget(hotkeyLabel);
    layout->addWidget(hotkeyEdit);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);
    layout->addLayout(buttonLayout);

    setLayout(layout);
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

void SettingsDialog::setHotkeySequence(const QString &sequence) {
    hotkeyEdit->setText(sequence);
}

QString SettingsDialog::hotkeySequence() const {
    return hotkeyEdit->text();
}
