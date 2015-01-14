#include "viewport.h"
#include "mainwindow.h"
#include <QtOpenGL>
#include <Windows.h>

Viewport::Viewport(QWidget *parent, QWidget *mainWindow)
    : QGLWidget(QGLFormat(QGL::SampleBuffers), parent)
{
    startTimer(5);

    rad_p = PI/4.0;
    rad_q = PI/4.0;
    r = 240.0;
    target_y = sqrt(r);
    target_x = target_z = 0.0;
    eyex = eyey = eyez = 0.0;
    win_w = win_h = 0;

    //Meshes:
    meshes = new QMultiMap<QString, MeshInfo*>();
    backup = new QMultiMap<QString, MeshInfo*>();
    //Selection:
    selection = new QVector<SelectionInfo*>();
    isMultiSelection = false;
    //Rectangle Pick:
    isRectanglePick = false;
    //Clipboard:
    clipBoard = new QVector<MeshInfo*>();
    //GCode
    gps = new QVector<Point>();
    preview = false;
    changed = false;
    //Draw Mesh:
    isMeshDrawn = true;
    //Redo and Undo:
    isCurrentStepCanUndo = false;
    isRecorded = false;

    //Operators
    transOpt = new TranslateOperator(aiVector3D(0,0,0), aiVector3D(20,20,20) ,aiVector3D(10,10,10));
    scaleOpt = new ScaleOperator(aiVector3D(0,0,0), aiVector3D(20,20,20) ,aiVector3D(10,10,10));
    rotateOpt = new RotateOperator(aiVector3D(0,0,0), aiVector3D(20,20,20) ,aiVector3D(10,10,10));
    onOperator = false;
    onOperatorMove = false;

    //Connect:
    connect(this,SIGNAL(fileStatusUpdated()),mainWindow,SLOT(refreshCmb()));

    //operation checked
    operation_checked = OPT_NONE;
}

Viewport::~Viewport()
{
    //Meshes:
    QMultiMap<QString, MeshInfo*>::iterator it;
    for(it=meshes->begin();it!=meshes->end();it++)
        delete it.value();
    delete meshes;
    //Backup:
    for(it=backup->begin();it!=backup->end();it++)
        delete it.value();
    delete backup;
    //Selection:
    for(int i=0;i<selection->size();i++)
        delete selection->at(i);
    delete selection;
    //Clipboard:
    for(int i=0;i<clipBoard->size();i++)
        delete clipBoard->at(i);
    delete clipBoard;
    //GCode
    delete gps;

    // Operators
    delete transOpt;
    delete scaleOpt;
    delete rotateOpt;

    glDeleteLists(theBackground, 1);
}

int Viewport::getModelIndex(QString fileName){
    int index = 1;
    QList<MeshInfo*> list = meshes->values(fileName);
    if(list.isEmpty())
        index = 1;
    else{
        for(int i=0;i<list.size();i++)
            if(list.at(i)->getIndex() > index)
                index = list.at(i)->getIndex();
        index = index + 1;
    }
    return index;
}

bool Viewport::loadFile(QString fileName){
    MeshInfo* info = new MeshInfo();
    //Get the index of the file:
    int index = getModelIndex(fileName);
	//Import:
    bool success = info->doImport(fileName, index);
    if(success){
        //Record:
        if(!isRecorded){
            record();
            isRecorded = true;
        }

        meshes->insert(fileName, info);
        emit fileStatusUpdated();

		//Clear selection and select the imported file:
		clearSelection();
        SelectionInfo* selectionInfo = new SelectionInfo(info, -999, -1);
        selection->append(selectionInfo);

        //Update selection:
        updateSelection();
        updateSelectionBoundingBox();
        emit selectionChanged(); //Set the sizeLabel etc.
        return true;
    }else{
        delete info;
        return false;
    }
}

bool Viewport::exportFile(MeshInfo* mesh, QString name)
{
    bool success = mesh->doExport(name);
    return success;
}

MeshInfo* Viewport::getModel(QString fullName){
    if(fullName == "")
        return NULL;
    QMap<QString, MeshInfo*>::iterator it;
    for(it=meshes->begin();it!=meshes->end();it++){
        MeshInfo* m = it.value();
        if(m->getFullName() == fullName)
            return m;
    }
    return NULL;
}

MeshInfo* Viewport::getModelAt(int index){
    if (index < 0 || index >= meshNum())
        return NULL;
    QMap<QString, MeshInfo*>::iterator it = meshes->begin();
    it += index;
    return it.value();
}

void Viewport::initializeGL()
{
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    theBackground = glGenLists(1);
    glNewList(theBackground, GL_COMPILE);
    this->draw();
    glEndList();
}

void Viewport::resizeGL(int w, int h)
{
    glViewport(0,0,w,h);
    win_w = w;
    win_h = h;
}

void Viewport::wheelEvent(QWheelEvent *e)
{
    r +=  GLfloat(e->delta())/(-15.0);

    if (r >= 500.0)
        r = 500.0;
    else if (r <= 10.0)
        r = 10.0;
}

void Viewport::mousePressEvent(QMouseEvent *e)
{
    if(e->buttons() & Qt::RightButton){
        QCursor cur;
        cur.setShape(Qt::ClosedHandCursor);
        QApplication::setOverrideCursor(cur);
        mouse = e->pos();
    }else if(e->buttons() & Qt::LeftButton){
        //If "Shift" is also pressed?
        if(QApplication::keyboardModifiers() == Qt::ShiftModifier)
            isMultiSelection = true;
        else
            isMultiSelection = false;

        //Mouse Position:
        mouse = e->pos();
        int x = e->pos().x();
        int y = e->pos().y();

        onOperatorMove = false;

		//Get ray:
        Ray ray = getRay(x,y);
        aiVector3D a(ray.ax, ray.ay, ray.az);
        aiVector3D direction(ray.dx, ray.dy, ray.dz);
        //If "Alignment" is checked:
        if(operation_checked == OPT_ALIGN){
            //Intersect for alignment:
            modelIntersectForAlignment(a, direction);
        }else{
            onOperator = updateHitPosition(a,direction);
            if (!onOperator || selection->size() == 0){
                //Clear selection:
                if(!isMultiSelection)
                    clearSelection();
                //Intersect:
                bool intersect = modelIntersect(a, direction);
                if(!intersect){
                    startPos = mouse;
                    isRectanglePick = true;
                }
            }
        }
    }
}

void Viewport::mouseMoveEvent(QMouseEvent *e)
{
    if (e->buttons() & Qt::RightButton){
        QPoint off = e->pos() - mouse;
        mouse = e->pos();
        int x = off.x();
        int y = off.y();

        rad_q += GLfloat(x)/(-300.0);
        if (rad_q <= 0)
            rad_q += 2*PI;
        else if (rad_q >= 2*PI)
            rad_q -= 2*PI;

        rad_p += GLfloat(y)/300.0;
        if (rad_p >= PI/2)
            rad_p = PI/2;
        else if (rad_p <= -PI/4)
            rad_p = -PI/4;
    }
    else if (e->buttons() & Qt::LeftButton){
        if(!isRectanglePick){
            mouseDrag(mouse, e->pos());
            onOperatorMove = true;
        }
        /* startPos: the start position of the drag;
         * mouse: the current position of the drag;
         */
        mouse = e->pos();
    }
}

void Viewport::mouseReleaseEvent(QMouseEvent* e)
{
    if(isRectanglePick){
        isRectanglePick = false;
        mouseRelease_for_RectanglePick();
    }else if(e->button() == Qt::LeftButton && onOperator && onOperatorMove){
        mouseRelease();
    }
    QApplication::restoreOverrideCursor();
}

void Viewport::paintGL()
{
    //OpenGL operations:
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    glClearColor(0.4,0.4,0.4,0.4);

    //Draw
    glCallList(theBackground);
	//Set Light:
    setLight();

    if(preview)
        //Preview GCode
        previewGCode();
    else{
        glPushMatrix();
            QMultiMap<QString, MeshInfo*>::iterator it;
            //draw all models and update @selected and @mesh_list of each model;
            for(it=meshes->begin();it!=meshes->end();it++){
                it.value()->getMesh()->selected = false;
                if (selection->size() != 0){
                    for (int i=0;i<selection->size();i++){
                        if (it.value() == selection->at(i)->mesh){
                            it.value()->getMesh()->selected = true;
                            break;
                        }
                    }
                }
                if (!it.value()->getMesh()->selected){
                    glDeleteLists(it.value()->getMesh()->mesh_list, 1);
                    it.value()->getMesh()->mesh_list = 0;
                }
                it.value()->render();
            }
        glPopMatrix();

        //isMeshDrawn?
        if(isMeshDrawn){
            if (selection->size() != 0){
                glDisable(GL_DEPTH_TEST);
                for (int i=0;i<selection->size();i++){
                    selection->at(i)->mesh->renderMesh();
                }
                glEnable(GL_DEPTH_TEST);
            }
        }

        //Draw Operator:
        drawOperator();
        //Draw Rectangle:
        drawRectangle();
    }
	//Look at:
    lookAt();
}

void Viewport::timerEvent(QTimerEvent *)
{
    updateGL();
}

void Viewport::lookAt()
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(30.0, (GLfloat)win_w/(GLfloat)win_h, 1.0, 1000.0);
    eyex = r*cos(rad_p)*sin(rad_q)+target_x;
    eyey = r*sin(rad_p);
    eyez = r*cos(rad_p)*cos(rad_q)+target_z;
    target_y = sqrt(r);
    gluLookAt(eyex ,eyey ,eyez ,target_x ,target_y ,target_z ,0.0 ,1.0 ,0.0);
    glMatrixMode(GL_MODELVIEW);
}

void Viewport::setLight()
{
    glDisable(GL_LIGHTING);
    GLfloat ambient[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    GLfloat diffuse[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    GLfloat specular[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    GLfloat position[4] = {eyex, eyey, eyez, 1.0};
    glLightfv(GL_LIGHT0,GL_AMBIENT,ambient);
    glLightfv(GL_LIGHT0,GL_DIFFUSE,diffuse);
    glLightfv(GL_LIGHT0,GL_SPECULAR,specular);
    glLightfv(GL_LIGHT0,GL_POSITION,position);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);    // Uses default lighting parameters.

    glEnable(GL_COLOR_MATERIAL);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE);
}

void Viewport::draw()
{
    drawCoordinate();
    drawBox();
}

void Viewport::drawBox()
{
    glDisable(GL_LIGHTING);
    glPushMatrix();
    glLineWidth(2);
    glBegin(GL_QUADS);
        glColor4f(0.9,0.9,0.9,0.2);
        glVertex3f(-84.5,0.0,82.5);
        glVertex3f(84.5,0.0,82.5);
        glVertex3f(84.5,0.0,-82.5);
        glVertex3f(-84.5,0.0,-82.5);
    glEnd();
    glLineWidth(2);
    glBegin(GL_LINE_LOOP);
        glColor4f(0.9,0.9,0.9,0.2);
        glVertex3f(-84.5,160.0,82.5);
        glVertex3f(84.5,160.0,82.5);
        glVertex3f(84.5,160.0,-82.5);
        glVertex3f(-84.5,160.0,-82.5);
    glEnd();
    glBegin(GL_LINES);
        glColor4f(0.9,0.9,0.9,0.2);
        glVertex3f(-84.5,160.0,82.5);
        glVertex3f(-84.5,0.0,82.5);
        glVertex3f(84.5,160.0,82.5);
        glVertex3f(84.5,0.0,82.5);
        glVertex3f(84.5,160.0,-82.5);
        glVertex3f(84.5,0.0,-82.5);
        glVertex3f(-84.5,160.0,-82.5);
        glVertex3f(-84.5,0.0,-82.5);
    glEnd();
    glPopMatrix();
    glEnable(GL_LIGHTING);
}

void Viewport::drawCoordinate()
{
    glDisable(GL_LIGHTING);
    glPushMatrix();
        glColor4f(0.0,0.0,0.1,1.0);
        glLineWidth(0.2);
        for (float i=-85;i<=85;i+=5){
            glBegin(GL_LINES);
                glVertex3f(GLfloat(i),0.0,-100);
                glVertex3f(GLfloat(i),0.0,100);
            glEnd();
        }
        for (float i=-85;i<=85;i+=5){
            glBegin(GL_LINES);
                glVertex3f(-100,0.0,GLfloat(i));
                glVertex3f(100,0.0,GLfloat(i));
            glEnd();
        }
        glLineWidth(2);
        // z axis
        glBegin(GL_LINES);
            glColor4f(0.0,0.0,1.0,1.0);
            glVertex3f(0.0,0.0,0.0);
            glVertex3f(0.0,170.0,0.0);
        glEnd();
        // y axis
        glBegin(GL_LINES);
            glColor4f(0.0,1.0,0.0,1.0);
            glVertex3f(0.0,0.0,0);
            glVertex3f(0.0,0.0,-110);
        glEnd();
        // x axis
        glBegin(GL_LINES);
            glColor4f(1.0,0.0,0.0,1.0);
            glVertex3f(0,0.0,0.0);
            glVertex3f(110,0.0,0.0);
        glEnd();
        if (r < 40.0){
            glColor4f(0.0,0.0,0.1,1.0);
            glLineWidth(0.1);
            for (float i=-85;i<=85;i+=1){
                glBegin(GL_LINES);
                    glVertex3f(GLfloat(i),0.0,-100);
                    glVertex3f(GLfloat(i),0.0,100);
                glEnd();
            }
            for (float i=-85;i<=85;i+=1){
                glBegin(GL_LINES);
                    glVertex3f(-100,0.0,GLfloat(i));
                    glVertex3f(100,0.0,GLfloat(i));
                glEnd();
            }
        }
    glPopMatrix();
    glEnable(GL_LIGHTING);
}

//GL_PICK
/* {1, 0, 0, 0,
 * 0, 0, -1, 0,
 * 0, 1, 0, 0,
 * 0, 0, 0, 1} */
//The above matrix LEFT multiplys with mat.
double* getRotateRes(double* mat){
    double* res = new double[16];
    for(int i=0; i<16; i++){
        if(i/4 == 1)
            res[i] = -mat[i+4];
        else if(i/4 == 2)
            res[i] = mat[i-4];
        else
            res[i] = mat[i];
    }
    delete mat;
    return res;
}

Ray Viewport::getRay(int x, int y)
{
    Ray ret;

    double* mv = new double[16];
    double* prj = new double[16]; //MODELVIEW and PROJECTION.

    glGetDoublev(GL_MODELVIEW_MATRIX , mv);
    glGetDoublev(GL_PROJECTION_MATRIX, prj);

    double* mvRotated = getRotateRes(mv);
    double* prjRotated = prj;

    //Get viewport:
    int viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    double x1, y1, z1;
    double x2, y2, z2;
    //The winZ of the nearest plane of the camera is approximately 0.0(or 1.0? I just don't know.);
    gluUnProject(x, viewport[3] - y, 0.0, mvRotated, prjRotated, viewport, &x1, &y1, &z1);
    gluUnProject(x, viewport[3] - y, 1.0, mvRotated, prjRotated, viewport, &x2, &y2, &z2);
    aiVector3D a;
    a.Set(x1, y1, z1);
    aiVector3D b;
    b.Set(x2, y2, z2);
    aiVector3D direction = b - a;
    direction.Normalize();

    ret.ax = a.x;
    ret.ay = a.y;
    ret.az = a.z;
    ret.dx = direction.x;
    ret.dy = direction.y;
    ret.dz = direction.z;

    delete mvRotated;
    delete prjRotated;

    return ret;
}

bool Viewport::modelIntersect(aiVector3D a, aiVector3D direction)
{
    //Get line intersect:
    QMultiMap<QString, MeshInfo*>::iterator it;

    MeshInfo* min_Mesh = NULL;
    float min_tHit = -1;
    int sceneMeshIndex = -1;
    int meshFaceIndex = -1;

    for(it=meshes->begin();it!=meshes->end();it++){
        float tmpMin;
        int tmpSceneMesh, tmpMeshFace;
        it.value()->intersect(a, direction, tmpMin, tmpSceneMesh, tmpMeshFace);

        //Is the mininum distance?
        if(tmpMin > 0){
            if(min_tHit == -1 || tmpMin < min_tHit){
                min_tHit = tmpMin;
                sceneMeshIndex = tmpSceneMesh;
                meshFaceIndex = tmpMeshFace;
                min_Mesh = it.value();
            }
        }
    }

    if(min_Mesh){
        //Selected a mesh:
        if(isMultiSelection){
            //Already selected this mesh?
            for(int i=0;i<selection->size();i++){
                if(selection->at(i)->mesh == min_Mesh){
                    delete selection->at(i);
                    selection->remove(i);
                    break;
                }
            }
            SelectionInfo* info = new SelectionInfo(min_Mesh, -999, -1);
            selection->append(info);
        }else{
            SelectionInfo* info = new SelectionInfo(min_Mesh, -999, -1);
            selection->append(info);
        }
        //Update selection:
        updateSelection();
        updateSelectionBoundingBox();
        emit selectionChanged();
        return true;
    }else{
        //Selected an empty space:
        //If "Shift", then nothing happened.
        //If not "Shift", then all selected thing is ignored.
        updateSelection();
        updateSelectionBoundingBox();
        emit selectionChanged();
        return false;
    }
}

void Viewport::modelIntersectForAlignment(aiVector3D a, aiVector3D direction){
    //Get line intersect:
    QMultiMap<QString, MeshInfo*>::iterator it;

    MeshInfo* min_Mesh = NULL;
    float min_tHit = -1;
    int sceneMeshIndex = -1;
    int meshFaceIndex = -1;

    for(it=meshes->begin();it!=meshes->end();it++){
        float tmpMin;
        int tmpSceneMesh, tmpMeshFace;
        it.value()->intersect(a, direction, tmpMin, tmpSceneMesh, tmpMeshFace);

        //Is the mininum distance?
        if(tmpMin > 0){
            if(min_tHit == -1 || tmpMin < min_tHit){
                min_tHit = tmpMin;
                sceneMeshIndex = tmpSceneMesh;
                meshFaceIndex = tmpMeshFace;
                min_Mesh = it.value();
            }
        }
    }

    //Alignment: select single mesh face.
    //User can only select up to 2 faces in different meshes.
    if(min_Mesh){
        int size = selection->size();
        if(size == 0){
            SelectionInfo* info = new SelectionInfo(min_Mesh, sceneMeshIndex, meshFaceIndex);
            selection->append(info);
        }else if(size == 1){
            MeshInfo* src = selection->at(0)->mesh;
            if(min_Mesh != src){
                SelectionInfo* info = new SelectionInfo(min_Mesh, sceneMeshIndex, meshFaceIndex);
                selection->append(info);
            }
        }
    }else
        clearSelection();
    updateSelection();
    updateSelectionBoundingBox();
    emit selectionChanged();
    emit selectionChanged_for_AlignDlg(selection->size()); //Let the alignDlg know the change.
}

void Viewport::updateMeshes(QString fullName){
    MeshInfo* selected = getModel(fullName);
    clearSelection();
    if(selected){
        SelectionInfo* info = new SelectionInfo(selected, -999, -1);
        selection->append(info);
    }
    //Update selection:
    updateSelection();
    updateSelectionBoundingBox();
}

//Duplicate:
MeshInfo* Viewport::duplicateMesh(MeshInfo *source){
    MeshInfo* dest = new MeshInfo();

    //Name:
    QString name = source->getName();
    int index = source->getIndex();
    dest->setName(name);
    dest->setIndex(index);
    //Mesh:
    ModelLoader* sourceLoader = source->getMesh();
    dest->setMesh(sourceLoader->duplicate());

    //Return:
    return dest;
}

//Mesh Copy:
void Viewport::meshCopy(){
    //Clear clipboard:
    clearClipboard();

    //Get selection info:
    for(int i=0;i<selection->size();i++){
        MeshInfo* mesh = selection->at(i)->mesh;
        clipBoard->push_back(duplicateMesh(mesh));
    }
}

//Mesh Paste:
void Viewport::meshPaste(){
    if(clipBoard->isEmpty())
        return; //No mesh in the clipboard.

    clearSelection();
    record();
    setChanged(true);

    for(int i=0;i<clipBoard->size();i++){
        MeshInfo* meshToPaste = duplicateMesh(clipBoard->at(i));

        //Get the index of the file:
        QString name = meshToPaste->getName();
        int index = getModelIndex(name);
        meshToPaste->setIndex(index);

        //Paste!
        meshes->insert(name, meshToPaste);
        //Add to selection:
        SelectionInfo* selectionInfo = new SelectionInfo(meshToPaste, -999, -1);
        selection->append(selectionInfo);
    }
    emit fileStatusUpdated();
    //Update selection:
    updateSelection();
    updateSelectionBoundingBox();
    emit selectionChanged();
}

//Select All:
void Viewport::selectAll(){
    if(operation_checked == OPT_ALIGN){
        MainWindow::showMessageBox("Select All", "You cannot select all meshes in ALIGN mode.");
        return;
    }
    clearSelection();
    //Add all to selection:
    QMultiMap<QString, MeshInfo*>::iterator it;
    for(it=meshes->begin();it!=meshes->end();it++){
        MeshInfo* mesh = it.value();
        SelectionInfo* selectionInfo = new SelectionInfo(mesh, -999, -1);
        selection->append(selectionInfo);
    }
    //Update selection:
    updateSelection();
    updateSelectionBoundingBox();
    emit selectionChanged();
}

//Delete
void Viewport::doDelete(){
    if(operation_checked == OPT_ALIGN){
        MainWindow::showMessageBox("Delete", "You cannot delete any mesh in ALIGN mode.");
        return;
    }
    if(selection->size() == 0)
        return;

    record();
    setChanged(true);

    for(int i=0;i<selection->size();i++){
        MeshInfo* info = selection->at(i)->mesh;
        //Delete the data in [meshes].
        QMultiMap<QString, MeshInfo*>::iterator it;
        for(it=meshes->begin();it!=meshes->end();it++){
            MeshInfo* m = it.value();
            if(m == info){
                delete m;
                meshes->erase(it);
                break;
            }
        }
    }
    emit fileStatusUpdated();
    clearSelection();
    //Update selection:
    //No need to update selection.
    updateSelectionBoundingBox();
    emit selectionChanged();
}

// Operators' function
bool Viewport::updateHitPosition(aiVector3D a, aiVector3D direction)
{
    if(selection->size() == 0)
        return false;
    switch(operation_checked){
    case OPT_TRANS:{
        return transOpt->updateHitPosition(a,direction);
    }
    case OPT_SCALE:{
        return scaleOpt->updateHitPosition(a,direction);
    }
    case OPT_ROTATE:{
        return rotateOpt->updateHitPosition(a,direction);
    }
    default:break;
    }
    return false;
}

void Viewport::mouseDrag(QPoint o, QPoint n)
{
    if(operation_checked == OPT_NONE)
        return;
    if(selection->size() == 0)
        return;

    Ray oldRay = getRay(o.x(), o.y());
    Ray newRay = getRay(n.x(), n.y());

    QPoint off = n - o;
    int x = off.x();
    int y = off.y();

    float length = sqrt(float(x*x + y*y));
    aiVector3D direction(newRay.dx - oldRay.dx,newRay.dy - oldRay.dy,newRay.dz - oldRay.dz);

    switch(operation_checked){
    case OPT_TRANS:{
        aiVector3D movement = transOpt->dragToMove(direction,length);

        double dx = ROUNDTOZERO(movement.x);
        double dy = ROUNDTOZERO(movement.y);
        double dz = ROUNDTOZERO(movement.z);
        if(dx==0 && dy==0 && dz==0)
            break;

        if(!isRecorded){
            record();
            isRecorded = true;
        }
        emit onMove(movement.x, movement.y, movement.z);
        break;
    }
    case OPT_SCALE:{
        aiVector3D times = scaleOpt->dragToScale(direction, length);

        double tx = ROUNDTOZERO(times.x);
        double ty = ROUNDTOZERO(times.y);
        double tz = ROUNDTOZERO(times.z);
        if(tx==1 && ty==1 && tz==1)
            break;

        if(!isRecorded){
            record();
            isRecorded = true;
        }
        emit onScale(times.x, times.y, times.z);
        break;
    }
    case OPT_ROTATE:{
        aiVector3D angle = rotateOpt->dragToRotate(direction, length);

        double rx = ROUNDTOZERO(angle.x);
        double ry = ROUNDTOZERO(angle.y);
        double rz = ROUNDTOZERO(angle.z);
        if(rx==0 && ry==0 && rz==0)
            break;

        if(!isRecorded){
            record();
            isRecorded = true;
        }
        emit onRotate(angle.x, angle.y, angle.z);
        break;
    }
    default:break;
    }
}

void Viewport::drawOperator()
{
    switch(operation_checked){
    case OPT_TRANS:{
        if (selection->size() != 0){
            transOpt->setBoundingBox(selection->at(0)->mesh->getMesh()->scene_min, selection->at(0)->mesh->getMesh()->scene_max, selection->at(0)->mesh->getMesh()->geometry_center);
            transOpt->draw();
        }
        else{
            transOpt->hitWhere = T_NONE;
        }
        break;
    }
    case OPT_SCALE:{
        if (selection->size() != 0){
            scaleOpt->setBoundingBox(selection->at(0)->mesh->getMesh()->scene_min, selection->at(0)->mesh->getMesh()->scene_max, selection->at(0)->mesh->getMesh()->geometry_center);
            scaleOpt->draw();
        }
        else{
            scaleOpt->hitWhere = S_NONE;
        }
        break;
    }
    case OPT_ROTATE:{
        if (selection->size() != 0){
            rotateOpt->setBoundingBox(selection->at(0)->mesh->getMesh()->scene_min, selection->at(0)->mesh->getMesh()->scene_max, selection->at(0)->mesh->getMesh()->geometry_center);
            rotateOpt->draw(aiVector3D(eyex, eyey, eyez));
        }
        else{
            rotateOpt->hitWhere = R_NONE;
        }
        break;
    }
    default:break;
    }
}

void Viewport::mouseRelease()
{
    switch(operation_checked){
    case OPT_SCALE:{
        if (selection->size() != 0){
            scaleOpt->scaleFinish();
            updateSelectionBoundingBox();
            boundingBoxAdjustment();
        }
        isRecorded = false;
        break;
    }
    case OPT_ROTATE:{
        if (selection->size() != 0){
            rotateOpt->rotateFinish();
            updateSelectionBoundingBox();
            boundingBoxAdjustment();
        }
        isRecorded = false;
        break;
    }
    case OPT_TRANS:{
        if (selection->size() != 0){
            updateSelectionBoundingBox();
            boundingBoxAdjustment();
        }
        isRecorded = false;
        break;
    }
    default:break;
    }
}

//Rectangle Pick:
void Viewport::drawRectangle(){
    if(isRectanglePick){
        double* mv = new double[16];
        double* prj = new double[16]; //MODELVIEW and PROJECTION.

        glGetDoublev(GL_MODELVIEW_MATRIX , mv);
        glGetDoublev(GL_PROJECTION_MATRIX, prj);

        //Get viewport:
        int viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);

        //Get the corner points:
        int x1 = startPos.x(), x2 = mouse.x();
        int y1 = viewport[3] - startPos.y(), y2 = viewport[3] - mouse.y();

        //Get the world coordinates:
        GLdouble wx1, wy1, wz1;
        GLdouble wx2, wy2, wz2;
        GLdouble wx3, wy3, wz3;
        GLdouble wx4, wy4, wz4;
        gluUnProject(x1, y1, 0.01, mv, prj, viewport, &wx1, &wy1, &wz1);
        gluUnProject(x2, y1, 0.01, mv, prj, viewport, &wx2, &wy2, &wz2);
        gluUnProject(x2, y2, 0.01, mv, prj, viewport, &wx3, &wy3, &wz3);
        gluUnProject(x1, y2, 0.01, mv, prj, viewport, &wx4, &wy4, &wz4);

        //Draw Rectangle:
        glColor4f(1.0f,0.35f,0.0f,1.0f);
        glLineWidth(1);
        glBegin(GL_LINE_LOOP);
            glVertex3d(wx1, wy1, wz1);
            glVertex3d(wx2, wy2, wz2);
            glVertex3d(wx3, wy3, wz3);
            glVertex3d(wx4, wy4, wz4);
        glEnd();

        delete mv;
        delete prj;
    }
}

void Viewport::mouseRelease_for_RectanglePick(){
    double* mv = new double[16];
    double* prj = new double[16]; //MODELVIEW and PROJECTION.

    glGetDoublev(GL_MODELVIEW_MATRIX , mv);
    glGetDoublev(GL_PROJECTION_MATRIX, prj);

    double* mvRotated = getRotateRes(mv);
    double* prjRotated = prj;

    //Get viewport:
    int viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    //Get the corner points:
    int x1 = startPos.x(), x2 = mouse.x();
    int y1 = viewport[3] - startPos.y(), y2 = viewport[3] - mouse.y();

    //Intersect:
    if(isMultiSelection){
        for(int i=0;i<meshNum();i++){
            MeshInfo* mesh = getModelAt(i);
            bool result = mesh->rectangleIntersect(x1, x2, y1, y2, mvRotated, prjRotated, viewport);
            if(result){
                //Already selected this mesh?
                for(int i=0;i<selection->size();i++){
                    if(selection->at(i)->mesh == mesh){
                        delete selection->at(i);
                        selection->remove(i);
                        break;
                    }
                }
                SelectionInfo* info = new SelectionInfo(mesh, -999, -1);
                selection->append(info);
            }
        }
    }else{
        for(int i=0;i<meshNum();i++){
            MeshInfo* mesh = getModelAt(i);
            bool result = mesh->rectangleIntersect(x1, x2, y1, y2, mvRotated, prjRotated, viewport);
            if(result){
                SelectionInfo* info = new SelectionInfo(mesh, -999, -1);
                selection->append(info);
            }
        }
    }
    //Update selection:
    updateSelection();
    updateSelectionBoundingBox();
    emit selectionChanged();

    delete mvRotated;
    delete prjRotated;
}

//Deal with bounding box:
void Viewport::updateSelectionBoundingBox()
{
    int size = selection->size();
    if (size == 0){
        selection_max.Set(0,0,0);
        selection_min.Set(0,0,0);
        global_center.Set(0,0,0);
        global_box_center.Set(0,0,0);
        return;
    }
    else if (size == 1){
        ModelLoader *tmp = selection->at(0)->mesh->getMesh();
        selection_max = tmp->scene_max;
        selection_min = tmp->scene_min;
        global_center = tmp->geometry_center;
        global_box_center = tmp->box_center;
        return;
    }
    else {
        ModelLoader *tmp;
        unsigned long count = 0;
        selection_max.Set(-999,-999,-999);
        selection_min.Set(999,999,999);
        global_center.Set(0,0,0);

        for (int i=0;i<size;i++){
            tmp = selection->at(i)->mesh->getMesh();
            selection_max.x = (selection_max.x > tmp->scene_max.x ? selection_max.x : tmp->scene_max.x);
            selection_max.y = (selection_max.y > tmp->scene_max.y ? selection_max.y : tmp->scene_max.y);
            selection_max.z = (selection_max.z > tmp->scene_max.z ? selection_max.z : tmp->scene_max.z);

            selection_min.x = (selection_min.x < tmp->scene_min.x ? selection_min.x : tmp->scene_min.x);
            selection_min.y = (selection_min.y < tmp->scene_min.y ? selection_min.y : tmp->scene_min.y);
            selection_min.z = (selection_min.z < tmp->scene_min.z ? selection_min.z : tmp->scene_min.z);

            global_center.x += tmp->geometry_center.x * tmp->total_num;
            global_center.y += tmp->geometry_center.y * tmp->total_num;
            global_center.z += tmp->geometry_center.z * tmp->total_num;
            count += tmp->total_num;
        }
        global_center /= (count * 1.0f);

        global_box_center.x = (selection_max.x + selection_min.x) / 2;
        global_box_center.y = (selection_max.y + selection_min.y) / 2;
        global_box_center.z = (selection_max.z + selection_min.z) / 2;
        return;
    }
}

int Viewport::scaleCheck()
{
    float length = selection_max.x - selection_min.x;
    float width = selection_max.y - selection_min.y;
    float height = selection_max.z - selection_min.z;

    if(length <= 0.05*BOX_LENGTH && width <= 0.05*BOX_WIDTH && height <= 0.05*BOX_HEIGHT)
        return -1; //The mesh is too small.
    if(length > BOX_LENGTH || width > BOX_WIDTH || height > BOX_HEIGHT)
        return 1; //The mesh is too large.
    return 0;
}

void Viewport::boundingBoxAdjustment()
{
    int ret = scaleCheck();
    if(ret == -1)
        MainWindow::showMessageBox("3Dit", "Your model is too small,\n which can result in low printing quality.");
    else if(ret == 1){
        float length = selection_max.x - selection_min.x;
        float width = selection_max.y - selection_min.y;
        float height = selection_max.z - selection_min.z;

        //Automatically adjust the size:
        float tmp[3];
        tmp[0]=length/BOX_LENGTH;
        tmp[1]=width/BOX_WIDTH;
        tmp[2]=height/BOX_HEIGHT;
        //Find the maximum ratio:
        int x=0;
        if(tmp[x]<tmp[1])
            x=1;
        if(tmp[x]<tmp[2])
            x=2;
        //Scale:
        float rate;
        switch(x){
        case 0:
            rate=0.8*BOX_LENGTH/length;
            break;
        case 1:
            rate=0.8*BOX_WIDTH/width;
            break;
        case 2:
            rate=0.8*BOX_HEIGHT/height;
            break;
        }
        emit onScale(rate,rate,rate);
        updateSelectionBoundingBox();

        //ShowMessage:
        MainWindow::showMessageBox("3Dit", "Your model is too large.\n It has been readjusted to fit the printer.");
    }

    //Check the POSITION of the mesh:
    float px=0, py=0, pz=0;
    //x:
    if(selection_max.x > BOX_LENGTH/2)
        px = BOX_LENGTH/2 - selection_max.x;
    else if(selection_min.x < -BOX_LENGTH/2)
        px = -BOX_LENGTH/2 - selection_min.x;
    //y:
    if(selection_max.y > BOX_WIDTH/2)
        py = BOX_WIDTH/2 - selection_max.y;
    else if(selection_min.y < -BOX_WIDTH/2)
        py = -BOX_WIDTH/2 - selection_min.y;
    //z:
    if(selection_max.z > BOX_HEIGHT)
        pz = BOX_HEIGHT - selection_max.z;
    else if(selection_min.z < 0)
        pz = -selection_min.z;

    if(px!=0 || py!=0 || pz!=0){
        emit onMove(px,py,pz);
        updateSelectionBoundingBox();
    }
}

//GCode previewing:
void  Viewport::setPoints(QVector<Point> *ps){
    delete gps;
    gps = ps;
}

void Viewport::previewGCode(){
    int size = gps->size();
    if(size == 0)
        return;
    glPushMatrix();
        GLfloat rotate[16] = {
            1, 0, 0, 0,
            0, 0, -1, 0,
            0, 1, 0, 0,
            0, 0, 0, 1
        };
        glMultMatrixf(&rotate[0]);
        for(int i = 0;i < size - 1;i++){
            Point gps_i = gps->at(i);
            Point gps_j = gps->at(i+1);
            if(gps_j.draw){
                int b = i * 255 /size;
                qglColor(qRgb((b)*0.3,(b)*0.3,(b)*0.9));
                glLineWidth(1);
                glBegin(GL_LINES);
                    glVertex3f(gps_i.x, gps_i.y, gps_i.z);
                    glVertex3f(gps_j.x, gps_j.y, gps_j.z);
                glEnd();
            }
        }
    glPopMatrix();
}

//Update the backup:
void Viewport::updateBackup(){
    QMultiMap<QString, MeshInfo*>::iterator it;
    for(it=backup->begin();it!=backup->end();it++)
        delete it.value();
    backup->clear();
    //Copy each mesh of [meshes] to backup:
    for(it=meshes->begin();it!=meshes->end();it++){
        MeshInfo* source = it.value();
        MeshInfo* dest = this->duplicateMesh(source);
        backup->insert(dest->getName(), dest);
    }
}

//Swap backup and meshes:
void Viewport::swapBackup(){
    QMultiMap<QString, MeshInfo*>* tmp;
    tmp = meshes;
    meshes = backup;
    backup = tmp;
}

//Record change:
void Viewport::record(){
    updateBackup();
    isCurrentStepCanUndo = true;
}

//Update fileCmb, selection and dialogs.
void Viewport::updateUI(){
    //FileCmb:
    emit fileStatusUpdated();
    //Selection:
    //Now, the pointers in Selection should point to the meshes in [backup]. (Remeber it's a DEEP COPY)
    QVector<SelectionInfo*>* vec = selection;
    selection = new QVector<SelectionInfo*>();
    for(int i=0;i<vec->size();i++){
        QString fullName = vec->at(i)->mesh->getFullName();
        //Is fullName exists in new vector [meshes]?
        MeshInfo* mesh = getModel(fullName);
        if(mesh){
            SelectionInfo* info = new SelectionInfo(mesh, vec->at(i)->sceneMeshIndex, vec->at(i)->meshFaceIndex);
            selection->append(info);
        }
    }
    //Delete vec:
    for(int i=0;i<vec->size();i++)
        delete vec->at(i);
    delete vec;
    //Update selection:
    updateSelection();
    updateSelectionBoundingBox();
    emit selectionChanged();
}

//Undo:
void Viewport::undo(){
    if(meshes->size()==0 && backup->size()==0)
        return;
    if(operation_checked == OPT_ALIGN){
        MainWindow::showMessageBox("Undo", "You cannot undo in ALIGN mode.");
        return;
    }
    if(isCurrentStepCanUndo){
        isCurrentStepCanUndo = false;
        swapBackup();
        updateUI();
        setChanged(true);
    }
}

//Redo:
void Viewport::redo(){
    if(meshes->size()==0 && backup->size()==0)
        return;
    if(operation_checked == OPT_ALIGN){
        MainWindow::showMessageBox("Redo", "You cannot redo in ALIGN mode.");
        return;
    }
    if(!isCurrentStepCanUndo){
        isCurrentStepCanUndo = true;
        swapBackup();
        updateUI();
        setChanged(true);
    }
}
