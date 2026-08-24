#include "SettingsManager.h"
#include <QJsonArray>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>
#include <QDir>
#include <QDateTime>
#include <QUuid>
#include <QMessageBox>

SettingsManager::SettingsManager() {
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/qt-sd-animator";
    QDir().mkpath(configDir);
}

QString SettingsManager::settingsPath() const {
    return QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/qt-sd-animator/settings.json";
}

QString SettingsManager::recentFilesPath() const {
    return QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/qt-sd-animator/recently_open.json";
}

void SettingsManager::load() {
    QFile file(settingsPath());
    if (file.exists() && file.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        if (doc.isObject()) {
            loadFromJson(doc.object());
        }
    }

    QFile recentFile(recentFilesPath());
    if (recentFile.exists() && recentFile.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(recentFile.readAll());
        if (doc.isArray()) {
            recentlyOpened.clear();
            for (const auto& item : doc.array()) {
                recentlyOpened << item.toString();
            }
        }
    }
}

void SettingsManager::save() {
    QJsonObject obj = toJson();
    QFile file(settingsPath());
    if (file.open(QIODevice::WriteOnly)) {
        file.write(QJsonDocument(obj).toJson(QJsonDocument::Indented));
    }

    QJsonArray arr;
    for (const auto& path : recentlyOpened) {
        arr << path;
    }
    QFile recentFile(recentFilesPath());
    if (recentFile.open(QIODevice::WriteOnly)) {
        recentFile.write(QJsonDocument(arr).toJson());
    }
}

void SettingsManager::addToRecentlyOpened(const QString& path) {
    recentlyOpened.removeAll(path);
    recentlyOpened.prepend(path);
    if (recentlyOpened.size() > 10) {
        recentlyOpened.removeLast();
    }
}

void SettingsManager::loadFromJson(const QJsonObject& obj) {
    general.bin_sd_cli = obj.value("bin_sd_cli").toString();
    general.output_path = obj.value("output_path").toString();

    models.clear();
    QJsonArray modelsArr = obj.value("models").toArray();
    for (const auto& modelVal : modelsArr) {
        ModelSettings m;
        m.name = modelVal.toObject().value("name").toString();
        m.diffusionModel = modelVal.toObject().value("diffusion-model").toString();
        m.llm = modelVal.toObject().value("llm").toString();
        m.vae = modelVal.toObject().value("vae").toString();
        m.uuid = modelVal.toObject().value("uuid").toString();
        m.width = modelVal.toObject().value("width").toInt(512);
        m.height = modelVal.toObject().value("height").toInt(512);
        m.modelBasePath = modelVal.toObject().value("model_base_path").toString();
        models << m;
    }

    presets.clear();
    QJsonArray presetsArr = obj.value("presets").toArray();
    for (const auto& presetVal : presetsArr) {
        PresetSettings p;
        p.name = presetVal.toObject().value("name").toString();
        p.prompt = presetVal.toObject().value("prompt").toString();
        p.negativePrompt = presetVal.toObject().value("negative_prompt").toString();
        p.uuid = presetVal.toObject().value("uuid").toString();
        presets << p;
    }
}

QJsonObject SettingsManager::toJson() const {
    QJsonObject obj;
    obj["bin_sd_cli"] = general.bin_sd_cli;
    obj["output_path"] = general.output_path;

    QJsonArray modelsArr;
    for (const auto& m : models) {
        QJsonObject mObj;
        mObj["name"] = m.name;
        mObj["diffusion-model"] = m.diffusionModel;
        mObj["llm"] = m.llm;
        mObj["vae"] = m.vae;
        mObj["uuid"] = m.uuid;
        mObj["width"] = m.width;
        mObj["height"] = m.height;
        mObj["model_base_path"] = m.modelBasePath;
        modelsArr << mObj;
    }
    obj["models"] = modelsArr;

    QJsonArray presetsArr;
    for (const auto& p : presets) {
        QJsonObject pObj;
        pObj["name"] = p.name;
        pObj["prompt"] = p.prompt;
        pObj["negative_prompt"] = p.negativePrompt;
        pObj["uuid"] = p.uuid;
        presetsArr << pObj;
    }
    obj["presets"] = presetsArr;

    return obj;
}
