#ifndef VISUALEDITOR_H
#define VISUALEDITOR_H

#include <settingshandler.h>
#include <QDialog>
#include <QTreeWidget>
#include <QSettings>
#include <QPushButton>
#include <QVector>

class VisualEditor : public QDialog
{
    Q_OBJECT

public:
    explicit VisualEditor(QWidget *parent = nullptr);
    ~VisualEditor();

private slots:
    void onItemClicked(QTreeWidgetItem *item, int column);
    void onItemDoubleClicked(QTreeWidgetItem *item, int column);
    void onSaveClicked();
    void onCancelClicked();

private:
    void setupUI();
    void loadSettings();
    void initializeConfigItems();
    void closeEditors();
    QWidget* createEditorWidget(const SettingsHandler::ConfigItem &item, const QVariant &value);

    QTreeWidget *treeWidget;
    QPushButton *saveButton;
    QPushButton *cancelButton;
    QString configPath;
    QVector<SettingsHandler::ConfigItem> configItems;
};

#endif // VISUALEDITOR_H
