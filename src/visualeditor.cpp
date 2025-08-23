#include "visualeditor.h"
#include "settingshandler.h"
#include <QCoreApplication>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QSpinBox>
#include <QComboBox>
#include <QFile>

VisualEditor::VisualEditor(QWidget *parent)
    : QDialog(parent), configPath(CONFIG_PATH)
{
    setWindowTitle(tr("RP Visualized Configurator"));
    resize(600, 400);
    setMinimumSize(450, 300);
    setModal(true);

    initializeConfigItems();
    setupUI();
    loadSettings();
}

VisualEditor::~VisualEditor()
{
}

void VisualEditor::initializeConfigItems()
{
    configItems = {
        //{section, key, friendlyName, defaultValue, type},
        {"Window", "OpenRandMirageWhenClose", tr("Open RandMirage When Closing Program"), false, "bool"},
        {"Window", "UseLightTheme", tr("Use Light Theme"), false, "bool"},
        {"InPick", "InstantPickByDefault", tr("Enable Instant Pick by default"), false, "bool"},
        {"InPick", "TopmostByDefault", tr("Enable Topmost by default"), false, "bool"},
        {"RPWeb", "RunAsClient", tr("Run as Client"), false, "bool"},
        {"RPWeb", "Server", tr("Server Host"), "localhost", "string"},
        {"RPWeb", "Port", tr("Server Port"), "8080", "string"}
    };
}

void VisualEditor::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    treeWidget = new QTreeWidget(this);
    treeWidget->setColumnCount(2);
    treeWidget->setHeaderLabels(QStringList() << tr("Properties") << tr("Value"));
    treeWidget->header()->setSectionResizeMode(QHeaderView::Stretch);
    treeWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    saveButton = new QPushButton(tr("Save"), this);
    cancelButton = new QPushButton(tr("Cancel"), this);

    buttonLayout->addStretch();
    buttonLayout->addWidget(saveButton);
    buttonLayout->addWidget(cancelButton);

    mainLayout->addWidget(treeWidget);
    mainLayout->addLayout(buttonLayout);

    connect(treeWidget, &QTreeWidget::itemDoubleClicked, this, &VisualEditor::onItemDoubleClicked);
    connect(treeWidget, &QTreeWidget::itemClicked, this, &VisualEditor::onItemClicked);
    connect(saveButton, &QPushButton::clicked, this, &VisualEditor::onSaveClicked);
    connect(cancelButton, &QPushButton::clicked, this, &VisualEditor::onCancelClicked);
}

void VisualEditor::loadSettings()
{
    treeWidget->clear();
    QSettings settings(configPath, QSettings::IniFormat);

    QMap<QString, QTreeWidgetItem*> sectionItems;

    for (const auto &item : configItems) {
        if (!sectionItems.contains(item.section)) {
            QTreeWidgetItem *sectionItem = new QTreeWidgetItem(treeWidget);
            sectionItem->setText(0, QString("[%1]").arg(item.section));
            sectionItems.insert(item.section, sectionItem);
            treeWidget->addTopLevelItem(sectionItem);
        }

        QTreeWidgetItem *configItem = new QTreeWidgetItem(sectionItems[item.section]);
        configItem->setText(0, item.name);

        QVariant value = settings.value(item.section + "/" + item.key, item.defaultValue);

        configItem->setData(0, Qt::UserRole, item.key);
        configItem->setData(0, Qt::UserRole+1, item.section);
        configItem->setData(0, Qt::UserRole+2, item.type);

        if (item.type == "bool") {
            QCheckBox *checkbox = new QCheckBox();
            checkbox->setChecked(value.toBool());
            treeWidget->setItemWidget(configItem, 1, checkbox);
        } else {
            configItem->setText(1, value.toString());
        }
    }

    treeWidget->expandAll();
}

QWidget* VisualEditor::createEditorWidget(const ConfigItem &item, const QVariant &value)
{
    if (item.type == "bool") {
        QCheckBox *editor = new QCheckBox();
        editor->setChecked(value.toBool());
        return editor;
    }
    else if (item.type == "int") {
        QSpinBox *editor = new QSpinBox();
        editor->setValue(value.toInt());
        return editor;
    }
    else {
        QLineEdit *editor = new QLineEdit();
        editor->setText(value.toString());
        return editor;
    }
}

void VisualEditor::onItemDoubleClicked(QTreeWidgetItem *item, int column)
{
    if (column == 1 && item->parent() != nullptr) {
        QString type = item->data(0, Qt::UserRole+2).toString();

        if (type == "bool") return;

        QString section = item->data(0, Qt::UserRole+1).toString();
        QString key = item->data(0, Qt::UserRole).toString();

        auto it = std::find_if(configItems.begin(), configItems.end(),
                               [&](const ConfigItem &ci) { return ci.section == section && ci.key == key; });

        if (it != configItems.end()) {
            QWidget *editor = createEditorWidget(*it, item->text(1));
            treeWidget->setItemWidget(item, 1, editor);
            editor->setFocus();
        }
    }
}

void VisualEditor::onItemClicked(QTreeWidgetItem *item, int column)
{
    if (item == nullptr || column != 1 || item->parent() == nullptr) {
        closeEditors();
    }
}

void VisualEditor::closeEditors()
{
    for (int i = 0; i < treeWidget->topLevelItemCount(); ++i) {
        QTreeWidgetItem *sectionItem = treeWidget->topLevelItem(i);
        for (int j = 0; j < sectionItem->childCount(); ++j) {
            QTreeWidgetItem *child = sectionItem->child(j);
            QString type = child->data(0, Qt::UserRole+2).toString();

            if (type != "bool") {
                QWidget *editor = treeWidget->itemWidget(child, 1);
                if (editor) {
                    if (auto lineEdit = qobject_cast<QLineEdit*>(editor)) {
                        child->setText(1, lineEdit->text());
                    }
                    else if (auto spinBox = qobject_cast<QSpinBox*>(editor)) {
                        child->setText(1, QString::number(spinBox->value()));
                    }
                    treeWidget->removeItemWidget(child, 1);
                }
            }
        }
    }
}

void VisualEditor::onSaveClicked()
{
    closeEditors();

    QSettings settings(configPath, QSettings::IniFormat);
    bool saveSuccess = true;

    for (int i = 0; i < treeWidget->topLevelItemCount(); ++i) {
        QTreeWidgetItem *sectionItem = treeWidget->topLevelItem(i);
        QString section = sectionItem->text(0).mid(1, sectionItem->text(0).length() - 2);

        for (int j = 0; j < sectionItem->childCount(); ++j) {
            QTreeWidgetItem *child = sectionItem->child(j);
            QString key = child->data(0, Qt::UserRole).toString();
            QString type = child->data(0, Qt::UserRole+2).toString();
            QVariant value;

            if (type == "bool") {
                QCheckBox *checkbox = qobject_cast<QCheckBox*>(treeWidget->itemWidget(child, 1));
                if (checkbox) {
                    value = checkbox->isChecked();
                }
            } else {
                value = child->text(1);
            }

            settings.setValue(section + "/" + key, value);

            if (settings.status() != QSettings::NoError) {
                saveSuccess = false;
            }
        }
    }

    settings.sync();

    if (settings.status() != QSettings::NoError) {
        saveSuccess = false;
    }

    if (saveSuccess) {
        QMessageBox::information(this, tr("Success"),
                                 tr("Settings saved successfully. Please restart the application for changes to take effect."));
        accept();
    } else {
        QMessageBox::critical(this, tr("Error"),
                              tr("Failed to save settings. Please check file permissions and try again."));
    }
}

void VisualEditor::onCancelClicked()
{
    closeEditors();
    reject();
}

