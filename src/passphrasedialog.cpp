#include "passphrasedialog.h"
#include "ui_passphrasedialog.h"
#include <QMessageBox>
#include <QDebug>

PassphraseDialog::PassphraseDialog(const QString &jsonFilePath,
                                   Mode mode,
                                   QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PassphraseDialog),
    jsonFilePath(jsonFilePath),
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
        correctPassphrase = jsonHandler.getPassphrase(jsonFilePath, error);
        if (!error.isEmpty()) {
            qWarning() << "Failed to load passphrase:" << error;
            QMessageBox::warning(this, tr("Error"), tr("Cannot load passphrase config:") + error);
            correctPassphrase = ""; // 设置为空字符串作为默认值
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
        // 输入密码模式
        if (input == correctPassphrase) {
            accept();  // 只有密码正确才接受对话框
        } else {
            QMessageBox::warning(this, tr("Error"), tr("Incorrect passphrase!"));
            ui->lineEdit->clear();
        }
    } else {
        // 设置新密码模式
        if (input.isEmpty()) {
            // 询问用户是否确定使用空密码
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
            accept();  // 只有保存成功才接受对话框
        }
    }
}

bool PassphraseDialog::saveNewPassphrase()
{
    QString error;
    bool success = jsonHandler.savePassphrase(newPassphrase, jsonFilePath, error);

    if (!success) {
        QMessageBox::warning(this, tr("Error"), tr("Failed to save passphrase:") + error);
    }

    return success;
}
