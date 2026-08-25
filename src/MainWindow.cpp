#include "MainWindow.h"
#include "SettingsManager.h"
#include "dialogs/GeneralSettingsDialog.h"
#include "dialogs/ModelManagerDialog.h"
#include "dialogs/PresetManagerDialog.h"
#include "dialogs/NewModelDialog.h"
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QFileDialog>
#include <QFileInfo>
#include <QDateTime>
#include <QDir>
#include <QMessageBox>
#include <QStandardPaths>
#include <QSettings>
#include <QUuid>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QProcess>
#include <QTextEdit>
#include <QComboBox>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QColorDialog>
#include <QIcon>


struct PixelSize {
    int w;
    int h;
    QString s;
};

PixelSize sizes[] = {
    {512, 512, "1:1 512x512 (small)"},
    {848, 1264, "2:3 848x1264"},
    {1024, 1024, "1:1 1024x1024"},
    {1024, 768, "4:3 1024x768"},
    {1376, 768, "16:9 1376x768"},
};

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent) {
    setWindowTitle("qt-sd-animator");
    setMinimumSize(800, 600);

    auto* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    auto* mainLayout = new QVBoxLayout(centralWidget);

    // Menu bar
    auto* file_menu = menuBar()->addMenu("File");

    auto* open_act = new QAction("Open", this);
    connect(open_act, &QAction::triggered, this, &MainWindow::openFile);
    file_menu->addAction(open_act);

    auto* recent_menu = file_menu->addMenu("Recently Open");
    for (const auto& path : SettingsManager::instance().recentlyOpened) {
        auto* act = new QAction(path, this);
        connect(act, &QAction::triggered, [this, path]() {
            if (m_dirty) {
                QMessageBox::StandardButton ret = QMessageBox::warning(this, "Unsaved Changes", "Save changes before opening?", QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
                if (ret == QMessageBox::Save) {
                    if (!saveFile()) return;
                } else if (ret == QMessageBox::Cancel) {
                    return;
                }
            }
            SettingsManager::instance().addToRecentlyOpened(path);
            if (SettingsManager::instance().loadSettings(path)) {
                refreshDropDowns();
                m_currentSavePath = path;
                m_dirty = false;
                setWindowTitle("qt-sd-animator - " + QFileInfo(path).fileName());
                statusBar()->showMessage("Opened: " + path);
            }
        });
        recent_menu->addAction(act);
    }

    auto* save_act = new QAction("Save", this);
    connect(save_act, &QAction::triggered, this, &MainWindow::saveFile);
    file_menu->addAction(save_act);

    auto* save_as_act = new QAction("Save As", this);
    connect(save_as_act, &QAction::triggered, this, &MainWindow::saveAsFile);
    file_menu->addAction(save_as_act);

    file_menu->addSeparator();

    auto* tool_menu = menuBar()->addMenu("&Tool");

    auto* settings_act = new QAction("Settings", this);
    connect(settings_act, &QAction::triggered, this, &MainWindow::showGeneralSettings);
    tool_menu->addAction(settings_act);

    auto* model_mgr_act = new QAction("Model Manager", this);
    connect(model_mgr_act, &QAction::triggered, this, &MainWindow::showModelManager);
    tool_menu->addAction(model_mgr_act);

    /*auto* preset_mgr_act = new QAction("Preset Manager", this);
    connect(preset_mgr_act, &QAction::triggered, this, &MainWindow::showPresetManager);
    tool_menu->addAction(preset_mgr_act);*/

    auto* exit_act = new QAction("Exit", this);
    connect(exit_act, &QAction::triggered, this, &QMainWindow::close);
    file_menu->addAction(exit_act);

    // Main workspace
    auto* workspace = new QWidget(this);
    auto* ws_layout = new QVBoxLayout(workspace);
    mainLayout->addWidget(workspace);

    // Model selector
    auto* model_layout = new QHBoxLayout;
    model_layout->addWidget(new QLabel("Model:"));
    m_modelCombo = new QComboBox;
    for (const auto& m : SettingsManager::instance().models) {
        m_modelCombo->addItem(m.name, QVariant(m.uuid));
    }
    model_layout->addWidget(m_modelCombo);
    /*
    auto* new_model_btn = new QPushButton("NEW MODEL");
    model_layout->addWidget(new_model_btn);
    connect(new_model_btn, &QPushButton::clicked, this, &MainWindow::newModel);
    */
    ws_layout->addLayout(model_layout);

    // Size selector
    auto* size_layout = new QHBoxLayout;
    size_layout->addWidget(new QLabel("Size:"));
    m_sizeCombo = new QComboBox;
    m_sizeCombo->addItem("1:1 512x512 (small)", "512,512");
    m_sizeCombo->addItem("2:3 848x1264", "848,1264");
    m_sizeCombo->addItem("1:1 1024x1024", "1024,1024");
    m_sizeCombo->addItem("4:3 1024x768", "1024,768");
    m_sizeCombo->addItem("16:9 1376x768", "1376,768");
    m_sizeCombo->setCurrentIndex(0);
    size_layout->addWidget(m_sizeCombo);
    ws_layout->addLayout(size_layout);

    // Preset selector
    /*
    auto* preset_layout = new QHBoxLayout;
    preset_layout->addWidget(new QLabel("Preset:"));
    preset_combo = new QComboBox;
    for (const auto& p : SettingsManager::instance().presets) {
        preset_combo->addItem(p.name, QVariant(p.uuid));
    }
    preset_layout->addWidget(preset_combo);
    auto* save_preset_btn = new QPushButton("SAVE");
    preset_layout->addWidget(save_preset_btn);
    connect(save_preset_btn, &QPushButton::clicked, this, &MainWindow::savePreset);
    ws_layout->addLayout(preset_layout);*/

    // Background color
    auto* bg_layout = new QHBoxLayout;
    //bg_layout->addWidget(new QLabel("Background Color:"));
    m_bgColorLabel = new QLabel;
    m_bgColorLabel->setFixedSize(100, 20);
    m_bgColorLabel->setStyleSheet("border: 1px solid black;");
   // bg_layout->addWidget(m_bgColorLabel);
    auto* color_btn = new QPushButton("Select Color");
   // bg_layout->addWidget(color_btn);
    connect(color_btn, &QPushButton::clicked, this, &MainWindow::selectBgColor);
    ws_layout->addLayout(bg_layout);

    // Source image
    auto* src_layout = new QHBoxLayout;
    src_layout->addWidget(new QLabel("Source Image:"));
    m_sourceImageEdit = new QLineEdit;
    m_sourceImageEdit->setFixedWidth(400);
    src_layout->addWidget(m_sourceImageEdit);
    auto* browse_btn = new QPushButton("Browse...");
    src_layout->addWidget(browse_btn);
    connect(browse_btn, &QPushButton::clicked, this, &MainWindow::selectSourceImage);
    ws_layout->addLayout(src_layout);

    // Prompt
    auto* prompt_layout = new QHBoxLayout;
    prompt_layout->addWidget(new QLabel("Prompt:"));
    m_promptEdit = new QPlainTextEdit;
    m_promptEdit->setPlaceholderText("Enter prompt...");
    m_promptEdit->setMaximumHeight(150);
    m_promptEdit->setFixedWidth(600);
    m_promptEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_promptEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    m_promptEdit->setWordWrapMode(QTextOption::WordWrap);
    prompt_layout->addWidget(m_promptEdit);
    ws_layout->addLayout(prompt_layout);

    // Negative prompt
    auto* neg_layout = new QHBoxLayout;
    neg_layout->addWidget(new QLabel("Negative Prompt:"));
    m_negativePromptEdit = new QPlainTextEdit;
    m_negativePromptEdit->setPlaceholderText("Enter negative prompt...");
    m_negativePromptEdit->setMaximumHeight(150);
    m_negativePromptEdit->setFixedWidth(600);
    m_negativePromptEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_negativePromptEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    m_negativePromptEdit->setWordWrapMode(QTextOption::WordWrap);
    neg_layout->addWidget(m_negativePromptEdit);
    ws_layout->addLayout(neg_layout);

    // Seed
    auto* seed_layout = new QHBoxLayout;
    seed_layout->addWidget(new QLabel("Seed:"));
    m_seedEdit = new QLineEdit;
    m_seedEdit->setPlaceholderText("Enter seed value...");
    m_seedEdit->setFixedWidth(200);
    seed_layout->addWidget(m_seedEdit);
    m_randomSeedBtn = new QPushButton("Random");
    m_randomSeedBtn->setToolTip("Random seed");
    seed_layout->addWidget(m_randomSeedBtn);
    connect(m_randomSeedBtn, &QPushButton::clicked, this, [this]() {
        m_seedEdit->setText(QString::number(QRandomGenerator::global()->generate()));
    });
    ws_layout->addLayout(seed_layout);

    // Basename
    auto* dest_layout = new QHBoxLayout;
    dest_layout->addWidget(new QLabel("Basename:"));
    m_filenameEdit = new QLineEdit;
    m_filenameEdit->setPlaceholderText("basename");
    m_filenameEdit->setFixedWidth(400);
    dest_layout->addWidget(m_filenameEdit);
    //auto* browse_dest_btn = new QPushButton("Browse...");
    //dest_layout->addWidget(browse_dest_btn);
    ws_layout->addLayout(dest_layout);
    /*connect(browse_dest_btn, &QPushButton::clicked, [this]() {
        QString dir = QFileDialog::getExistingDirectory(this, "Select Output Destination");
        if (!dir.isEmpty()) {
            m_outputDestEdit->setText(dir);
            m_lastOutputDest = dir;
        }
    });*/

    // Datetime checkbox
    auto* datetime_layout = new QHBoxLayout;
    datetime_layout->addWidget(new QLabel("Add Datetime:"));
    m_datetimeCheckbox = new QCheckBox;
    m_datetimeCheckbox->setChecked(true);
    datetime_layout->addWidget(m_datetimeCheckbox);
    ws_layout->addLayout(datetime_layout);

    // Process buttons
    auto* btn_layout = new QHBoxLayout;
    m_processBtn = new QPushButton("PROCESS");
    m_stopBtn = new QPushButton("STOP");
    m_stopBtn->setEnabled(false);
    btn_layout->addWidget(m_processBtn);
    btn_layout->addWidget(m_stopBtn);
    ws_layout->addLayout(btn_layout);
    connect(m_processBtn, &QPushButton::clicked, this, &MainWindow::processClicked);
    connect(m_stopBtn, &QPushButton::clicked, this, &MainWindow::stopClicked);

    // Output window
    m_outputWidget = new QTextEdit;
    m_outputWidget->setReadOnly(true);
    m_outputWidget->setPlaceholderText("Output will appear here...");
    mainLayout->addWidget(m_outputWidget);

    // Execution time label
    m_executionTimeLabel = new QLabel("Execution Time: 00:00:00");
    m_executionTimeLabel->setAlignment(Qt::AlignRight);
    m_executionTimeLabel->setStyleSheet("font-weight: bold; font-size: 14px;");
    mainLayout->addWidget(m_executionTimeLabel);

    // Connect preset selection to populate prompt fields
    /*
    connect(preset_combo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
        if (index < 0 || index >= SettingsManager::instance().presets.size()) return;
        const auto& p = SettingsManager::instance().presets[index];
        m_promptEdit->setPlainText(p.prompt);
        m_negativePromptEdit->setPlainText(p.negativePrompt);
        m_seedEdit->setText(p.seed);
    });*/

    // Load saved settings
    m_bgColor = Qt::black;
    m_bgColorLabel->setStyleSheet("background-color: " + m_bgColor.name() + ";");

    // Status bar
    statusBar()->show();
    statusBar()->showMessage("Ready");

    // Execution timer
    m_executionTimer = new QTimer(this);
    connect(m_executionTimer, &QTimer::timeout, this, [this]() {
        QString timeStr =executionTimeStr();
        m_executionTimeLabel->setText("Execution Time: " + timeStr);
    });
}


QString MainWindow::executionTimeStr()
{
    qint64 elapsedMs = m_executionTimerElapsed.elapsed();
    int totalSeconds = elapsedMs / 1000;
    int hours = totalSeconds / 3600;
    int minutes = (totalSeconds % 3600) / 60;
    int seconds = totalSeconds % 60;
    return QString("%1:%2:%3")
                          .arg(hours, 2, 10, QChar('0'))
                          .arg(minutes, 2, 10, QChar('0'))
                          .arg(seconds, 2, 10, QChar('0'));
}

MainWindow::~MainWindow() {
    if (m_currentProcess && m_currentProcess->state() == QProcess::Running) {
        m_currentProcess->kill();
    }
}

void MainWindow::closeEvent(QCloseEvent* event) {
    if (m_dirty) {
        QMessageBox::StandardButton ret = QMessageBox::warning(this, "Unsaved Changes", "Save changes before exiting?", QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        if (ret == QMessageBox::Save) {
            if (saveFile()) {
                event->accept();
            } else {
                event->ignore();
            }
        } else if (ret == QMessageBox::Discard) {
            event->accept();
        } else {
            event->ignore();
        }
    } else {
        event->accept();
    }
}

void MainWindow::refreshDropDowns()
{
    // Refresh dropdowns
    m_modelCombo->clear();
    for (const auto& m : SettingsManager::instance().models) {
        m_modelCombo->addItem(m.name, QVariant(m.uuid));
    }

    /*
    preset_combo->clear();
    for (const auto& p : SettingsManager::instance().presets) {
        preset_combo->addItem(p.name, QVariant(p.uuid));
    }*/
}

void MainWindow::openFile() {
    if (m_dirty) {
        QMessageBox::StandardButton ret = QMessageBox::warning(this, "Unsaved Changes", "Save changes before opening?", QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        if (ret == QMessageBox::Save) {
            if (!saveFile()) return;
        } else if (ret == QMessageBox::Cancel) {
            return;
        }
    }
    QString file = QFileDialog::getOpenFileName(this, "Open Settings", "", "JSON Files (*.json);;All Files (*)");
    if (!file.isEmpty()) {
        if (!file.endsWith(".json")) {
            file += ".json";
        }
        m_currentSavePath = file;
        
        if (SettingsManager::instance().loadSettings(file)) {
            refreshDropDowns();
            m_dirty = false;
        }
        
        SettingsManager::instance().addToRecentlyOpened(file);
        setWindowTitle("qt-sd-animator - " + QFileInfo(file).fileName());
        statusBar()->showMessage("Opened: " + file);
    }
}

void MainWindow::openRecent() {
    // Handled via menu actions
}

bool MainWindow::saveFile() {
    if (m_currentSavePath.isEmpty()) {
        return saveAsFile();
    }
    if (!m_currentSavePath.endsWith(".json")) {
        m_currentSavePath += ".json";
    }
    QJsonObject obj = SettingsManager::instance().toJson();
    QFile file(m_currentSavePath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(QJsonDocument(obj).toJson(QJsonDocument::Indented));
        SettingsManager::instance().addToRecentlyOpened(m_currentSavePath);
        SettingsManager::instance().save();
        setWindowTitle("qt-sd-animator - " + QFileInfo(m_currentSavePath).fileName());
        statusBar()->showMessage("Saved to: " + m_currentSavePath);
        m_dirty = false;
        return true;
    }
    return false;
}

bool MainWindow::saveAsFile() {
    QString file = QFileDialog::getSaveFileName(this, "Save Settings As", m_currentSavePath, "JSON Files (*.json);;All Files (*)");
    if (file.isEmpty()) {
        return false;
    }
    if (!file.endsWith(".json")) {
        file += ".json";
    }
    m_currentSavePath = file;
    QJsonObject obj = SettingsManager::instance().toJson();
    QFile outFile(file);
    if (outFile.open(QIODevice::WriteOnly)) {
        outFile.write(QJsonDocument(obj).toJson(QJsonDocument::Indented));
        SettingsManager::instance().addToRecentlyOpened(file);
        SettingsManager::instance().save();
        setWindowTitle("qt-sd-animator - " + QFileInfo(file).fileName());
        statusBar()->showMessage("Saved to: " + file);
        m_dirty = false;
        return true;
    }
    return false;
}

void MainWindow::showGeneralSettings() {
    GeneralSettingsDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        m_dirty = true;
    }
}

void MainWindow::showModelManager() {
    ModelManagerDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        refreshDropDowns();
        m_dirty = true;
    }
}

void MainWindow::showPresetManager() {
    PresetManagerDialog dlg(this);
    dlg.exec();
}

void MainWindow::newModel() {
    NewModelDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        ModelSettings m;
        m.name = dlg.modelName();
        m.diffusionModel = dlg.diffusionModel();
        m.llm = dlg.llm();
        m.vae = dlg.vae();
        m.sourceImageRequired = dlg.sourceImageRequired();
        m.uuid = dlg.uuid();
        SettingsManager::instance().models << m;
        SettingsManager::instance().save();
        m_modelCombo->clear();
        for (const auto& mod : SettingsManager::instance().models) {
            m_modelCombo->addItem(mod.name, QVariant(mod.uuid));
        }
        m_dirty = true;
    }
}

void MainWindow::savePreset() {
    QString name = QInputDialog::getText(this, "Save Preset", "Enter preset name:");
    if (name.isEmpty()) return;

    PresetSettings p;
    p.name = name;
    p.prompt = m_promptEdit->toPlainText();
    p.negativePrompt = m_negativePromptEdit->toPlainText();
    p.seed = m_seedEdit->text();
    p.uuid = QUuid::createUuid().toString().remove("{").remove("}");
    SettingsManager::instance().presets << p;
    SettingsManager::instance().save();
    m_dirty = true;
}

void MainWindow::processClicked() {
    int modelIndex = m_modelCombo->currentIndex();
    if (modelIndex < 0 || modelIndex >= SettingsManager::instance().models.size()) {
        m_outputWidget->append("[ERROR] No model selected. cannot continue without model settings.");
        return;
    }
    const auto& model = SettingsManager::instance().models[modelIndex];

    // Sanity checks
    if (SettingsManager::instance().general.bin_sd_cli.isEmpty()) {
        m_outputWidget->append("[ERROR] SD-CLI binary path not set. Please configure in Settings.");
        return;
    }
    if (!QFileInfo::exists(SettingsManager::instance().general.bin_sd_cli)) {
        m_outputWidget->append("[ERROR] SD-CLI binary not found at: " + SettingsManager::instance().general.bin_sd_cli);
        return;
    }
    if (model.sourceImageRequired && m_sourceImageEdit->text().isEmpty()) {
        m_outputWidget->append("[ERROR] Source image is required.");
        return;
    }
    if (model.sourceImageRequired && !QFileInfo::exists(m_sourceImageEdit->text())) {
        m_outputWidget->append("[ERROR] Source image not found: " + m_sourceImageEdit->text());
        return;
    }
    if (m_promptEdit->toPlainText().isEmpty()) {
        m_outputWidget->append("[ERROR] Prompt is required.");
        return;
    }
    //if (m_filenameEdit->text().isEmpty()) {
    //    m_outputWidget->append("[ERROR] Output filename is required.");
    //    return;
   // }

    // Disable PROCESS button, enable STOP button
    m_processBtn->setEnabled(false);
    m_stopBtn->setEnabled(true);

    // Start execution timer
    m_executionTimerElapsed.start();
    m_executionTimer->start(1000);

    // Create output directory if not exists
    QString outputDirPath =  SettingsManager::instance().general.output_path;// m_filenameEdit->text();
    if (outputDirPath.isEmpty()) {
        QMessageBox::critical(this, "Error", "Output directory is empty.");
        return;
    }

    QDir outputDir(outputDirPath);
    if (!outputDir.exists()) {
        if (!QDir().mkpath(outputDirPath)) {
            QMessageBox::critical(this, "Error", "Failed to create output directory.");
            return;
        }
    }

    // Build filename
    QString sourceBase = QFileInfo(m_sourceImageEdit->text()).completeBaseName();
    QString filename;
    if (m_lastOutputFilename.isEmpty()) {
        filename = sourceBase;
    } else {
        filename = m_lastOutputFilename;
    }
    QString userFilename = m_filenameEdit->text();
    if (!userFilename.isEmpty()) {
        filename = userFilename;
    }
    if (userFilename.isEmpty()) {
        userFilename = "test";
    }
    m_lastOutputFilename = userFilename;
    QString outputPath;
    QString ext = model.ext;
    if (m_datetimeCheckbox->isChecked()) {
        QString dateTime = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
        outputPath = outputDirPath + "/" + userFilename + "_" + dateTime + ext;
    } else {
        outputPath = outputDirPath + "/" + userFilename + ext;
    }
    m_lastOutputDest = outputDirPath;

    // Build command arguments
    QString binaryPath = SettingsManager::instance().general.bin_sd_cli;
    QStringList args;

    if (!m_sourceImageEdit->text().isEmpty())
        args << "--init-img" << m_sourceImageEdit->text();
    args << "--output" << outputPath;
    args << "--prompt" << m_promptEdit->toPlainText();
    if (!m_negativePromptEdit->toPlainText().isEmpty()) {
        args << "--negative-prompt" << m_negativePromptEdit->toPlainText();
    }
    if (!m_seedEdit->text().isEmpty()) {
        args << "--seed" << m_seedEdit->text();
    }

    args << "--diffusion-model" << model.diffusionModel;
    if (!model.llm.isEmpty())
        args << "--llm" << model.llm;
    if (!model.vae.isEmpty())
        args << "--vae" << model.vae;
    QStringList sizeParts = m_sizeCombo->currentData().toString().split(',');
    int width = 512;
    int height = 512;
    if (sizeParts.size() == 2) {
        width = sizeParts[0].toInt();
        height = sizeParts[1].toInt();
    }
    args << "--width" << QString::number(width);
    args << "--height" << QString::number(height);
    
    // Parse additional parameters (one per line)
    for (const auto& param : model.additionalParameters) {
        QString trimmed = param.trimmed();
        if (!trimmed.isEmpty()) {
            // Split each line on space character (0x20)
            QStringList parts = trimmed.split(' ', Qt::SkipEmptyParts);
            for (const auto& part : parts) {
                if (!part.isEmpty()) {
                    args << part;
                }
            }
        }
    }

    // Log command
    QString command = binaryPath;
    for (const auto& arg : args) {
        command += " \"" + arg + "\"";
    }
    m_outputWidget->append("\n[CMD] " + command);
    m_outputWidget->append("[INFO] Starting process...");

    // Start process
    m_currentProcess = new QProcess(this);
    connect(m_currentProcess, &QProcess::readyReadStandardOutput, this, [this]() {
        QByteArray out = m_currentProcess->readAllStandardOutput();
        m_outputWidget->append("[STDOUT] " + out);
    });
    connect(m_currentProcess, &QProcess::readyReadStandardError, this, [this]() {
        QByteArray err = m_currentProcess->readAllStandardError();
        m_outputWidget->append("[STDERR] " + err);
    });
    connect(m_currentProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), [this, outputPath, command, modelIndex]() {
        const auto& model = SettingsManager::instance().models[modelIndex];

        int exitCode = m_currentProcess->exitCode();
        m_outputWidget->append("\n[INFO] Process finished with exit code: " + QString::number(exitCode));

        // Write companion JSON
        QJsonObject presetObj;
     //   presetObj["name"] = preset_combo->currentText();
        presetObj["prompt"] = m_promptEdit->toPlainText();
        presetObj["negative_prompt"] = m_negativePromptEdit->toPlainText();
        presetObj["seed"] = m_seedEdit->text();
        QStringList sizeParts = m_sizeCombo->currentData().toString().split(',');
        int width = 0;
        int height = 0;
        if (sizeParts.size() == 2) {
            width = sizeParts[0].toInt();
            height = sizeParts[1].toInt();
        }
        presetObj["width"] = QString::number(width);
        presetObj["height"] = QString::number(height);
        writeCompanionJson(outputPath, command,  model, presetObj, m_sourceImageEdit->text(), m_bgColor.name(), exitCode);

        m_currentProcess->deleteLater();
        m_currentProcess = nullptr;

        // Stop execution timer
        m_executionTimer->stop();
        m_executionTimeLabel->setText("Execution Time: " + executionTimeStr() + "[stopped]");

        // Re-enable PROCESS button, disable STOP button
        m_processBtn->setEnabled(true);
        m_stopBtn->setEnabled(false);
    });

    m_currentProcess->start(binaryPath, args);
}

void MainWindow::stopClicked() {
    if (m_currentProcess && m_currentProcess->state() == QProcess::Running) {
        m_currentProcess->kill();
        m_outputWidget->append("[INFO] Process stopped.");
    }
    
    // Stop execution timer
    m_executionTimer->stop();
    m_executionTimeLabel->setText("Execution Time: " + executionTimeStr() + "[stopped]");
    
    m_processBtn->setEnabled(true);
    m_stopBtn->setEnabled(false);
}

void MainWindow::selectSourceImage() {
    QString file = QFileDialog::getOpenFileName(this, "Select Source Image", m_lastSourcePath, "Images (*.png *.jpg *.jpeg *.bmp *.tiff)");
    if (!file.isEmpty()) {
        m_sourceImageEdit->setText(file);
        m_lastSourcePath = QFileInfo(file).absolutePath();
        QSettings settings("qt-sd-animator", "Settings");
        settings.setValue("lastSourcePath", m_lastSourcePath);
    }
}

void MainWindow::selectBgColor() {
    QColor color = QColorDialog::getColor(Qt::black, this, "Select Background Color");
    if (color.isValid()) {
        m_bgColor = color;
        m_bgColorLabel->setStyleSheet("background-color: " + color.name() + ";");
    }
}

void MainWindow::buildProcessArgs(QString& binaryPath, QStringList& args) {
    // Used for debugging - constructs the full command string
    binaryPath = SettingsManager::instance().general.bin_sd_cli;
    args << "--test";
}

void MainWindow::writeCompanionJson(const QString& outputPath, const QString& command, const ModelSettings& model, const QJsonObject &presetObj, const QString& sourceImage, const QString& bgColor, int exitCode) {
    QJsonObject json;
    json["command"] = command;
    json["source_image"] = sourceImage;
 //   json["bg_color"] = bgColor;
    json["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    json["exit_code"] = exitCode;

    QJsonObject modelObj;
    modelObj["name"] = model.name;
    modelObj["diffusion-model"] = model.diffusionModel;
    modelObj["llm"] = model.llm;
    modelObj["vae"] = model.vae;
  //  modelObj["width"] = model.width;
  //  modelObj["height"] = model.height;
    modelObj["source_image_required"] = model.sourceImageRequired;
    json["model"] = modelObj;
    json["preset"] = presetObj;
    json["execution_time"] = executionTimeStr();

    QFile jsonFile(outputPath + ".json");
    if (jsonFile.open(QIODevice::WriteOnly)) {
        jsonFile.write(QJsonDocument(json).toJson(QJsonDocument::Indented));
    }
}
