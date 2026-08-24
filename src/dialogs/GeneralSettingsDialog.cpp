#include "dialogs/GeneralSettingsDialog.h"
#include "SettingsManager.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QFileInfo>
#include <QStandardPaths>
#include <QDir>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLineEdit>

GeneralSettingsDialog::GeneralSettingsDialog(QWidget* parent)
    : QDialog(parent) {
    setWindowTitle("General Settings");
    setMinimumWidth(500);

    auto* mainLayout = new QVBoxLayout(this);

    auto* formLayout = new QFormLayout;

    auto* bin_label = new QLabel("SD-CLI Binary Path:");
    formLayout->addRow(bin_label, bin_sd_cli_edit = new QLineEdit);
    bin_sd_cli_edit->setText(SettingsManager::instance().general.bin_sd_cli);
    bin_sd_cli_edit->setPlaceholderText("/path/to/sd-cli");

    auto* browse_bin_btn = new QPushButton("Browse...");
    auto* bin_layout = new QHBoxLayout;
    bin_layout->addWidget(bin_sd_cli_edit);
    bin_layout->addWidget(browse_bin_btn);
    formLayout->addRow(bin_layout);

    connect(browse_bin_btn, &QPushButton::clicked, this, &GeneralSettingsDialog::browseBinary);

    auto* output_label = new QLabel("Output Path:");
    formLayout->addRow(output_label, output_path_edit = new QLineEdit);
    output_path_edit->setText(SettingsManager::instance().general.output_path);
    output_path_edit->setPlaceholderText("/path/to/output");

    auto* browse_output_btn = new QPushButton("Browse...");
    auto* output_layout = new QHBoxLayout;
    output_layout->addWidget(output_path_edit);
    output_layout->addWidget(browse_output_btn);
    formLayout->addRow(output_layout);

    connect(browse_output_btn, &QPushButton::clicked, this, &GeneralSettingsDialog::browseOutputPath);

    mainLayout->addLayout(formLayout);

    auto* btn_layout = new QHBoxLayout;
    auto* ok_btn = new QPushButton("OK");
    auto* cancel_btn = new QPushButton("Cancel");
    btn_layout->addWidget(ok_btn);
    btn_layout->addWidget(cancel_btn);
    mainLayout->addLayout(btn_layout);

    connect(ok_btn, &QPushButton::clicked, this, &GeneralSettingsDialog::accept);
    connect(cancel_btn, &QPushButton::clicked, this, &QDialog::reject);
}

void GeneralSettingsDialog::browseBinary() {
    QString file = QFileDialog::getOpenFileName(this, "Select SD-CLI Binary", "", "All Files (*);;Executable (*.exe)");
    if (!file.isEmpty()) {
        bin_sd_cli_edit->setText(file);
    }
}

void GeneralSettingsDialog::browseOutputPath() {
    QString dir = QFileDialog::getExistingDirectory(this, "Select Output Directory");
    if (!dir.isEmpty()) {
        output_path_edit->setText(dir);
    }
}

void GeneralSettingsDialog::accept() {
    if (bin_sd_cli_edit->text().isEmpty()) {
        QMessageBox::warning(this, "Validation Error", "SD-CLI binary path is required.");
        return;
    }
    if (!QFileInfo::exists(bin_sd_cli_edit->text())) {
        QMessageBox::warning(this, "Validation Error", "SD-CLI binary does not exist at the specified path.");
        return;
    }
    if (output_path_edit->text().isEmpty()) {
        QMessageBox::warning(this, "Validation Error", "Output path is required.");
        return;
    }
    SettingsManager::instance().general.bin_sd_cli = bin_sd_cli_edit->text();
    SettingsManager::instance().general.output_path = output_path_edit->text();
    SettingsManager::instance().save();
    QDialog::accept();
}
