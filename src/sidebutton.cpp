#include "sidebutton.h"
#include <QPainter>
#include <QStyleOption>

SideButton::SideButton(QWidget *parent)
    : QPushButton(parent)
{
    setFixedSize(30, 60);
    setWindowOpacity(0.85);
    setStyleSheet("background-color: #353535; color: white; border: none;");
    connect(this, &SideButton::clicked, this, &SideButton::clickedToShowMain);
}

void SideButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.fillRect(rect(), QColor(53, 53, 53));

    QPen pen(Qt::white, 2);
    painter.setPen(pen);

    int arrowSize = 10;
    int centerX = width() / 2;
    int centerY = height() / 2;

    QPolygon arrow;
    arrow << QPoint(centerX + arrowSize/2, centerY - arrowSize)
          << QPoint(centerX - arrowSize/2, centerY)
          << QPoint(centerX + arrowSize/2, centerY + arrowSize);

    painter.drawPolygon(arrow);
}
