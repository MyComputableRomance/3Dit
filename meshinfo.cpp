#include "meshinfo.h"
#include "mainwindow.h"

MeshInfo::MeshInfo()
{
    mesh = NULL;
    meshName = QString();
    meshIndex = 0;
}

void MeshInfo::updateBoundingBox()
{
    mesh->updateBoundingBox();
}

MeshInfo::~MeshInfo(){
    delete mesh;
}

QString MeshInfo::getFullName(){
    if(meshIndex == 1)
        return meshName;
    else
        return QString("%1 (%2)").arg(meshName).arg(meshIndex);
}

QString MeshInfo::getSizeText(){
    QString str;
    str.sprintf("Model Size: %.2f*%.2f*%.2f(mm*mm*mm)",getLength(),getWidth(),getHeight());
    return str;
}

bool MeshInfo::doImport(QString fileName, int id){
    meshName = fileName;
    meshIndex = id;
    mesh = new ModelLoader();
    bool result = mesh->load(fileName.toStdString().c_str());
    if(result){
        boundingBoxAdjustment();

        //Stick to the bottom face:
        aiVector3D translate(0,0,-mesh->scene_min.z);
        mesh->position(translate.x, translate.y, translate.z);
        updateBoundingBox();
    }
    return result;
}

bool MeshInfo::doExport(QString fileName){
    return mesh->doExport(fileName.toStdString().c_str());
}

void MeshInfo::render(){
    if(mesh)
        mesh->render();
}

void MeshInfo::renderMesh(){
    if(mesh)
        mesh->renderMesh();
}

//Coordinate Operations:
bool MeshInfo::doScale(float tx, float ty, float tz, aiVector3D ctr)
{
    mesh->scale(tx, ty, tz, ctr);
    updateBoundingBox();
    return true;
}

bool MeshInfo::doPosition(float dx, float dy, float dz)
{
    mesh->position(dx, dy, dz);
    updateBoundingBox();
    return true;
}

bool MeshInfo::doRotation(float rx, float ry, float rz, aiVector3D ctr)
{
    mesh->rotation(rx * PI / 180.0, ry * PI / 180.0, rz * PI / 180.0, ctr);
    updateBoundingBox();
    return true;
}

void MeshInfo::doAlign(aiVector3D normal){
    mesh->alignTo(normal);
    updateBoundingBox();
}

void MeshInfo::doStickToBottom(){
    mesh->alignTo(aiVector3D(0,0,1));
    updateBoundingBox();
    aiVector3D translate(0,0,-mesh->scene_min.z);
    mesh->position(translate.x, translate.y, translate.z);
    updateBoundingBox();
}

void MeshInfo::doStick(aiVector3D normal, aiVector3D center){
    mesh->stickTo(normal, center);
    updateBoundingBox();
}

//Adjust bounding box:
int MeshInfo::scaleCheck()
{
    float length = getLength();
    float width = getWidth();
    float height = getHeight();

    if(length <= 0.05*BOX_LENGTH && width <= 0.05*BOX_WIDTH && height <= 0.05*BOX_HEIGHT)
        return -1; //The mesh is too small.
    if(length > BOX_LENGTH || width > BOX_WIDTH || height > BOX_HEIGHT)
        return 1; //The mesh is too large.
    return 0;
}

void MeshInfo::boundingBoxAdjustment()
{
    int ret = scaleCheck();
    if(ret == -1)
        MainWindow::showMessageBox("3Dit", "Your model is too small,\n which can result in low printing quality.");
    else if(ret == 1){
        //Automatically adjust the size:
        float tmp[3];
        tmp[0]=getLength()/BOX_LENGTH;
        tmp[1]=getWidth()/BOX_WIDTH;
        tmp[2]=getHeight()/BOX_HEIGHT;
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
            rate=0.8*BOX_LENGTH/getLength();
            break;
        case 1:
            rate=0.8*BOX_WIDTH/getWidth();
            break;
        case 2:
            rate=0.8*BOX_HEIGHT/getHeight();
            break;
        }
        mesh->scale(rate, rate, rate, getCenter());
        mesh->updateBoundingBox();
        MainWindow::showMessageBox("3Dit", "Your model is too large.\n It has been readjusted to fit the printer.");
    }

    //Check the POSITION of the mesh:
    float px=0, py=0, pz=0;
    //x:
    if(mesh->scene_max.x > BOX_LENGTH/2)
        px = BOX_LENGTH/2 - mesh->scene_max.x;
    else if(mesh->scene_min.x < -BOX_LENGTH/2)
        px = -BOX_LENGTH/2 - mesh->scene_min.x;
    //y:
    if(mesh->scene_max.y > BOX_WIDTH/2)
        py = BOX_WIDTH/2 - mesh->scene_max.y;
    else if(mesh->scene_min.y < -BOX_WIDTH/2)
        py = -BOX_WIDTH/2 - mesh->scene_min.y;
    //z:
    if(mesh->scene_max.z > BOX_HEIGHT)
        pz = BOX_HEIGHT - mesh->scene_max.z;
    else if(mesh->scene_min.z < 0)
        pz = -mesh->scene_min.z;

    if(px!=0 || py!=0 || pz!=0){
        mesh->position(px,py,pz);
        mesh->updateBoundingBox();
    }
}
