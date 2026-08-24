#include "MainWindow.h"
#include "SettingsManager.h"
#include <QApplication>
#include <QStandardPaths>
#include <QDir>
#include <QSettings>
#include <QMessageBox>
#include <QLockFile>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    // Prevent multiple instances
    QString lockFilePath = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/qt-sd-animator.lock";
    QLockFile lockFile(lockFilePath);
    
    if (!lockFile.tryLock()) {
        QMessageBox::warning(nullptr, "Already Running", 
                           "qt-sd-animator is already running. Please use the existing instance.");
        return 1;
    }

    // Load settings
    SettingsManager::instance().load();

    // Create and show main window
    MainWindow window;
    window.show();

    return app.exec();
}
