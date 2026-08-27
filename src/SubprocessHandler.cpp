#include "SubprocessHandler.h"
#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QMessageBox>
#include <QStandardPaths>

SubprocessHandler::SubprocessHandler(QObject* parent, QTextEdit* outputWidget)
    : QObject(parent), m_outputWidget(outputWidget) {
}

QString SubprocessHandler::buildCommand(const QString& binaryPath, const QStringList& args) const {
    QString cmd = binaryPath;
    for (const auto& arg : args) {
        cmd += " \"" + arg + "\"";
    }
    return cmd;
}

void SubprocessHandler::startProcess(const QString& binaryPath, const QStringList& args) {
    if (m_process && m_process->state() == QProcess::Running) {
        return;
    }

    m_process = new QProcess(this);
    connect(m_process, &QProcess::readyReadStandardOutput, this, [this]() {
        QByteArray out = m_process->readAllStandardOutput();
        if (m_outputWidget) {
            m_outputWidget->append("[STDOUT] " + out);
        }
    });

    connect(m_process, &QProcess::readyReadStandardError, this, [this]() {
        QByteArray err = m_process->readAllStandardError();
        if (m_outputWidget) {
            m_outputWidget->append("[STDERR] " + err);
        }
    });

    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this](int exitCode, QProcess::ExitStatus exitStatus) {
        if (m_outputWidget) {
            m_outputWidget->append("\n[INFO] Process finished with exit code: " + QString::number(exitCode));
        }
    });

    QString cmd = buildCommand(binaryPath, args);
    if (m_outputWidget) {
        m_outputWidget->append("\n[CMD] " + cmd);
    }

    m_process->start(binaryPath, args);
}

void SubprocessHandler::stopProcess() {
    if (m_process && m_process->state() == QProcess::Running) {
        m_process->kill();
        m_process->waitForFinished(1000);
    }
}

bool SubprocessHandler::isRunning() const {
    return m_process && m_process->state() == QProcess::Running;
}
