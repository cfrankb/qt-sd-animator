#pragma once

#include <QDialog>
#include <QListWidget>
#include <QLineEdit>
#include <QSpinBox>
#include <QComboBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFormLayout>
#include <QMessageBox>
#include <QUuid>
#include <QTextEdit>
#include <QCheckBox>
#include <QPlainTextEdit>

class ModelManagerDialog : public QDialog {
    Q_OBJECT

public:
    explicit ModelManagerDialog(QWidget* parent = nullptr);

    QString selectedModelUuid() const;
    QString selectedModelName() const;

private slots:
    void addModel();
    void editModel();
    void duplicateModel();
    void deleteModel();

private:
    void populateModels();
    void clearForm();
    void loadForm(const QString& uuid);

    QListWidget* models_list;
    QLineEdit* name_edit;
    QLineEdit* diffusion_model_edit;
    QLineEdit* llm_edit;
    QLineEdit* vae_edit;
    QCheckBox* source_image_required_check;
    QComboBox* ext_combo;
    QPlainTextEdit* parameters_edit;
    QPlainTextEdit* notes_edit;
};
