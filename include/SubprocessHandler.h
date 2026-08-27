#pragma once

#include <QProcess>
#include <QTextEdit>
#include <QStringList>
#include <QString>
#include <QObject>

class SubprocessHandler : public QObject {
public:
    SubprocessHandler(QObject* parent, QTextEdit* outputWidget);

    void startProcess(const QString& binaryPath, const QStringList& args);
    void stopProcess();
    bool isRunning() const;

    QString buildCommand(const QString& binaryPath, const QStringList& args) const;

private:
    QProcess* m_process = nullptr;
    QTextEdit* m_outputWidget = nullptr;
};
