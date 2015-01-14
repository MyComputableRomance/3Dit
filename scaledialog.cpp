#include "scaledialog.h"
#include "ui_scaledialog.h"
#include <QPainter>

ScaleDialog::ScaleDialog(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ScaleDialog)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::FramelessWindowHint);
    this->setAttribute(Qt::WA_TranslucentBackground, true);

    normalStyleSheet = "QDoubleSpinBox{\n	border:none;\n	font: 14px \"Arial\";\n	color: rgb(255, 255, 255);\n	border-image: url(:/Tabwidget/Image/Tabwidget/drop_down_list_background.png);\n	border-left-width: 6px;\n	border-right-width: 6px;\n\n	padding-left: 4px;\n}\nQDoubleSpinBox::up-button {\n	border:none;\n	width:19px;\n	border-image: url(:/Dialog/Image/Dialog/UpArrow.png);\n}\nQDoubleSpinBox::down-button {\n	border:none;\n	width:19px;\n	border-image: url(:/Dialog/Image/Dialog/DownArrow.png);\n}";
    redStyleSheet = "QDoubleSpinBox{\n	border:none;\n	font: bold 14px \"Arial\";\n	color: rgb(255, 0, 0);\n	border-image: url(:/Tabwidget/Image/Tabwidget/drop_down_list_background.png);\n	border-left-width: 6px;\n	border-right-width: 6px;\n\n	padding-left: 4px;\n}\nQDoubleSpinBox::up-button {\n	border:none;\n	width:19px;\n	border-image: url(:/Dialog/Image/Dialog/UpArrow.png);\n}\nQDoubleSpinBox::down-button {\n	border:none;\n	width:19px;\n	border-image: url(:/Dialog/Image/Dialog/DownArrow.png);\n}";
}

ScaleDialog::~ScaleDialog()
{
    delete ui;
}

void ScaleDialog::paintEvent(QPaintEvent *)
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

void ScaleDialog::on_btn_determine_clicked()
{
    emit onScale(getScaleX(), getScaleY(),getScaleZ());
}

double ScaleDialog::getScaleX(){
    return ui->scale_x->value();
}

double ScaleDialog::getScaleY(){
    return ui->scale_y->value();
}

double ScaleDialog::getScaleZ(){
    return ui->scale_z->value();
}

void ScaleDialog::setValue(double x, double y, double z, double rate){
    modelLength = x;
    modelWidth = y;
    modelHeight = z;
    ui->scale_x->setValue(x);
    ui->scale_y->setValue(y);
    ui->scale_z->setValue(z);
    ui->scale_percentage->setValue(rate);
}

void ScaleDialog::on_close_clicked()
{
    this->hide();
}

void ScaleDialog::on_scale_percentage_valueChanged(double percentage){
    // percentage change the scale value at same time
    ui->scale_x->setValue(percentage*modelLength);
    ui->scale_y->setValue(percentage*modelWidth);
    ui->scale_z->setValue(percentage*modelHeight);
}

void ScaleDialog::on_scale_x_valueChanged(double val)
{
    if(val > BOX_LENGTH)
        ui->scale_x->setStyleSheet(redStyleSheet);
    else
        ui->scale_x->setStyleSheet(normalStyleSheet);
}

void ScaleDialog::on_scale_y_valueChanged(double val)
{
    if(val > BOX_WIDTH)
        ui->scale_y->setStyleSheet(redStyleSheet);
    else
        ui->scale_y->setStyleSheet(normalStyleSheet);
}

void ScaleDialog::on_scale_z_valueChanged(double val)
{
    if(val > BOX_HEIGHT)
        ui->scale_z->setStyleSheet(redStyleSheet);
    else
        ui->scale_z->setStyleSheet(normalStyleSheet);
}
