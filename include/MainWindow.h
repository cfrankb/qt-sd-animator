#pragma once

#include <QMainWindow>
#include <QComboBox>
#include <QLineEdit>
#include <QTextEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QLabel>
#include <QCheckBox>
#include <QColorDialog>
#include <QFileDialog>
#include <QProcess>
#include <QDateTime>
#include <QFileInfo>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QMessageBox>
#include <QElapsedTimer>
#include <QStandardPaths>
#include <QSettings>
#include <QUuid>
#include <QInputDialog>
#include <QStatusBar>
#include "SettingsManager.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void openFile();
    void openRecent();
    void saveFile();
    void saveAsFile();
    void showGeneralSettings();
    void showModelManager();
    void showPresetManager();
    void newModel();
    void savePreset();
    void processClicked();
    void stopClicked();
    void selectSourceImage();
    void selectBgColor();
    QString executionTimeStr();


private:
    void buildProcessArgs(QString& binaryPath, QStringList& args);
    void writeCompanionJson(const QString& outputPath, const QString& command, const ModelSettings& model, const QJsonObject &presetObj, const QString& sourceImage, const QString& bgColor, int exitCode);
    void refreshDropDowns();

    QComboBox* model_combo;
    QComboBox* preset_combo;
    QLineEdit* source_image_edit;
    QPlainTextEdit* prompt_edit;
    QPlainTextEdit* negative_prompt_edit;
    QLineEdit* seed_edit;
    QPushButton* random_seed_btn;
    QTextEdit* output_widget;
    QLabel* execution_time_label;
    QPushButton* process_btn;
    QPushButton* stop_btn;
    QLabel* bg_color_label;
    //QLineEdit* output_dest_edit;
    QLineEdit* filename_edit;
    QCheckBox* datetime_checkbox;
    QColor bg_color;

    QString lastSourcePath;
    QString lastOutputDest;
    QString lastOutputFilename;
    QString currentSavePath;
    QProcess* currentProcess = nullptr;
    QTimer* execution_timer = nullptr;
    QElapsedTimer execution_timer_elapsed;
};
