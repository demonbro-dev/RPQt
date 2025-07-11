#ifndef RANDMIRAGE_H
#define RANDMIRAGE_H

#include <QWidget>
#include <QPropertyAnimation>

class RandMirage : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(qreal scrollPos READ scrollPos WRITE setScrollPos)
public:
    explicit RandMirage(QWidget *parent = nullptr);
    void setDisplayText(const QString &text);

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
    qreal m_scrollPos;
    QString m_displayText;
    int m_textWidth;
    QPropertyAnimation *m_scrollAnimation;

    qreal scrollPos() const;
    void setScrollPos(qreal pos);
    void startScrolling();
};

#endif // RANDMIRAGE_H
