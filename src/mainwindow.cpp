#include "mainwindow.h"
#include "rpweb.h"
#include "scheduledpickdialog.h"
#include "ui_mainwindow.h"
#include "aboutdialog.h"
#include <QLibraryInfo>
#include <QMessageBox>
#include <QResizeEvent>
#include <QFile>
#include <QFileInfo>
#include <QFileDialog>
#include <QDir>
#include <QGesture>
#include <QtConcurrent>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::MainWindow),
    nameManager(nullptr),
    pickerLogic(new PickerLogic(this)),
    sideButton(nullptr),
    historyDialog(nullptr),
    m_globalTrackingEnabled(false),
    m_parallelPickEnabled(true),
    m_trayIcon(nullptr)
{
    ui->setupUi(this);

    gestureHintLabel = new QLabel(this);
    gestureHintLabel->setAlignment(Qt::AlignCenter);
    gestureHintLabel->setStyleSheet(
        "QLabel {"
        "  background-color: rgba(50, 50, 50, 150);"
        "  color: white;"
        "  font-size: 24px;"
        "  border-radius: 10px;"
        "  padding: 20px;"
        "}"
        );
    gestureHintLabel->hide();
    gestureHintLabel->resize(200, 100);

    if (!loadTranslation(QLocale(QLocale::system()))) {
        loadTranslation(QLocale(QLocale::English, QLocale::UnitedStates));
    }

    loadNameLists();
    setupConnections();
    grabGesture(Qt::PanGesture);
    grabGesture(Qt::SwipeGesture);
}

MainWindow::~MainWindow()
{
    if(pickerLogic && pickerLogic->isRunning()) {
        pickerLogic->stopPicking();
        pickerLogic = nullptr;
    }

    if(nameManager) {
        nameManager->deleteLater();
        nameManager = nullptr;
    }

    if (sideButton) {
        sideButton->deleteLater();
    }

    delete ui;
}

void MainWindow::setupConnections()
{
    connect(ui->actionNameManager, &QAction::triggered, this, [this]() {
        if (!nameManager) {
            nameManager = new NameManager(this);
            connect(nameManager, &NameManager::destroyed, this, [this]() {
                nameManager = nullptr;
                loadNameLists();
            });
        }
        nameManager->show();
        nameManager->raise();
        nameManager->activateWindow();
    });
    connect(ui->actionSidebar, &QAction::triggered, this, [this]() {
        showSideButton(true);
    });
    connect(ui->actionSidebarToLeft, &QAction::triggered, this, [this]() {
        showSideButton(false);
    });
    connect(ui->actionHidetoTray, &QAction::triggered, this, [this]() {
        if (!m_trayIcon) {
            m_trayIcon = new QSystemTrayIcon(this);
            m_trayIcon->setIcon(QIcon(":/data/RandPickerLogo.ico"));
            m_trayIcon->setToolTip("RandPicker");

            QMenu *trayMenu = new QMenu(this);
            trayMenu->addAction(tr("Show"), this, [this]() {
                showMainWindow();
            });
            trayMenu->addSeparator();
            trayMenu->addAction(tr("Exit"), qApp, &QApplication::quit);

            m_trayIcon->setContextMenu(trayMenu);
            connect(m_trayIcon, &QSystemTrayIcon::activated, this, [this](QSystemTrayIcon::ActivationReason reason) {
                if (reason == QSystemTrayIcon::Trigger) {
                    showMainWindow();
                }
            });
        }
        m_trayIcon->show();
        hide();
    });
    connect(ui->actionRPWeb, &QAction::triggered, this, [this]() {
        RPWeb *rpwDialog = new RPWeb(this);
        rpwDialog->setAttribute(Qt::WA_DeleteOnClose);
        rpwDialog->show();
    });
    connect(ui->actionGlobalTracking, &QAction::toggled, this, [this](bool checked) {
        m_globalTrackingEnabled = checked;
    });
    connect(ui->actionDisableParallelPick, &QAction::toggled, this, [this](bool checked) {
        m_parallelPickEnabled = !checked;
    });
    connect(ui->actionAboutQt, &QAction::triggered, this, [this]() {
        QMessageBox::aboutQt(this, tr("About Qt"));
    });
    connect(ui->actionAboutApp, &QAction::triggered, this, [this]() {
        AboutDialog *about = new AboutDialog(this);
        about->setAttribute(Qt::WA_DeleteOnClose);
        about->show();
    });
    connect(ui->actionFollowSystem, &QAction::triggered, this, [this]() {
        QLocale locale = QLocale::system();
        if (!loadTranslation(locale)) {
            locale = QLocale(QLocale::English, QLocale::UnitedStates);
        }
        loadTranslation(locale);
        if (nameManager) nameManager->onLanguageChanged(locale);
    });
    connect(ui->actionenUS, &QAction::triggered, this, [this]() {
        QLocale locale(QLocale::English, QLocale::UnitedStates);
        loadTranslation(locale);
        if (nameManager) nameManager->onLanguageChanged(locale);
    });
    connect(ui->actionzhCN, &QAction::triggered, this, [this]() {
        QLocale locale(QLocale::Chinese, QLocale::China);
        loadTranslation(locale);
        if (nameManager) nameManager->onLanguageChanged(locale);
    });
    connect(ui->actionScheduledPick, &QAction::triggered, this, [this]() {
        ScheduledPickDialog *dialog = new ScheduledPickDialog(this);
        connect(dialog, &ScheduledPickDialog::timeElapsed, this, [this]() {
            QStringList picked = pickerLogic->pickNames(ui->countSpin->value(), m_parallelPickEnabled);
            ui->nameLabel->setText(pickerLogic->formatNamesWithLineBreak(picked));
            pickHistory.append(picked);
            if (historyDialog) historyDialog->updateHistory(pickHistory);
        });
        dialog->setAttribute(Qt::WA_DeleteOnClose);
        dialog->show();
    });
    connect(ui->actionHistory, &QAction::triggered, this, [this]() {
        if (!historyDialog) {
            historyDialog = new PickHistoryDialog(pickHistory, this);
            historyDialog->setAttribute(Qt::WA_DeleteOnClose);
            connect(historyDialog, &PickHistoryDialog::destroyed, this, [this]() {
                historyDialog = nullptr;
            });
            connect(historyDialog, &PickHistoryDialog::ClearHistory, this, [this]() {
                pickHistory.clear();
            });
        }
        historyDialog->show();
        historyDialog->raise();
        historyDialog->activateWindow();
    });
    connect(ui->actionImportTemp, &QAction::triggered, this, &MainWindow::onImportTempList);
    connect(ui->actionClearPicked, &QAction::triggered, this, [this]() {
        pickerLogic->resetPickedNames();
        ui->nameLabel->clear();
        QMessageBox::information(this, tr("Picked Names Cleared"),
                                 tr("The list of picked names has been reset.\nAll names are now available for picking again."));
    });
    connect(ui->pickButton, &QPushButton::clicked, this, &MainWindow::onPickButtonClicked);
    connect(pickerLogic, &PickerLogic::namesPicked, this, &MainWindow::onNamesPicked);
    connect(pickerLogic, &PickerLogic::previewNames, this, &MainWindow::onPreviewNames);
    connect(ui->nameListCombo, &QComboBox::currentTextChanged,
            this, [this](const QString &groupName){
                currentNames = nameGroups.value(groupName);
                pickerLogic->setNames(currentNames);
            });
#ifdef Q_OS_WIN
    connect(ui->topmostRadio, &QCheckBox::toggled,
            this, &MainWindow::onTopmostToggled);
#else
    ui->topmostRadio->setCheckable(false);
    ui->topmostRadio->setToolTip(tr("Topmost is not available on your system."));
#endif
}

void MainWindow::loadNameLists()
{
    QFuture<void> future = QtConcurrent::run([this]() {
        QString error;
        QString filePath = NAMELIST_PATH;

        QFile file(filePath);
        if (!file.exists()) {
            QMetaObject::invokeMethod(this, [this]() {
                QMessageBox::information(this, tr("Info"), QString(tr("Namelist not found. Default namelist will be created.")));
            }, Qt::BlockingQueuedConnection);

            // 使用JsonHandler创建默认文件
            bool createDefaultNamelist = jsonHandler.createDefaultNamelist(filePath, error);
            if (!createDefaultNamelist) {
                QMetaObject::invokeMethod(this, [this, error]() {
                    QMessageBox::warning(this, tr("Error"), QString(tr("Failed to create default namelist file: %1")).arg(error));
                }, Qt::BlockingQueuedConnection);
                nameGroups["Default1"] = {"12", "34", "56"};
                return;
            }
        }

        auto loadedGroups = jsonHandler.loadFromFile(filePath, error);

        QMetaObject::invokeMethod(this, [this, loadedGroups, error]() {
            if (!error.isEmpty()) {
                QMessageBox::warning(this, tr("Failed to load"), error);
                nameGroups["Default1"] = {"12", "34", "56"};
            } else {
                nameGroups = loadedGroups;
            }

            if (!nameGroups.isEmpty()) {
                currentNames = nameGroups.first();
                pickerLogic->setNames(currentNames);
                ui->nameListCombo->addItems(nameGroups.keys());
            }
        }, Qt::BlockingQueuedConnection);
    });
}

void MainWindow::onPickButtonClicked()
{
    if (ui->instantModeRadio->isChecked()) {
        // 立即抽选模式
        QStringList picked = pickerLogic->pickNames(ui->countSpin->value(), m_parallelPickEnabled);
        ui->nameLabel->setText(pickerLogic->formatNamesWithLineBreak(picked));
        pickHistory.append(picked);
        if (historyDialog) {
            historyDialog->updateHistory(pickHistory);
        }
    } else {
        // 动态抽选模式
        if (!pickerLogic->isRunning()) {
            pickerLogic->setPickCount(ui->countSpin->value());
            pickerLogic->startPicking(m_parallelPickEnabled);
            ui->pickButton->setText(tr("Stop"));
            ui->instantModeRadio->setCheckable(false);
            ui->nameListCombo->setEnabled(false);
            ui->countSpin->setEnabled(false);
            ui->actionClearPicked->setEnabled(false);
        } else {
            pickerLogic->stopPicking();
            ui->pickButton->setText(tr("Start"));
            ui->instantModeRadio->setCheckable(true);
            ui->nameListCombo->setEnabled(true);
            ui->countSpin->setEnabled(true);
            ui->actionClearPicked->setEnabled(true);
        }
    }
}


void MainWindow::onNameListComboActivated(int index)
{
    QString groupName = ui->nameListCombo->itemText(index);
    if (nameGroups.contains(groupName)) {
        currentNames = nameGroups.value(groupName);
        pickerLogic->setNames(currentNames);
        pickerLogic->resetPickedNames();
        ui->nameLabel->clear();
    }
}

void MainWindow::onTopmostToggled(bool checked)
{
    QPoint pos = this->pos();
    QSize size = this->size();

    Qt::WindowFlags flags = windowFlags();
    flags.setFlag(Qt::WindowStaysOnTopHint, checked);

    hide();
    setWindowFlags(flags);
    show();

    move(pos);
    resize(size);
    activateWindow();
}

bool MainWindow::loadTranslation(const QLocale& locale)
{
    QApplication::removeTranslator(&qtTranslator);
    QApplication::removeTranslator(&appTranslator);

    bool qtLoaded = qtTranslator.load(locale, "qt", "_",
                                      QLibraryInfo::path(QLibraryInfo::TranslationsPath));
    if (qtLoaded) {
        QApplication::installTranslator(&qtTranslator);
    }

    QString translationFile = QString("RPQt_%1").arg(locale.name());
    bool appLoaded = appTranslator.load(translationFile, ":/translations") ||
                     appTranslator.load(translationFile,
                                        QCoreApplication::applicationDirPath() + "/translations");

    // 如果没有找到翻译，回退到en-US
    if (!appLoaded && locale.name() != "en_US") {
        QLocale fallbackLocale(QLocale::English, QLocale::UnitedStates);
        translationFile = QString("RPQt_%1").arg(fallbackLocale.name());
        appLoaded = appTranslator.load(translationFile, ":/translations") ||
                    appTranslator.load(translationFile,
                                       QCoreApplication::applicationDirPath() + "/translations");
    }

    if (appLoaded) {
        QApplication::installTranslator(&appTranslator);
    }

    ui->retranslateUi(this);
#ifndef Q_OS_WIN
    ui->topmostRadio->setToolTip(tr("Topmost is not available on your system."));
#endif
    return appLoaded;
}

void MainWindow::onPreviewNames(const QStringList &names)
{
    ui->nameLabel->setText(pickerLogic->formatNamesWithLineBreak(names));
}

void MainWindow::onNamesPicked(const QStringList &names)
{
    ui->nameLabel->setText(pickerLogic->formatNamesWithLineBreak(names));
    pickHistory.append(names);

    if (historyDialog) {
        historyDialog->updateHistory(pickHistory);
    }
}

void MainWindow::onImportTempList()
{
#ifdef Q_OS_WIN
    QString dir = QCoreApplication::applicationDirPath();
#else
    QString dir = QDir::homePath();
#endif
    QString fileName = QFileDialog::getOpenFileName(this,tr("Import Temporary Namelist"), dir, tr("Text Files (*.txt)"));

    if (fileName.isEmpty()) {
        return;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("Error"), tr("Could not open file!"));
        return;
    }

    QTextStream in(&file);
    QString content = in.readAll();
    file.close();

    QStringList names = content.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);

    if (names.isEmpty()) {
        QMessageBox::warning(this, tr("Error"), tr("No valid names found in the file."));
        return;
    }

    QFileInfo fileInfo(fileName);
    QString listName = fileInfo.baseName();

    if (nameGroups.contains(listName)) {
        QMessageBox::warning(this, tr("Warning"), tr("This list exists!"));
        return;
    }

    nameGroups.insert(listName, names);
    ui->nameListCombo->addItem(listName);
    ui->nameListCombo->setCurrentText(listName);
    currentNames = names;
    pickerLogic->setNames(currentNames);
    pickerLogic->resetPickedNames();
    ui->nameLabel->clear();

    QMessageBox::information(this,tr("Success"),tr("Imported %1 names as temporary list '%2'").arg(names.size()).arg(listName));
}

void MainWindow::showSideButton(bool toRight)
{
    hide();

    if (!sideButton) {
        sideButton = new SideButton(nullptr, toRight);
        sideButton->setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
        connect(sideButton, &SideButton::clickedToShowMain, this, &MainWindow::showMainWindow);
    }

    QRect screenGeometry = QApplication::primaryScreen()->availableGeometry();

    int x = toRight ? screenGeometry.right() - sideButton->width() : screenGeometry.left();
    int y = screenGeometry.top() + screenGeometry.height() / 4;

    y = qMin(y, screenGeometry.bottom() - sideButton->height());

    sideButton->move(x, y);
    sideButton->show();
}

void MainWindow::showMainWindow()
{
    show();
    if (sideButton) {
        sideButton->deleteLater();
        sideButton = nullptr;
    }
    if (m_trayIcon) {
        m_trayIcon->hide();
        m_trayIcon = nullptr;
    }
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    updateFontSize();
}

void MainWindow::updateFontSize()
{
    const int minSide = qMin(width(), height());
    float ratio = 1.0f / 18;
    const int maxFontSize = 64;

    int baseSize = static_cast<int>(minSide * ratio);
    int fontSize = qBound(16, baseSize, maxFontSize);

    QFont font = ui->nameLabel->font();
    font.setPixelSize(fontSize);
    ui->nameLabel->setFont(font);

    int labelWidth = width() * 4 / 5;
    int labelHeight = height() / 2;
    ui->nameLabel->setFixedSize(qMax(labelWidth, 200), qMax(labelHeight, 80));
}

void MainWindow::showGestureHint(const QString &text) {
    gestureHintLabel->setText(text);
    gestureHintLabel->move(
        (width() - gestureHintLabel->width()) / 2,
        (height() - gestureHintLabel->height()) / 2
        );
    gestureHintLabel->show();
    gestureHintLabel->raise();
}

void MainWindow::hideGestureHint() {
    gestureHintLabel->hide();
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if (m_globalTrackingEnabled && event->button() == Qt::LeftButton) {
        m_dragPosition = event->globalPosition().toPoint() - frameGeometry().topLeft();
        event->accept();
    }
    QMainWindow::mousePressEvent(event);
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (m_globalTrackingEnabled && (event->buttons() & Qt::LeftButton)) {
        move(event->globalPosition().toPoint() - m_dragPosition);
        event->accept();
    }
    QMainWindow::mouseMoveEvent(event);
}

bool MainWindow::event(QEvent *event)
{
    if (event->type() == QEvent::TouchBegin ||
        event->type() == QEvent::TouchUpdate ||
        event->type() == QEvent::TouchEnd)
    {
        QTouchEvent *touchEvent = static_cast<QTouchEvent*>(event);
        if (touchEvent->points().size() == 2) {
            return handleTwoFingerSwipe(touchEvent);
        }
    }
    return QMainWindow::event(event);
}

bool MainWindow::handleTwoFingerSwipe(QTouchEvent *touchEvent) {
    static QPointF startPos1, startPos2;
    static bool isTracking = false;

    const auto &points = touchEvent->points();
    if (points.size() < 2) return false;

    QPointF currentPos1 = points[0].position();
    QPointF currentPos2 = points[1].position();

    switch (touchEvent->type()) {
    case QEvent::TouchBegin:
        startPos1 = currentPos1;
        startPos2 = currentPos2;
        isTracking = true;
        return true;

    case QEvent::TouchUpdate:
        if (isTracking) {
            qreal totalDeltaX = (currentPos1.x() - startPos1.x() + currentPos2.x() - startPos2.x()) / 2;
            QString direction = (totalDeltaX > 50) ? "Right" : (totalDeltaX < -50) ? "Left" : "";

            if (!direction.isEmpty()) {
                showGestureHint(direction == "Right" ? "→" : "←");
            }
        }
        return true;

    case QEvent::TouchEnd:
        if (isTracking) {
            qreal totalDeltaX = (currentPos1.x() - startPos1.x() + currentPos2.x() - startPos2.x()) / 2;
            if (totalDeltaX > 50) {
                showSideButton(true);
            } else if (totalDeltaX < -50) {
                showSideButton(false);
            }
            hideGestureHint();
        }
        isTracking = false;
        return true;

    default:
        return false;
    }
}
