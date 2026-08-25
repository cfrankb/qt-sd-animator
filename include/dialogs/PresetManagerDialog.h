#pragma once

#include <QDialog>
#include <QListWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QUuid>
#include <QRandomGenerator>

class PresetManagerDialog : public QDialog {
    Q_OBJECT

public:
    explicit PresetManagerDialog(QWidget* parent = nullptr);

    QString selectedPresetUuid() const;
    QString selectedPresetName() const;

private slots:
    void addPreset();
    void editPreset();
    void duplicatePreset();
    void deletePreset();

private:
    void populatePresets();
    void clearForm();
    void loadForm(const QString& uuid);

    QListWidget* m_presetsList;
    QLineEdit* m_nameEdit;
    QTextEdit* m_promptEdit;
    QTextEdit* m_negativePromptEdit;
    QLineEdit* m_seedEdit;
    QPushButton* m_randomSeedBtn;
};
