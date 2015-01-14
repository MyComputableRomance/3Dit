#include "aligndialog.h"
#include "ui_aligndialog.h"
#include <QPainter>

AlignDialog::AlignDialog(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AlignDialog)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);
    this->setAttribute(Qt::WA_TranslucentBackground, true);

    updateDialog(0); //size = 0;
}

AlignDialog::~AlignDialog()
{
    delete ui;
}

void AlignDialog::paintEvent(QPaintEvent *)
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

void AlignDialog::updateDialog(int size){
    selectedSize = size;
    if(size == 0){
        ui->labelAlign->setVisible(false);
        ui->alignTo->setVisible(false);
        ui->isStick->setVisible(false);
        ui->OKWidget->setVisible(false);
        ui->Information->setVisible(true);
    }else if(size == 1){
        ui->labelAlign->setVisible(true);
        ui->alignTo->setVisible(true);
        ui->isStick->setVisible(true);
        ui->OKWidget->setVisible(true);
        ui->Information->setVisible(false);

        //Set comboBox components:
        if(ui->isStick->isChecked()){
            while(ui->alignTo->count() > 0)
                ui->alignTo->removeItem(0);
            ui->alignTo->addItem("Negative Z");
        }else{
            while(ui->alignTo->count() > 0)
                ui->alignTo->removeItem(0);
            ui->alignTo->addItem("Negative Z");
            ui->alignTo->addItem("Positive Z");
            ui->alignTo->addItem("Negative X");
            ui->alignTo->addItem("Positive X");
            ui->alignTo->addItem("Negative Y");
            ui->alignTo->addItem("Positive Y");
        }
    }else if(size == 2){
        ui->labelAlign->setVisible(true);
        ui->alignTo->setVisible(true);
        ui->isStick->setVisible(true);
        ui->OKWidget->setVisible(true);
        ui->Information->setVisible(false);

        //Set comboBox components:
        while(ui->alignTo->count() > 0)
            ui->alignTo->removeItem(0);
        ui->alignTo->addItem("Custom Face");
    }
}

void AlignDialog::on_isStick_toggled(bool checked)
{
    if(checked)
        ui->labelAlign->setText("Stick to:");
    else
        ui->labelAlign->setText("Align to:");

    //Change the comboBox items:
    if(selectedSize == 1){
        if(checked){
            while(ui->alignTo->count() > 0)
                ui->alignTo->removeItem(0);
            ui->alignTo->addItem("Negative Z");
        }else{
            while(ui->alignTo->count() > 0)
                ui->alignTo->removeItem(0);
            ui->alignTo->addItem("Negative Z");
            ui->alignTo->addItem("Positive Z");
            ui->alignTo->addItem("Negative X");
            ui->alignTo->addItem("Positive X");
            ui->alignTo->addItem("Negative Y");
            ui->alignTo->addItem("Positive Y");
        }
    }
}

void AlignDialog::on_btn_determine_clicked()
{
    if(ui->isStick->isChecked()){
        if(ui->alignTo->currentText()=="Custom Face")
            emit onStick(true);
        else
            emit onStick(false);
    }else
        emit onAlign(ui->alignTo->currentText());
}

void AlignDialog::on_close_clicked()
{
    this->close();
}
