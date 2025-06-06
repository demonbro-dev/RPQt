#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStringList>
#include <QMap>
#include <QTranslator>
#include "jsonhandler.h"
#include "namemanager.h"
#include "pickhistorydialog.h"
#include "sidebutton.h"
#include "pickerlogic.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void showSideButton();

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void onPickButtonClicked();
    void onNamesPicked(const QStringList &names);
    void onPreviewNames(const QStringList &names);
    void onTopmostToggled(bool checked);
    bool loadTranslation(const QLocale& locale);

private:
    Ui::MainWindow *ui;
    JsonHandler jsonHandler;
    NameManager *nameManager;
    PickerLogic *pickerLogic;
    SideButton *sideButton;
    PickHistoryDialog *historyDialog;
    QMap<QString, QStringList> nameGroups;
    QStringList currentNames;
    QTranslator qtTranslator;
    QTranslator appTranslator;
    QList<QStringList> pickHistory;
    bool m_globalTrackingEnabled;
    QPoint m_dragPosition;

    void loadNameLists();
    void setupConnections();
    void updateFontSize();
    void setupNameListCombo();
    void onNameListComboActivated(int index);
    void showMainWindow();
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
};

#endif // MAINWINDOW_H
