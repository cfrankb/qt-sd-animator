#include "dialogs/SizeManagerDialog.h"
#include "SettingsManager.h"
#include <QMessageBox>
#include <QUuid>
#include <QInputDialog>
#include <QValidator>

SizeManagerDialog::SizeManagerDialog(QWidget* parent)
    : QDialog(parent) {
    setWindowTitle("Size Manager");
    setMinimumSize(500, 300);

    auto* mainLayout = new QVBoxLayout(this);

    // Top row: size selector + buttons
    auto* topLayout = new QHBoxLayout;
    m_sizeCombo = new QComboBox;
    topLayout->addWidget(new QLabel("Size:"));
    topLayout->addWidget(m_sizeCombo);

    auto* add_btn = new QPushButton("Add");
    auto* del_btn = new QPushButton("Delete");
    topLayout->addWidget(add_btn);
    topLayout->addWidget(del_btn);
    mainLayout->addLayout(topLayout);

    connect(add_btn, &QPushButton::clicked, this, &SizeManagerDialog::addSize);
    connect(del_btn, &QPushButton::clicked, this, &SizeManagerDialog::deleteSize);
    connect(m_sizeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SizeManagerDialog::onSizeChanged);

    // Form
    auto* form_layout = new QFormLayout;
    mainLayout->addLayout(form_layout);

    // Label field with validation
    m_textEdit = new QLineEdit;
    m_textEdit->setPlaceholderText("Enter size label (e.g., '1:1 512x512')");
    form_layout->addRow("Label:", m_textEdit);

    // Width field with validation
    m_widthEdit = new QLineEdit;
    m_widthEdit->setPlaceholderText("Width in pixels");
    auto* widthValidator = new QIntValidator(1, 10000, this);
    m_widthEdit->setValidator(widthValidator);
    form_layout->addRow("Width:", m_widthEdit);

    // Height field with validation
    m_heightEdit = new QLineEdit;
    m_heightEdit->setPlaceholderText("Height in pixels");
    auto* heightValidator = new QIntValidator(1, 10000, this);
    m_heightEdit->setValidator(heightValidator);
    form_layout->addRow("Height:", m_heightEdit);

    // UUID field (read-only)
    m_uuidEdit = new QLineEdit;
    m_uuidEdit->setReadOnly(true);
    m_uuidEdit->setPlaceholderText("Auto-generated UUID");
    form_layout->addRow("UUID:", m_uuidEdit);

    // OK / Cancel
    auto* ok_cancel_layout = new QHBoxLayout;
    auto* ok_btn = new QPushButton("OK");
    auto* cancel_btn = new QPushButton("Cancel");
    ok_cancel_layout->addStretch();
    ok_cancel_layout->addWidget(ok_btn);
    ok_cancel_layout->addWidget(cancel_btn);
    mainLayout->addLayout(ok_cancel_layout);

    connect(ok_btn, &QPushButton::clicked, this, &SizeManagerDialog::acceptDialog);
    connect(cancel_btn, &QPushButton::clicked, this, &QDialog::reject);

    // Load local copy
    m_localSizes = SettingsManager::instance().m_sizes;
    populateCombo();
    if (m_sizeCombo->count() > 0) {
        m_sizeCombo->setCurrentIndex(0);
        m_currentSizeIndex = 0;
        loadFormFromLocal(0);
    } else {
        m_currentSizeIndex = -1;
    }
}

void SizeManagerDialog::populateCombo() {
    m_sizeCombo->blockSignals(true);
    m_sizeCombo->clear();
    for (const auto& s : m_localSizes) {
        QString display = s.text.isEmpty() ? QString("(%1 x %2)") .arg(s.w).arg(s.h) : s.text;
        m_sizeCombo->addItem(display, s.uuid);
    }
    m_sizeCombo->blockSignals(false);
}

void SizeManagerDialog::onSizeChanged(int index) {
    // Save current form before switching
    if (m_currentSizeIndex >= 0 && m_currentSizeIndex < m_localSizes.size()) {
        m_sizeCombo->setItemText(m_currentSizeIndex, m_textEdit->text());
        saveFormToLocal();
    }
    m_currentSizeIndex = index;
    if (index >= 0 && index < m_localSizes.size()) {
        loadFormFromLocal(index);
    }
}

void SizeManagerDialog::saveFormToLocal() {
    if (m_currentSizeIndex < 0 || m_currentSizeIndex >= m_localSizes.size()) return;
    
    PixelSize& s = m_localSizes[m_currentSizeIndex];
    
    // Validate and save text
    QString text = m_textEdit->text().trimmed();
    if (text.isEmpty()) {
        text = QString("(%1 x %2)").arg(m_widthEdit->text().toInt(0)).arg(m_heightEdit->text().toInt(0));
    }
    s.text = text;
    
    // Validate and save dimensions
    bool ok_w = false;
    bool ok_h = false;
    int w = m_widthEdit->text().toInt(&ok_w);
    int h = m_heightEdit->text().toInt(&ok_h);
    
    if (!ok_w || !ok_h || w <= 0 || h <= 0) {
        QMessageBox::warning(this, "Validation Error", 
            "Width and Height must be positive integers.");
        return;
    }
    

    s.w = w;
    s.h = h;
}

void SizeManagerDialog::loadFormFromLocal(int index) {
    if (index < 0 || index >= m_localSizes.size()) return;
    const PixelSize& s = m_localSizes[index];
    m_textEdit->setText(s.text);
    m_widthEdit->setText(QString::number(s.w));
    m_heightEdit->setText(QString::number(s.h));
    m_uuidEdit->setText(s.uuid);
}

void SizeManagerDialog::addSize() {
    PixelSize s;
    s.uuid = QUuid::createUuid().toString().remove("{").remove("}");
    s.text = "";
    s.w = 512;
    s.h = 512;
    m_localSizes.append(s);
    populateCombo();
    int idx = m_localSizes.size() - 1;
    m_sizeCombo->setCurrentIndex(idx);
    m_currentSizeIndex = idx;
    loadFormFromLocal(idx);
    m_textEdit->setFocus();
    m_textEdit->selectAll();
}

void SizeManagerDialog::deleteSize() {
    if (m_currentSizeIndex < 0 || m_currentSizeIndex >= m_localSizes.size()) {
        QMessageBox::warning(this, "Validation Error", "Select a size to delete.");
        return;
    }
    
    QString confirmMsg = "Delete size: " + m_localSizes[m_currentSizeIndex].text + "?";
    auto result = QMessageBox::question(this, "Confirm Delete", confirmMsg,
        QMessageBox::Yes | QMessageBox::No);
    
    if (result != QMessageBox::Yes) return;
    
    m_localSizes.removeAt(m_currentSizeIndex);
    populateCombo();
    
    if (m_localSizes.isEmpty()) {
        m_currentSizeIndex = -1;
        m_textEdit->clear();
        m_widthEdit->clear();
        m_heightEdit->clear();
        m_uuidEdit->clear();
    } else {
        int newIndex = qMin(m_currentSizeIndex, m_localSizes.size() - 1);
        m_currentSizeIndex = newIndex;
        m_sizeCombo->setCurrentIndex(newIndex);
        loadFormFromLocal(newIndex);
    }
}

void SizeManagerDialog::acceptDialog() {
    // Save any pending changes
    if (m_currentSizeIndex >= 0) {
        saveFormToLocal();
    }
    
    // Validate that we have at least one size
    if (m_localSizes.isEmpty()) {
        QMessageBox::warning(this, "Validation Error", "At least one size preset is required.");
        return;
    }
    
    // Sync to SettingsManager
    SettingsManager::instance().m_sizes = m_localSizes;
    SettingsManager::instance().save();
    accept();
}
