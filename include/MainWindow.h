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
    bool saveFile();
    bool saveAsFile();
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
    void closeEvent(QCloseEvent* event);


private:
    void buildProcessArgs(QString& binaryPath, QStringList& args);
    void writeCompanionJson(const QString& outputPath, const QString& command, const ModelSettings& model, const QJsonObject &presetObj, const QString& sourceImage, const QString& bgColor, int exitCode);
    void refreshDropDowns();

    QComboBox* m_modelCombo;
 //   QComboBox* preset_combo;
    QComboBox* m_sizeCombo;
    QLineEdit* m_sourceImageEdit;
    QPlainTextEdit* m_promptEdit;
    QPlainTextEdit* m_negativePromptEdit;
    QLineEdit* m_seedEdit;
    QPushButton* m_randomSeedBtn;
    QTextEdit* m_outputWidget;
    QLabel* m_executionTimeLabel;
    QPushButton* m_processBtn;
    QPushButton* m_stopBtn;
    QLabel* m_bgColorLabel;
    QLineEdit* m_outputDestEdit;
    QLineEdit* m_filenameEdit;
    QCheckBox* m_datetimeCheckbox;
    QColor m_bgColor;

    QString m_lastSourcePath;
    QString m_lastOutputDest;
    QString m_lastOutputFilename;
    QString m_currentSavePath;
    bool m_dirty = false;
    QProcess* m_currentProcess = nullptr;
    QTimer* m_executionTimer = nullptr;
    QElapsedTimer m_executionTimerElapsed;
};
