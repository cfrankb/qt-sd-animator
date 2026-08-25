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

    QComboBox* model_combo;
    QList<ModelSettings> localModels;
    int currentModelIndex = -1;

    QLineEdit* name_edit;
    QLineEdit* diffusion_model_edit;
    QLineEdit* llm_edit;
    QLineEdit* vae_edit;
    QCheckBox* source_image_required_check;
    QComboBox* ext_combo;
    QPlainTextEdit* parameters_edit;
    QPlainTextEdit* notes_edit;
};
