#ifndef SCALEDIALOG_H
#define SCALEDIALOG_H

#include <QWidget>
#include <QMouseEvent>
#include <QPaintEvent>
#include "printerSize.h"

namespace Ui {
class ScaleDialog;
}

class ScaleDialog : public QWidget
{
    Q_OBJECT

protected:
    void paintEvent(QPaintEvent *);

public:
    explicit ScaleDialog(QWidget *parent = 0);
    ~ScaleDialog();

    double getScaleX();
    double getScaleY();
    double getScaleZ();
    void setValue(double x, double y, double z, double rate);
    void showFront(){
        show();
        raise();
    }

private slots:
    void on_btn_determine_clicked();
    void on_close_clicked();
    void on_scale_percentage_valueChanged(double percentage);
    void on_scale_x_valueChanged(double val);
    void on_scale_y_valueChanged(double val);
    void on_scale_z_valueChanged(double val);

signals:
    void onScale(double x, double y, double z);

private:
    Ui::ScaleDialog *ui;
    QString normalStyleSheet, redStyleSheet;

    //Save the size of the model:
    double modelLength, modelWidth, modelHeight;
};

#endif // SCALEDIALOG_H
