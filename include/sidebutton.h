#ifndef SIDEBUTTON_H
#define SIDEBUTTON_H

#include <QPushButton>

class SideButton : public QPushButton
{
    Q_OBJECT
public:
    explicit SideButton(QWidget *parent = nullptr, bool showOnRight = true);

signals:
    void clickedToShowMain();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    bool m_showOnRight;
};

#endif // SIDEBUTTON_H
