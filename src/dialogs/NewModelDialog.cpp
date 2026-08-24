#include "dialogs/NewModelDialog.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QFileInfo>
#include <QDir>

static char fileFilters[] = "All Supported(*.gguf *.safetensors);;GGUF Files (*.gguf);;SafeTensors Files (*.safetensors);;All Files (*)";

NewModelDialog::NewModelDialog(QWidget* parent)
    : QDialog(parent) {
    setWindowTitle("New Model");
    setMinimumWidth(400);

    auto* layout = new QVBoxLayout(this);

    auto* form_layout = new QVBoxLayout;
    layout->addLayout(form_layout);

    form_layout->addWidget(new QLabel("Name:"));
    name_edit = new QLineEdit;
    form_layout->addWidget(name_edit);

    form_layout->addWidget(new QLabel("Diffusion Model:"));
    auto* diffusion_layout = new QHBoxLayout;
    diffusion_model_edit = new QLineEdit;
    diffusion_layout->addWidget(diffusion_model_edit);
    auto* diffusion_btn = new QPushButton("Browse...");
    diffusion_layout->addWidget(diffusion_btn);
    form_layout->addLayout(diffusion_layout);
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

    form_layout->addWidget(new QLabel("LLM:"));
    auto* llm_layout = new QHBoxLayout;
    llm_edit = new QLineEdit;
    llm_layout->addWidget(llm_edit);
    auto* llm_btn = new QPushButton("Browse...");
    llm_layout->addWidget(llm_btn);
    form_layout->addLayout(llm_layout);
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

    form_layout->addWidget(new QLabel("VAE:"));
    auto* vae_layout = new QHBoxLayout;
    vae_edit = new QLineEdit;
    vae_layout->addWidget(vae_edit);
    auto* vae_btn = new QPushButton("Browse...");
    vae_layout->addWidget(vae_btn);
    form_layout->addLayout(vae_layout);
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

    form_layout->addWidget(new QLabel("Width:"));
    width_spin = new QSpinBox;
    width_spin->setRange(64, 2048);
    width_spin->setValue(512);
    form_layout->addWidget(width_spin);

    form_layout->addWidget(new QLabel("Height:"));
    height_spin = new QSpinBox;
    height_spin->setRange(64, 2048);
    height_spin->setValue(512);
    form_layout->addWidget(height_spin);

    form_layout->addWidget(new QLabel("Source Image Required:"));
    source_image_required_check = new QCheckBox;
    source_image_required_check->setChecked(true);
    form_layout->addWidget(source_image_required_check);

    auto* btn_layout = new QHBoxLayout;
    auto* ok_btn = new QPushButton("OK");
    auto* cancel_btn = new QPushButton("Cancel");
    btn_layout->addWidget(ok_btn);
    btn_layout->addWidget(cancel_btn);
    layout->addLayout(btn_layout);

    connect(ok_btn, &QPushButton::clicked, this, [this]() {
        if (name_edit->text().isEmpty()) {
            QMessageBox::warning(this, "Validation Error", "Model name is required.");
            return;
        }
        accept();
    });
    connect(cancel_btn, &QPushButton::clicked, this, &QDialog::reject);
}
