#include "draggablewidget.h"

DraggableWidget::DraggableWidget(QWidget *parent) :
    QWidget(parent)
{
    leftPressed = false;
    ptPress = QPoint(0,0);
    ptMove = QPoint(0,0);
    this->setMouseTracking(true);
    this->setAcceptDrops(true);
}

DraggableWidget::~DraggableWidget()
{
}

void DraggableWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton){
        ptPress = event->globalPos();
        leftPressed = true;
    }
    event->ignore();
}

void DraggableWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (leftPressed){
        ptMove = event->globalPos();
        this->parentWidget()->move(this->parentWidget()->pos() + ptMove - ptPress);
        ptPress = ptMove;
    }
    event->ignore();
}

void DraggableWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton){
        leftPressed = false;
    }
    event->ignore();
}
