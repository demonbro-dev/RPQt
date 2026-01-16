#include "namemanager.h"
#include "passphrasedialog.h"
#include "ui_namemanager.h"
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QMenuBar>
#include <QMenu>
#include <QInputDialog>
#include <QLibraryInfo>

NameManager::NameManager(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::NameManager)
{
    ui->setupUi(this);

    connect(ui->actionUnlock, &QAction::triggered, this, &NameManager::unlockApplication);
    connect(ui->actionSave, &QAction::triggered, this, &NameManager::saveChanges);
    connect(ui->actionReload, &QAction::triggered, this, &NameManager::reloadNameLists);
    connect(ui->actionChangePassphrase, &QAction::triggered, this, &NameManager::changePassphrase);
    connect(ui->actionImport, &QAction::triggered, this, &NameManager::importFromTxt);

    resize(800, 600);
    setWindowFlags(windowFlags() &~ Qt::WindowMinimizeButtonHint);
    setupUI();
}

NameManager::~NameManager()
{
    delete ui;
    disconnect();
}

void NameManager::setupUI()
{
    setWindowTitle(isUnlocked ? tr("RandPicker Namelist Manager") : tr("RandPicker Namelist Manager [Locked]"));

    ui->menuUnlock->menuAction()->setVisible(!isUnlocked);
    ui->menuFile->menuAction()->setVisible(isUnlocked);
    ui->menuEdit->menuAction()->setVisible(isUnlocked);

    ui->listWidget->setEnabled(isUnlocked);
    ui->memberListWidget->setEnabled(isUnlocked);

    if (isUnlocked) {
        // 左侧名单
        ui->listWidget->setSelectionMode(QAbstractItemView::SingleSelection);
        ui->listWidget->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);

        // 右侧成员
        ui->memberListWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
        ui->memberListWidget->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);

        connect(ui->listWidget, &QListWidget::currentRowChanged, this, &NameManager::onListSelected);
        connect(ui->listWidget, &QListWidget::itemChanged, this, &NameManager::onListItemChanged);
        connect(ui->memberListWidget, &QListWidget::itemChanged, this, &NameManager::onMemberItemChanged);

        connect(ui->actionAddList, &QAction::triggered, this, &NameManager::addNameGroup);
        connect(ui->actionRemoveList, &QAction::triggered, this, &NameManager::removeNameGroup);
        connect(ui->actionAddMember, &QAction::triggered, this, &NameManager::addMember);
        connect(ui->actionRemoveMember, &QAction::triggered, this, &NameManager::removeMember);

        setupContextMenus();
        loadNameLists();
    }
}

void NameManager::unlockApplication()
{
    currentFilePath = NAMELIST_PATH_BINARY;
    PassphraseDialog dialog(currentFilePath, PassphraseDialog::InputMode, this);
    if (dialog.exec() == QDialog::Accepted) {
        isUnlocked = true;
        setupUI();
    }
}

void NameManager::loadNameLists()
{
    QString error;

    nameGroups = fbsHandler.loadFromFile(currentFilePath, error);

    if (!error.isEmpty()) {
        QMessageBox::warning(this, tr("Error"), error);
        return;
    }

    // 清空列表
    ui->listWidget->clear();

    // 填充名单组列表
    for (auto it = nameGroups.cbegin(); it != nameGroups.cend(); ++it) {
        QListWidgetItem *item = new QListWidgetItem(it.key());
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        ui->listWidget->addItem(item);
    }

    if (!nameGroups.isEmpty()) {
        ui->listWidget->setCurrentRow(0);
    }
}

void NameManager::reloadNameLists()
{
    // 保存当前选中的列表
    QString currentSelection;
    if (ui->listWidget->currentItem()) {
        currentSelection = ui->listWidget->currentItem()->text();
    }

    // 重新加载数据
    QString error;
    QMap<QString, QStringList> newNameGroups = fbsHandler.loadFromFile(currentFilePath, error);

    if (!error.isEmpty()) {
        QMessageBox::warning(this, tr("Error"), error);
        return;
    }

    // 更新数据
    nameGroups = newNameGroups;

    // 刷新UI
    ui->listWidget->clear();
    for (auto it = nameGroups.cbegin(); it != nameGroups.cend(); ++it) {
        QListWidgetItem *item = new QListWidgetItem(it.key());
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        ui->listWidget->addItem(item);
    }

    // 尝试恢复之前的选中状态
    if (!currentSelection.isEmpty()) {
        QList<QListWidgetItem*> items = ui->listWidget->findItems(currentSelection, Qt::MatchExactly);
        if (!items.isEmpty()) {
            ui->listWidget->setCurrentItem(items.first());
        } else if (ui->listWidget->count() > 0) {
            ui->listWidget->setCurrentRow(0);
        }
    } else if (ui->listWidget->count() > 0) {
        ui->listWidget->setCurrentRow(0);
    }

    QMessageBox::information(this, tr("Success"), tr("Namelists have been reloaded successfully."));
}

void NameManager::onListSelected(int index)
{
    if (index < 0 || index >= ui->listWidget->count()) {
        ui->memberListWidget->clear();
        return;
    }

    // 获取当前选中的名单组名
    currentSelectedList = ui->listWidget->item(index)->text();

    // 刷新右侧成员列表
    refreshMemberList(QStringList(nameGroups.value(currentSelectedList)));
}

void NameManager::refreshMemberList(const QStringList &members)
{
    ui->memberListWidget->clear();

    // 填充成员列表
    for (const QString &member : members) {
        QListWidgetItem *item = new QListWidgetItem(member);
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        ui->memberListWidget->addItem(item);
    }
}

void NameManager::onListItemChanged(QListWidgetItem *item)
{
    if (!item || currentSelectedList.isEmpty()) return;

    QString newName = item->text();
    if (newName.isEmpty()) {
        item->setText(currentSelectedList);
        return;
    }

    if (nameGroups.contains(newName) && newName != currentSelectedList) {
        QMessageBox::warning(this, tr("Warning"), tr("This list name already exists!"));
        item->setText(currentSelectedList);
        return;
    }

    if (nameGroups.contains(currentSelectedList)) {
        QStringList members = nameGroups.value(currentSelectedList);
        nameGroups.remove(currentSelectedList);
        nameGroups.insert(newName, members);
        currentSelectedList = newName;
    }
}

void NameManager::onMemberItemChanged(QListWidgetItem *item)
{
    if (currentSelectedList.isEmpty()) return;

    int row = ui->memberListWidget->row(item);
    QStringList members;

    for (int i = 0; i < ui->memberListWidget->count(); ++i) {
        members.append(ui->memberListWidget->item(i)->text());
    }

    nameGroups[currentSelectedList] = members;
}

void NameManager::setupContextMenus()
{
    // 左侧名单列表的右键菜单
    ui->listWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->listWidget, &QListWidget::customContextMenuRequested, this, [this](const QPoint &pos) {
        QMenu menu;
        QAction *addAction = menu.addAction(tr("Add List"));
        QAction *removeAction = menu.addAction(tr("Remove List"));
        QAction *renameAction = menu.addAction(tr("Rename List")); // 新增

        connect(addAction, &QAction::triggered, this, &NameManager::addNameGroup);
        connect(removeAction, &QAction::triggered, this, &NameManager::removeNameGroup);
        connect(renameAction, &QAction::triggered, this, [this]() {
            if (ui->listWidget->currentItem()) {
                ui->listWidget->editItem(ui->listWidget->currentItem());
            }
        });

        menu.exec(ui->listWidget->mapToGlobal(pos));
    });

    // 右侧成员列表的右键菜单
    ui->memberListWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->memberListWidget, &QListWidget::customContextMenuRequested, this, [this](const QPoint &pos) {
        QMenu menu;
        QAction *addAction = menu.addAction(tr("Add Member"));
        QAction *removeAction = menu.addAction(tr("Remove Member"));
        QAction *renameAction = menu.addAction(tr("Rename Member")); // 新增

        connect(addAction, &QAction::triggered, this, &NameManager::addMember);
        connect(removeAction, &QAction::triggered, this, &NameManager::removeMember);
        connect(renameAction, &QAction::triggered, this, [this]() {
            if (ui->memberListWidget->currentItem()) {
                ui->memberListWidget->editItem(ui->memberListWidget->currentItem());
            }
        });

        menu.exec(ui->memberListWidget->mapToGlobal(pos));
    });
}

// 添加名单组
void NameManager::addNameGroup()
{
    bool ok;
    QString name = QInputDialog::getText(this, tr("Add List"), tr("Please type the name of new list:"), QLineEdit::Normal, "", &ok);

    if (ok && !name.isEmpty()) {
        if (nameGroups.contains(name)) {
            QMessageBox::warning(this, tr("Warning"), tr("This list exists!"));
            return;
        }

        nameGroups.insert(name, QStringList());
        QListWidgetItem *item = new QListWidgetItem(name);
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        ui->listWidget->addItem(name);
        ui->listWidget->setCurrentRow(ui->listWidget->count() - 1);
    }
}

// 删除名单组
void NameManager::removeNameGroup()
{
    int currentRow = ui->listWidget->currentRow();
    if (currentRow < 0) return;

    QString name = ui->listWidget->currentItem()->text();
    if (QMessageBox::question(this, tr("Confirm Removal"), tr("Are you sure to remove list '%1'?").arg(name)) == QMessageBox::Yes) {
        nameGroups.remove(name);
        delete ui->listWidget->takeItem(currentRow);

        if (ui->listWidget->count() > 0) {
            ui->listWidget->setCurrentRow(qMin(currentRow, ui->listWidget->count() - 1));
        } else {
            ui->memberListWidget->clear();
            currentSelectedList.clear();
        }
    }
}

// 添加成员
void NameManager::addMember()
{
    if (currentSelectedList.isEmpty()) return;

    QListWidgetItem *item = new QListWidgetItem(tr("New Member"));
    item->setFlags(item->flags() | Qt::ItemIsEditable);
    ui->memberListWidget->addItem(item);

    ui->memberListWidget->scrollToItem(item, QAbstractItemView::PositionAtBottom);
    // 触发编辑
    ui->memberListWidget->editItem(item);

    // 更新内存数据
    QStringList members;
    for (int i = 0; i < ui->memberListWidget->count(); ++i) {
        members.append(ui->memberListWidget->item(i)->text());
    }
    nameGroups[currentSelectedList] = members;
}

// 删除成员
void NameManager::removeMember()
{
    if (currentSelectedList.isEmpty()) return;

    QList<QListWidgetItem*> selectedItems = ui->memberListWidget->selectedItems();
    if (selectedItems.isEmpty()) return;

    for (QListWidgetItem *item : selectedItems) {
        delete ui->memberListWidget->takeItem(ui->memberListWidget->row(item));
    }

    // 更新内存数据
    QStringList members;
    for (int i = 0; i < ui->memberListWidget->count(); ++i) {
        members.append(ui->memberListWidget->item(i)->text());
    }
    nameGroups[currentSelectedList] = members;
}

void NameManager::changePassphrase()
{
    if (!isUnlocked || currentFilePath.isEmpty()) return;

    PassphraseDialog dialog(currentFilePath, PassphraseDialog::SetNewMode, this);
    if (dialog.exec() == QDialog::Accepted) {
        QMessageBox::information(this, tr("Success"), tr("Your passphrase has been edited!"));
    }
}

void NameManager::saveChanges()
{
    QString error;

    // 首先保存到主文件
    if (!fbsHandler.saveToFile(nameGroups, currentFilePath, error)) {
        QMessageBox::warning(this, tr("Failed to save."), error);
        return;
    }

    // 然后保存到持久化路径
    if (!fbsHandler.saveToPersistFile(nameGroups, error)) {
        QMessageBox::warning(this, tr("Warning"),
                             tr("Failed to save namelist to persistent path: %1").arg(error));
    }

    QMessageBox::information(this, tr("Success"), tr("The namelist has been saved!"));
}

void NameManager::importFromTxt()
{
    if (!isUnlocked) return;

#ifdef Q_OS_WIN
    QString fileName = QFileDialog::getOpenFileName(this, tr("Import TXT File"), "", tr("Text Files (*.txt)"));
#else
    QString fileName = QFileDialog::getOpenFileName(this, tr("Import TXT File"), QDir::homePath(), tr("Text Files (*.txt)"));
#endif

    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("Error"), tr("Could not open file!"));
        return;
    }

    // 从文件名获取列表名
    QFileInfo fileInfo(fileName);
    QString listName = fileInfo.baseName();

    if (listName.isEmpty()) {
        listName = tr("Imported List");
    }

    // 检查是否已存在同名列表
    if (nameGroups.contains(listName)) {
        if (QMessageBox::question(this, tr("Confirm Overwrite"),
                                  QString(tr("A list named '%1' already exists. Overwrite?")).arg(listName),
                                  QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) {
            return;
        }
    }

    QTextStream in(&file);
    QString content = in.readAll();
    file.close();

    // 分割内容为名字列表（支持空格和换行分隔）
    QStringList nameList = content.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);

    if (nameList.isEmpty()) {
        QMessageBox::warning(this, tr("Error"), tr("No valid names found in the file!"));
        return;
    }

    // 添加到名单组
    nameGroups[listName] = nameList;

    bool found = false;
    for (int i = 0; i < ui->listWidget->count(); ++i) {
        if (ui->listWidget->item(i)->text() == listName) {
            ui->listWidget->setCurrentRow(i);
            found = true;
            break;
        }
    }

    if (!found) {
        ui->listWidget->addItem(listName);
        ui->listWidget->setCurrentRow(ui->listWidget->count() - 1);
    }

    refreshMemberList(nameList);
    QMessageBox::information(this, tr("Success"),
                             QString(tr("Imported %1 names to list '%2'")).arg(nameList.size()).arg(listName));
}

bool NameManager::loadTranslation(const QLocale &locale)
{
    QApplication::removeTranslator(&qtTranslator);
    QApplication::removeTranslator(&appTranslator);

    bool qtLoaded = qtTranslator.load(locale, "qt", "_", QLibraryInfo::path(QLibraryInfo::TranslationsPath));
    if (qtLoaded) {
        QApplication::installTranslator(&qtTranslator);
    }

    QString translationFile = QString("RPQt_%1").arg(locale.name());
    bool appLoaded = appTranslator.load(translationFile, ":/translations") ||
                     appTranslator.load(translationFile, QCoreApplication::applicationDirPath() + "/translations");

    if (!appLoaded && locale.name() != "en_US") {
        QLocale fallbackLocale(QLocale::English, QLocale::UnitedStates);
        translationFile = QString("RPQt_%1").arg(fallbackLocale.name());
        appLoaded = appTranslator.load(translationFile, ":/translations") ||
                    appTranslator.load(translationFile, QCoreApplication::applicationDirPath() + "/translations");
    }

    if (appLoaded) {
        QApplication::installTranslator(&appTranslator);
    }

    ui->retranslateUi(this);
    setWindowTitle(isUnlocked ? tr("RandPicker Namelist Manager") : tr("RandPicker Namelist Manager [Locked]"));
    return appLoaded;
}

void NameManager::onLanguageChanged(const QLocale &locale)
{
    loadTranslation(locale);
}
