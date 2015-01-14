#ifndef SCALEOPERATOR_H
#define SCALEOPERATOR_H

#include <assimp/scene.h>
#include "glut.h"

// where the mouse hit
#define S_NONE  0
#define S_X     1
#define S_Y     2
#define S_Z     3
#define S_CTR   4

class ScaleOperator
{
public:
    ScaleOperator(aiVector3D minCorner, aiVector3D maxCorner, aiVector3D gCenter);
    void setBoundingBox(aiVector3D minCorner, aiVector3D maxCorner, aiVector3D gCenter);

    void draw();
    void scale(aiVector3D times);

    bool updateHitPosition(aiVector3D a, aiVector3D direction);
    int intersect(aiVector3D a, aiVector3D direction);

    aiVector3D dragToScale(aiVector3D direction, float length);

    void scaleFinish();

    int hitWhere;

private:

    aiVector3D center;
    aiVector3D min;
    aiVector3D max;
    GLuint render_list;

    float axis_length_x;
    float axis_length_y;
    float axis_length_z;

    float cube_half_width;
    float threshold;

    void render();
    void drawCube(aiVector3D pos, aiVector3D color);
};

#endif // SCALEOPERATOR_H
