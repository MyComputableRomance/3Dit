#include "messagedialog.h"
#include "ui_messagedialog.h"
#include <QPainter>

MessageDialog::MessageDialog(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MessageDialog)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);
    this->setAttribute(Qt::WA_TranslucentBackground, true);
}

MessageDialog::MessageDialog(QString title, QString text, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MessageDialog)
{
    ui->setupUi(this);
    this->setWindowTitle(title);
    ui->title->setText(title);
    this->setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);
    this->setAttribute(Qt::WA_TranslucentBackground, true);
    ui->Text->setText(text);
}

MessageDialog::~MessageDialog()
{
    delete ui;
}

void MessageDialog::paintEvent(QPaintEvent *)
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

void MessageDialog::on_btn_determine_clicked()
{
    this->close();
}

void MessageDialog::on_close_clicked()
{
    this->close();
}
