#include <QListWidget>
#include <QDialog>

namespace Ui {
class PickHistoryDialog;
}

class PickHistoryDialog : public QDialog
{
    Q_OBJECT

signals:
    void ClearHistory();

public:
    explicit PickHistoryDialog(const QList<QStringList> &history, QWidget *parent = nullptr);
    ~PickHistoryDialog();
    void updateHistory(const QList<QStringList> &history);
    void clearHistory();

private:
    void populateList(const QList<QStringList> &history);
    Ui::PickHistoryDialog *ui;
    QListWidget *historyListWidget;
};
