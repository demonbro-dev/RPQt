#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStringList>
#include <QMap>
#include <QTranslator>
#include <QSystemTrayIcon>
#include <QLabel>
#include "jsonhandler.h"
#include "settingshandler.h"
#include "namemanager.h"
#include "pickhistorydialog.h"
#include "sidebutton.h"
#include "pickerlogic.h"
#include "randmirage.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void showSideButton(bool toRight);
    void showTrayIcon();
    void showRandMirage();
    void showMainWindow();

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
    PickerLogic::RandomGeneratorType m_currentRandomType;
    SettingsHandler settingsHandler;
    JsonHandler jsonHandler;
    NameManager *nameManager;
    PickerLogic *pickerLogic;
    SideButton *sideButton;
    PickHistoryDialog *historyDialog;
    RandMirage *randMirage;
    QMap<QString, QStringList> nameGroups;
    QStringList currentNames;
    QTranslator qtTranslator;
    QTranslator appTranslator;
    QList<QStringList> pickHistory;
    bool m_globalTrackingEnabled = false;
    bool m_parallelPickEnabled = true;
    bool m_isClientMode = false;
    bool use_E2EE = false;
    QPoint m_dragPosition;
    QSystemTrayIcon *m_trayIcon;

    enum class WebSocketRequestType {
        FetchAvailableLists,
        GetRandomNames,
        E2EEStatus
    };

    void setupClientUI();
    void loadNameLists();
    void setupConnections();
    void updateFontSize();
    void setupNameListCombo();
    void onNameListComboActivated(int index);
    void onImportTempList();
    void handleWebSocketRequest(WebSocketRequestType requestType, const QString& listNameAndPickCount = QString());
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
};

#endif // MAINWINDOW_H
