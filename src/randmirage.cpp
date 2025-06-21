#include "randmirage.h"
#include <QMouseEvent>
#include <QPainter>
#include <QApplication>
#include <QFontMetrics>

RandMirage::RandMirage(QWidget *parent) : QWidget(parent), m_isDragging(false)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(200, 100);

    int closeButtonSize = 20;
    m_closeButtonRect = QRect(width() - closeButtonSize, 0, closeButtonSize, closeButtonSize);
}

void RandMirage::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        if (m_closeButtonRect.contains(event->pos())) {
            emit closeRequested();
            return;
        }

        m_dragPosition = event->globalPosition().toPoint() - frameGeometry().topLeft();
        m_isDragging = false;
        event->accept();
    }
    QWidget::mousePressEvent(event);
}

void RandMirage::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        if ((event->globalPosition().toPoint() - m_dragPosition - frameGeometry().topLeft()).manhattanLength()
            > QApplication::startDragDistance()) {
            m_isDragging = true;
        }
        if (m_isDragging) {
            move(event->globalPosition().toPoint() - m_dragPosition);
            event->accept();
        }
    }
    QWidget::mouseMoveEvent(event);
}

void RandMirage::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && !m_isDragging) {
        emit clicked();
    }
    m_isDragging = false;
    QWidget::mouseReleaseEvent(event);
}

void RandMirage::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.setBrush(QColor(0, 0, 0, 204));
    painter.setPen(Qt::NoPen);
    painter.drawRect(rect());

    painter.setPen(Qt::white);

    QFont font = painter.font();
    font.setPointSize(10);
    painter.setFont(font);
    painter.drawText(10, 20, "RandMirage");

    int closeButtonSize = m_closeButtonRect.width();
    int crossPadding = 5;
    painter.drawLine(m_closeButtonRect.left() + crossPadding,
                     m_closeButtonRect.top() + crossPadding,
                     m_closeButtonRect.right() - crossPadding,
                     m_closeButtonRect.bottom() - crossPadding);
    painter.drawLine(m_closeButtonRect.left() + crossPadding,
                     m_closeButtonRect.bottom() - crossPadding,
                     m_closeButtonRect.right() - crossPadding,
                     m_closeButtonRect.top() + crossPadding);
}
