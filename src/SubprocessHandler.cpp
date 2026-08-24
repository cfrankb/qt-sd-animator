#include "SubprocessHandler.h"
#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QMessageBox>
#include <QStandardPaths>

SubprocessHandler::SubprocessHandler(QObject* parent, QTextEdit* outputWidget)
    : QObject(parent), outputWidget(outputWidget) {
}

QString SubprocessHandler::buildCommand(const QString& binaryPath, const QStringList& args) const {
    QString cmd = binaryPath;
    for (const auto& arg : args) {
        cmd += " \"" + arg + "\"";
    }
    return cmd;
}

void SubprocessHandler::startProcess(const QString& binaryPath, const QStringList& args) {
    if (process && process->state() == QProcess::Running) {
        return;
    }

    process = new QProcess(this);
    connect(process, &QProcess::readyReadStandardOutput, this, [this]() {
        QByteArray out = process->readAllStandardOutput();
        if (outputWidget) {
            outputWidget->append("[STDOUT] " + out);
        }
    });

    connect(process, &QProcess::readyReadStandardError, this, [this]() {
        QByteArray err = process->readAllStandardError();
        if (outputWidget) {
            outputWidget->append("[STDERR] " + err);
        }
    });

    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this](int exitCode, QProcess::ExitStatus exitStatus) {
        if (outputWidget) {
            outputWidget->append("\n[INFO] Process finished with exit code: " + QString::number(exitCode));
        }
    });

    QString cmd = buildCommand(binaryPath, args);
    if (outputWidget) {
        outputWidget->append("\n[CMD] " + cmd);
    }

    process->start(binaryPath, args);
}

void SubprocessHandler::stopProcess() {
    if (process && process->state() == QProcess::Running) {
        process->kill();
        process->waitForFinished(1000);
    }
}

bool SubprocessHandler::isRunning() const {
    return process && process->state() == QProcess::Running;
}
