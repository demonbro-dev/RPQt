#include "passphrasedialog.h"
#include "ui_passphrasedialog.h"
#include <QMessageBox>
#include <QDebug>

PassphraseDialog::PassphraseDialog(const QString &fbsFilePath,
                                   Mode mode,
                                   QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PassphraseDialog),
    fbsFilePath(fbsFilePath),
    fbsHandler(),
    currentMode(mode)
{
    ui->setupUi(this);
    setModal(true);

    // 根据模式设置界面
    if (currentMode == InputMode) {
        setWindowTitle(tr("Type Passphrase"));
        ui->label->setText(tr("Please type your passphrase:"));

        // 从JSON文件加载密码
        QString error;
        correctPassphrase = fbsHandler.getPassphrase(fbsFilePath, error);
        if (!error.isEmpty()) {
            qWarning() << "Failed to load passphrase:" << error;
            QMessageBox::warning(this, tr("Error"), tr("Cannot load passphrase config:") + error);
            correctPassphrase = "";
        }
    } else {
        setWindowTitle(tr("Set new passphrase"));
        ui->label->setText(tr("Type your new passphrase:"));
    }
}

PassphraseDialog::~PassphraseDialog()
{
    delete ui;
}

QString PassphraseDialog::getCorrectPassphrase() const
{
    return correctPassphrase;
}

QString PassphraseDialog::getNewPassphrase() const
{
    return newPassphrase;
}

void PassphraseDialog::on_submitButton_clicked()
{
    QString input = ui->lineEdit->text();

    if (currentMode == InputMode) {
        if (input == correctPassphrase) {
            accept();
        } else {
            QMessageBox::warning(this, tr("Error"), tr("Incorrect passphrase!"));
            ui->lineEdit->clear();
        }
    } else {
        // 设置新密码模式
        if (input.isEmpty()) {
            QMessageBox::StandardButton reply;
            reply = QMessageBox::warning(this, tr("Warning"),
                                         tr("You are setting an empty passphrase.\nAre you sure to continue?"),
                                         QMessageBox::Yes | QMessageBox::No);
            if (reply == QMessageBox::No) {
                return;
            }
        }

        newPassphrase = input;
        if (saveNewPassphrase()) {
            accept();
        }
    }
}

bool PassphraseDialog::saveNewPassphrase()
{
    QString error;
    bool success = fbsHandler.savePassphrase(newPassphrase, fbsFilePath, error);

    if (!success) {
        QMessageBox::warning(this, tr("Error"), tr("Failed to save passphrase:") + error);
    }

    return success;
}
