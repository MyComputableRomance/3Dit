#ifndef ROTATEDIALOG_H
#define ROTATEDIALOG_H

#include <QWidget>

namespace Ui {
class RotateDialog;
}

class RotateDialog : public QWidget
{
    Q_OBJECT

protected:
    void paintEvent(QPaintEvent *);

public:
    explicit RotateDialog(QWidget *parent = 0);
    ~RotateDialog();

    double getRotateX();
    double getRotateY();
    double getRotateZ();
    void setValue(double x, double y, double z);
    void showFront(){
        show();
        raise();
    }

private slots:
    void on_close_clicked();
    void on_btn_determine_clicked();

signals:
    void onRotate(double x, double y, double z);

private:
    Ui::RotateDialog *ui;
};

#endif // ROTATEDIALOG_H
