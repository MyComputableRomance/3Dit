#ifndef MESHINFO_H
#define MESHINFO_H

#include "ModelLoader.h"
#include "printerSize.h"
#include <QString>

class MeshInfo
{
private:
    ModelLoader* mesh;
    QString meshName;
    int meshIndex; //How many existing models with the same name?

public:
    MeshInfo();
    ~MeshInfo();

    //Mesh I/O:
    bool doImport(QString fileName, int id);
    bool doExport(QString fileName);

    //Getters:
	ModelLoader * getMesh(){return mesh;}
    QString getName(){return meshName;}
    int getIndex(){return meshIndex;}
    float getLength(){return mesh->scene_max.x - mesh->scene_min.x;}
    float getWidth(){return mesh->scene_max.y - mesh->scene_min.y;}
    float getHeight(){return mesh->scene_max.z - mesh->scene_min.z;}
    aiVector3D getCenter(){return mesh->geometry_center;}
    aiVector3D getBoxCenter(){return mesh->box_center;}
    QString getFullName();
    QString getSizeText();
    aiVector3D getSelectedNormal(){
        return mesh->getSelectedNormal();
    }
    aiVector3D getSelectedCenter(){
        return mesh->getSelectedCenter();
    }

    //Setters:
    void setMesh(ModelLoader* m){mesh = m;}
    void setName(QString n){meshName = n;}
    void setIndex(int i){meshIndex = i;}

    //Render:
    void render();
    void renderMesh();

    //Coordinate operations:
    bool doScale(float tx, float ty, float tz, aiVector3D ctr);
    bool doPosition(float dx, float dy, float dz);
    bool doRotation(float rx, float ry, float rz, aiVector3D ctr);
    void doAlign(aiVector3D normal);
    void doStickToBottom();
    void doStick(aiVector3D normal, aiVector3D center);

    //GL_PICK
    void intersect(aiVector3D a, aiVector3D direction, float& min_tHit, int& sceneMeshIndex, int& meshFaceIndex){
        mesh->setSelectionIndex(-1, -1);
        mesh->intersect(a, direction, min_tHit, sceneMeshIndex, meshFaceIndex);
    }
    bool rectangleIntersect(int x1, int x2, int y1, int y2, double* mvRotated, double* prjRotated, int* viewport){
        return mesh->rectangleIntersect(x1, x2, y1, y2, mvRotated, prjRotated, viewport);
    }
    void setSelectionIndex(int sceneMesh, int meshFace){
        mesh->setSelectionIndex(sceneMesh, meshFace);
    }

    // Bounding box adjustment
    int scaleCheck();
    void updateBoundingBox();
    void boundingBoxAdjustment();
};

#endif // MESHINFO_H
