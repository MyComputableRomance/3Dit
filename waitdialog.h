#ifndef WAITDIALOG_H
#define WAITDIALOG_H

#include <QDialog>

namespace Ui {
class WaitDialog;
}

class WaitDialog : public QWidget
{
    Q_OBJECT

protected:
    void paintEvent(QPaintEvent *);

public:
    explicit WaitDialog(QWidget *parent = 0);
    ~WaitDialog();
    void clearText();
    void setText(QString qs);
    void setProgress(int m);

private:
    Ui::WaitDialog *ui;
};

#endif // WAITDIALOG_H
