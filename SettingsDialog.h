#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QComboBox>
#include <QSpinBox>
#include <QLineEdit>
#include <QPushButton>

class SettingsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SettingsDialog(QWidget *parent = nullptr);

    void setSlideDirection(int directionIndex);
    int slideDirection() const;

    void setHeightPercent(double percent);
    double heightPercent() const;

    void setHotkeySequence(const QString &sequence);
    QString hotkeySequence() const;

private:
    QComboBox *directionCombo;
    QSpinBox *heightSpin;
    QLineEdit *hotkeyEdit;
};

#endif // SETTINGSDIALOG_H
