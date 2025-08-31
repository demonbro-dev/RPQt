#include "randmirage.h"
#include <QMouseEvent>
#include <QPainter>
#include <QApplication>
#include <QFontMetrics>

#ifdef Q_OS_WIN
#include <windows.h>
#include <QOperatingSystemVersion>
#endif

RandMirage::RandMirage(QWidget *parent) : QWidget(parent), m_isDragging(false)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(200, 100);

    int closeButtonSize = 20;
    m_closeButtonRect = QRect(width() - closeButtonSize, 0, closeButtonSize, closeButtonSize);

    m_scrollAnimation = new QPropertyAnimation(this, "scrollPos", this);
    m_scrollAnimation->setDuration(7500);
    m_scrollAnimation->setLoopCount(-1);
    m_scrollPos = 0;
    m_displayText = "";

    m_opacityAnimation = new QPropertyAnimation(this, "opacityValue", this);
    m_opacityAnimation->setDuration(300);
    m_opacityAnimation->setEasingCurve(QEasingCurve::InOutQuad);
    m_currentOpacity = 0x80000000;

#ifdef Q_OS_WIN
    if (QOperatingSystemVersion::current() >= QOperatingSystemVersion::Windows10) {
        updateWindowTransparency(0x80000000);
    }
#endif
}

void RandMirage::setDisplayText(const QString &text)
{
    m_displayText = text;
    update();
    startScrolling();
}

void RandMirage::startScrolling()
{
    if (m_displayText.isEmpty()) return;

    QFont font;
    font.setPointSize(24);
    QFontMetrics fm(font);
    m_textWidth = fm.horizontalAdvance(m_displayText);

    m_scrollAnimation->stop();
    m_scrollAnimation->setStartValue(width());
    m_scrollAnimation->setEndValue(-m_textWidth);
    m_scrollAnimation->start();
}

qreal RandMirage::scrollPos() const
{
    return m_scrollPos;
}

int RandMirage::opacityValue() const
{
    return (m_currentOpacity >> 24) & 0xFF;
}

void RandMirage::setScrollPos(qreal pos)
{
    m_scrollPos = pos;
    update();
}

void RandMirage::setOpacityValue(int opacityAlpha)
{
    unsigned int baseColor = 0x00000000;
    unsigned int newOpacity = (opacityAlpha << 24) | (baseColor & 0x00FFFFFF);
    m_currentOpacity = newOpacity;

#ifdef Q_OS_WIN
    if (QOperatingSystemVersion::current() >= QOperatingSystemVersion::Windows10) {
        updateWindowTransparency(newOpacity);
    }
#endif
    update();
}

void RandMirage::updateWindowTransparency(unsigned int opacity)
{
#ifdef Q_OS_WIN
    if (QOperatingSystemVersion::current() >= QOperatingSystemVersion::Windows10) {
        HWND hwnd = (HWND)winId();

        typedef struct _ACCENTPOLICY
        {
            int nAccentState;
            int nFlags;
            unsigned int nColor;
            int nAnimationId;
        } ACCENTPOLICY;

        typedef struct _WINCOMPATTRDATA
        {
            int nAttribute;
            PVOID pData;
            ULONG ulDataSize;
        } WINCOMPATTRDATA;

        typedef BOOL (WINAPI *pSetWindowCompositionAttribute)(HWND, WINCOMPATTRDATA*);

        HMODULE hUser = GetModuleHandle(L"user32.dll");
        if (hUser) {
            pSetWindowCompositionAttribute SetWindowCompositionAttribute =
                (pSetWindowCompositionAttribute)GetProcAddress(hUser, "SetWindowCompositionAttribute");

            if (SetWindowCompositionAttribute) {
                ACCENTPOLICY policy = { 4, 0, opacity, 0 };
                WINCOMPATTRDATA data = { 19, &policy, sizeof(ACCENTPOLICY) };
                SetWindowCompositionAttribute(hwnd, &data);
            }
        }
    }
#endif
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

#ifdef Q_OS_WIN
    if (QOperatingSystemVersion::current() >= QOperatingSystemVersion::Windows10) {
        unsigned int alpha = (m_currentOpacity >> 24) & 0xFF;
        painter.setBrush(QColor(0, 0, 0, alpha));
    } else {
        painter.setBrush(QColor(0, 0, 0, 204));
    }
#else
    painter.setBrush(QColor(0, 0, 0, 204));
#endif

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

    if (!m_displayText.isEmpty()) {
        QFont textFont;
        textFont.setPointSize(24);
        painter.setFont(textFont);
        painter.setPen(Qt::white);

        QFontMetrics fm(textFont);
        int textHeight = fm.height();
        int yPos = (height() - textHeight) / 2 + fm.ascent();

        painter.drawText(m_scrollPos, yPos, m_displayText);
    }
}

void RandMirage::enterEvent(QEnterEvent *event)
{
    Q_UNUSED(event);
    m_isHovered = true;

#ifdef Q_OS_WIN
    if (QOperatingSystemVersion::current() >= QOperatingSystemVersion::Windows10) {
        m_opacityAnimation->stop();
        m_opacityAnimation->setStartValue((m_currentOpacity >> 24) & 0xFF);
        m_opacityAnimation->setEndValue(77);
        m_opacityAnimation->start();
    }
#endif
    update();
}

void RandMirage::leaveEvent(QEvent *event)
{
    Q_UNUSED(event);
    m_isHovered = false;

#ifdef Q_OS_WIN
    if (QOperatingSystemVersion::current() >= QOperatingSystemVersion::Windows10) {
        m_opacityAnimation->stop();
        m_opacityAnimation->setStartValue((m_currentOpacity >> 24) & 0xFF);
        m_opacityAnimation->setEndValue(128);
        m_opacityAnimation->start();
    }
#endif
    update();
}
