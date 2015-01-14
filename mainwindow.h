#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedLayout>
#include <QPainter>
#include <QMouseEvent>
#include <QDesktopWidget>
#include <QBitmap>
#include <QTextStream>
#include <QFileDialog>
#include <QVector>
#include <QMultiMap>
#include <QProcess>
#include <Windows.h>

#include "titlewidget.h"
#include "translatedialog.h"
#include "scaledialog.h"
#include "messagedialog.h"
#include "rotatedialog.h"
#include "aligndialog.h"
#include "viewport.h"
#include "SelectionInfo.h"
#include "gthread.h"
#include "createmodeldialog.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QWidget
{
    Q_OBJECT

protected:
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *me);
    void mouseMoveEvent(QMouseEvent *me);
    void mouseReleaseEvent(QMouseEvent *me);
    void resizeEvent(QResizeEvent * re);
    //Windows event!
    bool nativeEvent(const QByteArray &eventType, void *message, long *);

public:
    explicit MainWindow(QWidget *parent = 0);
    void doMaximize();
    void doNormalize();
    ~MainWindow();
    //Show Message Box:
    static MessageDialog* msgDlg;
    static void showMessageBox(QString title, QString content);

    //File I/O: the name of the file to save.
    QString fileName;

public slots:
    void on_left_expand_btn_clicked();
    void on_left_collapse_btn_clicked();
    void on_close_clicked();
    void on_min_clicked();
    void on_max_clicked();

    //Custom slots:
    void onTitleMoved(QPoint);

    void refreshCmb(); //Update the contents of fileCmb.
    void updateCmb(); //Change the selected item of fileCmb.
    void updateSizeLbl(QString); //Update the sizeLbl when user activated fileCmb.
    void resize(); //Update the sizeLbl after scale, rotation and translation etc.

    //File I/O:
    void doImport();
    void doExport();
    void doSave();

    //Operations:
    void doScale(double nx, double ny, double nz);
    void doScaleDrag(double tx, double ty, double tz);
    void doPosition(double px, double py, double pz);
    void doPositionDrag(double dx, double dy, double dz);
    void doRotation(double rx, double ry, double rz);
    void doRotationDrag(double rx, double ry, double rz);
    void doAlign(QString selectedText);
    void doStick(bool flag);
    void doPreview(bool isChecked);
    void doCopy();
    void doPaste();
    void doCreate();
    void doDelete();
    void doSelectAll();
    void doRedo();
    void doUndo();
    void doImportModel(QString filepath);
    void doImportPrefab(int index);

private:
    Ui::MainWindow *ui;
    QRect getRect();

    //isMaximized
    bool maxNormal;
    //isMousePressed
    bool pressed;

    //Mouse Dragging:
    QPoint start_point;
    QRect start_geometry;
    QPoint mouse_point;
    bool left, right, top, bottom;

    //Dialogs:
    TranslateDialog* transDlg;
    ScaleDialog* scaleDlg;
    RotateDialog* rotateDlg;
    AlignDialog* alignDlg;
    CreateModelDialog* createModelDlg;
    //Viewport:
    Viewport* viewport;

    void getParameter(int& perimeter, double& density, double& resolution);
    void readGCode(QFile &gfile, QVector<Point>* ps);

    // Set checked
    void setChecked(int o, int n);

    GThread* gt;
    GThread* pgt;
    QString configFile;

signals:
    void coordinateChanged();

private slots:
    void on_left_scale_c_1_clicked(bool checked);
    void on_left_scale_e_2_clicked(bool checked);
    void on_left_rotate_e_2_clicked(bool checked);
    void on_left_rotate_c_1_clicked(bool checked);
    void on_left_alignment_c_1_clicked(bool checked);
    void on_left_alignment_e_2_clicked(bool checked);
    void on_left_move_e_2_clicked(bool checked);
    void on_left_move_c_1_clicked(bool checked);
    void on_left_print_c_1_clicked();
    void on_left_print_e_2_clicked();
    void on_fileCmb_activated(QString fullName);
    void on_checkBox_toggled(bool checked);
    void onDrawPreview();
};

#endif // MAINWINDOW_H
