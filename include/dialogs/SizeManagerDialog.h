#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFormLayout>
#include <QMessageBox>
#include <QUuid>
#include "SettingsManager.h"

class SizeManagerDialog : public QDialog {
    Q_OBJECT

public:
    explicit SizeManagerDialog(QWidget* parent = nullptr);

private slots:
    void addSize();
    void deleteSize();
    void onSizeChanged(int index);
    void acceptDialog();

private:
    void populateCombo();
    void saveFormToLocal();
    void loadFormFromLocal(int index);

    QComboBox* m_sizeCombo;
    QList<PixelSize> m_localSizes;
    int m_currentSizeIndex = -1;

    QLineEdit* m_textEdit;
    QLineEdit* m_widthEdit;
    QLineEdit* m_heightEdit;
    QLineEdit* m_uuidEdit;
};
