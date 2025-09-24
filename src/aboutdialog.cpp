#include "aboutdialog.h"
#include "ui_aboutdialog.h"
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QtConcurrent>

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);

    // 加载 license.txt 文件并设置到 licenseTextEdit
    ui->licenseTextEdit->setPlainText("Loading License, please wait...");

    QThreadPool::globalInstance()->start([this]() {
        QFile licenseFile(":/data/license.txt");
        QString licenseText;

        if (licenseFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream stream(&licenseFile);
            licenseText = stream.readAll();
            licenseFile.close();
        } else {
            licenseText = "Failed to load license file.\nThis software uses MIT License.";
            qWarning() << "Failed to open license file:" << licenseFile.errorString();
        }

        QMetaObject::invokeMethod(this, [this, licenseText]() {
            ui->licenseTextEdit->setText(licenseText);
        }, Qt::QueuedConnection);
    });

    QFile commitFile(":/data/commit.md");
    if (commitFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream stream(&commitFile);
        QString commitText = stream.readAll();
        commitFile.close();

        if (commitText.trimmed().isEmpty()) {
            ui->commitLabel->setText("Built on Commit: Unknown");
        } else {
            ui->commitLabel->setText(commitText);
            ui->commitLabel->setTextFormat(Qt::MarkdownText);
        }
    } else {
        ui->commitLabel->setText("Built on Commit: Unknown");
    }

}

AboutDialog::~AboutDialog()
{
    delete ui;
}
