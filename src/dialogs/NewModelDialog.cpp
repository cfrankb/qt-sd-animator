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
    m_nameEdit = new QLineEdit;
    form_layout->addWidget(m_nameEdit);

    form_layout->addWidget(new QLabel("Diffusion Model:"));
    auto* diffusion_layout = new QHBoxLayout;
    m_diffusionModelEdit = new QLineEdit;
    diffusion_layout->addWidget(m_diffusionModelEdit);
    auto* diffusion_btn = new QPushButton("Browse...");
    diffusion_layout->addWidget(diffusion_btn);
    form_layout->addLayout(diffusion_layout);
    connect(diffusion_btn, &QPushButton::clicked, [this]() {
        QString currentPath = m_diffusionModelEdit->text();
        QString startDir = QFileInfo(currentPath).absolutePath();
        if (startDir.isEmpty()) {
            startDir = QDir::homePath();
        }
        QString file = QFileDialog::getOpenFileName(this, "Select Diffusion Model", startDir, fileFilters);
        if (!file.isEmpty()) {
            m_diffusionModelEdit->setText(file);
        }
    });

    form_layout->addWidget(new QLabel("LLM:"));
    auto* llm_layout = new QHBoxLayout;
    m_llmEdit = new QLineEdit;
    llm_layout->addWidget(m_llmEdit);
    auto* llm_btn = new QPushButton("Browse...");
    llm_layout->addWidget(llm_btn);
    form_layout->addLayout(llm_layout);
    connect(llm_btn, &QPushButton::clicked, [this]() {
        QString currentPath = m_llmEdit->text();
        QString startDir = QFileInfo(currentPath).absolutePath();
        if (startDir.isEmpty()) {
            startDir = QDir::homePath();
        }
        QString file = QFileDialog::getOpenFileName(this, "Select LLM", startDir, fileFilters);
        if (!file.isEmpty()) {
            m_llmEdit->setText(file);
        }
    });

    form_layout->addWidget(new QLabel("VAE:"));
    auto* vae_layout = new QHBoxLayout;
    m_vaeEdit = new QLineEdit;
    vae_layout->addWidget(m_vaeEdit);
    auto* vae_btn = new QPushButton("Browse...");
    vae_layout->addWidget(vae_btn);
    form_layout->addLayout(vae_layout);
    connect(vae_btn, &QPushButton::clicked, [this]() {
        QString currentPath = m_vaeEdit->text();
        QString startDir = QFileInfo(currentPath).absolutePath();
        if (startDir.isEmpty()) {
            startDir = QDir::homePath();
        }
        QString file = QFileDialog::getOpenFileName(this, "Select VAE", startDir, fileFilters);
        if (!file.isEmpty()) {
            m_vaeEdit->setText(file);
        }
    });

    form_layout->addWidget(new QLabel("Width:"));
    m_widthSpin = new QSpinBox;
    m_widthSpin->setRange(64, 2048);
    m_widthSpin->setValue(512);
    form_layout->addWidget(m_widthSpin);

    form_layout->addWidget(new QLabel("Height:"));
    m_heightSpin = new QSpinBox;
    m_heightSpin->setRange(64, 2048);
    m_heightSpin->setValue(512);
    form_layout->addWidget(m_heightSpin);

    form_layout->addWidget(new QLabel("Source Image Required:"));
    m_sourceImageRequiredCheck = new QCheckBox;
    m_sourceImageRequiredCheck->setChecked(true);
    form_layout->addWidget(m_sourceImageRequiredCheck);

    auto* btn_layout = new QHBoxLayout;
    auto* ok_btn = new QPushButton("OK");
    auto* cancel_btn = new QPushButton("Cancel");
    btn_layout->addWidget(ok_btn);
    btn_layout->addWidget(cancel_btn);
    layout->addLayout(btn_layout);

    connect(ok_btn, &QPushButton::clicked, this, [this]() {
        if (m_nameEdit->text().isEmpty()) {
            QMessageBox::warning(this, "Validation Error", "Model name is required.");
            return;
        }
        accept();
    });
    connect(cancel_btn, &QPushButton::clicked, this, &QDialog::reject);
}
