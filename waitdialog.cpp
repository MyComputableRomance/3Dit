#include "waitdialog.h"
#include "ui_waitdialog.h"
#include <QPainter>
#include <QDesktopWidget>

WaitDialog::WaitDialog(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WaitDialog)
{
    ui->setupUi(this);
    this->setWindowTitle("Generating GCode...");
    this->setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);
    this->setAttribute(Qt::WA_TranslucentBackground, true);

    //Move to screen center:
    QDesktopWidget* desktop = QApplication::desktop();
    QRect rect = desktop->availableGeometry();
    this->move(rect.width()/2 - this->width()/2, rect.height()/2 - this->height()/2);

    //Progress bar:
    ui->progressBar->setRange(0,100);
}

void WaitDialog::paintEvent(QPaintEvent *)
{
    // Paint the shadow
    QPainterPath path;
    path.setFillRule(Qt::WindingFill);
    path.addRect(5, 5, this->width(), this->height());

    // Make the window transparent
    QPainter painter(this);
    painter.fillRect(this->rect(), QColor(255, 255, 255, 0));
    painter.setRenderHint(QPainter::Antialiasing, true);

    QColor color(0, 0, 0, 50);
    for(int i=0; i<5; i++)
    {
        QPainterPath path;
        path.setFillRule(Qt::WindingFill);

        path.addRect(5-i, 5, this->width()-(5-i)*2, this->height()-10);
        color.setAlpha(160 - sqrt(1.0*i)*80);
        painter.setPen(color);
        painter.drawPath(path);

        path.addRect(5, 5-i, this->width()-10, this->height()-(5-i)*2);
        color.setAlpha(160 - sqrt(1.0*i)*80);
        painter.setPen(color);
        painter.drawPath(path);
    }
}

WaitDialog::~WaitDialog()
{
    delete ui;
}

void WaitDialog::clearText()
{
    ui->output->clear();
}

void WaitDialog::setText(QString qs)
{
    ui->output->setPlainText(ui->output->toPlainText()+qs);
}

void WaitDialog::setProgress(int m)
{
    ui->progressBar->setValue(m);
}
