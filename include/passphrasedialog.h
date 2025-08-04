#ifndef PASSPHRASEDIALOG_H
#define PASSPHRASEDIALOG_H

#include <QDialog>
#include "fbshandler.h"

namespace Ui {
class PassphraseDialog;
}

class PassphraseDialog : public QDialog
{
    Q_OBJECT

public:
    enum Mode {
        InputMode,
        SetNewMode
    };

    explicit PassphraseDialog(const QString &fbsFilePath,
                              Mode mode = InputMode,
                              QWidget *parent = nullptr);
    ~PassphraseDialog();

    QString getCorrectPassphrase() const;
    QString getNewPassphrase() const;

private slots:
    void on_submitButton_clicked();

private:
    Ui::PassphraseDialog *ui;
    QString correctPassphrase;
    QString newPassphrase;
    bool saveNewPassphrase();
    QString fbsFilePath;
    FbsHandler fbsHandler;
    Mode currentMode;
};

#endif // PASSPHRASEDIALOG_H
