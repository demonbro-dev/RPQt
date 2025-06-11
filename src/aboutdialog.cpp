#include "aboutdialog.h"
#include "ui_aboutdialog.h"
#include <QFile>
#include <QTextStream>
#include <QMessageBox>

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);

    // 加载 license.md 文件并设置到 licenseTextEdit
    QFile licenseFile(":/data/license.md");
    QFile commitFile(":/data/commit.md");
    if (licenseFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream stream(&licenseFile);
        QString licenseText = stream.readAll();
        ui->licenseTextEdit->setMarkdown(licenseText);
        licenseFile.close();
    } else {
        ui->licenseTextEdit->setPlainText("Failed to load license file.\nThis software uses GPLv3.");
        qWarning() << "Failed to open license file:" << licenseFile.errorString();
    }
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
