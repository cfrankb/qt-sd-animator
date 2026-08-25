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
#include <QCheckBox>

class NewModelDialog : public QDialog {
    Q_OBJECT

public:
    explicit NewModelDialog(QWidget* parent = nullptr);

    QString modelName() const { return m_nameEdit->text(); }
    QString diffusionModel() const { return m_diffusionModelEdit->text(); }
    QString llm() const { return m_llmEdit->text(); }
    QString vae() const { return m_vaeEdit->text(); }
    int width() const { return m_widthSpin->value(); }
    int height() const { return m_heightSpin->value(); }
    bool sourceImageRequired() const { return m_sourceImageRequiredCheck->isChecked(); }
    QString uuid() const { return QUuid::createUuid().toString().remove("{").remove("}"); }

private:
    QLineEdit* m_nameEdit;
    QLineEdit* m_diffusionModelEdit;
    QLineEdit* m_llmEdit;
    QLineEdit* m_vaeEdit;
    QSpinBox* m_widthSpin;
    QSpinBox* m_heightSpin;
    QCheckBox* m_sourceImageRequiredCheck;
};
