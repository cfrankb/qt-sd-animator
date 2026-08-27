#include "dialogs/ModelManagerDialog.h"
#include "SettingsManager.h"
#include <QMessageBox>
#include <QUuid>
#include <QFileDialog>
#include <QDir>
#include <QFileInfo>

static char fileFilters[] = "All Supported(*.gguf *.safetensors);;GGUF Files (*.gguf);;SafeTensors Files (*.safetensors);;All Files (*)";

ModelManagerDialog::ModelManagerDialog(QWidget* parent)
    : QDialog(parent) {
    setWindowTitle("Model Manager");
    setMinimumSize(700, 500);

    auto* mainLayout = new QVBoxLayout(this);

    // Top row: model selector + buttons
    auto* topLayout = new QHBoxLayout;
    m_modelCombo = new QComboBox;
    topLayout->addWidget(new QLabel("Model:"));
    topLayout->addWidget(m_modelCombo);

    auto* add_btn = new QPushButton("Add");
    auto* dup_btn = new QPushButton("Duplicate");
    auto* del_btn = new QPushButton("Delete");
    topLayout->addWidget(add_btn);
    topLayout->addWidget(dup_btn);
    topLayout->addWidget(del_btn);
    mainLayout->addLayout(topLayout);

    connect(add_btn, &QPushButton::clicked, this, &ModelManagerDialog::addModel);
    connect(dup_btn, &QPushButton::clicked, this, &ModelManagerDialog::duplicateModel);
    connect(del_btn, &QPushButton::clicked, this, &ModelManagerDialog::deleteModel);
    connect(m_modelCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ModelManagerDialog::onModelChanged);

    // Form
    auto* form_layout = new QFormLayout;
    mainLayout->addLayout(form_layout);

    form_layout->addRow("Name:", m_nameEdit = new QLineEdit);

    auto* diffusion_layout = new QHBoxLayout;
    m_diffusionModelEdit = new QLineEdit;
    diffusion_layout->addWidget(m_diffusionModelEdit);
    auto* diffusion_btn = new QPushButton("Browse...");
    diffusion_layout->addWidget(diffusion_btn);
    form_layout->addRow("Diffusion Model:", diffusion_layout);
    connect(diffusion_btn, &QPushButton::clicked, [this]() {
        QString currentPath = m_diffusionModelEdit->text();
        QString startDir = QFileInfo(currentPath).absolutePath();
        if (startDir.isEmpty()) startDir = QDir::homePath();
        QString file = QFileDialog::getOpenFileName(this, "Select Diffusion Model", startDir, fileFilters);
        if (!file.isEmpty()) m_diffusionModelEdit->setText(file);
    });

    auto* llm_layout = new QHBoxLayout;
    m_llmEdit = new QLineEdit;
    llm_layout->addWidget(m_llmEdit);
    auto* llm_btn = new QPushButton("Browse...");
    llm_layout->addWidget(llm_btn);
    form_layout->addRow("LLM:", llm_layout);
    connect(llm_btn, &QPushButton::clicked, [this]() {
        QString currentPath = m_llmEdit->text();
        QString startDir = QFileInfo(currentPath).absolutePath();
        if (startDir.isEmpty()) startDir = QDir::homePath();
        QString file = QFileDialog::getOpenFileName(this, "Select LLM", startDir, fileFilters);
        if (!file.isEmpty()) m_llmEdit->setText(file);
    });

    auto* vae_layout = new QHBoxLayout;
    m_vaeEdit = new QLineEdit;
    vae_layout->addWidget(m_vaeEdit);
    auto* vae_btn = new QPushButton("Browse...");
    vae_layout->addWidget(vae_btn);
    form_layout->addRow("VAE:", vae_layout);
    connect(vae_btn, &QPushButton::clicked, [this]() {
        QString currentPath = m_vaeEdit->text();
        QString startDir = QFileInfo(currentPath).absolutePath();
        if (startDir.isEmpty()) startDir = QDir::homePath();
        QString file = QFileDialog::getOpenFileName(this, "Select VAE", startDir, fileFilters);
        if (!file.isEmpty()) m_vaeEdit->setText(file);
    });

    form_layout->addRow("Source Image Required:", m_sourceImageRequiredCheck = new QCheckBox);
    m_sourceImageRequiredCheck->setChecked(true);

    form_layout->addRow("Extension:", m_extCombo = new QComboBox);
    m_extCombo->addItems({".png", ".avi"});
    m_extCombo->setCurrentText(".avi");

    form_layout->addRow("Additional Parameters:", m_parametersEdit = new QPlainTextEdit);
    m_parametersEdit->setPlaceholderText("One argument per line");
    m_parametersEdit->setMaximumHeight(100);

    form_layout->addRow("Notes:", m_notesEdit = new QPlainTextEdit);
    m_notesEdit->setPlaceholderText("Notes about this model");
    m_notesEdit->setMaximumHeight(100);

    // OK / Cancel
    auto* ok_cancel_layout = new QHBoxLayout;
    auto* ok_btn = new QPushButton("OK");
    auto* cancel_btn = new QPushButton("Cancel");
    ok_cancel_layout->addStretch();
    ok_cancel_layout->addWidget(ok_btn);
    ok_cancel_layout->addWidget(cancel_btn);
    mainLayout->addLayout(ok_cancel_layout);

    connect(ok_btn, &QPushButton::clicked, this, &ModelManagerDialog::acceptDialog);
    connect(cancel_btn, &QPushButton::clicked, this, &QDialog::reject);

    // Load local copy
    m_localModels = SettingsManager::instance().m_models;
    populateCombo();
    if (m_modelCombo->count() > 0) {
        m_modelCombo->setCurrentIndex(0);
        m_currentModelIndex = 0;
        loadFormFromLocal(0);
    } else {
        m_currentModelIndex = -1;
    }
}

void ModelManagerDialog::populateCombo() {
    m_modelCombo->blockSignals(true);
    m_modelCombo->clear();
    for (const auto& m : m_localModels) {
        m_modelCombo->addItem(m.name.isEmpty() ? "(unnamed)" : m.name, m.uuid);
    }
    m_modelCombo->blockSignals(false);
}

void ModelManagerDialog::onModelChanged(int index) {
    if (m_currentModelIndex >= 0 && m_currentModelIndex < m_localModels.size()) {
        saveFormToLocal();
    }
    m_currentModelIndex = index;
    if (index >= 0 && index < m_localModels.size()) {
        // Update combo text if name changed
        QString display = m_localModels[index].name.isEmpty() ? "(unnamed)" : m_localModels[index].name;
        if (m_modelCombo->itemText(index) != display) {
            m_modelCombo->blockSignals(true);
            m_modelCombo->setItemText(index, display);
            m_modelCombo->blockSignals(false);
        }
        loadFormFromLocal(index);
    }
}

void ModelManagerDialog::saveFormToLocal() {
    if (m_currentModelIndex < 0 || m_currentModelIndex >= m_localModels.size()) return;
    ModelSettings &m = m_localModels[m_currentModelIndex];
    m.name = m_nameEdit->text();
    m.diffusionModel = m_diffusionModelEdit->text();
    m.llm = m_llmEdit->text();
    m.vae = m_vaeEdit->text();
    m.sourceImageRequired = m_sourceImageRequiredCheck->isChecked();
    m.ext = m_extCombo->currentText();
    QStringList params = m_parametersEdit->toPlainText().split('\n', Qt::SkipEmptyParts);
    for (int i = 0; i < params.size(); ++i) params[i] = params[i].trimmed();
    m.additionalParameters = params;
    m.notes = m_notesEdit->toPlainText();
}

void ModelManagerDialog::loadFormFromLocal(int index) {
    if (index < 0 || index >= m_localModels.size()) return;
    const ModelSettings &m = m_localModels[index];
    m_nameEdit->setText(m.name);
    m_diffusionModelEdit->setText(m.diffusionModel);
    m_llmEdit->setText(m.llm);
    m_vaeEdit->setText(m.vae);
    m_sourceImageRequiredCheck->setChecked(m.sourceImageRequired);
    m_extCombo->setCurrentText(m.ext);
    m_parametersEdit->setPlainText(m.additionalParameters.join('\n'));
    m_notesEdit->setPlainText(m.notes);
}

void ModelManagerDialog::addModel() {
    ModelSettings m;
    m.name = "New Model";
    m.uuid = QUuid::createUuid().toString().remove("{").remove("}");
    m.sourceImageRequired = true;
    m.ext = ".avi";
    m_localModels.append(m);
    populateCombo();
    int idx = m_localModels.size() - 1;
    m_modelCombo->setCurrentIndex(idx);
    m_currentModelIndex = idx;
    loadFormFromLocal(idx);
}

void ModelManagerDialog::duplicateModel() {
    if (m_currentModelIndex < 0 || m_currentModelIndex >= m_localModels.size()) {
        QMessageBox::warning(this, "Validation Error", "Select a model to duplicate.");
        return;
    }
    ModelSettings new_m = m_localModels[m_currentModelIndex];
    new_m.name = new_m.name + " (copy)";
    new_m.uuid = QUuid::createUuid().toString().remove("{").remove("}");
    m_localModels.append(new_m);
    populateCombo();
    int idx = m_localModels.size() - 1;
    m_modelCombo->setCurrentIndex(idx);
    m_currentModelIndex = idx;
    loadFormFromLocal(idx);
}

void ModelManagerDialog::deleteModel() {
    if (m_currentModelIndex < 0 || m_currentModelIndex >= m_localModels.size()) {
        QMessageBox::warning(this, "Validation Error", "Select a model to delete.");
        return;
    }
    m_localModels.removeAt(m_currentModelIndex);
    populateCombo();
    if (m_localModels.isEmpty()) {
        m_currentModelIndex = -1;
        m_nameEdit->clear();
        m_diffusionModelEdit->clear();
        m_llmEdit->clear();
        m_vaeEdit->clear();
        m_sourceImageRequiredCheck->setChecked(true);
        m_extCombo->setCurrentText(".avi");
        m_parametersEdit->clear();
        m_notesEdit->clear();
        return;
    }
    if (m_currentModelIndex >= m_localModels.size()) {
        m_currentModelIndex = m_localModels.size() - 1;
    }
    m_modelCombo->setCurrentIndex(m_currentModelIndex);
    loadFormFromLocal(m_currentModelIndex);
}

void ModelManagerDialog::acceptDialog() {
    if (m_currentModelIndex >= 0) {
        saveFormToLocal();
    }
    SettingsManager::instance().m_models = m_localModels;
    SettingsManager::instance().save();
    accept();
}

QString ModelManagerDialog::selectedModelUuid() const {
    if (m_currentModelIndex >= 0 && m_currentModelIndex < m_localModels.size())
        return m_localModels[m_currentModelIndex].uuid;
    return QString();
}

QString ModelManagerDialog::selectedModelName() const {
    if (m_currentModelIndex >= 0 && m_currentModelIndex < m_localModels.size())
        return m_localModels[m_currentModelIndex].name;
    return QString();
}
