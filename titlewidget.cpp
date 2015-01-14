#include "titlewidget.h"
#include <QMouseEvent>

TitleWidget::TitleWidget(QWidget *parent) :
    QWidget(parent)
{
    mouse_press = false;
}

void TitleWidget::mouseDoubleClickEvent(QMouseEvent *){
    emit titleDoubleClicked();
}

void TitleWidget::mousePressEvent(QMouseEvent *me){
    if(me->button() == Qt::LeftButton){
        mouse_press = true;
        press_point = me->globalPos();
    }
}

void TitleWidget::mouseMoveEvent(QMouseEvent *me)
{
    if(mouse_press){
        move_point = me->globalPos();
        emit titleMoved(move_point - press_point);
        press_point = move_point;
    }
}

void TitleWidget::mouseReleaseEvent(QMouseEvent *){
    mouse_press = false;
}
