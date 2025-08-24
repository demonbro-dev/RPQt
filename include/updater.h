#ifndef UPDATER_H
#define UPDATER_H

#include <QDialog>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>

namespace Ui {
class Updater;
}

class Updater : public QDialog
{
    Q_OBJECT

public:
    explicit Updater(QWidget *parent = nullptr);
    ~Updater();

    void checkForUpdates();

private slots:
    void onCheckFinished();
    void onCheckTimeout();
    void onCloseButtonClicked();
    void onUpdateButtonClicked();

private:
    Ui::Updater *ui;
    QNetworkAccessManager *networkManager;
    QNetworkReply *currentReply;
    QTimer *timeoutTimer;
    QString latestVersion;
    QString downloadUrl;

    void parseGitHubReleases(const QByteArray &data);
    bool isNewerVersion(const QString &current, const QString &latest);
    void showUpdateAvailable(const QString &version);
    void showNoUpdateAvailable();
    void showError(const QString &errorMessage);
};

#endif // UPDATER_H
