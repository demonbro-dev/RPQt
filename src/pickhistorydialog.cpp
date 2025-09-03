#include "pickhistorydialog.h"
#include "ui_pickhistorydialog.h"
#include <QFileDialog>

PickHistoryDialog::PickHistoryDialog(const QList<QStringList> &history, QWidget *parent)
    : QDialog(parent),
    ui(new Ui::PickHistoryDialog)
{
    ui->setupUi(this);
    historyListWidget = ui->historyListWidget;
    populateList(history);

    connect(ui->ClearButton, &QPushButton::clicked, this, &PickHistoryDialog::clearHistory);
    connect(ui->ExportButton, &QPushButton::clicked, this, &PickHistoryDialog::exportHistory);
}

PickHistoryDialog::~PickHistoryDialog()
{
    delete ui;
}

void PickHistoryDialog::populateList(const QList<QStringList> &history)
{
    historyListWidget->clear();

    if (history.isEmpty()) {
        historyListWidget->addItem(tr("No history entries yet"));
        return;
    }

    for (const QStringList &entry : history) {
        historyListWidget->addItem(entry.join(", "));
    }
    historyListWidget->scrollToBottom();
}

void PickHistoryDialog::updateHistory(const QList<QStringList> &history)
{
    populateList(history);
}

void PickHistoryDialog::clearHistory()
{
    historyListWidget->clear();
    historyListWidget->addItem(tr("No history entries yet"));
    emit ClearHistory();
}

void PickHistoryDialog::exportHistory()
{
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("Export History"), "",
                                                    tr("Text Files (*.txt)"));

    if (fileName.isEmpty())
        return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream out(&file);
    for (int i = 0; i < historyListWidget->count(); ++i) {
        QListWidgetItem *item = historyListWidget->item(i);
        out << item->text() << "\n";
    }

    file.close();
}
