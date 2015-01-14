#ifndef DRAGGABLEWIDGET_H
#define DRAGGABLEWIDGET_H

#include <QWidget>
#include <QMouseEvent>

class DraggableWidget : public QWidget
{
    Q_OBJECT
public:
    explicit DraggableWidget(QWidget *parent = 0);
    ~DraggableWidget();

signals:

public slots:

private:
    QPoint ptPress;
    QPoint ptMove;
    bool leftPressed;

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

};

#endif // DRAGGABLEWIDGET_H
