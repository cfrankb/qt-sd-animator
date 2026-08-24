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
    setMinimumSize(600, 400);

    auto* mainLayout = new QVBoxLayout(this);

    auto* list_layout = new QHBoxLayout;
    mainLayout->addLayout(list_layout);

    models_list = new QListWidget;
    list_layout->addWidget(models_list);

    auto* btn_layout = new QVBoxLayout;
    auto* add_btn = new QPushButton("Add");
    auto* edit_btn = new QPushButton("Save");
    auto* dup_btn = new QPushButton("Duplicate");
    auto* del_btn = new QPushButton("Delete");
    btn_layout->addWidget(add_btn);
    btn_layout->addWidget(edit_btn);
    btn_layout->addWidget(dup_btn);
    btn_layout->addWidget(del_btn);
    list_layout->addLayout(btn_layout);

    connect(add_btn, &QPushButton::clicked, this, &ModelManagerDialog::addModel);
    connect(edit_btn, &QPushButton::clicked, this, &ModelManagerDialog::editModel);
    connect(dup_btn, &QPushButton::clicked, this, &ModelManagerDialog::duplicateModel);
    connect(del_btn, &QPushButton::clicked, this, &ModelManagerDialog::deleteModel);
    connect(models_list, &QListWidget::itemSelectionChanged, this, [this]() {
        auto items = models_list->selectedItems();
        if (items.isEmpty()) {
            clearForm();
            return;
        }
        int row = models_list->row(items.first());
        if (row >= 0 && row < SettingsManager::instance().models.size()) {
            loadForm(SettingsManager::instance().models[row].uuid);
        }
    });

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
        if (startDir.isEmpty()) {
            startDir = QDir::homePath();
        }
        QString file = QFileDialog::getOpenFileName(this, "Select Diffusion Model", startDir, fileFilters);
        if (!file.isEmpty()) {
            diffusion_model_edit->setText(file);
        }
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
        if (startDir.isEmpty()) {
            startDir = QDir::homePath();
        }
        QString file = QFileDialog::getOpenFileName(this, "Select LLM", startDir, fileFilters);
        if (!file.isEmpty()) {
            llm_edit->setText(file);
        }
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
        if (startDir.isEmpty()) {
            startDir = QDir::homePath();
        }
        QString file = QFileDialog::getOpenFileName(this, "Select VAE", startDir, fileFilters);
        if (!file.isEmpty()) {
            vae_edit->setText(file);
        }
    });

    form_layout->addRow("Width:", width_spin = new QSpinBox);
    width_spin->setRange(64, 2048);
    width_spin->setValue(512);
    form_layout->addRow("Height:", height_spin = new QSpinBox);
    height_spin->setRange(64, 4096);
    height_spin->setValue(512);
    form_layout->addRow("Source Image Required:", source_image_required_check = new QCheckBox);
    source_image_required_check->setChecked(true);
    
    form_layout->addRow("Extension:", ext_combo = new QComboBox);
    ext_combo->addItems({".png", ".avi"});
    ext_combo->setCurrentText(".avi");

    form_layout->addRow("Additional Parameters:", parameters_edit = new QPlainTextEdit);
    parameters_edit->setPlaceholderText("One argument per line");
    parameters_edit->setMaximumHeight(100);

    auto* ok_cancel_layout = new QHBoxLayout;
    auto* ok_btn = new QPushButton("OK");
    auto* cancel_btn = new QPushButton("Cancel");
    ok_cancel_layout->addWidget(ok_btn);
    ok_cancel_layout->addWidget(cancel_btn);
    mainLayout->addLayout(ok_cancel_layout);

    connect(ok_btn, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancel_btn, &QPushButton::clicked, this, &QDialog::reject);

    populateModels();
}

void ModelManagerDialog::populateModels() {
    models_list->clear();
    for (const auto& m : SettingsManager::instance().models) {
        models_list->addItem(m.name);
    }
}

void ModelManagerDialog::clearForm() {
    name_edit->clear();
    diffusion_model_edit->clear();
    llm_edit->clear();
    vae_edit->clear();
    width_spin->setValue(512);
    height_spin->setValue(512);
    source_image_required_check->setChecked(true);
    ext_combo->setCurrentText(".avi");
    parameters_edit->clear();
}

void ModelManagerDialog::loadForm(const QString& uuid) {
    for (const auto& m : SettingsManager::instance().models) {
        if (m.uuid == uuid) {
            name_edit->setText(m.name);
            diffusion_model_edit->setText(m.diffusionModel);
            llm_edit->setText(m.llm);
            vae_edit->setText(m.vae);
            width_spin->setValue(m.width);
            height_spin->setValue(m.height);
            source_image_required_check->setChecked(m.sourceImageRequired);
            ext_combo->setCurrentText(m.ext);
            parameters_edit->setPlainText(m.additionalParameters.join('\n'));
            return;
        }
    }
}

QString ModelManagerDialog::selectedModelUuid() const {
    auto items = models_list->selectedItems();
    if (items.isEmpty()) return QString();
    int row = models_list->row(items.first());
    if (row < 0 || row >= SettingsManager::instance().models.size()) return QString();
    return SettingsManager::instance().models[row].uuid;
}

QString ModelManagerDialog::selectedModelName() const {
    auto items = models_list->selectedItems();
    if (items.isEmpty()) return QString();
    return items.first()->text();
}

void ModelManagerDialog::addModel() {
    QString name = name_edit->text();
    if (name.isEmpty()) {
        QMessageBox::warning(this, "Validation Error", "Model name is required.");
        return;
    }
    ModelSettings m;
    m.name = name;
    m.diffusionModel = diffusion_model_edit->text();
    m.llm = llm_edit->text();
    m.vae = vae_edit->text();
    m.width = width_spin->value();
    m.height = height_spin->value();
    m.sourceImageRequired = source_image_required_check->isChecked();
    m.ext = ext_combo->currentText();
    QStringList params = parameters_edit->toPlainText().split('\n', Qt::SkipEmptyParts);
    for (int i = 0; i < params.size(); ++i) {
        params[i] = params[i].trimmed();
    }
    m.additionalParameters = params;
    m.uuid = QUuid::createUuid().toString().remove("{").remove("}");
    SettingsManager::instance().models << m;
    SettingsManager::instance().save();
    populateModels();
    clearForm();
}

void ModelManagerDialog::editModel() {
    auto uuid = selectedModelUuid();
    if (uuid.isEmpty()) {
        QMessageBox::warning(this, "Validation Error", "Select a model to edit.");
        return;
    }
    for (auto& m : SettingsManager::instance().models) {
        if (m.uuid == uuid) {
            m.name = name_edit->text();
            m.diffusionModel = diffusion_model_edit->text();
            m.llm = llm_edit->text();
            m.vae = vae_edit->text();
            m.width = width_spin->value();
            m.height = height_spin->value();
            m.sourceImageRequired = source_image_required_check->isChecked();
            m.ext = ext_combo->currentText();
            QStringList params = parameters_edit->toPlainText().split('\n', Qt::SkipEmptyParts);
            for (int i = 0; i < params.size(); ++i) {
                params[i] = params[i].trimmed();
            }
            m.additionalParameters = params;
            SettingsManager::instance().save();
            populateModels();
            return;
        }
    }
}

void ModelManagerDialog::duplicateModel() {
    auto uuid = selectedModelUuid();
    if (uuid.isEmpty()) {
        QMessageBox::warning(this, "Validation Error", "Select a model to duplicate.");
        return;
    }
    for (const auto& m : SettingsManager::instance().models) {
        if (m.uuid == uuid) {
            ModelSettings new_m = m;
            new_m.name = m.name + " (copy)";
            new_m.uuid = QUuid::createUuid().toString().remove("{").remove("}");
            SettingsManager::instance().models << new_m;
            SettingsManager::instance().save();
            populateModels();
            return;
        }
    }
}

void ModelManagerDialog::deleteModel() {
    auto uuid = selectedModelUuid();
    if (uuid.isEmpty()) {
        QMessageBox::warning(this, "Validation Error", "Select a model to delete.");
        return;
    }
    auto& models = SettingsManager::instance().models;
    for (int i = 0; i < models.size(); ++i) {
        if (models[i].uuid == uuid) {
            models.removeAt(i);
            break;
        }
    }
    SettingsManager::instance().save();
    populateModels();
    clearForm();
}
