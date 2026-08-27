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
    bool sourceImageRequired = false;
    QString ext = ".png";
    QStringList additionalParameters;
    QString notes;
};

struct PresetSettings {
    QString name;
    QString prompt;
    QString negativePrompt;
    QString seed;
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
    QString timestamp;
    int exitCode = -1;
};

struct PixelSize {
    int w;
    int h;
    QString text;
    QString uuid;
};


class SettingsManager {
public:
    static SettingsManager& instance() {
        static SettingsManager inst;
        return inst;
    }

    void load();
    void save();

    GeneralSettings m_general;
    QList<ModelSettings> m_models;
    QList<PresetSettings> m_presets;
    QStringList m_recentlyOpened;
    QList<PixelSize> m_sizes;

    void addToRecentlyOpened(const QString& path);
    void loadFromJson(const QJsonObject& obj);
    QJsonObject toJson() const;

    bool loadSettings(const QString &filename);
    bool saveSettings(const QString &filename);


private:
    SettingsManager();
    QString settingsPath() const;
    QString recentFilesPath() const;
};
