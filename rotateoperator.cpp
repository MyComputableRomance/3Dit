#include "rotateoperator.h"

RotateOperator::RotateOperator(aiVector3D minCorner, aiVector3D maxCorner, aiVector3D gCenter)
{
    this->min = minCorner;
    this->max = maxCorner;
    this->center = gCenter;
    threshold = 2.5;
    radius = 24.0;
    angle_acc = aiVector3D(0,0,0);

    render_list = 0;
    hitWhere = R_NONE;
}

void RotateOperator::setBoundingBox(aiVector3D minCorner, aiVector3D maxCorner, aiVector3D gCenter)
{
    this->min = minCorner;
    this->max = maxCorner;
    this->center = gCenter;
    glDeleteLists(render_list, 1);
    render_list = 0;
}

void RotateOperator::draw(aiVector3D eye)
{
    if (render_list == 0){
        render_list = glGenLists(1);
        glNewList(render_list, GL_COMPILE);
        this->render(eye);
        glEndList();
    }
    glCallList(render_list);
}

void RotateOperator::rotate(aiVector3D)
{
    // Not need to do anything.
}

void RotateOperator::rotateFinish()
{
    angle_acc.Set(0,0,0);
}

void RotateOperator::viewChange()
{
    glDeleteLists(render_list, 1);
    render_list = 0;
}

bool RotateOperator::updateHitPosition(aiVector3D a, aiVector3D direction)
{
    hitWhere = this->intersect(a, direction);
    if (hitWhere == R_NONE)
        return false;
    else
        return true;
}

int RotateOperator::intersect(aiVector3D a, aiVector3D direction)
{
    // First of all, make a = a - center
    a = a - center;

    /*
     * the line equation
     * x = x0 + tm
     * y = y0 + tn
     * z = z0 + tp
     *
     */

    // calculate the equation to get the hit point(s)
    // equation: ka*x*x + kb*x + kc = 0
    float ka, kb, kc;
    ka = direction.x * direction.x + direction.y * direction.y + direction.z * direction.z;
    kb = 2.0 * (direction.x * a.x + direction.y * a.y + direction.z * a.z);
    kc = a.x * a.x + a.y * a.y + a.z * a.z - radius * radius;

    // calculate delta to see whether the equation has solution(s)
    float delta = kb * kb - 4.0 * ka * kc;
    float sln = 0.0;
    // no solution
    if (delta < -0.0001){
        return R_NONE;
    }
    // one solution
    else if (delta >= -0.0001 && delta < 0.0001){
        // calculate the solution
        sln = - kb / (2.0 * ka);
        if (sln <= 0)
            return R_NONE;
    }
    // two solutions
    else{
        // calculate solutions
        float sqrtDelta = sqrt(delta);
        float doubleKa = 2.0 * ka;
        float sln1 = (-kb + sqrtDelta) / doubleKa;
        float sln2 = (-kb - sqrtDelta) / doubleKa;
        if (sln1 <= 0 && sln2 <= 0)
            return R_NONE;
        else if (sln1 > 0 && sln2 > 0)
            sln = sln2;
        else if (sln1 > 0 && sln2 <= 0)
            sln = sln1;
    }

    // after get the solution in @sln, calculate where the ray hits sphere.
    // the hit point
    aiVector3D pHit = a + direction * sln;
    if (abs(pHit.x) < threshold)
    {
        if (pHit.y > 0 && pHit.z > 0)
            return R_YZ_1;
        else if (pHit.y < 0 && pHit.z > 0)
            return R_YZ_2;
        else if (pHit.y < 0 && pHit.z < 0)
            return R_YZ_3;
        else
            return R_YZ_4;
    }
    else if (abs(pHit.y) < threshold)
    {
        if (pHit.x > 0 && pHit.z > 0)
            return R_XZ_1;
        else if (pHit.x < 0 && pHit.z > 0)
            return R_XZ_2;
        else if (pHit.x < 0 && pHit.z < 0)
            return R_XZ_3;
        else
            return R_XZ_4;
    }
    else if (abs(pHit.z) < threshold)
    {
        if (pHit.x > 0 && pHit.y > 0)
            return R_XY_1;
        else if (pHit.x < 0 && pHit.y > 0)
            return R_XY_2;
        else if (pHit.x < 0 && pHit.y < 0)
            return R_XY_3;
        else
            return R_XY_4;
    }
    else
    {
        if (pHit.x > 0 && pHit.y > 0 && pHit.z > 0)
            return R_SUR_1;
        else if (pHit.x < 0 && pHit.y > 0 && pHit.z > 0)
            return R_SUR_2;
        else if (pHit.x < 0 && pHit.y > 0 && pHit.z < 0)
            return R_SUR_3;
        else if (pHit.x > 0 && pHit.y > 0 && pHit.z < 0)
            return R_SUR_4;
        else if (pHit.x > 0 && pHit.y < 0 && pHit.z > 0)
            return R_SUR_5;
        else if (pHit.x < 0 && pHit.y < 0 && pHit.z > 0)
            return R_SUR_6;
        else if (pHit.x < 0 && pHit.y < 0 && pHit.z < 0)
            return R_SUR_7;
        else if (pHit.x > 0 && pHit.y < 0 && pHit.z < 0)
            return R_SUR_8;
    }
    return R_NONE;
}

aiVector3D RotateOperator::dragToRotate(aiVector3D direction, float length)
{
    aiVector3D angle(0,0,0);

    if (hitWhere == R_NONE)
        return angle;

    direction.Normalize();
    angle = direction * length;
    float rx = 0.0;
    float ry = 0.0;
    float rz = 0.0;

    switch(hitWhere){
    // x axis
    case R_YZ_1:{
        rx = angle.z - angle.y;
        break;
    }
    case R_YZ_2:{
        rx = - angle.z - angle.y;
        break;
    }
    case R_YZ_3:{
        rx = angle.y - angle.z;
        break;
    }
    case R_YZ_4:{
        rx = angle.y + angle.z;
        break;
    }
    // y axis
    case R_XZ_1:{
        ry = angle.x - angle.z;
        break;
    }
    case R_XZ_2:{
        ry = angle.x + angle.z;
        break;
    }
    case R_XZ_3:{
        ry = angle.z - angle.x;
        break;
    }
    case R_XZ_4:{
        ry = - angle.z - angle.x;
        break;
    }
    // z axis
    case R_XY_1:{
        rz = angle.y - angle.x;
        break;
    }
    case R_XY_2:{
        rz = - angle.y - angle.x;
        break;
    }
    case R_XY_3:{
        rz = angle.x - angle.y;
        break;
    }
    case R_XY_4:{
        rz = angle.x + angle.y;
        break;
    }
    // surface
    case R_SUR_1:{
        rx = angle.z - angle.y;
        ry = angle.x - angle.z;
        rz = angle.y - angle.x;
        break;
    }
    case R_SUR_2:{
        rx = angle.z - angle.y;
        ry = angle.x + angle.z;
        rz = - angle.y - angle.x;
        break;
    }
    case R_SUR_3:{
        rx = angle.y + angle.z;
        ry = angle.z - angle.x;
        rz = - angle.y - angle.x;
        break;
    }
    case R_SUR_4:{
        rx = angle.y + angle.z;
        ry = - angle.z - angle.x;
        rz = angle.y - angle.x;
        break;
    }
    case R_SUR_5:{
        rx = - angle.z - angle.y;
        ry = angle.x - angle.z;
        rz = angle.x + angle.y;
        break;
    }
    case R_SUR_6:{
        rx = - angle.z - angle.y;
        ry = angle.x + angle.z;
        rz = angle.x - angle.y;
        break;
    }
    case R_SUR_7:{
        rx = angle.y - angle.z;
        ry = angle.z - angle.x;
        rz = angle.x - angle.y;
        break;
    }
    case R_SUR_8:{
        rx = angle.y - angle.z;
        ry = - angle.z - angle.x;
        rz = angle.x + angle.y;
        break;
    }
    default:{
        return angle;
    }
    }


    // apply feature
    angle.Set(rx * 0.012, ry * 0.012, rz * 0.012);

    angle_acc.x = angle_acc.x - angle.x;
    angle_acc.y = angle_acc.y + angle.y;
    angle_acc.z = angle_acc.z + angle.z;

    angle.Set(angle.x * RTOA, angle.y * RTOA, angle.z * RTOA);
    return angle;
}

void RotateOperator::render(aiVector3D eye)
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

    glTranslatef(center.x, center.y, center.z);
    // draw center
    glPointSize(8);
    glBegin(GL_POINTS);
        glColor4f(0.7,0.7,0.7,1);
        glVertex3i(0,0,0);
    glEnd();

    glLineWidth(1);

    // set new eye position
    eye.Set(eye.x, -eye.z ,eye.y);

    drawCircle(eye);

    // draw circle
    float a;

    glEnable(GL_BLEND);
    glBlendFunc( GL_SRC_ALPHA , GL_ONE_MINUS_SRC_ALPHA );

    glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
    glDisable(GL_CULL_FACE);

    // draw the transparent face
    if (angle_acc.x != 0){
        int times = angle_acc.x / ANGL_ACC;
        if (times != 0){
            // allocate times in -180 ~ 180
            times = times > LINE_NUM ? LINE_NUM : times;
            times = times < -LINE_NUM ? -LINE_NUM : times;

            if (times > 0){
                a = 0.0;
                glBegin(GL_TRIANGLE_FAN);
                    glColor4f(0.95,0.28,0.14,0.6);
                    glVertex3f(0.0,0.0,0.0);
                    for (int i=0;i<times;i++){
                        float tmpz = radius * cos(a);
                        float tmpy = radius * sin(a);
                        glVertex3f(0, tmpy, tmpz);
                        a += ANGL_ACC;
                    }
                glEnd();
            }
            else if (times < 0){
                a = 0.0;
                glBegin(GL_TRIANGLE_FAN);
                    glColor4f(0.95,0.28,0.14,0.6);
                    glVertex3f(0.0,0.0,0.0);
                    for (int i=times;i<0;i++){
                        float tmpz = radius * cos(a);
                        float tmpy = radius * sin(a);
                        glVertex3f(0, tmpy, tmpz);
                        a -= ANGL_ACC;
                    }
                glEnd();
            }
        }
    }

    if (angle_acc.y != 0){
        int times = angle_acc.y / ANGL_ACC;
        if (times != 0){
            // allocate times in -180 ~ 180
            times = times > LINE_NUM ? LINE_NUM : times;
            times = times < -LINE_NUM ? -LINE_NUM : times;

            if (times > 0){
                a = 0.0;
                glBegin(GL_TRIANGLE_FAN);
                    glColor4f(0.58,0.95,0.26,0.6);
                    glVertex3f(0.0,0.0,0.0);
                    for (int i=0;i<times;i++){
                        float tmpz = radius * cos(a);
                        float tmpx = radius * sin(a);
                        glVertex3f(tmpx, 0, tmpz);
                        a += ANGL_ACC;
                    }
                glEnd();
            }
            else if (times < 0){
                a = 0.0;
                glBegin(GL_TRIANGLE_FAN);
                    glColor4f(0.58,0.95,0.26,0.6);
                    glVertex3f(0.0,0.0,0.0);
                    for (int i=times;i<0;i++){
                        float tmpz = radius * cos(a);
                        float tmpx = radius * sin(a);
                        glVertex3f(tmpx, 0, tmpz);
                        a -= ANGL_ACC;
                    }
                glEnd();
            }
        }
    }

    if (angle_acc.z != 0){
        int times = angle_acc.z / ANGL_ACC;
        if (times != 0){
            // allocate times in -180 ~ 180
            times = times > LINE_NUM ? LINE_NUM : times;
            times = times < -LINE_NUM ? -LINE_NUM : times;

            if (times > 0){
                a = 0.0;
                glBegin(GL_TRIANGLE_FAN);
                    glColor4f(0.26,0.57,0.95,0.6);
                    glVertex3f(0.0,0.0,0.0);
                    for (int i=0;i<times;i++){
                        float tmpx = radius * cos(a);
                        float tmpy = radius * sin(a);
                        glVertex3f(tmpx, tmpy, 0);
                        a += ANGL_ACC;
                    }
                glEnd();
            }
            else if (times < 0){
                a = 0.0;
                glBegin(GL_TRIANGLE_FAN);
                    glColor4f(0.26,0.57,0.95,0.6);
                    glVertex3f(0.0,0.0,0.0);
                    for (int i=times;i<0;i++){
                        float tmpx = radius * cos(a);
                        float tmpy = radius * sin(a);
                        glVertex3f(tmpx, tmpy, 0);
                        a -= ANGL_ACC;
                    }
                glEnd();
            }
        }
    }

    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);

    glPopMatrix();
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
}


#define EQUALZERO(x) ((x < 0.0001 && x >-0.0001) ? true : false)
// define the location of the circle
#define CIRCLE_FREE 0
#define CIRCLE_YZ   1
#define CIRCLE_XZ   2
#define CIRCLE_XY   3

void RotateOperator::drawCircle(aiVector3D eye)
{
    aiVector3D ray;
    ray.x = this->center.x - eye.x;
    ray.y = this->center.y - eye.y;
    ray.z = this->center.z - eye.z;
    float m = ray.x; float m2 = m*m;
    float n = ray.y; float n2 = n*n;
    float p = ray.z; float p2 = p*p;
    int cloc = CIRCLE_FREE; // circle location

    // plane:   mx + ny + pz = 0
    // sphere:  x2 + y2 + z2 = r2

    // calculate the intersection points
    aiVector3D yz1,yz2;
    aiVector3D xz1,xz2;
    aiVector3D xy1,xy2;

    // caculate circle location
    if (EQUALZERO(m2) && EQUALZERO(n2) && EQUALZERO(p2))
        return;
    else if (EQUALZERO(m2) && EQUALZERO(n2))
        cloc = CIRCLE_XY;
    else if (EQUALZERO(m2) && EQUALZERO(p2))
        cloc = CIRCLE_XZ;
    else if (EQUALZERO(n2) && EQUALZERO(p2))
        cloc = CIRCLE_XY;
    else
        cloc = CIRCLE_FREE;

    // calculate the intersection points
    // yz plane:
    if (cloc != CIRCLE_YZ){
        yz1.x = 0;
        yz1.y = -(p*radius)/sqrt(p2+n2);
        yz1.z = -(n * yz1.y/p);

        yz2.x = 0;
        yz2.y = - yz1.y;
        yz2.z = - yz1.z;
    }
    // xz plane
    if (cloc != CIRCLE_XZ){
        xz1.y = 0;
        xz1.x = -(p*radius)/sqrt(p2+m2);
        xz1.z = -(m * xz1.x/p);

        xz2.y = 0;
        xz2.x = - xz1.x;
        xz2.z = - xz1.z;
    }
    // xy plane
    if (cloc != CIRCLE_XY){
        xy1.z = 0;
        xy1.x = -(n*radius)/sqrt(n2+m2);
        xy1.y = -(m * xy1.x/n);

        xy2.z = 0;
        xy2.x = - xy1.x;
        xy2.y = - xy1.y;
    }

    glLineWidth(2);
    // draw circle
    float a;
    // x-y circle
    if (cloc == CIRCLE_XY){
        // draw
        a=0;
        glBegin(GL_LINE_STRIP);
            glColor4f(0.26,0.57,0.95,1);
            for (int i=0;i<LINE_NUM;i++){
                float tmpx = radius * cos(a);
                float tmpy = radius * sin(a);
                glVertex3f(tmpx, tmpy, 0);
                a += ANGL_ACC;
            }
        glEnd();

        // draw the out ring at same time

        a=0;
        float outR = radius + 8.0f;
        glBegin(GL_LINE_STRIP);
            glColor4f(0.78,0.28,0.70,1);
            for (int i=0;i<LINE_NUM;i++){
                float tmpx = outR * cos(a);
                float tmpy = outR * sin(a);
                glVertex3f(tmpx, tmpy, 0);
                a += ANGL_ACC;
            }
        glEnd();
    }
    else {
        float sinA = xy1.y / radius;
        float cosA = xy1.x / radius;
        float A;
        A = abs(asin(sinA));
        //if (sinA > 0 && cosA > 0) do nothing
        if (sinA > 0 && cosA < 0)
            A = PI - A;
        else if (sinA < 0 && cosA < 0)
            A = PI + A;
        else if (sinA < 0 && cosA > 0)
            A = 2.0f * PI - A;

        float tmpA = A + ANGL_ACC_10;
        aiVector3D tmpRay(cos(tmpA),sin(tmpA),0.0f);
        float tmpDot = ray.x * tmpRay.x + ray.y * tmpRay.y;
        if (tmpDot > 0){
            // A+ is back, A- is front
            //a=A;
            a = A + PI;
            glBegin(GL_LINE_STRIP);
                glColor4f(0.26,0.57,0.95,1);
                for (int i=0;i<LINE_NUM_HALF;i++){
                    float tmpx = radius * cos(a);
                    float tmpy = radius * sin(a);
                    glVertex3f(tmpx, tmpy, 0);
                    a += ANGL_ACC;
                }
            glEnd();
        }
        else {
            // A+ is front, A- is back
            a=A;
            glBegin(GL_LINE_STRIP);
                glColor4f(0.26,0.57,0.95,1);
                for (int i=0;i<LINE_NUM_HALF;i++){
                    float tmpx = radius * cos(a);
                    float tmpy = radius * sin(a);
                    glVertex3f(tmpx, tmpy, 0);
                    a += ANGL_ACC;
                }
            glEnd();
        }
    }

    // x-z circle
    if (cloc == CIRCLE_XZ){
        // draw
        a=0;
        glBegin(GL_LINE_STRIP);
            glColor4f(0.58,0.95,0.26,1);
            for (int i=0;i<LINE_NUM;i++){
                float tmpz = radius * cos(a);
                float tmpx = radius * sin(a);
                glVertex3f(tmpx, 0, tmpz);
                a += ANGL_ACC;
            }
        glEnd();

        // draw the out ring at same time

        a=0;
        float outR = radius + 8.0f;
        glBegin(GL_LINE_STRIP);
            glColor4f(0.78,0.28,0.70,1);
            for (int i=0;i<LINE_NUM;i++){
                float tmpz = outR * cos(a);
                float tmpx = outR * sin(a);
                glVertex3f(tmpx, 0, tmpz);
                a += ANGL_ACC;
            }
        glEnd();
    }
    else {
        float sinA = xz1.x / radius;
        float cosA = xz1.z / radius;
        float A;
        A = abs(asin(sinA));
        //if (sinA > 0 && cosA > 0) do nothing
        if (sinA > 0 && cosA < 0)
            A = PI - A;
        else if (sinA < 0 && cosA < 0)
            A = PI + A;
        else if (sinA < 0 && cosA > 0)
            A = 2.0f * PI - A;

        float tmpA = A + ANGL_ACC_10;
        aiVector3D tmpRay(sin(tmpA),0.0f, cos(tmpA));
        float tmpDot = ray.x * tmpRay.x + ray.z * tmpRay.z;
        if (tmpDot > 0){
            // A+ is back, A- is front
            a = A + PI;
            glBegin(GL_LINE_STRIP);
                glColor4f(0.58,0.95,0.26,1);
                for (int i=0;i<LINE_NUM_HALF;i++){
                    float tmpz = radius * cos(a);
                    float tmpx = radius * sin(a);
                    glVertex3f(tmpx, 0, tmpz);
                    a += ANGL_ACC;
                }
            glEnd();
        }
        else {
            // A+ is front, A- is back
            a=A;
            glBegin(GL_LINE_STRIP);
                glColor4f(0.58,0.95,0.26,1);
                for (int i=0;i<LINE_NUM_HALF;i++){
                    float tmpz = radius * cos(a);
                    float tmpx = radius * sin(a);
                    glVertex3f(tmpx, 0, tmpz);
                    a += ANGL_ACC;
                }
            glEnd();
        }
    }

    // y-z circle
    if (cloc == CIRCLE_YZ){
        // draw
        a=0;
        glBegin(GL_LINE_STRIP);
            glColor4f(0.95,0.28,0.14,1);
            for (int i=0;i<LINE_NUM;i++){
                float tmpz = radius * cos(a);
                float tmpy = radius * sin(a);
                glVertex3f(0, tmpy, tmpz);
                a += ANGL_ACC;
            }
        glEnd();

        // draw the out ring at same time

        a=0;
        float outR = radius + 8.0f;
        glBegin(GL_LINE_STRIP);
            glColor4f(0.78,0.28,0.70,1);
            for (int i=0;i<LINE_NUM;i++){
                float tmpz = outR * cos(a);
                float tmpy = outR * sin(a);
                glVertex3f(0, tmpy, tmpz);
                a += ANGL_ACC;
            }
        glEnd();
    }
    else {
        float sinA = yz1.y / radius;
        float cosA = yz1.z / radius;
        float A;
        A = abs(asin(sinA));
        //if (sinA > 0 && cosA > 0) do nothing
        if (sinA > 0 && cosA < 0)
            A = PI - A;
        else if (sinA < 0 && cosA < 0)
            A = PI + A;
        else if (sinA < 0 && cosA > 0)
            A = 2.0f * PI - A;

        float tmpA = A + ANGL_ACC_10;
        aiVector3D tmpRay(0.0f,sin(tmpA),cos(tmpA));
        float tmpDot = ray.z * tmpRay.z + ray.y * tmpRay.y;
        if (tmpDot > 0){
            // A+ is back, A- is front
            a = A + PI;
            glBegin(GL_LINE_STRIP);
                glColor4f(0.95,0.28,0.14,1);
                for (int i=0;i<LINE_NUM_HALF;i++){
                    float tmpz = radius * cos(a);
                    float tmpy = radius * sin(a);
                    glVertex3f(0, tmpy, tmpz);
                    a += ANGL_ACC;
                }
            glEnd();
        }
        else {
            // A+ is front, A- is back
            a=A;
            glBegin(GL_LINE_STRIP);
                glColor4f(0.95,0.28,0.14,1);
                for (int i=0;i<LINE_NUM_HALF;i++){
                    float tmpz = radius * cos(a);
                    float tmpy = radius * sin(a);
                    glVertex3f(0, tmpy, tmpz);
                    a += ANGL_ACC;
                }
            glEnd();
        }
    }

    if (cloc == CIRCLE_FREE){
        // draw out ring
        float sinA = xz1.x / radius;
        float cosA = xz1.z / radius;
        float A;
        A = abs(asin(sinA));
        //if (sinA > 0 && cosA > 0) do nothing
        if (sinA > 0 && cosA < 0)
            A = PI - A;
        else if (sinA < 0 && cosA < 0)
            A = PI + A;
        else if (sinA < 0 && cosA > 0)
            A = 2.0f * PI - A;

        float sinB = yz1.y / radius;
        float cosB = yz1.z / radius;
        float B;
        B = abs(asin(sinB));
        //if (sinB > 0 && cosB > 0) do nothing
        if (sinB > 0 && cosB < 0)
            B = PI - B;
        else if (sinB < 0 && cosB < 0)
            B = PI + B;
        else if (sinB < 0 && cosB > 0)
            B = 2.0f * PI - B;

        float sinC = xy1.y / radius;
        float cosC = xy1.x / radius;
        float C;
        C = abs(asin(sinC));
        //if (sinC > 0 && cosC > 0) do nothing
        if (sinC > 0 && cosC < 0)
            C = PI - C;
        else if (sinC < 0 && cosC < 0)
            C = PI + C;
        else if (sinC < 0 && cosC > 0)
            C = 2.0f * PI - C;

        glPushMatrix();

        float cosTheta = - p / (sqrt(m2 + n2 + p2));
        float theta = acos(cosTheta);
        aiVector3D axis(n, -m, 0);

        glRotatef(theta * 180.0f / PI, axis.x, axis.y, axis.z);

        float a=0;
        float outR = radius + 8.0f;
        glBegin(GL_LINE_STRIP);
            glColor4f(0.78,0.28,0.70,1);
            for (int i=0;i<=LINE_NUM;i++){
                float tmpx = outR * cos(a);
                float tmpy = outR * sin(a);
                glVertex3f(tmpx, tmpy, 0);
                a += ANGL_ACC;
            }
        glEnd();

        a=0;
        outR = radius;
        glBegin(GL_LINE_STRIP);
            glColor4f(0.98,0.48,0.99,1);
            for (int i=0;i<=LINE_NUM;i++){
                float tmpx = outR * cos(a);
                float tmpy = outR * sin(a);
                glVertex3f(tmpx, tmpy, 0);
                a += ANGL_ACC;
            }
        glEnd();

        glPopMatrix();
    }
}
