#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QComboBox>
#include <QSpinBox>
#include <QKeySequenceEdit>
#include <QPushButton>

class SettingsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SettingsDialog(QWidget *parent = nullptr);

    void setSlideDirection(int directionIndex);
    int slideDirection() const;

    void setHeightPercent(double hpercent);
    double heightPercent() const;
    void setWidthPercent(double wpercent);
    double widthPercent() const;

    void setHotkeySequence(const QString &sequence);
    QString hotkeySequence() const;

private:
    QComboBox *directionCombo;
    QSpinBox *heightSpin;
    QSpinBox *widthSpin;
    QKeySequenceEdit *hotkeyEdit;
};

#endif // SETTINGSDIALOG_H
