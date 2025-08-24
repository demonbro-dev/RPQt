#include "updater.h"
#include "ui_updater.h"
#include <QApplication>
#include <QMessageBox>
#include <QDesktopServices>
#include <QVersionNumber>
#include <QDebug>
#include <QPushButton>
#include <QJsonDocument>
#include <QJsonObject>

Updater::Updater(QWidget *parent)
    : QDialog(parent),
    ui(new Ui::Updater),
    networkManager(nullptr),
    currentReply(nullptr)
{
    ui->setupUi(this);

    setWindowTitle(tr("Software Update"));
    setModal(true);

    connect(ui->closeButton, &QPushButton::clicked, this, &Updater::onCloseButtonClicked);
    connect(ui->updateButton, &QPushButton::clicked, this, &Updater::onUpdateButtonClicked);

    ui->updateButton->setEnabled(false);

    networkManager = new QNetworkAccessManager(this);
    timeoutTimer = new QTimer(this);
    timeoutTimer->setSingleShot(true);
    connect(timeoutTimer, &QTimer::timeout, this, &Updater::onCheckTimeout);
}

Updater::~Updater()
{
    delete ui;
}

void Updater::checkForUpdates()
{
    ui->statusLabel->setText(tr("Checking for updates..."));
    ui->progressBar->setRange(0, 0);
    ui->updateButton->setEnabled(false);
    ui->closeButton->setEnabled(false);

    QUrl url("https://api.github.com/repos/demonbro-dev/RPQt/releases/latest");

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, "RandPickerUpdater/1.0");

    currentReply = networkManager->get(request);
    connect(currentReply, &QNetworkReply::finished, this, &Updater::onCheckFinished);

    timeoutTimer->start(30000);
}

void Updater::onCheckFinished()
{
    timeoutTimer->stop();
    ui->closeButton->setEnabled(true);

    if (currentReply->error() != QNetworkReply::NoError) {
        showError(tr("Network error: %1").arg(currentReply->errorString()));
        return;
    }

    QByteArray data = currentReply->readAll();
    currentReply->deleteLater();
    currentReply = nullptr;

    parseGitHubReleases(data);
}

void Updater::parseGitHubReleases(const QByteArray &data)
{
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        showError(tr("Failed to parse update information"));
        return;
    }

    QJsonObject json = doc.object();

    if (!json.contains("tag_name") || !json.contains("html_url")) {
        showError(tr("Invalid response format from GitHub"));
        return;
    }

    latestVersion = json["tag_name"].toString();
    downloadUrl = json["html_url"].toString();

    // Remove 'v' prefix if present for version comparison
    QString cleanLatestVersion = latestVersion.startsWith('v')
                                     ? latestVersion.mid(1)
                                     : latestVersion;

    // Get current version
    QString currentVersion = QApplication::applicationVersion();
    QString cleanCurrentVersion = currentVersion.startsWith('v')
                                      ? currentVersion.mid(1)
                                      : currentVersion;

    if (cleanLatestVersion.isEmpty()) {
        showError(tr("Invalid version information received"));
        return;
    }

    if (isNewerVersion(cleanCurrentVersion, cleanLatestVersion)) {
        showUpdateAvailable(latestVersion);
    } else {
        showNoUpdateAvailable();
    }
}

bool Updater::isNewerVersion(const QString &current, const QString &latest)
{
    try {
        QVersionNumber currentVersion = QVersionNumber::fromString(current);
        QVersionNumber latestVersion = QVersionNumber::fromString(latest);
        return latestVersion > currentVersion;
    } catch (...) {
        // Fallback string comparison if version parsing fails
        return latest > current;
    }
}

void Updater::showUpdateAvailable(const QString &version)
{
    ui->statusLabel->setText(tr("Update available: %1\n\nClick the button below to download the latest version.").arg(version));
    ui->progressBar->setRange(0, 100);
    ui->progressBar->setValue(100);
    ui->updateButton->setEnabled(true);
}

void Updater::showNoUpdateAvailable()
{
    ui->statusLabel->setText(tr("No updates available.\n\nYou are using the latest version."));
    ui->progressBar->setRange(0, 100);
    ui->progressBar->setValue(100);
    ui->updateButton->setEnabled(false);
}

void Updater::showError(const QString &errorMessage)
{
    ui->statusLabel->setText(tr("Update check failed:\n%1\n\nPlease check your internet connection and try again.").arg(errorMessage));
    ui->progressBar->setRange(0, 100);
    ui->progressBar->setValue(0);
    ui->updateButton->setEnabled(false);
}

void Updater::onCheckTimeout()
{
    if (currentReply) {
        currentReply->abort();
        showError(tr("Request timeout"));
        ui->closeButton->setEnabled(true);
    }
}

void Updater::onCloseButtonClicked()
{
    reject();
}

void Updater::onUpdateButtonClicked()
{
    QDesktopServices::openUrl(QUrl(downloadUrl));
    accept();
}
