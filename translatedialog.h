#ifndef TRANSLATEDIALOG_H
#define TRANSLATEDIALOG_H

#include <QWidget>
#include "printerSize.h"

namespace Ui {
class TranslateDialog;
}

class TranslateDialog : public QWidget
{
    Q_OBJECT

protected:
    void paintEvent(QPaintEvent *);

public:
    explicit TranslateDialog(QWidget *parent = 0);
    ~TranslateDialog();

    double getTranslateX();
    double getTranslateY();
    double getTranslateZ();
    void setValue(double x, double y, double z);
    void showFront(){
        show();
        raise();
    }

private slots:
    void on_close_clicked();
    void on_btn_determine_clicked();
    void on_translate_x_valueChanged(double val);
    void on_translate_y_valueChanged(double val);
    void on_translate_z_valueChanged(double val);

signals:
    void onTranslate(double x, double y, double z);

private:
    Ui::TranslateDialog *ui;
    QString normalStyleSheet, redStyleSheet;
};

#endif // TRANSLATEDIALOG_H
