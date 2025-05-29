#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QComboBox>
#include <QSpinBox>
#include <QKeySequenceEdit>
#include <QPushButton>

#include <QCheckBox>

class SettingsDialog : public QDialog {
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);

    void setSlideDirection(int directionIndex);
    int slideDirection() const;

    void setHeightPercent(double percent);
    double heightPercent() const;

    void setWidthPercent(double percent);
    double widthPercent() const;

    void setHotkeySequence(const QString &sequence);
    QString hotkeySequence() const;

    void setScreenIndex(int index);
    int screenIndex() const;

    void setReopenLastSession(bool enabled);
    bool reopenLastSession() const;

    void setAutosaveInterval(int seconds);
    int autosaveInterval() const;

    bool startVisible() const;
    void setStartVisible(bool visible);

private:
    QCheckBox *reopenCheck;
    QComboBox *directionCombo;
    QSpinBox *heightSpin;
    QSpinBox *widthSpin;
    QKeySequenceEdit *hotkeyEdit;
    QComboBox *screenCombo;
    QSpinBox *autosaveSpin;
    QCheckBox *startVisibleCheck;
};

#endif // SETTINGSDIALOG_H
