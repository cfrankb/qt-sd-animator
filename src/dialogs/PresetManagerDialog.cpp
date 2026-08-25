#include "dialogs/PresetManagerDialog.h"
#include "SettingsManager.h"
#include <QMessageBox>
#include <QUuid>

PresetManagerDialog::PresetManagerDialog(QWidget* parent)
    : QDialog(parent) {
    setWindowTitle("Preset Manager");
    setMinimumSize(600, 400);

    auto* mainLayout = new QVBoxLayout(this);

    auto* list_layout = new QHBoxLayout;
    mainLayout->addLayout(list_layout);

    m_presetsList = new QListWidget;
    list_layout->addWidget(m_presetsList);

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

    connect(add_btn, &QPushButton::clicked, this, &PresetManagerDialog::addPreset);
    connect(edit_btn, &QPushButton::clicked, this, &PresetManagerDialog::editPreset);
    connect(dup_btn, &QPushButton::clicked, this, &PresetManagerDialog::duplicatePreset);
    connect(del_btn, &QPushButton::clicked, this, &PresetManagerDialog::deletePreset);
    connect(m_presetsList, &QListWidget::itemSelectionChanged, this, [this]() {
        auto items = m_presetsList->selectedItems();
        if (items.isEmpty()) {
            clearForm();
            return;
        }
        int row = m_presetsList->row(items.first());
        if (row >= 0 && row < SettingsManager::instance().presets.size()) {
            loadForm(SettingsManager::instance().presets[row].uuid);
        }
    });

    auto* form_layout = new QVBoxLayout;
    mainLayout->addLayout(form_layout);

    form_layout->addWidget(new QLabel("Name:"));
    m_nameEdit = new QLineEdit;
    form_layout->addWidget(m_nameEdit);

    form_layout->addWidget(new QLabel("Prompt:"));
    m_promptEdit = new QTextEdit;
    m_promptEdit->setPlaceholderText("Enter prompt...");
    form_layout->addWidget(m_promptEdit);

    form_layout->addWidget(new QLabel("Negative Prompt:"));
    m_negativePromptEdit = new QTextEdit;
    m_negativePromptEdit->setPlaceholderText("Enter negative prompt...");
    form_layout->addWidget(m_negativePromptEdit);

    form_layout->addWidget(new QLabel("Seed:"));
    auto* seed_layout = new QHBoxLayout;
    m_seedEdit = new QLineEdit;
    m_seedEdit->setPlaceholderText("Enter seed value...");
    seed_layout->addWidget(m_seedEdit);
    m_randomSeedBtn = new QPushButton("Random");
    m_randomSeedBtn->setToolTip("Random seed");
    seed_layout->addWidget(m_randomSeedBtn);
    connect(m_randomSeedBtn, &QPushButton::clicked, this, [this]() {
        m_seedEdit->setText(QString::number(QRandomGenerator::global()->generate()));
    });
    form_layout->addLayout(seed_layout);

    auto* ok_cancel_layout = new QHBoxLayout;
    auto* ok_btn = new QPushButton("OK");
    auto* cancel_btn = new QPushButton("Cancel");
    ok_cancel_layout->addWidget(ok_btn);
    ok_cancel_layout->addWidget(cancel_btn);
    mainLayout->addLayout(ok_cancel_layout);

    connect(ok_btn, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancel_btn, &QPushButton::clicked, this, &QDialog::reject);

    populatePresets();
}

void PresetManagerDialog::populatePresets() {
    m_presetsList->clear();
    for (const auto& p : SettingsManager::instance().presets) {
        m_presetsList->addItem(p.name);
    }
}

void PresetManagerDialog::clearForm() {
    m_nameEdit->clear();
    m_promptEdit->clear();
    m_negativePromptEdit->clear();
    m_seedEdit->clear();
}

void PresetManagerDialog::loadForm(const QString& uuid) {
    for (const auto& p : SettingsManager::instance().presets) {
        if (p.uuid == uuid) {
            m_nameEdit->setText(p.name);
            m_promptEdit->setText(p.prompt);
            m_negativePromptEdit->setText(p.negativePrompt);
            m_seedEdit->setText(p.seed);
            return;
        }
    }
}

QString PresetManagerDialog::selectedPresetUuid() const {
    auto items = m_presetsList->selectedItems();
    if (items.isEmpty()) return QString();
    int row = m_presetsList->row(items.first());
    if (row < 0 || row >= SettingsManager::instance().presets.size()) return QString();
    return SettingsManager::instance().presets[row].uuid;
}

QString PresetManagerDialog::selectedPresetName() const {
    auto items = m_presetsList->selectedItems();
    if (items.isEmpty()) return QString();
    return items.first()->text();
}

void PresetManagerDialog::addPreset() {
    QString name = m_nameEdit->text();
    if (name.isEmpty()) {
        QMessageBox::warning(this, "Validation Error", "Preset name is required.");
        return;
    }
    PresetSettings p;
    p.name = name;
    p.prompt = m_promptEdit->toPlainText();
    p.negativePrompt = m_negativePromptEdit->toPlainText();
    p.seed = m_seedEdit->text();
    p.uuid = QUuid::createUuid().toString().remove("{").remove("}");
    SettingsManager::instance().presets << p;
    SettingsManager::instance().save();
    populatePresets();
    clearForm();
}

void PresetManagerDialog::editPreset() {
    auto uuid = selectedPresetUuid();
    if (uuid.isEmpty()) {
        QMessageBox::warning(this, "Validation Error", "Select a preset to edit.");
        return;
    }
    for (auto& p : SettingsManager::instance().presets) {
        if (p.uuid == uuid) {
            p.name = m_nameEdit->text();
            p.prompt = m_promptEdit->toPlainText();
            p.negativePrompt = m_negativePromptEdit->toPlainText();
            p.seed = m_seedEdit->text();
            SettingsManager::instance().save();
            populatePresets();
            return;
        }
    }
}

void PresetManagerDialog::duplicatePreset() {
    auto uuid = selectedPresetUuid();
    if (uuid.isEmpty()) {
        QMessageBox::warning(this, "Validation Error", "Select a preset to duplicate.");
        return;
    }
    for (const auto& p : SettingsManager::instance().presets) {
        if (p.uuid == uuid) {
            PresetSettings new_p = p;
            new_p.name = p.name + " (copy)";
            new_p.uuid = QUuid::createUuid().toString().remove("{").remove("}");
            SettingsManager::instance().presets << new_p;
            SettingsManager::instance().save();
            populatePresets();
            return;
        }
    }
}

void PresetManagerDialog::deletePreset() {
    auto uuid = selectedPresetUuid();
    if (uuid.isEmpty()) {
        QMessageBox::warning(this, "Validation Error", "Select a preset to delete.");
        return;
    }
    auto& presets = SettingsManager::instance().presets;
    for (int i = 0; i < presets.size(); ++i) {
        if (presets[i].uuid == uuid) {
            presets.removeAt(i);
            break;
        }
    }
    SettingsManager::instance().save();
    populatePresets();
    clearForm();
}
