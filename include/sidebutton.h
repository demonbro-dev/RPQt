#ifndef SIDEBUTTON_H
#define SIDEBUTTON_H

#include <QPushButton>

class SideButton : public QPushButton
{
    Q_OBJECT
public:
    explicit SideButton(QWidget *parent = nullptr);

signals:
    void clickedToShowMain();

protected:
    void paintEvent(QPaintEvent *event) override;
};

#endif // SIDEBUTTON_H
