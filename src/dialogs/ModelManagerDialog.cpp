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
    model_combo = new QComboBox;
    topLayout->addWidget(new QLabel("Model:"));
    topLayout->addWidget(model_combo);

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
    connect(model_combo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ModelManagerDialog::onModelChanged);

    // Form
    auto* form_layout = new QFormLayout;
    mainLayout->addLayout(form_layout);

    form_layout->addRow("Name:", name_edit = new QLineEdit);

    auto* diffusion_layout = new QHBoxLayout;
    diffusion_model_edit = new QLineEdit;
    diffusion_layout->addWidget(diffusion_model_edit);
    auto* diffusion_btn = new QPushButton("Browse...");
    diffusion_layout->addWidget(diffusion_btn);
    form_layout->addRow("Diffusion Model:", diffusion_layout);
    connect(diffusion_btn, &QPushButton::clicked, [this]() {
        QString currentPath = diffusion_model_edit->text();
        QString startDir = QFileInfo(currentPath).absolutePath();
        if (startDir.isEmpty()) startDir = QDir::homePath();
        QString file = QFileDialog::getOpenFileName(this, "Select Diffusion Model", startDir, fileFilters);
        if (!file.isEmpty()) diffusion_model_edit->setText(file);
    });

    auto* llm_layout = new QHBoxLayout;
    llm_edit = new QLineEdit;
    llm_layout->addWidget(llm_edit);
    auto* llm_btn = new QPushButton("Browse...");
    llm_layout->addWidget(llm_btn);
    form_layout->addRow("LLM:", llm_layout);
    connect(llm_btn, &QPushButton::clicked, [this]() {
        QString currentPath = llm_edit->text();
        QString startDir = QFileInfo(currentPath).absolutePath();
        if (startDir.isEmpty()) startDir = QDir::homePath();
        QString file = QFileDialog::getOpenFileName(this, "Select LLM", startDir, fileFilters);
        if (!file.isEmpty()) llm_edit->setText(file);
    });

    auto* vae_layout = new QHBoxLayout;
    vae_edit = new QLineEdit;
    vae_layout->addWidget(vae_edit);
    auto* vae_btn = new QPushButton("Browse...");
    vae_layout->addWidget(vae_btn);
    form_layout->addRow("VAE:", vae_layout);
    connect(vae_btn, &QPushButton::clicked, [this]() {
        QString currentPath = vae_edit->text();
        QString startDir = QFileInfo(currentPath).absolutePath();
        if (startDir.isEmpty()) startDir = QDir::homePath();
        QString file = QFileDialog::getOpenFileName(this, "Select VAE", startDir, fileFilters);
        if (!file.isEmpty()) vae_edit->setText(file);
    });

    form_layout->addRow("Source Image Required:", source_image_required_check = new QCheckBox);
    source_image_required_check->setChecked(true);

    form_layout->addRow("Extension:", ext_combo = new QComboBox);
    ext_combo->addItems({".png", ".avi"});
    ext_combo->setCurrentText(".avi");

    form_layout->addRow("Additional Parameters:", parameters_edit = new QPlainTextEdit);
    parameters_edit->setPlaceholderText("One argument per line");
    parameters_edit->setMaximumHeight(100);

    form_layout->addRow("Notes:", notes_edit = new QPlainTextEdit);
    notes_edit->setPlaceholderText("Notes about this model");
    notes_edit->setMaximumHeight(100);

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
    localModels = SettingsManager::instance().models;
    populateCombo();
    if (model_combo->count() > 0) {
        model_combo->setCurrentIndex(0);
        currentModelIndex = 0;
        loadFormFromLocal(0);
    } else {
        currentModelIndex = -1;
    }
}

void ModelManagerDialog::populateCombo() {
    model_combo->blockSignals(true);
    model_combo->clear();
    for (const auto& m : localModels) {
        model_combo->addItem(m.name.isEmpty() ? "(unnamed)" : m.name, m.uuid);
    }
    model_combo->blockSignals(false);
}

void ModelManagerDialog::onModelChanged(int index) {
    if (currentModelIndex >= 0 && currentModelIndex < localModels.size()) {
        saveFormToLocal();
    }
    currentModelIndex = index;
    if (index >= 0 && index < localModels.size()) {
        // Update combo text if name changed
        QString display = localModels[index].name.isEmpty() ? "(unnamed)" : localModels[index].name;
        if (model_combo->itemText(index) != display) {
            model_combo->blockSignals(true);
            model_combo->setItemText(index, display);
            model_combo->blockSignals(false);
        }
        loadFormFromLocal(index);
    }
}

void ModelManagerDialog::saveFormToLocal() {
    if (currentModelIndex < 0 || currentModelIndex >= localModels.size()) return;
    ModelSettings &m = localModels[currentModelIndex];
    m.name = name_edit->text();
    m.diffusionModel = diffusion_model_edit->text();
    m.llm = llm_edit->text();
    m.vae = vae_edit->text();
    m.sourceImageRequired = source_image_required_check->isChecked();
    m.ext = ext_combo->currentText();
    QStringList params = parameters_edit->toPlainText().split('\n', Qt::SkipEmptyParts);
    for (int i = 0; i < params.size(); ++i) params[i] = params[i].trimmed();
    m.additionalParameters = params;
    m.notes = notes_edit->toPlainText();
}

void ModelManagerDialog::loadFormFromLocal(int index) {
    if (index < 0 || index >= localModels.size()) return;
    const ModelSettings &m = localModels[index];
    name_edit->setText(m.name);
    diffusion_model_edit->setText(m.diffusionModel);
    llm_edit->setText(m.llm);
    vae_edit->setText(m.vae);
    source_image_required_check->setChecked(m.sourceImageRequired);
    ext_combo->setCurrentText(m.ext);
    parameters_edit->setPlainText(m.additionalParameters.join('\n'));
    notes_edit->setPlainText(m.notes);
}

void ModelManagerDialog::addModel() {
    ModelSettings m;
    m.name = "New Model";
    m.uuid = QUuid::createUuid().toString().remove("{").remove("}");
    m.sourceImageRequired = true;
    m.ext = ".avi";
    localModels.append(m);
    populateCombo();
    int idx = localModels.size() - 1;
    model_combo->setCurrentIndex(idx);
    currentModelIndex = idx;
    loadFormFromLocal(idx);
}

void ModelManagerDialog::duplicateModel() {
    if (currentModelIndex < 0 || currentModelIndex >= localModels.size()) {
        QMessageBox::warning(this, "Validation Error", "Select a model to duplicate.");
        return;
    }
    ModelSettings new_m = localModels[currentModelIndex];
    new_m.name = new_m.name + " (copy)";
    new_m.uuid = QUuid::createUuid().toString().remove("{").remove("}");
    localModels.append(new_m);
    populateCombo();
    int idx = localModels.size() - 1;
    model_combo->setCurrentIndex(idx);
    currentModelIndex = idx;
    loadFormFromLocal(idx);
}

void ModelManagerDialog::deleteModel() {
    if (currentModelIndex < 0 || currentModelIndex >= localModels.size()) {
        QMessageBox::warning(this, "Validation Error", "Select a model to delete.");
        return;
    }
    localModels.removeAt(currentModelIndex);
    populateCombo();
    if (localModels.isEmpty()) {
        currentModelIndex = -1;
        name_edit->clear();
        diffusion_model_edit->clear();
        llm_edit->clear();
        vae_edit->clear();
        source_image_required_check->setChecked(true);
        ext_combo->setCurrentText(".avi");
        parameters_edit->clear();
        notes_edit->clear();
        return;
    }
    if (currentModelIndex >= localModels.size()) {
        currentModelIndex = localModels.size() - 1;
    }
    model_combo->setCurrentIndex(currentModelIndex);
    loadFormFromLocal(currentModelIndex);
}

void ModelManagerDialog::acceptDialog() {
    if (currentModelIndex >= 0) {
        saveFormToLocal();
    }
    SettingsManager::instance().models = localModels;
    SettingsManager::instance().save();
    accept();
}

QString ModelManagerDialog::selectedModelUuid() const {
    if (currentModelIndex >= 0 && currentModelIndex < localModels.size())
        return localModels[currentModelIndex].uuid;
    return QString();
}

QString ModelManagerDialog::selectedModelName() const {
    if (currentModelIndex >= 0 && currentModelIndex < localModels.size())
        return localModels[currentModelIndex].name;
    return QString();
}
