#include "scaleoperator.h"
#include "ModelLoader.h"

ScaleOperator::ScaleOperator(aiVector3D minCorner, aiVector3D maxCorner, aiVector3D gCenter)
{
    this->min = minCorner;
    this->max = maxCorner;
    this->center = gCenter;
    axis_length_x = 24.0;
    axis_length_y = 24.0;
    axis_length_z = 24.0;
    cube_half_width = 1.2;
    threshold = 1.0;

    render_list = 0;
    hitWhere = S_NONE;
}

void ScaleOperator::draw()
{
    if (render_list == 0){
        render_list = glGenLists(1);
        glNewList(render_list, GL_COMPILE);
        this->render();
        glEndList();
    }
    glCallList(render_list);
}

bool ScaleOperator::updateHitPosition(aiVector3D a, aiVector3D direction)
{
    hitWhere = this->intersect(a, direction);
    if (hitWhere == S_NONE)
        return false;
    else
        return true;
}

int ScaleOperator::intersect(aiVector3D a, aiVector3D direction)
{
    aiVector3D norm;
    float range = cube_half_width + threshold;
    // plane xy
    norm.Set(0,0,1);
    if(abs(dotProduct(norm, direction)) > 0.00001){
        //The line and the plane xy is not parallel.
        float tHit = dotProduct(norm, center - a)/dotProduct(norm, direction);
        if(tHit > 0){
            //The plane xy is in the front of the line.
            aiVector3D pHit = a + direction*tHit;
            pHit = pHit - center;
            //The intersecting point is near the center or near the axis?
            if (abs(pHit.x) < range && abs(pHit.y) < range)
                return S_CTR;
            else if (abs(pHit.x) < range && abs(pHit.y - axis_length_y) < range)
                return S_Y;
            else if (abs(pHit.y) < range && abs(pHit.x - axis_length_x) < range)
                return S_X;
        }
    }

    // plane xz
    norm.Set(0,1,0);
    if(abs(dotProduct(norm, direction)) > 0.00001){
        //The line and the plane xz is not parallel.
        float tHit = dotProduct(norm, center - a)/dotProduct(norm, direction);
        if(tHit > 0){
            //The plane xz is in the front of the line.
            aiVector3D pHit = a + direction*tHit;
            pHit = pHit - center;
            //The intersecting point is near the center or near the axis?
            if (abs(pHit.x) < range && abs(pHit.z) < range)
                return S_CTR;
            else if (abs(pHit.x) < range && abs(pHit.z - axis_length_z) < range)
                return S_Z;
            else if (abs(pHit.z) < range && abs(pHit.x - axis_length_x) < range)
                return S_X;
        }
    }

    // plane yz
    norm.Set(1,0,0);
    if(abs(dotProduct(norm, direction)) > 0.00001){
        //The line and the plane xy is not parallel.
        float tHit = dotProduct(norm, center - a)/dotProduct(norm, direction);
        if(tHit > 0){
            //The plane xy is in the front of the line.
            aiVector3D pHit = a + direction*tHit;
            pHit = pHit - center;
            //The intersecting point is near the center or near the axis?
            if (abs(pHit.z) < range && abs(pHit.y) < range)
                return S_CTR;
            else if (abs(pHit.z) < range && abs(pHit.y - axis_length_y) < range)
                return S_Y;
            else if (abs(pHit.y) < range && abs(pHit.z - axis_length_z) < range)
                return S_Z;
        }
    }

    return S_NONE;
}

void ScaleOperator::scale(aiVector3D times)
{
    axis_length_x = 24.0 * times.x;
    axis_length_y = 24.0 * times.y;
    axis_length_z = 24.0 * times.z;
}

aiVector3D ScaleOperator::dragToScale(aiVector3D direction, float length)
{
    aiVector3D times(1,1,1);
    aiVector3D feature;

    switch(hitWhere){
    case S_X:{
        feature.Set(0.04,0,0);break;
    }
    case S_Y:{
        feature.Set(0,0.04,0);break;
    }
    case S_Z:{
        feature.Set(0,0,0.04);break;
    }
    case S_CTR:{
        feature.Set(0.06,0.06,0.06);break;
    }
    default:{
        return times;
    }
    }

    direction.Normalize();
    times = direction * length;

    // apply feature
    times.Set(times.x * feature.x + 1, times.y * feature.y + 1, times.z * feature.z + 1);

    if (hitWhere == S_CTR){
        float avg = (times.x + times.y + times.z) / 3.0;
        times.Set(avg, avg, avg);
    }

    // apply on axis
    axis_length_x *= times.x;
    axis_length_y *= times.y;
    axis_length_z *= times.z;
    return times;

}

void ScaleOperator::scaleFinish()
{
    axis_length_x = 24.0;
    axis_length_y = 24.0;
    axis_length_z = 24.0;
}

void ScaleOperator::setBoundingBox(aiVector3D minCorner, aiVector3D maxCorner, aiVector3D gCenter)
{
    this->min = minCorner;
    this->max = maxCorner;
    this->center = gCenter;
    glDeleteLists(render_list, 1);
    render_list = 0;
}

void ScaleOperator::render()
{
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glPushMatrix();

    GLfloat rotate[16] = {
        1, 0, 0, 0,
        0, 0, -1, 0,
        0, 1, 0, 0,
        0, 0, 0, 1
    };
    glMultMatrixf(&rotate[0]);

    glLineWidth(1);
    // x axis
    glBegin(GL_LINES);
        glColor4f(1.0,0.0,0.0,1.0);
        glVertex3f(center.x,center.y,center.z);
        glVertex3f(center.x + axis_length_x,center.y,center.z);
    glEnd();

    // y axis
    glBegin(GL_LINES);
        glColor4f(0.0,1.0,0.0,1.0);
        glVertex3f(center.x,center.y,center.z);
        glVertex3f(center.x,center.y + axis_length_y,center.z);
    glEnd();

    // z axis
    glBegin(GL_LINES);
        glColor4f(0.0,0.0,1.0,1.0);
        glVertex3f(center.x,center.y,center.z);
        glVertex3f(center.x,center.y,center.z + axis_length_z);
    glEnd();

    glEnable(GL_BLEND);
    glBlendFunc( GL_SRC_ALPHA , GL_ONE_MINUS_SRC_ALPHA );

    // center cube
    drawCube(aiVector3D(center.x,center.y,center.z), aiVector3D(0.7,0.7,0.7));

    // x cube
    drawCube(aiVector3D(center.x + axis_length_x, center.y, center.z), aiVector3D(0.95,0.28,0.14));

    // y cube
    drawCube(aiVector3D(center.x, center.y + axis_length_y, center.z), aiVector3D(0.58,0.95,0.26));

    // z cube
    drawCube(aiVector3D(center.x, center.y, center.z + axis_length_z), aiVector3D(0.26,0.57,0.95));

    glPopMatrix();

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);

    glEnable(GL_LIGHTING);
}

void ScaleOperator::drawCube(aiVector3D pos, aiVector3D color)
{
    glPushMatrix();
    glTranslatef(pos.x, pos.y, pos.z);

    glColor4f(color.x, color.y, color.z, 1);
    glutSolidCube(2 * cube_half_width);

    glPopMatrix();
}
