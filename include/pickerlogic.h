#ifndef PICKERLOGIC_H
#define PICKERLOGIC_H

#include <QObject>
#include <QStringList>
#include <QTimer>

class PickerLogic : public QObject
{
    Q_OBJECT

public:
    explicit PickerLogic(QObject *parent = nullptr);

    enum class RandomGeneratorType {
        QRandomGenerator,
        minstd_rand,
        mt19937,
        RdRand,
        BCryptGenRandomOrURandom,
        RandomSelect
    };

    void setNames(const QStringList &names);
    QStringList pickNames(int count, bool parallelPick, RandomGeneratorType generatorType);
    QString formatNamesWithLineBreak(const QStringList &names);

    bool isRunning() const;
    void startPicking(bool parallelPick);
    void stopPicking(RandomGeneratorType generatorType);
    void setPickCount(int count);
    void resetPickedNames();
    void setRandomGeneratorType(RandomGeneratorType type);
    bool cpuHasRdRand();

signals:
    void namesPicked(const QStringList &names);
    void previewNames(const QStringList &names);

private slots:
    void updateSelection(RandomGeneratorType generatorType);

private:
    QTimer *timer;
    QStringList currentNames;
    QStringList pickedNames;
    QStringList availableNames;
    RandomGeneratorType m_currentRandomType;
    int m_pickCount = 1;
    bool m_isRunning = false;
    bool m_parallelPick = true;

    void shuffleNames(QStringList &names, RandomGeneratorType generatorType);
    void shuffleNamesParallel(QStringList &names, RandomGeneratorType generatorType);
    void refreshAvailableNames();
};

#endif // PICKERLOGIC_H
