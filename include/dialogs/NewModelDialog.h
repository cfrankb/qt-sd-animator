#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QMessageBox>
#include <QUuid>

class NewModelDialog : public QDialog {
    Q_OBJECT

public:
    explicit NewModelDialog(QWidget* parent = nullptr);

    QString modelName() const { return name_edit->text(); }
    QString diffusionModel() const { return diffusion_model_edit->text(); }
    QString llm() const { return llm_edit->text(); }
    QString vae() const { return vae_edit->text(); }
    int width() const { return width_spin->value(); }
    int height() const { return height_spin->value(); }
    QString modelBasePath() const { return base_path_edit->text(); }
    QString uuid() const { return QUuid::createUuid().toString().remove("{").remove("}"); }

private:
    QLineEdit* name_edit;
    QLineEdit* diffusion_model_edit;
    QLineEdit* llm_edit;
    QLineEdit* vae_edit;
    QSpinBox* width_spin;
    QSpinBox* height_spin;
    QLineEdit* base_path_edit;
};
