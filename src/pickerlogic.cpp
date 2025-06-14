#include "pickerlogic.h"
#include <QRandomGenerator>
#include <QtConcurrent>

PickerLogic::PickerLogic(QObject *parent) : QObject(parent), timer(new QTimer(this))
{
    connect(timer, &QTimer::timeout, this, &PickerLogic::updateSelection);
}


void PickerLogic::setNames(const QStringList &names)
{
    currentNames = names;
    resetPickedNames();
}

QStringList PickerLogic::pickNames(int count, bool parallelPick)
{
    QStringList result;

    if(availableNames.isEmpty()) {
        // 如果所有名字都已被抽选，则重置
        resetPickedNames();
    }

    // 从可用名单中随机选取
    QStringList shuffled = availableNames;
    if (parallelPick) {
        shuffleNamesParallel(shuffled);
    } else {
        shuffleNames(shuffled);
    }

    // 取前count个名字
    int takeCount = qMin(count, shuffled.size());
    for(int i = 0; i < takeCount; ++i) {
        result << shuffled.at(i);
        pickedNames << shuffled.at(i);
    }

    refreshAvailableNames();

    return result;
}

void PickerLogic::refreshAvailableNames()
{
    availableNames.clear();
    for (const QString &name : currentNames) {
        if (!pickedNames.contains(name)) {
            availableNames << name;
        }
    }
}

QString PickerLogic::formatNamesWithLineBreak(const QStringList &names)
{
    if (names.isEmpty()) return "";

    QString result;
    const int maxPerLine = 4;

    for (int i = 0; i < names.size(); ++i) {
        if (i > 0) {
            if (i % maxPerLine == 0) {
                result += "\n";
            } else {
                result += ",  ";
            }
        }
        result += names.at(i);
    }

    return result;
}

bool PickerLogic::isRunning() const
{
    return m_isRunning;
}

void PickerLogic::startPicking(bool parallelPick)
{
    if (!m_isRunning) {
        refreshAvailableNames();
        m_parallelPick = parallelPick;
        timer->start(30);
        m_isRunning = true;
    }
}

void PickerLogic::stopPicking()
{
    if (m_isRunning) {
        timer->stop();
        m_isRunning = false;

        QStringList finalResult = pickNames(m_pickCount, m_parallelPick);
        emit namesPicked(finalResult);
    }
}

void PickerLogic::setPickCount(int count)
{
    m_pickCount = count;
}

void PickerLogic::resetPickedNames()
{
    pickedNames.clear();
    refreshAvailableNames();
}

void PickerLogic::updateSelection()
{
    // 动态抽选时从全部名字中随机显示
    QStringList shuffled = currentNames;
    if (m_parallelPick) {
        shuffleNamesParallel(shuffled);
    } else {
        shuffleNames(shuffled);
    }

    // 取前m_pickCount个名字用于预览
    QStringList preview;
    int takeCount = qMin(m_pickCount, shuffled.size());
    for(int i = 0; i < takeCount; ++i) {
        preview << shuffled.at(i);
    }

    emit previewNames(preview);
}

void PickerLogic::shuffleNames(QStringList &names)
{
    QList<int> indexes;
    for(int i = 0; i < names.size(); ++i) {
        indexes.append(i);
    }

    for (int i = indexes.size() - 1; i > 0; --i) {
        int j = QRandomGenerator::global()->bounded(i + 1);
        qSwap(indexes[i], indexes[j]);
    }

    QStringList shuffled;
    for (int i = 0; i < indexes.size(); ++i) {
        shuffled << names.at(indexes[i]);
    }
    names = shuffled;
}

void PickerLogic::shuffleNamesParallel(QStringList &names)
{
    int chunkSize = names.size() / QThread::idealThreadCount();
    if (chunkSize < 1) chunkSize = 1;

    QVector<QStringList> chunks;
    for (int i = 0; i < names.size(); i += chunkSize) {
        chunks.append(names.mid(i, chunkSize));
    }

    QtConcurrent::blockingMap(chunks, [this](QStringList &chunk) {
        shuffleNames(chunk);
    });

    names.clear();
    for (const QStringList &chunk : chunks) {
        names += chunk;
    }

    shuffleNames(names);
}
