#ifndef TRANSLATEOPERATOR_H
#define TRANSLATEOPERATOR_H

#include <assimp/scene.h>
#include "glut.h"

// where the mouse hit
#define T_NONE  0
#define T_X     1
#define T_Y     2
#define T_Z     3
#define T_XY    4
#define T_XZ    5
#define T_YZ    6
#define T_CTR   7

class TranslateOperator
{
public:
    TranslateOperator(aiVector3D minCorner, aiVector3D maxCorner, aiVector3D gCenter);
    void setBoundingBox(aiVector3D minCorner, aiVector3D maxCorner, aiVector3D gCenter);

    void draw();
    void move(aiVector3D movement);

    bool updateHitPosition(aiVector3D a, aiVector3D direction);
    int intersect(aiVector3D a, aiVector3D direction);

    aiVector3D dragToMove(aiVector3D direction, float length);

    int hitWhere;

private:
    aiVector3D center;
    aiVector3D min;
    aiVector3D max;
    GLuint render_list;

    float threshold;
    float axis_length;
    float plane_width;

    void render();
};

#endif // TRANSLATEOPERATOR_H
