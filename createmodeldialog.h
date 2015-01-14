#ifndef CREATEMODELDIALOG_H
#define CREATEMODELDIALOG_H

#include <QWidget>

namespace Ui {
class CreateModelDialog;
}

class CreateModelDialog : public QWidget
{
    Q_OBJECT

protected:
    void paintEvent(QPaintEvent *);

public:   
    explicit CreateModelDialog(QWidget *parent = 0);
    ~CreateModelDialog();
    void showFront(){
        show();
        raise();
    }

private slots:
    void on_Btn_Create_Cone_clicked();
    void on_Btn_Create_Cylinder_clicked();
    void on_Btn_Create_Ring_clicked();
    void on_Btn_Create_TriangularPyramid_clicked();
    void on_Btn_Create_TriangularPrism_clicked();
    void on_Btn_Create_TriangularRing_clicked();
    void on_Btn_Create_RectangularPyramid_clicked();
    void on_Btn_Create_RectangularPrism_clicked();
    void on_Btn_Create_RectangularRing_clicked();
    void on_Btn_Create_PentagonalPyramid_clicked();
    void on_Btn_Create_PentagonalPrism_clicked();
    void on_Btn_Create_PentagonalRing_clicked();
    void on_Btn_Create_HexagonalPyramid_clicked();
    void on_Btn_Create_HexagonalPrism_clicked();
    void on_Btn_Create_HexangonalRing_clicked();
    void on_Btn_Create_SevenPyramid_clicked();
    void on_Btn_Create_SevenPrism_clicked();
    void on_Btn_Create_SevenRing_clicked();
    void on_Btn_Create_EightPyramid_clicked();
    void on_Btn_Create_EightPrism_clicked();
    void on_Btn_Create_EightRing_clicked();
    void on_Btn_Create_Sphere_clicked();
    void on_close_clicked();

signals:
    void createPrefab(int index);

private:
    Ui::CreateModelDialog *ui;
};

#endif // CREATEDIALOG_H
