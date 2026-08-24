#pragma once

#include <QString>
#include <QJsonArray>
#include <QJsonObject>
#include <QDateTime>
#include <QUuid>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QSettings>
#include <QMessageBox>
#include <QFileDialog>
#include <QProcess>
#include <QTextEdit>
#include <QTimer>

struct ModelSettings {
    QString name;
    QString diffusionModel;
    QString llm;
    QString vae;
    QString uuid;
    int width = 512;
    int height = 512;
    QString modelBasePath;
    QStringList additionalParameters;
};

struct PresetSettings {
    QString name;
    QString prompt;
    QString negativePrompt;
    QString uuid;
};

struct GeneralSettings {
    QString bin_sd_cli;
    QString output_path;
};

struct ExecutedCommand {
    QString command;
    ModelSettings model;
    PresetSettings preset;
    GeneralSettings general;
    QString sourceImage;
    QString bgColor;
    QString timestamp;
    int exitCode = -1;
};

class SettingsManager {
public:
    static SettingsManager& instance() {
        static SettingsManager inst;
        return inst;
    }

    void load();
    void save();

    GeneralSettings general;
    QList<ModelSettings> models;
    QList<PresetSettings> presets;
    QStringList recentlyOpened;

    void addToRecentlyOpened(const QString& path);
    void loadFromJson(const QJsonObject& obj);
    QJsonObject toJson() const;

private:
    SettingsManager();
    QString settingsPath() const;
    QString recentFilesPath() const;
};
