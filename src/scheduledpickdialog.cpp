#include "scheduledpickdialog.h"
#include "ui_scheduledpickdialog.h"
#include <QMessageBox>

ScheduledPickDialog::ScheduledPickDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ScheduledPickDialog),
    countdownTimer(new QTimer(this))
{
    ui->setupUi(this);
    setWindowTitle(tr("Scheduled Pick"));

    ui->hourSpinBox->setRange(0, 23);
    ui->minuteSpinBox->setRange(0, 59);
    ui->secondSpinBox->setRange(0, 59);

    connect(ui->startButton, &QPushButton::clicked, this, &ScheduledPickDialog::onStartClicked);
    connect(countdownTimer, &QTimer::timeout, this, &ScheduledPickDialog::updateCountdown);
}

ScheduledPickDialog::~ScheduledPickDialog()
{
    delete ui;
}

void ScheduledPickDialog::onStartClicked()
{
    int hours = ui->hourSpinBox->value();
    int minutes = ui->minuteSpinBox->value();
    int seconds = ui->secondSpinBox->value();

    remainingSeconds = hours * 3600 + minutes * 60 + seconds;

    if (remainingSeconds <= 0) {
        QMessageBox::warning(this, tr("Invalid Time"), tr("Please set a valid countdown time."));
        return;
    }

    ui->hourSpinBox->setEnabled(false);
    ui->minuteSpinBox->setEnabled(false);
    ui->secondSpinBox->setEnabled(false);
    ui->startButton->setEnabled(false);

    updateCountdown();

    countdownTimer->start(1000);
}

void ScheduledPickDialog::updateCountdown()
{
    remainingSeconds--;

    if (remainingSeconds <= 0) {
        countdownTimer->stop();
        emit timeElapsed();
        accept();
        return;
    }

    // 更新显示
    int hours = remainingSeconds / 3600;
    int minutes = (remainingSeconds % 3600) / 60;
    int seconds = remainingSeconds % 60;

    ui->timeLabel->setText(QString("%1:%2:%3")
                               .arg(hours, 2, 10, QLatin1Char('0'))
                               .arg(minutes, 2, 10, QLatin1Char('0'))
                               .arg(seconds, 2, 10, QLatin1Char('0')));
}
