#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFormLayout>
#include <QMessageBox>
#include <QUuid>
#include <QCheckBox>
#include <QPlainTextEdit>
#include "SettingsManager.h"

class ModelManagerDialog : public QDialog {
    Q_OBJECT

public:
    explicit ModelManagerDialog(QWidget* parent = nullptr);

    QString selectedModelUuid() const;
    QString selectedModelName() const;

private slots:
    void addModel();
    void duplicateModel();
    void deleteModel();
    void onModelChanged(int index);
    void acceptDialog();

private:
    void populateCombo();
    void saveFormToLocal();
    void loadFormFromLocal(int index);

    QComboBox* m_modelCombo;
    QList<ModelSettings> m_localModels;
    int m_currentModelIndex = -1;

    QLineEdit* m_nameEdit;
    QLineEdit* m_diffusionModelEdit;
    QLineEdit* m_llmEdit;
    QLineEdit* m_vaeEdit;
    QCheckBox* m_sourceImageRequiredCheck;
    QComboBox* m_extCombo;
    QPlainTextEdit* m_parametersEdit;
    QPlainTextEdit* m_notesEdit;
};
