#include "createmodeldialog.h"
#include "ui_createmodeldialog.h"
#include <QPainter>
#include <QDesktopWidget>

CreateModelDialog::CreateModelDialog(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CreateModelDialog)
{
    ui->setupUi(this);
    this->setWindowTitle("Create");
    this->setWindowFlags(Qt::FramelessWindowHint);
    this->setAttribute(Qt::WA_TranslucentBackground, true);

    //Move to screen center:
    QDesktopWidget* desktop = QApplication::desktop();
    QRect rect = desktop->availableGeometry();
    this->move(rect.width()/2 - this->width()/2, rect.height()/2 - this->height()/2);
}

void CreateModelDialog::paintEvent(QPaintEvent *)
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

CreateModelDialog::~CreateModelDialog()
{
    delete ui;
}

void CreateModelDialog::on_Btn_Create_Cone_clicked()
{
    emit createPrefab(1);
}

void CreateModelDialog::on_Btn_Create_Cylinder_clicked()
{
    emit createPrefab(2);
}

void CreateModelDialog::on_Btn_Create_Ring_clicked()
{
    emit createPrefab(3);
}

void CreateModelDialog::on_Btn_Create_TriangularPyramid_clicked()
{
    emit createPrefab(4);
}

void CreateModelDialog::on_Btn_Create_TriangularPrism_clicked()
{
    emit createPrefab(5);
}

void CreateModelDialog::on_Btn_Create_TriangularRing_clicked()
{
    emit createPrefab(6);
}

void CreateModelDialog::on_Btn_Create_RectangularPyramid_clicked()
{
    emit createPrefab(7);
}

void CreateModelDialog::on_Btn_Create_RectangularPrism_clicked()
{
    emit createPrefab(8);
}

void CreateModelDialog::on_Btn_Create_RectangularRing_clicked()
{
    emit createPrefab(9);
}

void CreateModelDialog::on_Btn_Create_PentagonalPyramid_clicked()
{
    emit createPrefab(10);
}

void CreateModelDialog::on_Btn_Create_PentagonalPrism_clicked()
{
    emit createPrefab(11);
}

void CreateModelDialog::on_Btn_Create_PentagonalRing_clicked()
{
    emit createPrefab(12);
}

void CreateModelDialog::on_Btn_Create_HexagonalPyramid_clicked()
{
    emit createPrefab(13);
}

void CreateModelDialog::on_Btn_Create_HexagonalPrism_clicked()
{
    emit createPrefab(14);
}

void CreateModelDialog::on_Btn_Create_HexangonalRing_clicked()
{
    emit createPrefab(15);
}

void CreateModelDialog::on_Btn_Create_SevenPyramid_clicked()
{
    emit createPrefab(16);
}

void CreateModelDialog::on_Btn_Create_SevenPrism_clicked()
{
    emit createPrefab(17);
}

void CreateModelDialog::on_Btn_Create_SevenRing_clicked()
{
    emit createPrefab(18);
}

void CreateModelDialog::on_Btn_Create_EightPyramid_clicked()
{
    emit createPrefab(19);
}

void CreateModelDialog::on_Btn_Create_EightPrism_clicked()
{
    emit createPrefab(20);
}

void CreateModelDialog::on_Btn_Create_EightRing_clicked()
{
    emit createPrefab(21);
}

void CreateModelDialog::on_Btn_Create_Sphere_clicked()
{
    emit createPrefab(22);
}

void CreateModelDialog::on_close_clicked()
{
    this->hide();
}
