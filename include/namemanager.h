#ifndef NAMEMANAGER_H
#define NAMEMANAGER_H

#include <QMainWindow>
#include <QListWidget>
#include <QTranslator>
#include "fbshandler.h"

namespace Ui {
class NameManager;
}

class NameManager : public QMainWindow
{
    Q_OBJECT

public:
    explicit NameManager(QWidget *parent = nullptr);
    ~NameManager();

private slots:
    void onListSelected(int index);
    void onListItemChanged(QListWidgetItem *item);
    void onMemberItemChanged(QListWidgetItem *item);
    void saveChanges();
    void unlockApplication();
    void changePassphrase();

public slots:
    void onLanguageChanged(const QLocale &locale);
    bool loadTranslation(const QLocale &locale);
    void reloadNameLists();

private:
    Ui::NameManager *ui;
    FbsHandler fbsHandler;
    QMap<QString, QStringList> nameGroups;
    QString currentFilePath;
    QString currentSelectedList;
    bool isUnlocked = false;
    QTranslator qtTranslator;
    QTranslator appTranslator;

    void loadNameLists();
    void setupUI();
    void refreshMemberList(const QStringList &members);
    void setupContextMenus();
    void addNameGroup();
    void removeNameGroup();
    void addMember();
    void removeMember();
    void importFromTxt();

signals:
    void languageChanged(const QLocale &locale);
};

#endif // NAMEMANAGER_H
