#include "MainWindow.h"
#include "SettingsManager.h"
#include <QApplication>
#include <QStandardPaths>
#include <QDir>
#include <QSettings>
#include <QMessageBox>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    // Load settings
    SettingsManager::instance().load();

    // Create and show main window
    MainWindow window;
    window.show();

    return app.exec();
}
