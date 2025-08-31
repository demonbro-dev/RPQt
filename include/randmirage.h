#ifndef RANDMIRAGE_H
#define RANDMIRAGE_H

#include <QWidget>
#include <QPropertyAnimation>

class RandMirage : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(qreal scrollPos READ scrollPos WRITE setScrollPos)
    Q_PROPERTY(int opacityValue READ opacityValue WRITE setOpacityValue)
public:
    explicit RandMirage(QWidget *parent = nullptr);
    void setDisplayText(const QString &text);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;

signals:
    void clicked();
    void closeRequested();

private:
    QPoint m_dragPosition;
    bool m_isDragging;
    bool m_isHovered;
    QRect m_closeButtonRect;
    qreal m_scrollPos;
    QString m_displayText;
    int m_textWidth;
    unsigned int m_currentOpacity;
    QPropertyAnimation *m_scrollAnimation;
    QPropertyAnimation *m_opacityAnimation;

    qreal scrollPos() const;
    int opacityValue() const;
    void setScrollPos(qreal pos);
    void setOpacityValue(int opacityAlpha);
    void startScrolling();
    void updateWindowTransparency(unsigned int opacity);
};

#endif // RANDMIRAGE_H
