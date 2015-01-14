#ifndef VIEWPORT_H
#define VIEWPORT_H

#include <QGLWidget>
#include <QVector>
#include <QList>
#include <QMultiMap>
#include "meshinfo.h"
#include "SelectionInfo.h"
#include "ModelLoader.h"
#include "point.h"
#include "translateoperator.h"
#include "scaleoperator.h"
#include "rotateoperator.h"
#include "ray.h"

// define which operation is checked
#define OPT_NONE   0
#define OPT_TRANS  1
#define OPT_SCALE  2
#define OPT_ROTATE 3
#define OPT_ALIGN  4

class Viewport : public QGLWidget
{
    Q_OBJECT
public:
    explicit Viewport(QWidget *parent, QWidget *mainWindow);
    ~Viewport();

    //Mesh I/O:
    bool loadFile(QString fileName);
    bool exportFile(MeshInfo *mesh, QString name);
    int meshNum(){return meshes->size();}
    MeshInfo* getModel(QString fullName);
    MeshInfo* getModelAt(int index);
    int getModelIndex(QString fileName);

    //Selection:
    QVector<SelectionInfo*>* getSelection(){return selection;}
    void updateSelection(){
       QMultiMap<QString, MeshInfo*>::iterator it;
       for(it=meshes->begin();it!=meshes->end();it++)
           it.value()->setSelectionIndex(-1, -1);
       for(int i=0;i<selection->size();i++){
          SelectionInfo* info = selection->at(i);
          info->mesh->setSelectionIndex(info->sceneMeshIndex, info->meshFaceIndex);
       }
    }
    void clearSelection(){
        for(int i=0;i<selection->size();i++)
            delete selection->at(i);
        selection->clear();
    }

    //Draw Mesh:
    bool isMeshDrawn;
    void drawMesh(bool d){isMeshDrawn = d;}

    //GL_PICK:
    Ray getRay(int x, int y);
    bool modelIntersect(aiVector3D a, aiVector3D direction);
    void modelIntersectForAlignment(aiVector3D a, aiVector3D direction);

    //Model preview:
    void setPoints(QVector<Point>* ps);
    bool isChanged(){return changed;}
    void setChanged(bool c){changed = c;}
    void setPreviewStatus(bool p){preview = p;}

    //Clipboard and Mesh Copy:
    MeshInfo* duplicateMesh(MeshInfo* source);
    void meshCopy();//Copy the selected mesh to the clipboard;
    void meshPaste(); //Paste the mesh in the clipboard to the viewport;
    void clearClipboard(){
        for(int i=0;i<clipBoard->size();i++)
            delete clipBoard->at(i);
        clipBoard->clear();
    }
    void selectAll();
    void doDelete();

    //Meshes:
    QMultiMap<QString, MeshInfo*>* meshes;
    //Backup Meshes for redo and undo:
    QMultiMap<QString, MeshInfo*>* backup;
    bool isCurrentStepCanUndo; // isCurrentStepCanUndo = true: [meshes] stores current world, [backup] stores the world before changed.
                               // isCurrentStepCanUndo = false: [meshes] stores the world before changed, [backup] stores current world.
    bool isRecorded; // In a series of operations, we just need to record once.
    void updateBackup();
    void swapBackup();
    void record();
    void updateUI();
    void undo();
    void redo();

    // Operation checked
    int operation_checked;

    // Selection Bounding Box
    aiVector3D selection_max;
    aiVector3D selection_min;
    aiVector3D global_center; //Global [geometry] center.
    aiVector3D global_box_center; //Global [box] center.
    int scaleCheck();
    void updateSelectionBoundingBox();
    void boundingBoxAdjustment();

protected:
    void initializeGL();
    void resizeGL(int w, int h);
    void wheelEvent(QWheelEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void paintGL();
    void timerEvent(QTimerEvent *);
    void lookAt();
    void draw();
    void drawBox();
    void drawCoordinate();
    void previewGCode();
    void setLight();

private:
    //Viewpoint operations:
    double rad_p;
    double rad_q;
    double r;
    double target_x;
    double target_y;
    double target_z;
    double eyex;
    double eyey;
    double eyez;
    int win_w;
    int win_h;
    QPoint mouse;

    //Selection:
    QVector<SelectionInfo*>* selection;
    bool isMultiSelection;

    //Rectangle Pick:
    bool isRectanglePick;
    QPoint startPos;
    void drawRectangle();
    void mouseRelease_for_RectanglePick();

    //Clipboard:
    QVector<MeshInfo*>* clipBoard;

    //GCode:
    QVector<Point>* gps;
    bool preview;
    bool changed; //Record whether the status is changed. If not changed, DO NOT run slic3r when previewing.

    // Operators
    TranslateOperator *transOpt;
    ScaleOperator *scaleOpt;
    RotateOperator *rotateOpt;
    bool onOperator;
    bool onOperatorMove;

    // Render List
    GLuint theBackground;

    // Functions depends on operators
    bool updateHitPosition(aiVector3D a, aiVector3D direction);
    void mouseDrag(QPoint o, QPoint n);
    void drawOperator();
    void mouseRelease();

signals:
    void fileStatusUpdated();
    void onMove(double dx, double dy, double dz);
    void onScale(double tx, double ty, double tz);
    void onRotate(double ax, double ay, double az);
    // Selection changed:
    void selectionChanged();
    void selectionChanged_for_AlignDlg(int);

public slots:
    void change(){changed = true;}
    void updateMeshes(QString fullName); //When the user changed the item of the cmbbox.
};

#endif // VIEWPORT_H
