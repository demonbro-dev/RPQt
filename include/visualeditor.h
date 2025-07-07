#ifndef VISUALEDITOR_H
#define VISUALEDITOR_H

#include <QDialog>
#include <QTreeWidget>
#include <QSettings>
#include <QPushButton>
#include <QVector>

class VisualEditor : public QDialog
{
    Q_OBJECT

public:
    struct ConfigItem {
        QString section;
        QString key;
        QString name;
        QVariant defaultValue;
        QString type;
    };

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
    QWidget* createEditorWidget(const ConfigItem &item, const QVariant &value);

    QTreeWidget *treeWidget;
    QPushButton *saveButton;
    QPushButton *cancelButton;
    QString configPath;
    QVector<ConfigItem> configItems;
};

#endif // VISUALEDITOR_H
