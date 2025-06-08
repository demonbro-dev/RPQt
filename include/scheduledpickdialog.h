#ifndef SCHEDULEDPICKDIALOG_H
#define SCHEDULEDPICKDIALOG_H

#include <QDialog>
#include <QTimer>

namespace Ui {
class ScheduledPickDialog;
}

class ScheduledPickDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ScheduledPickDialog(QWidget *parent = nullptr);
    ~ScheduledPickDialog();

signals:
    void timeElapsed();

private slots:
    void onStartClicked();
    void updateCountdown();

private:
    Ui::ScheduledPickDialog *ui;
    QTimer *countdownTimer;
    int remainingSeconds = 0;
};

#endif // SCHEDULEDPICKDIALOG_H
