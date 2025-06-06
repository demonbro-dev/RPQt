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

    void setNames(const QStringList &names);
    QStringList pickNames(int count);
    QString formatNamesWithLineBreak(const QStringList &names);

    bool isRunning() const;
    void startPicking();
    void stopPicking();
    void setPickCount(int count);
    void resetPickedNames();

signals:
    void namesPicked(const QStringList &names);
    void previewNames(const QStringList &names);

private slots:
    void updateSelection();

private:
    QTimer *timer;
    QStringList currentNames;
    QStringList pickedNames;
    QStringList availableNames;
    int m_pickCount = 1;
    bool m_isRunning = false;

    void shuffleNames(QStringList &names);
    void refreshAvailableNames();
};

#endif // PICKERLOGIC_H
