#ifndef MODELLOADER_H
#define MODELLOADER_H

#pragma once

#include "glut.h"
#include "modelcopy.h"
#include <QVector>

// assimp include files. These four are usually needed.
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/cexport.h>

#define aisgl_min(x,y) (x<y?x:y)
#define aisgl_max(x,y) (y>x?y:x)
#define ROUNDTOZERO(x) (abs(x) <= 0.005 ? 0 : x)
#define ROUNDTOONE(x) (abs(x-1) <= 0.005 ? 1 : x)
#define PI 3.1415926

using namespace std;

struct Selection{
    const aiFace* face;
    aiVector3D vertices[3];
};

class ModelLoader{
public :
    const aiScene* scene;
    bool loaded;
    /* If this instance is duplicated, then "scene" is created by "new",
     * so we need to delete it in the destructor.
     * Otherwise we need not do this.
     */
    bool isDuplicated;

    GLuint scene_list;
    GLuint mesh_list; //Used to draw the mesh.

    aiVector3D scene_min;
    aiVector3D scene_max;
    aiVector3D geometry_center;
    aiVector3D box_center;

    /*
     * If the model is selected, the mesh will be drawn.
     */
    bool selected;

    //GL_PICK
    int sceneMeshIndex; //-999 stands for global selection.
    int meshFaceIndex;
    QVector<Selection>* selection;
    aiVector3D selectedNormal;

    unsigned long total_num; //total vertex number.

public:
    ModelLoader();
    ~ModelLoader(void);

    bool load(const char *filename);
    /*
     * Assimp support 4 file formats to export. They are:
     * index    format
     * 0        dae
     * 1        obj
     * 2        stl
     * 3        ply
     */
    bool doExport(const char *filename);
    void render();
    void renderMesh();
    void scale(float xt, float yt, float zt, aiVector3D ctr); //ctr: geometry center.
    void position(float nx, float ny, float nz);
    void rotation(float rx, float ry, float rz, aiVector3D ctr); //ctr: geometry center.
    void freeRotation(aiVector2D SCTheta, aiVector2D SCGama, aiVector3D p, float xSin, float xCos);
    void updateBoundingBox();

    //GL_PICK
    void setSelectionIndex(int sceneMesh, int meshFace);
    void intersect(aiVector3D a, aiVector3D direction, float &min_tHit, int &sceneMeshIndex, int &meshFaceIndex);
    void getOnPlaneFaces();
    bool rectangleIntersect(int x1, int x2, int y1, int y2, double* mvRotated, double* prjRotated, int* viewport);

    //Align and Stick
    aiVector3D getSelectedNormal();
    aiVector3D getSelectedCenter();
    void alignTo(aiVector3D normAlignTo);
    void stickTo(aiVector3D normAlignTo, aiVector3D point);

    //Copy ModelLoader:
    ModelLoader* duplicate();

private:
    void getBoundingBoxForNode (const aiNode* nd, aiVector3D* min, aiVector3D* max, aiMatrix4x4* trafo);
    void getBoundingBox(aiVector3D* min, aiVector3D* max);
    void color4ToFloat4(const aiColor4D *c, float f[4]);
    void setFloat4(float f[4], float a, float b, float c, float d);
    void applyMaterial(const aiMaterial *mtl);
    void recursiveRender (const aiScene *sc, const aiNode* nd);
    void recursiveRenderMesh (const aiScene *sc, const aiNode* nd);
    void recursiveScale(const aiScene *sc, const aiNode* nd, float xt, float yt, float zt, aiVector3D ctr);
    void recursivePosition(const aiScene *sc, const aiNode* nd, float nx, float ny, float nz);
    void recursiveRotation(const aiScene *sc, const aiNode* nd, float rx, float ry, float rz, aiVector3D ctr);
    void freeRecursiveRotation(const aiScene *sc, const aiNode* nd, aiVector2D SCTheta, aiVector2D SCGama, aiVector3D p, float xSin, float xCos);
    void recursiveIntersect(const aiScene *sc, const aiNode* nd, aiVector3D a, aiVector3D direction, float &min_tHit, int &sceneMeshIndex, int &meshFaceIndex);
    bool recursiveRectangleIntersect(const aiNode* nd, int x1, int x2, int y1, int y2, double* mvRotated, double* prjRotated, int* viewport);
};

//Vector calculations:
float dotProduct(aiVector3D a, aiVector3D b);
aiVector3D crossProduct(aiVector3D a, aiVector3D b);

aiVector2D getSinAndCos(aiVector3D vec1, aiVector3D vec2);
aiVector2D xGetSinAndCos(aiVector3D vec1, aiVector3D vec2);

aiVector3D nRotate(aiVector3D from, float nSin, float nCos, char axis);
aiVector3D xRotate(aiVector2D SCTheta, aiVector2D SCGama, aiVector3D p, float xSin, float xCos, aiVector3D in);

#endif // MODELLOADER_H
