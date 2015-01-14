#ifndef ALIGNDIALOG_H
#define ALIGNDIALOG_H

#include <QWidget>

namespace Ui {
class AlignDialog;
}

class AlignDialog : public QWidget
{
    Q_OBJECT

protected:
    void paintEvent(QPaintEvent *);

public:
    explicit AlignDialog(QWidget *parent = 0);
    ~AlignDialog();
    void showFront(){
        updateDialog(0);
        show();
        raise();
    }

public slots:
    void updateDialog(int size);
    void on_isStick_toggled(bool checked);
    void on_btn_determine_clicked();
    void on_close_clicked();

signals:
    void onAlign(QString selectedText);
    void onStick(bool flag); //true: custom face; false: bottom face.

private:
    Ui::AlignDialog *ui;
    int selectedSize;
};

#endif // ALIGNDIALOG_H
