#include "rotatedialog.h"
#include "ui_rotatedialog.h"
#include <QPainter>

RotateDialog::RotateDialog(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RotateDialog)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::FramelessWindowHint);
    this->setAttribute(Qt::WA_TranslucentBackground, true);
}

RotateDialog::~RotateDialog()
{
    delete ui;
}

void RotateDialog::paintEvent(QPaintEvent *)
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

void RotateDialog::on_close_clicked()
{
    this->hide();
}

void RotateDialog::on_btn_determine_clicked()
{
    emit onRotate(getRotateX(), getRotateY(), getRotateZ());
}

double RotateDialog::getRotateX(){
    return ui->rotate_x->value();
}

double RotateDialog::getRotateY(){
    return ui->rotate_y->value();
}

double RotateDialog::getRotateZ(){
    return ui->rotate_z->value();
}

void RotateDialog::setValue(double x, double y, double z){
    ui->rotate_x->setValue(x);
    ui->rotate_y->setValue(y);
    ui->rotate_z->setValue(z);
}
