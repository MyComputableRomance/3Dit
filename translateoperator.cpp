#include "translateoperator.h"
#include "ModelLoader.h"

TranslateOperator::TranslateOperator(aiVector3D minCorner, aiVector3D maxCorner, aiVector3D gCenter)
{
    this->min = minCorner;
    this->max = maxCorner;
    this->center = gCenter;
    threshold = 2.0;
    axis_length = 24.0;
    plane_width = 8.0;

    render_list = 0;
    hitWhere = T_NONE;
}

void TranslateOperator::draw()
{
    if (render_list == 0){
        render_list = glGenLists(1);
        glNewList(render_list, GL_COMPILE);
        this->render();
        glEndList();
    }
    glCallList(render_list);
}

bool TranslateOperator::updateHitPosition(aiVector3D a, aiVector3D direction)
{
    hitWhere = this->intersect(a, direction);
    if (hitWhere == T_NONE)
        return false;
    else
        return true;
}

int TranslateOperator::intersect(aiVector3D a, aiVector3D direction)
{
    aiVector3D norm;
    // plane xy
    norm.Set(0,0,1);
    if(abs(dotProduct(norm, direction)) > 0.00001){
        //The line and the plane xy is not parallel.
        float tHit = dotProduct(norm, center - a)/dotProduct(norm, direction);
        if(tHit > 0){
            //The plane xy is in the front of the line.
            aiVector3D pHit = a + direction*tHit;
            pHit = pHit - center;
            //The intersecting point is near the center, near the axis or inside the plane xy?
            if (abs(pHit.x) < threshold && abs(pHit.y) < threshold)
                return T_CTR;
            else if (abs(pHit.x) < threshold && pHit.y > -threshold && pHit.y < axis_length + 3.0f)
                return T_Y;
            else if (abs(pHit.y) < threshold && pHit.x > -threshold && pHit.x < axis_length + 3.0f)
                return T_X;
            else if (pHit.x > -threshold && pHit.x < plane_width && pHit.y > -threshold && pHit.y < plane_width)
                return T_XY;
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
            //The intersecting point is near the center, near the axis or inside the plane xz?
            if (abs(pHit.x) < threshold && abs(pHit.z) < threshold)
                return T_CTR;
            else if (abs(pHit.x) < threshold && pHit.z > -threshold && pHit.z < axis_length + 3.0f)
                return T_Z;
            else if (abs(pHit.z) < threshold && pHit.x > -threshold && pHit.x < axis_length + 3.0f)
                return T_X;
            else if (pHit.x > -threshold && pHit.x < plane_width && pHit.z > -threshold && pHit.z < plane_width)
                return T_XZ;
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
            //The intersecting point is near the center, near the axis or inside the plane xy?
            if (abs(pHit.y) < threshold && abs(pHit.z) < threshold)
                return T_CTR;
            else if (abs(pHit.z) < threshold && pHit.y > -threshold && pHit.y < axis_length + 3.0f)
                return T_Y;
            else if (abs(pHit.y) < threshold && pHit.z > -threshold && pHit.z < axis_length + 3.0f)
                return T_Z;
            else if (pHit.z > -threshold && pHit.z < plane_width && pHit.y > -threshold && pHit.y < plane_width)
                return T_YZ;
        }
    }

    return T_NONE;
}

void TranslateOperator::move(aiVector3D movement)
{
    this->center += movement;
    this->min += movement;
    this->max += movement;
    glDeleteLists(render_list, 1);
    render_list = 0;
}

aiVector3D TranslateOperator::dragToMove(aiVector3D direction, float length)
{
    aiVector3D movement(0,0,0);
    aiVector3D feature;

    switch(hitWhere){
    case T_X:{
        feature.Set(0.3,0,0);break;
    }
    case T_Y:{
        feature.Set(0,0.3,0);break;
    }
    case T_Z:{
        feature.Set(0,0,0.3);break;
    }
    case T_XY:{
        feature.Set(0.212,0.212,0);break;
    }
    case T_XZ:{
        feature.Set(0.212,0,0.212);break;
    }
    case T_YZ:{
        feature.Set(0,0.212,0.212);break;
    }
    case T_CTR:{
        feature.Set(0.173,0.173,0.173);break;
    }
    default:{
        return movement;
    }
    }
    direction.Normalize();
    movement = direction * length;

    // apply feature
    movement.Set(movement.x * feature.x, movement.y * feature.y, movement.z * feature.z);
    return movement;
}

void TranslateOperator::setBoundingBox(aiVector3D minCorner, aiVector3D maxCorner, aiVector3D gCenter)
{
    this->min = minCorner;
    this->max = maxCorner;
    this->center = gCenter;
    glDeleteLists(render_list, 1);
    render_list = 0;
}

void TranslateOperator::render()
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

    // draw center and other point
    glPointSize(10);
    glBegin(GL_POINTS);
        glColor4f(0.7,0.7,0.7,1);
        glVertex3f(center.x,center.y,center.z);
    glEnd();

    // x
    glBegin(GL_POINTS);
        glColor4f(0.95,0.28,0.14,1);
        glVertex3f(center.x + axis_length,center.y,center.z);
    glEnd();

    // y
    glBegin(GL_POINTS);
        glColor4f(0.58,0.95,0.26,1);
        glVertex3f(center.x,center.y + axis_length,center.z);
    glEnd();

    // z
    glBegin(GL_POINTS);
        glColor4f(0.26,0.57,0.95,1);
        glVertex3f(center.x,center.y,center.z + axis_length);
    glEnd();

    glLineWidth(1);
    // x axis
    glBegin(GL_LINES);
        glColor4f(1.0,0.0,0.0,1.0);
        glVertex3f(center.x,center.y,center.z);
        glVertex3f(center.x + axis_length,center.y,center.z);
    glEnd();

    // y axis
    glBegin(GL_LINES);
        glColor4f(0.0,1.0,0.0,1.0);
        glVertex3f(center.x,center.y,center.z);
        glVertex3f(center.x,center.y + axis_length,center.z);
    glEnd();

    // z axis
    glBegin(GL_LINES);
        glColor4f(0.0,0.0,1.0,1.0);
        glVertex3f(center.x,center.y,center.z);
        glVertex3f(center.x,center.y,center.z + axis_length);
    glEnd();

    glEnable(GL_BLEND);
    glBlendFunc( GL_SRC_ALPHA , GL_ONE_MINUS_SRC_ALPHA );

    // xy plane
    glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
    glDisable(GL_CULL_FACE);
    glBegin(GL_POLYGON);
        glColor4f(0.26,0.57,0.95,0.8);
        glVertex3f(center.x,center.y,center.z);
        glVertex3f(center.x + plane_width,center.y,center.z);
        glVertex3f(center.x + plane_width,center.y + plane_width,center.z);
        glVertex3f(center.x,center.y + plane_width,center.z);
    glEnd();

    // xz plane
    glBegin(GL_POLYGON);
        glColor4f(0.58,0.95,0.26,0.8);
        glVertex3f(center.x,center.y,center.z);
        glVertex3f(center.x + plane_width,center.y,center.z);
        glVertex3f(center.x + plane_width,center.y,center.z + plane_width);
        glVertex3f(center.x,center.y,center.z + plane_width);
    glEnd();

    // yz plane
    glBegin(GL_POLYGON);
        glColor4f(0.95,0.28,0.14,0.8);
        glVertex3f(center.x,center.y,center.z);
        glVertex3f(center.x,center.y + plane_width,center.z);
        glVertex3f(center.x,center.y + plane_width,center.z + plane_width);
        glVertex3f(center.x,center.y,center.z + plane_width);
    glEnd();
    glEnable(GL_CULL_FACE);

    glPopMatrix();

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);

    glEnable(GL_LIGHTING);
}

