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

    presets_list = new QListWidget;
    list_layout->addWidget(presets_list);

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
    connect(presets_list, &QListWidget::itemSelectionChanged, this, [this]() {
        auto items = presets_list->selectedItems();
        if (items.isEmpty()) {
            clearForm();
            return;
        }
        int row = presets_list->row(items.first());
        if (row >= 0 && row < SettingsManager::instance().presets.size()) {
            loadForm(SettingsManager::instance().presets[row].uuid);
        }
    });

    auto* form_layout = new QVBoxLayout;
    mainLayout->addLayout(form_layout);

    form_layout->addWidget(new QLabel("Name:"));
    name_edit = new QLineEdit;
    form_layout->addWidget(name_edit);

    form_layout->addWidget(new QLabel("Prompt:"));
    prompt_edit = new QTextEdit;
    prompt_edit->setPlaceholderText("Enter prompt...");
    form_layout->addWidget(prompt_edit);

    form_layout->addWidget(new QLabel("Negative Prompt:"));
    negative_prompt_edit = new QTextEdit;
    negative_prompt_edit->setPlaceholderText("Enter negative prompt...");
    form_layout->addWidget(negative_prompt_edit);

    form_layout->addWidget(new QLabel("Seed:"));
    auto* seed_layout = new QHBoxLayout;
    seed_edit = new QLineEdit;
    seed_edit->setPlaceholderText("Enter seed value...");
    seed_layout->addWidget(seed_edit);
    random_seed_btn = new QPushButton("Random");
    seed_layout->addWidget(random_seed_btn);
    connect(random_seed_btn, &QPushButton::clicked, this, [this]() {
        seed_edit->setText(QString::number(QRandomGenerator::global()->generate()));
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
    presets_list->clear();
    for (const auto& p : SettingsManager::instance().presets) {
        presets_list->addItem(p.name);
    }
}

void PresetManagerDialog::clearForm() {
    name_edit->clear();
    prompt_edit->clear();
    negative_prompt_edit->clear();
    seed_edit->clear();
}

void PresetManagerDialog::loadForm(const QString& uuid) {
    for (const auto& p : SettingsManager::instance().presets) {
        if (p.uuid == uuid) {
            name_edit->setText(p.name);
            prompt_edit->setText(p.prompt);
            negative_prompt_edit->setText(p.negativePrompt);
            seed_edit->setText(p.seed);
            return;
        }
    }
}

QString PresetManagerDialog::selectedPresetUuid() const {
    auto items = presets_list->selectedItems();
    if (items.isEmpty()) return QString();
    int row = presets_list->row(items.first());
    if (row < 0 || row >= SettingsManager::instance().presets.size()) return QString();
    return SettingsManager::instance().presets[row].uuid;
}

QString PresetManagerDialog::selectedPresetName() const {
    auto items = presets_list->selectedItems();
    if (items.isEmpty()) return QString();
    return items.first()->text();
}

void PresetManagerDialog::addPreset() {
    QString name = name_edit->text();
    if (name.isEmpty()) {
        QMessageBox::warning(this, "Validation Error", "Preset name is required.");
        return;
    }
    PresetSettings p;
    p.name = name;
    p.prompt = prompt_edit->toPlainText();
    p.negativePrompt = negative_prompt_edit->toPlainText();
    p.seed = seed_edit->text();
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
            p.name = name_edit->text();
            p.prompt = prompt_edit->toPlainText();
            p.negativePrompt = negative_prompt_edit->toPlainText();
            p.seed = seed_edit->text();
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
