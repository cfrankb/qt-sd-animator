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

bool SettingsManager::loadSettings(const QString &filename)
{
    QFile file(filename);
    if (file.exists() && file.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        if (doc.isObject()) {
            loadFromJson(doc.object());
        }
        return true;
    }
    return false;
}

bool SettingsManager::saveSettings(const QString &filename)
{
    QFile file(settingsPath());
    if (file.open(QIODevice::WriteOnly)) {
        file.write(QJsonDocument(toJson()).toJson(QJsonDocument::Indented));
        return true;
    }
    return false;
}


void SettingsManager::load() {
    QFile recentFile(recentFilesPath());
    if (recentFile.exists() && recentFile.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(recentFile.readAll());
        if (doc.isArray()) {
            m_recentlyOpened.clear();
            for (const auto& item : doc.array()) {
                m_recentlyOpened << item.toString();
            }
        }
    }
}

void SettingsManager::save() {
    QJsonArray arr;
    for (const auto& path : m_recentlyOpened) {
        arr << path;
    }
    QFile recentFile(recentFilesPath());
    if (recentFile.open(QIODevice::WriteOnly)) {
        recentFile.write(QJsonDocument(arr).toJson());
    }
}

void SettingsManager::addToRecentlyOpened(const QString& path) {
    m_recentlyOpened.removeAll(path);
    m_recentlyOpened.prepend(path);
    if (m_recentlyOpened.size() > 10) {
        m_recentlyOpened.removeLast();
    }
}

void SettingsManager::loadFromJson(const QJsonObject& obj) {
    m_general.bin_sd_cli = obj.value("bin_sd_cli").toString();
    m_general.output_path = obj.value("output_path").toString();

    m_models.clear();
    QJsonArray modelsArr = obj.value("models").toArray();
    for (const auto& modelVal : modelsArr) {
        ModelSettings m;
        m.name = modelVal.toObject().value("name").toString();
        m.diffusionModel = modelVal.toObject().value("diffusion-model").toString();
        m.llm = modelVal.toObject().value("llm").toString();
        m.vae = modelVal.toObject().value("vae").toString();
        m.uuid = modelVal.toObject().value("uuid").toString();
        m.sourceImageRequired = modelVal.toObject().value("source_image_required").toBool(true);
        m.ext = modelVal.toObject().value("ext").toString(".avi");
        m.notes = modelVal.toObject().value("notes").toString();

        // Handle backward compatibility: parameters may be string or array
        QJsonValue paramsVal = modelVal.toObject().value("additional_parameters");
        if (paramsVal.isString()) {
            QString paramsStr = paramsVal.toString();
            m.additionalParameters = paramsStr.split('\n', Qt::SkipEmptyParts);
        } else if (paramsVal.isArray()) {
            QJsonArray paramsArr = paramsVal.toArray();
            for (const auto& item : paramsArr) {
                m.additionalParameters << item.toString();
            }
        }
        m_models << m;
    }

    m_presets.clear();
    QJsonArray presetsArr = obj.value("presets").toArray();
    for (const auto& presetVal : presetsArr) {
        PresetSettings p;
        p.name = presetVal.toObject().value("name").toString();
        p.prompt = presetVal.toObject().value("prompt").toString();
        p.negativePrompt = presetVal.toObject().value("negative_prompt").toString();
        p.seed = presetVal.toObject().value("seed").toString();
        p.uuid = presetVal.toObject().value("uuid").toString();
        m_presets << p;
    }

    // Load sizes
    m_sizes.clear();
    QJsonArray sizesArr = obj.value("sizes").toArray();
    for (const auto& sizeVal : sizesArr) {
        PixelSize size;
        size.w = sizeVal.toObject().value("w").toInt();
        size.h = sizeVal.toObject().value("h").toInt();
        size.text = sizeVal.toObject().value("text").toString();
        size.uuid = sizeVal.toObject().value("uuid").toString();
        m_sizes.append(size);
    }
}

QJsonObject SettingsManager::toJson() const {
    QJsonObject obj;
    obj["bin_sd_cli"] = m_general.bin_sd_cli;
    obj["output_path"] = m_general.output_path;

    QJsonArray modelsArr;
    for (const auto& m : m_models) {
        QJsonObject mObj;
        mObj["name"] = m.name;
        mObj["diffusion-model"] = m.diffusionModel;
        mObj["llm"] = m.llm;
        mObj["vae"] = m.vae;
        mObj["uuid"] = m.uuid;
        mObj["source_image_required"] = m.sourceImageRequired;
        mObj["ext"] = m.ext;
        mObj["notes"] = m.notes;

        // Save additional_parameters as JSON array
        QJsonArray paramsArr;
        for (const auto& param : m.additionalParameters) {
            paramsArr << param;
        }
        mObj["additional_parameters"] = paramsArr;
        modelsArr << mObj;
    }
    obj["models"] = modelsArr;

    QJsonArray presetsArr;
    for (const auto& p : m_presets) {
        QJsonObject pObj;
        pObj["name"] = p.name;
        pObj["prompt"] = p.prompt;
        pObj["negative_prompt"] = p.negativePrompt;
        pObj["seed"] = p.seed;
        pObj["uuid"] = p.uuid;
        presetsArr << pObj;
    }
    obj["presets"] = presetsArr;

    // Save sizes
    QJsonArray sizesArr;
    for (const auto& size : m_sizes) {
        QJsonObject sizeObj;
        sizeObj["w"] = size.w;
        sizeObj["h"] = size.h;
        sizeObj["text"] = size.text;
        sizeObj["uuid"] = size.uuid;
        sizesArr << sizeObj;
    }
    obj["sizes"] = sizesArr;

    return obj;
}
