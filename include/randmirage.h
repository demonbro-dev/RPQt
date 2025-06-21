#ifndef RANDMIRAGE_H
#define RANDMIRAGE_H

#include <QWidget>

class RandMirage : public QWidget
{
    Q_OBJECT
public:
    explicit RandMirage(QWidget *parent = nullptr);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

signals:
    void clicked();
    void closeRequested();

private:
    QPoint m_dragPosition;
    bool m_isDragging;
    QRect m_closeButtonRect;
};

#endif // RANDMIRAGE_H
