#ifndef TITLEWIDGET_H
#define TITLEWIDGET_H

#include <QWidget>

class TitleWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TitleWidget(QWidget *parent = 0);

    //Moving:
    bool mouse_press;
    QPoint move_point;
    QPoint press_point;

signals:
    void titleDoubleClicked();
    void titleMoved(QPoint);

protected:
    void mouseDoubleClickEvent(QMouseEvent *);
    void mousePressEvent(QMouseEvent *me);
    void mouseReleaseEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
};

#endif // TITLEWIDGET_H
