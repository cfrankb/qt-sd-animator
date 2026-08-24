#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QFileDialog>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFormLayout>
#include <QMessageBox>
#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>

class GeneralSettingsDialog : public QDialog {
    Q_OBJECT

public:
    explicit GeneralSettingsDialog(QWidget* parent = nullptr);

    QString bin_sd_cli() const { return bin_sd_cli_edit->text(); }
    QString outputPath() const { return output_path_edit->text(); }

private slots:
    void browseBinary();
    void browseOutputPath();
    void accept();

private:
    QLineEdit* bin_sd_cli_edit;
    QLineEdit* output_path_edit;
};
