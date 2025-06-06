#include <QListWidget>
#include <QDialog>

namespace Ui {
class PickHistoryDialog;
}

class PickHistoryDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PickHistoryDialog(const QList<QStringList> &history, QWidget *parent = nullptr);
    ~PickHistoryDialog();
    void updateHistory(const QList<QStringList> &history);

private:
    void populateList(const QList<QStringList> &history);
    Ui::PickHistoryDialog *ui;
    QListWidget *historyListWidget;
};
