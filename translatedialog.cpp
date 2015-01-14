#include "translatedialog.h"
#include "ui_translatedialog.h"
#include <QPainter>

TranslateDialog::TranslateDialog(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TranslateDialog)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::FramelessWindowHint);
    this->setAttribute(Qt::WA_TranslucentBackground, true);

    normalStyleSheet = "QDoubleSpinBox{\n	border:none;\n	font: 14px \"Arial\";\n	color: rgb(255, 255, 255);\n	border-image: url(:/Tabwidget/Image/Tabwidget/drop_down_list_background.png);\n	border-left-width: 6px;\n	border-right-width: 6px;\n\n	padding-left: 4px;\n}\nQDoubleSpinBox::up-button {\n	border:none;\n	width:19px;\n	border-image: url(:/Dialog/Image/Dialog/UpArrow.png);\n}\nQDoubleSpinBox::down-button {\n	border:none;\n	width:19px;\n	border-image: url(:/Dialog/Image/Dialog/DownArrow.png);\n}";
    redStyleSheet = "QDoubleSpinBox{\n	border:none;\n	font: bold 14px \"Arial\";\n	color: rgb(255, 0, 0);\n	border-image: url(:/Tabwidget/Image/Tabwidget/drop_down_list_background.png);\n	border-left-width: 6px;\n	border-right-width: 6px;\n\n	padding-left: 4px;\n}\nQDoubleSpinBox::up-button {\n	border:none;\n	width:19px;\n	border-image: url(:/Dialog/Image/Dialog/UpArrow.png);\n}\nQDoubleSpinBox::down-button {\n	border:none;\n	width:19px;\n	border-image: url(:/Dialog/Image/Dialog/DownArrow.png);\n}";
}

TranslateDialog::~TranslateDialog()
{
    delete ui;
}

void TranslateDialog::paintEvent(QPaintEvent *)
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

void TranslateDialog::on_btn_determine_clicked()
{
    emit onTranslate(getTranslateX(), getTranslateY(), getTranslateZ());
}

double TranslateDialog::getTranslateX(){
    return ui->translate_x->value();
}

double TranslateDialog::getTranslateY(){
    return ui->translate_y->value();
}

double TranslateDialog::getTranslateZ(){
    return ui->translate_z->value();
}

void TranslateDialog::setValue(double x, double y, double z){
    ui->translate_x->setValue(x);
    ui->translate_y->setValue(y);
    ui->translate_z->setValue(z);
}

void TranslateDialog::on_close_clicked()
{
    this->hide();
}

void TranslateDialog::on_translate_x_valueChanged(double val)
{
    if(val > BOX_LENGTH/2 || val < -BOX_LENGTH/2)
        ui->translate_x->setStyleSheet(redStyleSheet);
    else
        ui->translate_x->setStyleSheet(normalStyleSheet);
}

void TranslateDialog::on_translate_y_valueChanged(double val)
{
    if(val > BOX_WIDTH/2 || val < -BOX_WIDTH/2)
        ui->translate_y->setStyleSheet(redStyleSheet);
    else
        ui->translate_y->setStyleSheet(normalStyleSheet);
}

void TranslateDialog::on_translate_z_valueChanged(double val)
{
    if(val > BOX_HEIGHT || val < 0)
        ui->translate_z->setStyleSheet(redStyleSheet);
    else
        ui->translate_z->setStyleSheet(normalStyleSheet);
}
