#ifndef MESSAGEDIALOG_H
#define MESSAGEDIALOG_H

#include <QWidget>

namespace Ui {
class MessageDialog;
}

class MessageDialog : public QWidget
{
    Q_OBJECT

protected:
    void paintEvent(QPaintEvent *);

public:
    MessageDialog(QWidget *parent = 0);
    MessageDialog(QString title, QString text, QWidget *parent = 0);
    ~MessageDialog();

private slots:
    void on_btn_determine_clicked();
    void on_close_clicked();

private:
    Ui::MessageDialog *ui;
};

#endif // MESSAGEBOX_H
