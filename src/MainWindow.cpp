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
            SettingsManager::instance().addToRecentlyOpened(path);
            SettingsManager::instance().save();
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

    auto* preset_mgr_act = new QAction("Preset Manager", this);
    connect(preset_mgr_act, &QAction::triggered, this, &MainWindow::showPresetManager);
    tool_menu->addAction(preset_mgr_act);

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
    model_combo = new QComboBox;
    for (const auto& m : SettingsManager::instance().models) {
        model_combo->addItem(m.name, QVariant(m.uuid));
    }
    model_layout->addWidget(model_combo);
    auto* new_model_btn = new QPushButton("NEW MODEL");
    model_layout->addWidget(new_model_btn);
    connect(new_model_btn, &QPushButton::clicked, this, &MainWindow::newModel);
    ws_layout->addLayout(model_layout);

    // Preset selector
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
    ws_layout->addLayout(preset_layout);

    // Background color
    auto* bg_layout = new QHBoxLayout;
    //bg_layout->addWidget(new QLabel("Background Color:"));
    bg_color_label = new QLabel;
    bg_color_label->setFixedSize(100, 20);
    bg_color_label->setStyleSheet("border: 1px solid black;");
   // bg_layout->addWidget(bg_color_label);
    auto* color_btn = new QPushButton("Select Color");
   // bg_layout->addWidget(color_btn);
    connect(color_btn, &QPushButton::clicked, this, &MainWindow::selectBgColor);
    ws_layout->addLayout(bg_layout);

    // Source image
    auto* src_layout = new QHBoxLayout;
    src_layout->addWidget(new QLabel("Source Image:"));
    source_image_edit = new QLineEdit;
    source_image_edit->setFixedWidth(400);
    src_layout->addWidget(source_image_edit);
    auto* browse_btn = new QPushButton("Browse...");
    src_layout->addWidget(browse_btn);
    connect(browse_btn, &QPushButton::clicked, this, &MainWindow::selectSourceImage);
    ws_layout->addLayout(src_layout);

    // Prompt
    auto* prompt_layout = new QHBoxLayout;
    prompt_layout->addWidget(new QLabel("Prompt:"));
    prompt_edit = new QPlainTextEdit;
    prompt_edit->setPlaceholderText("Enter prompt...");
    prompt_edit->setMaximumHeight(150);
    prompt_edit->setFixedWidth(600);
    prompt_edit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    prompt_edit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    prompt_edit->setWordWrapMode(QTextOption::WordWrap);
    prompt_layout->addWidget(prompt_edit);
    ws_layout->addLayout(prompt_layout);

    // Negative prompt
    auto* neg_layout = new QHBoxLayout;
    neg_layout->addWidget(new QLabel("Negative Prompt:"));
    negative_prompt_edit = new QPlainTextEdit;
    negative_prompt_edit->setPlaceholderText("Enter negative prompt...");
    negative_prompt_edit->setMaximumHeight(150);
    negative_prompt_edit->setFixedWidth(600);
    negative_prompt_edit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    negative_prompt_edit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    negative_prompt_edit->setWordWrapMode(QTextOption::WordWrap);
    neg_layout->addWidget(negative_prompt_edit);
    ws_layout->addLayout(neg_layout);

    // Output destination
    auto* dest_layout = new QHBoxLayout;
    dest_layout->addWidget(new QLabel("Output Destination:"));
    output_dest_edit = new QLineEdit;
    output_dest_edit->setPlaceholderText("/path/to/output");
    output_dest_edit->setFixedWidth(400);
    dest_layout->addWidget(output_dest_edit);
    auto* browse_dest_btn = new QPushButton("Browse...");
    dest_layout->addWidget(browse_dest_btn);
    ws_layout->addLayout(dest_layout);
    connect(browse_dest_btn, &QPushButton::clicked, [this]() {
        QString dir = QFileDialog::getExistingDirectory(this, "Select Output Destination");
        if (!dir.isEmpty()) {
            output_dest_edit->setText(dir);
            lastOutputDest = dir;
        }
    });

    // Datetime checkbox
    auto* datetime_layout = new QHBoxLayout;
    datetime_layout->addWidget(new QLabel("Add Datetime:"));
    datetime_checkbox = new QCheckBox;
    datetime_checkbox->setChecked(true);
    datetime_layout->addWidget(datetime_checkbox);
    ws_layout->addLayout(datetime_layout);

    // Process buttons
    auto* btn_layout = new QHBoxLayout;
    process_btn = new QPushButton("PROCESS");
    stop_btn = new QPushButton("STOP");
    stop_btn->setEnabled(false);
    btn_layout->addWidget(process_btn);
    btn_layout->addWidget(stop_btn);
    ws_layout->addLayout(btn_layout);
    connect(process_btn, &QPushButton::clicked, this, &MainWindow::processClicked);
    connect(stop_btn, &QPushButton::clicked, this, &MainWindow::stopClicked);

    // Output window
    output_widget = new QTextEdit;
    output_widget->setReadOnly(true);
    output_widget->setPlaceholderText("Output will appear here...");
    mainLayout->addWidget(output_widget);

    // Connect preset selection to populate prompt fields
    connect(preset_combo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
        if (index < 0 || index >= SettingsManager::instance().presets.size()) return;
        const auto& p = SettingsManager::instance().presets[index];
        prompt_edit->setPlainText(p.prompt);
        negative_prompt_edit->setPlainText(p.negativePrompt);
    });

    // Load saved settings
    bg_color = Qt::black;
    bg_color_label->setStyleSheet("background-color: " + bg_color.name() + ";");

    // Status bar
    statusBar()->show();
    statusBar()->showMessage("Ready");
}

MainWindow::~MainWindow() {
    if (currentProcess && currentProcess->state() == QProcess::Running) {
        currentProcess->kill();
    }
}

void MainWindow::openFile() {
    QString file = QFileDialog::getOpenFileName(this, "Open Settings", "", "JSON Files (*.json);;All Files (*)");
    if (!file.isEmpty()) {
        if (!file.endsWith(".json")) {
            file += ".json";
        }
        currentSavePath = file;
        SettingsManager::instance().addToRecentlyOpened(file);
        SettingsManager::instance().save();
        statusBar()->showMessage("Opened: " + file);
    }
}

void MainWindow::openRecent() {
    // Handled via menu actions
}

void MainWindow::saveFile() {
    if (currentSavePath.isEmpty()) {
        saveAsFile();
        return;
    }
    if (!currentSavePath.endsWith(".json")) {
        currentSavePath += ".json";
    }
    QJsonObject obj = SettingsManager::instance().toJson();
    QFile file(currentSavePath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(QJsonDocument(obj).toJson(QJsonDocument::Indented));
        SettingsManager::instance().addToRecentlyOpened(currentSavePath);
        SettingsManager::instance().save();
        statusBar()->showMessage("Saved to: " + currentSavePath);
    }
}

void MainWindow::saveAsFile() {
    QString file = QFileDialog::getSaveFileName(this, "Save Settings As", "", "JSON Files (*.json);;All Files (*)");
    if (!file.isEmpty()) {
        if (!file.endsWith(".json")) {
            file += ".json";
        }
        currentSavePath = file;
        QJsonObject obj = SettingsManager::instance().toJson();
        QFile outFile(file);
        if (outFile.open(QIODevice::WriteOnly)) {
            outFile.write(QJsonDocument(obj).toJson(QJsonDocument::Indented));
            SettingsManager::instance().addToRecentlyOpened(file);
            SettingsManager::instance().save();
            statusBar()->showMessage("Saved to: " + file);
        }
    }
}

void MainWindow::showGeneralSettings() {
    GeneralSettingsDialog dlg(this);
    dlg.exec();
}

void MainWindow::showModelManager() {
    ModelManagerDialog dlg(this);
    dlg.exec();
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
        m.width = dlg.width();
        m.height = dlg.height();
        m.modelBasePath = dlg.modelBasePath();
        m.uuid = dlg.uuid();
        SettingsManager::instance().models << m;
        SettingsManager::instance().save();
        model_combo->clear();
        for (const auto& mod : SettingsManager::instance().models) {
            model_combo->addItem(mod.name, QVariant(mod.uuid));
        }
    }
}

void MainWindow::savePreset() {
    QString name = QInputDialog::getText(this, "Save Preset", "Enter preset name:");
    if (name.isEmpty()) return;

    PresetSettings p;
    p.name = name;
    p.prompt = prompt_edit->toPlainText();
    p.negativePrompt = negative_prompt_edit->toPlainText();
    p.uuid = QUuid::createUuid().toString().remove("{").remove("}");
    SettingsManager::instance().presets << p;
    SettingsManager::instance().save();
    preset_combo->clear();
    for (const auto& pr : SettingsManager::instance().presets) {
        preset_combo->addItem(pr.name, QVariant(pr.uuid));
    }
}

void MainWindow::processClicked() {
    // Sanity checks
    if (SettingsManager::instance().general.bin_sd_cli.isEmpty()) {
        output_widget->append("[ERROR] SD-CLI binary path not set. Please configure in Settings.");
        return;
    }
    if (!QFileInfo::exists(SettingsManager::instance().general.bin_sd_cli)) {
        output_widget->append("[ERROR] SD-CLI binary not found at: " + SettingsManager::instance().general.bin_sd_cli);
        return;
    }
    if (source_image_edit->text().isEmpty()) {
        output_widget->append("[ERROR] Source image is required.");
        return;
    }
    if (!QFileInfo::exists(source_image_edit->text())) {
        output_widget->append("[ERROR] Source image not found: " + source_image_edit->text());
        return;
    }
    if (prompt_edit->toPlainText().isEmpty()) {
        output_widget->append("[ERROR] Prompt is required.");
        return;
    }
    if (output_dest_edit->text().isEmpty()) {
        output_widget->append("[ERROR] Output destination is required.");
        return;
    }

    // Disable PROCESS button, enable STOP button
    process_btn->setEnabled(false);
    stop_btn->setEnabled(true);

    // Create output directory if not exists
    QString outputDirPath = output_dest_edit->text();
    QDir outputDir(outputDirPath);
    if (!outputDir.exists()) {
        if (!QDir().mkpath(outputDirPath)) {
            QMessageBox::critical(this, "Error", "Failed to create output directory.");
            return;
        }
    }

    // Build filename
    QString sourceBase = QFileInfo(source_image_edit->text()).completeBaseName();
    QString outputPath;
    if (datetime_checkbox->isChecked()) {
        QString dateTime = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
        outputPath = outputDirPath + "/" + sourceBase + "_" + dateTime + ".avi";
    } else {
        outputPath = outputDirPath + "/" + sourceBase + ".avi";
    }
    lastOutputDest = outputDirPath;

    // Build command arguments
    QString binaryPath = SettingsManager::instance().general.bin_sd_cli;
    QStringList args;
    args << "--mode" << "vid_gen";
    args << "--init-img" << source_image_edit->text();
    args << "--output" << outputPath;
    args << "--prompt" << prompt_edit->toPlainText();
    if (!negative_prompt_edit->toPlainText().isEmpty()) {
        args << "--negative-prompt" << negative_prompt_edit->toPlainText();
    }
 //   args << "--bg-color" << bg_color.name();

    // Add model settings
    int modelIndex = model_combo->currentIndex();
    if (modelIndex < 0 || modelIndex >= SettingsManager::instance().models.size()) {
        output_widget->append("[ERROR] No model selected. Continuing without model settings.");
        return;
    }
    const auto& model = SettingsManager::instance().models[modelIndex];
    args << "--diffusion-model" << model.diffusionModel;
    args << "--llm" << model.llm;
    args << "--vae" << model.vae;
    args << "--width" << QString::number(model.width);
    args << "--height" << QString::number(model.height);
   // args << "--base-path" << m.modelBasePath;

    args << "--video-frames" << "25" ;
    args << "--steps" << "12" ;
    args << "--cfg-scale" << "1.0";
    args << "--backend" << "te=cpu";
    args << "--diffusion-fa";

    // Log command
    QString command = binaryPath;
    for (const auto& arg : args) {
        command += " \"" + arg + "\"";
    }
    output_widget->append("\n[CMD] " + command);
    output_widget->append("[INFO] Starting process...");

    // Start process
    currentProcess = new QProcess(this);
    connect(currentProcess, &QProcess::readyReadStandardOutput, this, [this]() {
        QByteArray out = currentProcess->readAllStandardOutput();
        output_widget->append("[STDOUT] " + out);
    });
    connect(currentProcess, &QProcess::readyReadStandardError, this, [this]() {
        QByteArray err = currentProcess->readAllStandardError();
        output_widget->append("[STDERR] " + err);
    });
    connect(currentProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), [this, outputPath, command, modelIndex]() {
        const auto& model = SettingsManager::instance().models[modelIndex];

        int exitCode = currentProcess->exitCode();
        output_widget->append("\n[INFO] Process finished with exit code: " + QString::number(exitCode));

        // Write companion JSON
        QJsonObject presetObj;
        presetObj["name"] = preset_combo->currentText();
        presetObj["prompt"] = prompt_edit->toPlainText();
        presetObj["negative_prompt"] = negative_prompt_edit->toPlainText();

        writeCompanionJson(outputPath, command,  model, presetObj, source_image_edit->text(), bg_color.name(), exitCode);

        currentProcess->deleteLater();
        currentProcess = nullptr;

        // Re-enable PROCESS button, disable STOP button
        process_btn->setEnabled(true);
        stop_btn->setEnabled(false);
    });

    currentProcess->start(binaryPath, args);
}

void MainWindow::stopClicked() {
    if (currentProcess && currentProcess->state() == QProcess::Running) {
        currentProcess->kill();
        output_widget->append("[INFO] Process stopped.");
    }
    process_btn->setEnabled(true);
    stop_btn->setEnabled(false);
}

void MainWindow::selectSourceImage() {
    QString file = QFileDialog::getOpenFileName(this, "Select Source Image", lastSourcePath, "Images (*.png *.jpg *.jpeg *.bmp *.tiff)");
    if (!file.isEmpty()) {
        source_image_edit->setText(file);
        lastSourcePath = QFileInfo(file).absolutePath();
        QSettings settings("qt-sd-animator", "Settings");
        settings.setValue("lastSourcePath", lastSourcePath);
    }
}

void MainWindow::selectBgColor() {
    QColor color = QColorDialog::getColor(Qt::black, this, "Select Background Color");
    if (color.isValid()) {
        bg_color = color;
        bg_color_label->setStyleSheet("background-color: " + color.name() + ";");
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
    modelObj["width"] = model.width;
    modelObj["height"] = model.height;
    modelObj["model_base_path"] = model.modelBasePath;
    json["model"] = modelObj;
    json["preset"] = presetObj;

    QFile jsonFile(outputPath + ".json");
    if (jsonFile.open(QIODevice::WriteOnly)) {
        jsonFile.write(QJsonDocument(json).toJson(QJsonDocument::Indented));
    }
}
