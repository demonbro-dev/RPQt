#include "pickerlogic.h"
#include <random>
#include <QRandomGenerator>
#include <QtConcurrent>

#ifdef Q_OS_WIN
#include <windows.h>
#include <bcrypt.h>
#endif

#if defined(__x86_64__) || defined(__i386__) || defined(_M_IX86) || defined(_M_X64)
#if defined(_MSC_VER)
#include <intrin.h>
#else
#include <cpuid.h>
#endif // defined(_MSC_VER)
#endif // defined(__x86_64__) || defined(__i386__) || defined(_M_IX86) || defined(_M_X64)

PickerLogic::PickerLogic(QObject *parent) : QObject(parent), timer(new QTimer(this))
{
    connect(timer, &QTimer::timeout, this, [this]() {
        updateSelection(m_currentRandomType);
    });
}


void PickerLogic::setNames(const QStringList &names)
{
    currentNames = names;
    resetPickedNames();
}

QStringList PickerLogic::pickNames(int count, bool parallelPick, RandomGeneratorType generatorType)
{
    QStringList result;

    if(availableNames.isEmpty()) {
        // 如果所有名字都已被抽选，则重置
        resetPickedNames();
    }

    // 从可用名单中随机选取
    QStringList shuffled = availableNames;
    if (parallelPick) {
        shuffleNamesParallel(shuffled, generatorType);
    } else {
        shuffleNames(shuffled, generatorType);
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
    const int maxPerLine = 5;

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

void PickerLogic::stopPicking(RandomGeneratorType generatorType)
{
    if (m_isRunning) {
        timer->stop();
        m_isRunning = false;

        QStringList finalResult = pickNames(m_pickCount, m_parallelPick, generatorType);
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

void PickerLogic::updateSelection(RandomGeneratorType generatorType)
{
    // 动态抽选时从全部名字中随机显示
    QStringList shuffled = currentNames;
    if (m_parallelPick) {
        shuffleNamesParallel(shuffled, generatorType);
    } else {
        shuffleNames(shuffled, generatorType);
    }

    // 取前m_pickCount个名字用于预览
    QStringList preview;
    int takeCount = qMin(m_pickCount, shuffled.size());
    for(int i = 0; i < takeCount; ++i) {
        preview << shuffled.at(i);
    }

    emit previewNames(preview);
}

void PickerLogic::shuffleNames(QStringList &names, RandomGeneratorType generatorType)
{
    if (names.isEmpty()) return;

    QList<int> indexes;
    for(int i = 0; i < names.size(); ++i) {
        indexes.append(i);
    }

    if (generatorType == RandomGeneratorType::RandomSelect) {
#ifdef Q_OS_WIN
        const int choice = QRandomGenerator::global()->bounded(5);
#else
        const int choice = QRandomGenerator::global()->bounded(4);
#endif
        generatorType = static_cast<RandomGeneratorType>(choice);
    }

    switch (generatorType) {
    case RandomGeneratorType::QRandomGenerator: {
        QRandomGenerator generator(QDateTime::currentMSecsSinceEpoch());
        for (int i = indexes.size() - 1; i > 0; --i) {
            int j = generator.bounded(i + 1);
            qSwap(indexes[i], indexes[j]);
        }
        break;
    }
    case RandomGeneratorType::minstd_rand: {
        std::random_device rd;
        std::minstd_rand gen(rd());
        for (int i = indexes.size() - 1; i > 0; --i) {
            std::uniform_int_distribution<int> dist(0, i);
            int j = dist(gen);
            qSwap(indexes[i], indexes[j]);
        }
        break;
    }
    case RandomGeneratorType::mt19937: {
        std::random_device rd;
        std::mt19937 gen(rd());
        for (int i = indexes.size() - 1; i > 0; --i) {
            std::uniform_int_distribution<int> dist(0, i);
            int j = dist(gen);
            qSwap(indexes[i], indexes[j]);
        }
        break;
    }
    case RandomGeneratorType::RdRand: {
#if defined(__x86_64__) || defined(__i386__) || defined(_M_IX86) || defined(_M_X64)
        static bool has_rdrand = cpuHasRdRand();

        if (has_rdrand) {
            for (int i = indexes.size() - 1; i > 0; --i) {
                unsigned int random_num;
                // 尝试使用RdRand
                int retries = 10;
                while (retries-- > 0) {
                    if (_rdrand32_step(&random_num)) {
                        int j = random_num % (i + 1);
                        qSwap(indexes[i], indexes[j]);
                        break;
                    }
                }
                if (retries < 0) {
                    int j = QRandomGenerator::global()->bounded(i + 1);
                    qSwap(indexes[i], indexes[j]);
                }
            }
        } else {
            for (int i = indexes.size() - 1; i > 0; --i) {
                int j = QRandomGenerator::global()->bounded(i + 1);
                qSwap(indexes[i], indexes[j]);
            }
        }
#else
        for (int i = indexes.size() - 1; i > 0; --i) {
            int j = QRandomGenerator::global()->bounded(i + 1);
            qSwap(indexes[i], indexes[j]);
        }
#endif
        break;
    }
    case RandomGeneratorType::BCryptGenRandom: {
#ifdef Q_OS_WIN
        for (int i = indexes.size() - 1; i > 0; --i) {
            ULONG randomNum;
            if (BCryptGenRandom(
                    NULL,
                    (PUCHAR)&randomNum,
                    sizeof(randomNum),
                    BCRYPT_USE_SYSTEM_PREFERRED_RNG) == 0) {
                int j = randomNum % (i + 1);
                qSwap(indexes[i], indexes[j]);
            }
        }
#endif
        break;
    }
    default:
        for (int i = indexes.size() - 1; i > 0; --i) {
            int j = QRandomGenerator::global()->bounded(i + 1);
            qSwap(indexes[i], indexes[j]);
        }
    }

    QStringList shuffled;
    for (int i = 0; i < indexes.size(); ++i) {
        shuffled << names.at(indexes[i]);
    }
    names = shuffled;
}

void PickerLogic::shuffleNamesParallel(QStringList &names, RandomGeneratorType generatorType)
{
    if (names.isEmpty()) return;

    int chunkSize = names.size() / QThread::idealThreadCount();
    if (chunkSize < 1) chunkSize = 1;

    QVector<QStringList> chunks;
    for (int i = 0; i < names.size(); i += chunkSize) {
        chunks.append(names.mid(i, chunkSize));
    }

    auto shuffleChunk = [this, generatorType](QStringList &chunk) {
        shuffleNames(chunk, generatorType);
    };

    QtConcurrent::blockingMap(chunks, shuffleChunk);

    names.clear();
    for (const QStringList &chunk : chunks) {
        names += chunk;
    }

    shuffleNames(names, generatorType);
}

void PickerLogic::setRandomGeneratorType(RandomGeneratorType type)
{
    m_currentRandomType = type;
}

bool PickerLogic::cpuHasRdRand()
{
#if defined(__x86_64__) || defined(__i386__) || defined(_M_IX86) || defined(_M_X64)
#if defined(_MSC_VER)
    int cpuInfo[4];
    __cpuid(cpuInfo, 1);
    return (cpuInfo[2] & (1 << 30)) != 0;
#else
    unsigned int eax, ebx, ecx, edx;
    __get_cpuid(1, &eax, &ebx, &ecx, &edx);
    return (ecx & (1 << 30)) != 0;
#endif // defined(_MSC_VER)
#else
    return false;
#endif // defined(__x86_64__) || defined(__i386__) || defined(_M_IX86) || defined(_M_X64)
}
