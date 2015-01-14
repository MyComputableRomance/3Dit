#ifndef ROTATEOPERATOR_H
#define ROTATEOPERATOR_H

#include <assimp/scene.h>
#include "glut.h"
#include <math.h>

// where the mouse hit
#define R_NONE  0
#define R_XY_1  10
#define R_XY_2  11
#define R_XY_3  12
#define R_XY_4  13
#define R_XZ_1  20
#define R_XZ_2  21
#define R_XZ_3  22
#define R_XZ_4  23
#define R_YZ_1  30
#define R_YZ_2  31
#define R_YZ_3  32
#define R_YZ_4  33
#define R_SUR_1 40
#define R_SUR_2 41
#define R_SUR_3 42
#define R_SUR_4 43
#define R_SUR_5 44
#define R_SUR_6 45
#define R_SUR_7 46
#define R_SUR_8 47

#define LINE_NUM        180
#define LINE_NUM_HALF   90
#define ANGL_ACC        0.035
#define ANGL_ACC_10     0.174533
#define RTOA            57.29578
#define PI              3.1415926

class RotateOperator
{
public:
    RotateOperator(aiVector3D minCorner, aiVector3D maxCorner, aiVector3D gCenter);
    void setBoundingBox(aiVector3D minCorner, aiVector3D maxCorner, aiVector3D gCenter);

    void draw(aiVector3D eye);
    void rotate(aiVector3D);

    bool updateHitPosition(aiVector3D a, aiVector3D direction);
    int intersect(aiVector3D a, aiVector3D direction);

    aiVector3D dragToRotate(aiVector3D direction, float length);

    void rotateFinish();
    void viewChange();
    int hitWhere;

private:
    aiVector3D center;
    aiVector3D min;
    aiVector3D max;
    GLuint render_list;

    float threshold;
    float radius;
    aiVector3D angle_acc;

    void render(aiVector3D eye);
    void drawCircle(aiVector3D eye);

};

#endif // ROTATEOPERATOR_H
