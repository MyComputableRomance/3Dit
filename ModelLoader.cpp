#include "ModelLoader.h"
#include <QString>
#include <math.h>

ModelLoader::ModelLoader()
{
    loaded = false;
    isDuplicated = false;

    sceneMeshIndex = -1;
    meshFaceIndex = -1;
    selected = false;

    scene_list = 0;
    mesh_list = 0;

    selection = new QVector<Selection>();
}

ModelLoader::~ModelLoader()
{
    delete selection;
    if(isDuplicated)
        delete scene;
}

bool ModelLoader::load(const char *filename)
{
    scene = aiImportFile(filename,aiProcessPreset_TargetRealtime_MaxQuality);

    if (scene) {
        getBoundingBox(&scene_min,&scene_max);

        loaded = true;
        return true;
    }
    return false;
}

bool ModelLoader::doExport(const char *filename)
{
    aiExportScene(scene,"stl",filename,aiProcessPreset_TargetRealtime_MaxQuality);
    return true;
}

void ModelLoader::updateBoundingBox()
{
    getBoundingBox(&scene_min,&scene_max);
}

void ModelLoader::getBoundingBoxForNode(const aiNode *nd, aiVector3D *min, aiVector3D *max, aiMatrix4x4 *trafo)
{
    aiMatrix4x4 prev;
    unsigned int n = 0, t;

    prev = *trafo;
    aiMultiplyMatrix4(trafo,&nd->mTransformation);

    for (; n < nd->mNumMeshes; ++n) {
        const aiMesh* mesh = scene->mMeshes[nd->mMeshes[n]];
        for (t = 0; t < mesh->mNumVertices; ++t) {

            aiVector3D tmp = mesh->mVertices[t];
            aiTransformVecByMatrix4(&tmp,trafo);

            min->x = aisgl_min(min->x,tmp.x);
            min->y = aisgl_min(min->y,tmp.y);
            min->z = aisgl_min(min->z,tmp.z);

            max->x = aisgl_max(max->x,tmp.x);
            max->y = aisgl_max(max->y,tmp.y);
            max->z = aisgl_max(max->z,tmp.z);

            geometry_center.x += tmp.x;
            geometry_center.y += tmp.y;
            geometry_center.z += tmp.z;

            total_num += 1;
        }
    }

    for (n = 0; n < nd->mNumChildren; ++n) {
        getBoundingBoxForNode(nd->mChildren[n],min,max,trafo);
    }
    *trafo = prev;
}

void ModelLoader::getBoundingBox(aiVector3D *min, aiVector3D *max)
{
    aiMatrix4x4 trafo;
    aiIdentityMatrix4(&trafo);

    min->x = min->y = min->z =  1e10f;
    max->x = max->y = max->z = -1e10f;

    total_num = 0;
    geometry_center.Set(0,0,0);
    getBoundingBoxForNode(scene->mRootNode,min,max,&trafo);

    // Get geometry center:
    geometry_center.x /= (total_num * 1.0f);
    geometry_center.y /= (total_num * 1.0f);
    geometry_center.z /= (total_num * 1.0f);

    // Get box center:
    box_center.x = (scene_max.x + scene_min.x)/2;
    box_center.y = (scene_max.y + scene_min.y)/2;
    box_center.z = (scene_max.z + scene_min.z)/2;
}

void ModelLoader::color4ToFloat4(const aiColor4D *c, float f[])
{
    f[0] = c->r;
    f[1] = c->g;
    f[2] = c->b;
    f[3] = c->a;
}

void ModelLoader::setFloat4(float f[], float a, float b, float c, float d)
{
    f[0] = a;
    f[1] = b;
    f[2] = c;
    f[3] = d;
}

void ModelLoader::applyMaterial(const aiMaterial *mtl)
{
    float c[4];

    GLenum fill_mode;
    int ret1, ret2;
    aiColor4D diffuse;
    aiColor4D specular;
    aiColor4D ambient;
    aiColor4D emission;
    float shininess, strength;
    int two_sided;
    int wireframe;
    unsigned int max;

    setFloat4(c, 0.8f, 0.8f, 0.8f, 1.0f);
    if(AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_DIFFUSE, &diffuse))
        color4ToFloat4(&diffuse, c);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, c);

    setFloat4(c, 0.0f, 0.0f, 0.0f, 1.0f);
    if(AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_SPECULAR, &specular))
        color4ToFloat4(&specular, c);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, c);

    setFloat4(c, 0.2f, 0.2f, 0.2f, 1.0f);
    if(AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_AMBIENT, &ambient))
        color4ToFloat4(&ambient, c);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, c);

    setFloat4(c, 0.0f, 0.0f, 0.0f, 1.0f);
    if(AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_EMISSIVE, &emission))
        color4ToFloat4(&emission, c);
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, c);

    max = 1;
    ret1 = aiGetMaterialFloatArray(mtl, AI_MATKEY_SHININESS, &shininess, &max);
    if(ret1 == AI_SUCCESS) {
        max = 1;
        ret2 = aiGetMaterialFloatArray(mtl, AI_MATKEY_SHININESS_STRENGTH, &strength, &max);
        if(ret2 == AI_SUCCESS)
            glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess * strength);
        else
            glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
    }
    else {
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 0.0f);
        setFloat4(c, 0.0f, 0.0f, 0.0f, 0.0f);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, c);
    }

    max = 1;
    if(AI_SUCCESS == aiGetMaterialIntegerArray(mtl, AI_MATKEY_ENABLE_WIREFRAME, &wireframe, &max))
        fill_mode = wireframe ? GL_LINE : GL_FILL;
    else
        fill_mode = GL_FILL;
    glPolygonMode(GL_FRONT_AND_BACK, fill_mode);

    max = 1;
    if((AI_SUCCESS == aiGetMaterialIntegerArray(mtl, AI_MATKEY_TWOSIDED, &two_sided, &max)) && two_sided)
        glDisable(GL_CULL_FACE);
    else
        glEnable(GL_CULL_FACE);
}

void ModelLoader::recursiveRender(const aiScene *sc, const aiNode *nd)
{
    unsigned int i;
    unsigned int n = 0, t;

    // update transform
    glPushMatrix();
        glPushMatrix();
            GLfloat rotate[16] = {
                1, 0, 0, 0,
                0, 0, -1, 0,
                0, 1, 0, 0,
                0, 0, 0, 1
            };
            glMultMatrixf(&rotate[0]);

            // draw all meshes assigned to this node
            for (; n < nd->mNumMeshes; ++n) {
                const aiMesh* mesh = scene->mMeshes[nd->mMeshes[n]];
                for (t = 0; t < mesh->mNumFaces; ++t) {
                    const aiFace* face = &mesh->mFaces[t];
                    GLenum face_mode;

                    switch(face->mNumIndices) {
                        case 1: face_mode = GL_POINTS; break;
                        case 2: face_mode = GL_LINES; break;
                        case 3: face_mode = GL_TRIANGLES; break;
                        default: face_mode = GL_POLYGON; break;
                    }

                    glBegin(face_mode);

                    //GL_PICK: isSelected?
                    if(sceneMeshIndex == -999){
                        glColor4f(1.0f,0.35f,0.0f,1.0f);
                    }else{
                        bool flag = false;
                        for(int index = 0; index < selection->size(); index++){
                            if(selection->at(index).face == face){
                                flag = true;
                                break;
                            }
                        }
                        if(flag)
                            glColor4f(1.0f,0.35f,0.0f,1.0f);
                        else
                            glColor4f(0.3f,0.3f,0.3f,1.0f);
                    }

                    for(i = 0; i < face->mNumIndices; i++) {
                        int index = face->mIndices[i];
                        if(mesh->mNormals != NULL)
                           glNormal3fv(&mesh->mNormals[index].x);
					
                        // Rotation -> Scale -> Translation
                        float x = mesh->mVertices[index].x;
                        float y = mesh->mVertices[index].y;
                        float z = mesh->mVertices[index].z;

                        glVertex3f(x,y,z);
                    }
                    glEnd();
                }
            }
        glPopMatrix();
        // draw all children
        for (n = 0; n < nd->mNumChildren; ++n) {
            recursiveRender(sc, nd->mChildren[n]);
        }

    glPopMatrix();
}

void ModelLoader::render()
{
    if(scene_list == 0) {
        scene_list = glGenLists(1);
        glNewList(scene_list, GL_COMPILE);
            // now begin at the root node of the imported data and traverse
            // the scenegraph by multiplying subsequent local transforms
            // together on GL's matrix stack.
        recursiveRender(scene, scene->mRootNode);
        glEndList();
    }
    glCallList(scene_list);
}

void ModelLoader::recursiveRenderMesh(const aiScene *sc, const aiNode *nd)
{
    unsigned int i;
    unsigned int n = 0, t;

    // update transform
    glPushMatrix();
        glPushMatrix();
            GLfloat rotate[16] = {
                1, 0, 0, 0,
                0, 0, -1, 0,
                0, 1, 0, 0,
                0, 0, 0, 1
            };
            glMultMatrixf(&rotate[0]);
            glLineWidth(0.5);

            // draw all meshes assigned to this node
            for (; n < nd->mNumMeshes; ++n) {
                const aiMesh* mesh = scene->mMeshes[nd->mMeshes[n]];
                for (t = 0; t < mesh->mNumFaces; ++t) {
                    const aiFace* face = &mesh->mFaces[t];
                    GLenum face_mode;

                    switch(face->mNumIndices) {
                        case 1: face_mode = GL_POINTS; break;
                        case 2: face_mode = GL_LINES; break;
                        default: face_mode = GL_LINE_STRIP; break;
                    }

                    glBegin(face_mode);

                    //Mesh Color:
                    glColor4f(0.58f, 0.95f, 0.26f, 1.0f);

                    for(i = 0; i < face->mNumIndices; i++) {
                        int index = face->mIndices[i];

                        // Rotation -> Scale -> Translation
                        float x = mesh->mVertices[index].x;
                        float y = mesh->mVertices[index].y;
                        float z = mesh->mVertices[index].z;

                        glVertex3f(x,y,z);
                    }
                    glEnd();
                }
            }
        glPopMatrix();
        // draw all children
        for (n = 0; n < nd->mNumChildren; ++n) {
            recursiveRenderMesh(sc, nd->mChildren[n]);
        }

    glPopMatrix();
}

void ModelLoader::renderMesh()
{
    if(selected){
        if(mesh_list == 0) {
            mesh_list = glGenLists(1);
            glNewList(mesh_list, GL_COMPILE);
                // now begin at the root node of the imported data and traverse
                // the scenegraph by multiplying subsequent local transforms
                // together on GL's matrix stack.
            recursiveRenderMesh(scene, scene->mRootNode);
            glEndList();
        }
        glCallList(mesh_list);
    }
}

void ModelLoader::recursiveScale(const aiScene *sc, const aiNode *nd,float xt, float yt, float zt, aiVector3D ctr)
{
    unsigned int n = 0, t;
    for (; n < nd->mNumMeshes; ++n) {
        const aiMesh* mesh = scene->mMeshes[nd->mMeshes[n]];
        for (t = 0; t < mesh->mNumVertices; ++t) {
            float x = mesh->mVertices[t].x;
            float y = mesh->mVertices[t].y;
            float z = mesh->mVertices[t].z;

            mesh->mVertices[t].x = ctr.x + (x - ctr.x) * xt;
            mesh->mVertices[t].y = ctr.y + (y - ctr.y) * yt;
            mesh->mVertices[t].z = ctr.z + (z - ctr.z) * zt;
        }
    }
	
	// if xt, yt and zt are not same, then re-calculate the normals
    if (!(xt == yt && xt == zt)){
        for (unsigned int n=0; n < nd->mNumMeshes; ++n) {
            const aiMesh* mesh = sc->mMeshes[nd->mMeshes[n]];
            for (unsigned int t = 0; t < mesh->mNumFaces; ++t) {
                const aiFace* face = &mesh->mFaces[t];

                // ignore all types except triangle
                if (face->mNumIndices == 3){
                    // get the 3 vertices
                    aiVector3D a = mesh->mVertices[face->mIndices[0]];
                    aiVector3D b = mesh->mVertices[face->mIndices[1]];
                    aiVector3D c = mesh->mVertices[face->mIndices[2]];

                    // caculate the normal
                    aiVector3D ab = b - a;
                    aiVector3D ac = c - a;
                    aiVector3D normal(ab.y*ac.z-ab.z*ac.y, ab.z*ac.x-ab.x*ac.z, ab.x*ac.y-ab.y*ac.x);
                    normal.Normalize();

                    // update the normals
                    mesh->mNormals[face->mIndices[0]].Set(normal.x, normal.y, normal.z);
                    mesh->mNormals[face->mIndices[1]].Set(normal.x, normal.y, normal.z);
                    mesh->mNormals[face->mIndices[2]].Set(normal.x, normal.y, normal.z);
                }
            }
        }
    }

    for (n = 0; n < nd->mNumChildren; ++n) {
        recursiveScale(sc, nd->mChildren[n], xt, yt, zt, ctr);
    }
}

void ModelLoader::scale(float xt, float yt, float zt, aiVector3D ctr)
{
    /*
     * Do scale at [geometry center].
     * In single-model-scale, the geometry center will not change.
     * In multi-model-scale, the geometry center will change
     */
    recursiveScale(scene, scene->mRootNode, xt, yt, zt, ctr);

    glDeleteLists(scene_list,1);
    scene_list = 0;
    glDeleteLists(mesh_list,1);
    mesh_list = 0;
}

void ModelLoader::recursivePosition(const aiScene *sc, const aiNode *nd, float nx, float ny, float nz)
{

    unsigned int n = 0, t;

    for (; n < nd->mNumMeshes; ++n) {
        const aiMesh* mesh = scene->mMeshes[nd->mMeshes[n]];
        for (t = 0; t < mesh->mNumVertices; ++t) {
            mesh->mVertices[t].x += nx;
            mesh->mVertices[t].y += ny;
            mesh->mVertices[t].z += nz;
        }
    }

    for (n = 0; n < nd->mNumChildren; ++n) {
        recursivePosition(sc, nd->mChildren[n], nx, ny, nz);
    }
}

void ModelLoader::position(float nx, float ny, float nz)
{
    recursivePosition(scene, scene->mRootNode, nx, ny, nz);

    glDeleteLists(scene_list,1);
    scene_list = 0;
    glDeleteLists(mesh_list,1);
    mesh_list = 0;
}

void ModelLoader::recursiveRotation(const aiScene *sc, const aiNode *nd, float rx, float ry, float rz, aiVector3D ctr)
{
    unsigned int n = 0, t;

    for (; n < nd->mNumMeshes; ++n) {
        const aiMesh* mesh = scene->mMeshes[nd->mMeshes[n]];
        for (t = 0; t < mesh->mNumVertices; ++t) {
            float x = mesh->mVertices[t].x;
            float y = mesh->mVertices[t].y;
            float z = mesh->mVertices[t].z;

            float nx, ny, nz;

            // to center
            x -= ctr.x;
            y -= ctr.y;
            z -= ctr.z;

            // x
            nx = x;
            ny = y*cos(rx) - z*sin(rx);
            nz = y*sin(rx) + z*cos(rx);
            x = nx;
            y = ny;
            z = nz;

            // y
            nx = z*sin(ry) + x*cos(ry);
            ny = y;
            nz = z*cos(ry) - x*sin(ry);
            x = nx;
            y = ny;
            z = nz;

            // z
            nx = x*cos(rz) - y*sin(rz);
            ny = x*sin(rz) + y*cos(rz);
            nz = z;
            x = nx;
            y = ny;
            z = nz;

            // back
            x += ctr.x;
            y += ctr.y;
            z += ctr.z;

            mesh->mVertices[t].x = x;
            mesh->mVertices[t].y = y;
            mesh->mVertices[t].z = z;
			
			// rotate the normals
            x = mesh->mNormals[t].x;
            y = mesh->mNormals[t].y;
            z = mesh->mNormals[t].z;

            // x
            nx = x;
            ny = y*cos(rx) - z*sin(rx);
            nz = y*sin(rx) + z*cos(rx);
            x = nx;
            y = ny;
            z = nz;

            // y
            nx = z*sin(ry) + x*cos(ry);
            ny = y;
            nz = z*cos(ry) - x*sin(ry);
            x = nx;
            y = ny;
            z = nz;

            // z
            nx = x*cos(rz) - y*sin(rz);
            ny = x*sin(rz) + y*cos(rz);
            nz = z;
            x = nx;
            y = ny;
            z = nz;

            mesh->mNormals[t].Set(x,y,z);
            mesh->mNormals[t].Normalize();
        }
    }

    for (n = 0; n < nd->mNumChildren; ++n) {
        recursiveRotation(sc, nd->mChildren[n], rx, ry, rz, ctr);
    }
}

void ModelLoader::rotation(float rx, float ry, float rz, aiVector3D ctr)
{
    /*
     * Do rotate at [geometry center].
     * In single-model-rotate, the geometry center will not change.
     * In multi-model-rotate, the geometry center will change
     */
    recursiveRotation(scene, scene->mRootNode, rx, ry, rz, ctr);

    glDeleteLists(scene_list,1);
    scene_list = 0;
    glDeleteLists(mesh_list,1);
    mesh_list = 0;
}

//GL_PICK
void ModelLoader::recursiveIntersect(const aiScene *sc, const aiNode *nd, aiVector3D a, aiVector3D direction, float& min_tHit, int& sceneMeshIndex, int& meshFaceIndex)
{
    unsigned int i;
    unsigned int n = 0, t;
    for (; n < nd->mNumMeshes; ++n) {
        const aiMesh* mesh = scene->mMeshes[nd->mMeshes[n]];
        for (t = 0; t < mesh->mNumFaces; ++t) {
            const aiFace* face = &mesh->mFaces[t];
            if (face->mNumIndices == 3){ //Must be triangle faces.
                aiVector3D vertices[3];
                for(i = 0; i < face->mNumIndices; i++) {
                    int index = face->mIndices[i];
                    vertices[i] = mesh->mVertices[index];
                }
                //Calculate normal:
                aiVector3D b1 = vertices[1] - vertices[0];
                aiVector3D b2 = vertices[2] - vertices[0];
                aiVector3D norm = crossProduct(b1, b2);
                //Calculate intersecting point:
                if(abs(dotProduct(norm, direction)) > 0.00001){
                    //The line and the plane is not parallel.
                    float tHit = dotProduct(norm, vertices[0] - a)/dotProduct(norm, direction);
                    if(tHit > 0){
                        if(min_tHit == -1 || tHit < min_tHit){
                            //The plane is in the front of the line.
                            aiVector3D pHit = a + direction*tHit;
                            //The intersecting point is inside the triangle?
                            aiVector3D c1 = crossProduct(vertices[0] - pHit, vertices[1] - vertices[0]);
                            aiVector3D c2 = crossProduct(vertices[1] - pHit, vertices[2] - vertices[1]);
                            aiVector3D c3 = crossProduct(vertices[2] - pHit, vertices[0] - vertices[2]);
                            if(dotProduct(c1, norm) >= 0 && dotProduct(c2, norm) >= 0 && dotProduct(c3, norm) >= 0){
                                //Update new tHit:
                                min_tHit = tHit;
                                sceneMeshIndex = nd->mMeshes[n];
                                meshFaceIndex = t;
                            }
                        }
                    }
                }
            }
        }
    }

    for (n = 0; n < nd->mNumChildren; ++n) {
        recursiveIntersect(sc, nd->mChildren[n], a, direction, min_tHit, sceneMeshIndex, meshFaceIndex);
    }
}

void ModelLoader::intersect(aiVector3D a, aiVector3D direction, float& min_tHit, int& sceneMeshIndex, int& meshFaceIndex){
    min_tHit = -1;
    sceneMeshIndex = -1;
    meshFaceIndex = -1;

    recursiveIntersect(scene, scene->mRootNode, a, direction, min_tHit, sceneMeshIndex, meshFaceIndex);
    glDeleteLists(scene_list,1);
    scene_list = 0;
}

bool ModelLoader::recursiveRectangleIntersect(const aiNode *nd, int x1, int x2, int y1, int y2, double *mvRotated, double *prjRotated, int *viewport){
    unsigned int n = 0, t;
    for (; n < nd->mNumMeshes; ++n) {
        const aiMesh* mesh = scene->mMeshes[nd->mMeshes[n]];
        for (t = 0; t < mesh->mNumVertices; ++t) {
            aiVector3D tmp = mesh->mVertices[t];
            //Test the projection of the vertex:
            GLdouble winx, winy, winz;
            gluProject(tmp.x, tmp.y, tmp.z, mvRotated, prjRotated, viewport, &winx, &winy, &winz);
            //Is within the boundary?
            if((x1 - winx)*(x2 - winx) > 0 || (y1 - winy)*(y2 - winy) > 0)
                return false;
        }
    }
    for (n = 0; n < nd->mNumChildren; ++n) {
        bool result = recursiveRectangleIntersect(nd->mChildren[n], x1, x2, y1, y2, mvRotated, prjRotated, viewport);
        if(!result)
            return false;
    }
    return true;
}

bool ModelLoader::rectangleIntersect(int x1, int x2, int y1, int y2, double *mvRotated, double *prjRotated, int *viewport){
    /* First test the bounding box.
     * If the bounding box is in the rectangle, return true.
     * If not, test each vertex.
     */
    aiVector3D boundingBox[6];
    boundingBox[0] = aiVector3D(scene_max.x, scene_max.y, scene_max.z);
    boundingBox[1] = aiVector3D(scene_max.x, scene_max.y, scene_min.z);
    boundingBox[2] = aiVector3D(scene_max.x, scene_min.y, scene_max.z);
    boundingBox[3] = aiVector3D(scene_max.x, scene_min.y, scene_min.z);
    boundingBox[4] = aiVector3D(scene_min.x, scene_max.y, scene_max.z);
    boundingBox[5] = aiVector3D(scene_min.x, scene_max.y, scene_min.z);
    boundingBox[6] = aiVector3D(scene_min.x, scene_min.y, scene_max.z);
    boundingBox[7] = aiVector3D(scene_min.x, scene_min.y, scene_min.z);
    //Test the projection of the bounding box:
    bool flag = true;
    for(int i=0;i<8;i++){
        GLdouble winx, winy, winz;
        gluProject(boundingBox[i].x, boundingBox[i].y, boundingBox[i].z, mvRotated, prjRotated, viewport, &winx, &winy, &winz);
        //Is within the boundary?
        if((x1 - winx)*(x2 - winx) > 0 || (y1 - winy)*(y2 - winy) > 0){
            flag = false;
            break;
        }
    }
    if(flag)
        return true;
    return recursiveRectangleIntersect(scene->mRootNode, x1, x2, y1, y2, mvRotated, prjRotated, viewport);
}

void ModelLoader::getOnPlaneFaces(){
    /* There're 2 steps to do this:
     * Step 1: traverse all the faces and find the faces with the same normal
     *         and store these faces in a vector [candidate].
     * Step 2: traverse the vector [candidate] and find the faces adjacent to the faces in [selection],
     *         move these faces to [selection] and retraverse [candidate] until the size of [selection] doesn't change.
     */
    QVector<Selection>* candidate = new QVector<Selection>();

    for(unsigned int i=0; i<scene->mNumMeshes; i++){
        const aiMesh* mesh = scene->mMeshes[i];
        for(unsigned int j=0; j<mesh->mNumFaces; j++){
            const aiFace* face = &mesh->mFaces[j];

            if(face->mNumIndices != 3)
                continue;

            aiVector3D vertices[3];
            for(int k = 0; k < 3; k++)
                vertices[k] = mesh->mVertices[face->mIndices[k]];
            //Test 1: is normal same as selected normal?
            aiVector3D b1 = vertices[1] - vertices[0];
            aiVector3D b2 = vertices[2] - vertices[0];
            aiVector3D norm = crossProduct(b1, b2);
            float tmp = dotProduct(norm, selectedNormal)/(norm.Length()*selectedNormal.Length()); //cos<b1, b2>
            if(tmp > 0.999){
                Selection s;
                s.face = face;
                for(int k = 0; k < 3; k++)
                    s.vertices[k] = vertices[k];
                candidate->append(s);
            }
        }
    }

    bool* taint = new bool[candidate->size()];
    for(int i=0;i<candidate->size();i++)
        taint[i] = false;

    QVector<Selection> backup;
    int lastSize = 0;
    while(lastSize != selection->size()){
        lastSize = selection->size();

        for(int i = 0; i<candidate->size();i++){
            if(taint[i])
                continue;

            Selection s = candidate->at(i);
            for(int j=0;j<selection->size();j++){
                Selection s2 = selection->at(j);

                //Adjacent?
                short hitCount = 0;
                if(s.vertices[0] == s2.vertices[0] || s.vertices[0] == s2.vertices[1] || s.vertices[0] == s2.vertices[2])
                    hitCount++;
                if(s.vertices[1] == s2.vertices[0] || s.vertices[1] == s2.vertices[1] || s.vertices[1] == s2.vertices[2])
                    hitCount++;
                if(s.vertices[2] == s2.vertices[0] || s.vertices[2] == s2.vertices[1] || s.vertices[2] == s2.vertices[2])
                    hitCount++;
                if(hitCount >= 2){
                    taint[i] = true;
                    backup.append(s);
                }
            }
        }

        for(int i = 0; i<backup.size();i++)
            selection->prepend(backup.at(i));
        backup.clear();
    }
    delete candidate;
}

void ModelLoader::setSelectionIndex(int sceneMesh, int meshFace){
    sceneMeshIndex = sceneMesh;
    meshFaceIndex = meshFace;

    selection->clear();
    //If select a single mesh face, select all the faces on the same plane:
    if(sceneMeshIndex >= 0 && meshFaceIndex >= 0){
        // Calculate selected face:
        const aiMesh* mesh = scene->mMeshes[sceneMeshIndex];
        const aiFace* face = &mesh->mFaces[meshFaceIndex];

        // Calculate selected vertices:
        aiVector3D vertices[3];
        for(int i = 0; i < 3; i++){
            int index = face->mIndices[i];
            vertices[i] = mesh->mVertices[index];
        }

        // Calculate selected normal:
        aiVector3D b1 = vertices[1] - vertices[0];
        aiVector3D b2 = vertices[2] - vertices[0];
        aiVector3D norm = crossProduct(b1, b2);
        selectedNormal = norm.Normalize();

        // Recursive test faces:
        Selection s;
        s.face = face;
        for(int i = 0; i < 3; i++)
            s.vertices[i] = vertices[i];
        selection->append(s);
        getOnPlaneFaces();
    }

    //Important:
    glDeleteLists(scene_list,1);
    scene_list = 0;
}

//Rotation and Align:
void ModelLoader::freeRecursiveRotation(const aiScene *sc, const aiNode *nd, aiVector2D SCTheta, aiVector2D SCGama, aiVector3D p, float xSin, float xCos)
{
    unsigned int n = 0, t;

    for (; n < nd->mNumMeshes; ++n) {
        const aiMesh* mesh = scene->mMeshes[nd->mMeshes[n]];
        for (t = 0; t < mesh->mNumVertices; ++t) {
            aiVector3D result = xRotate(SCTheta, SCGama, p, xSin, xCos, mesh->mVertices[t]);
            mesh->mVertices[t].Set(result.x, result.y, result.z);
        
			// rotate the normals
            aiVector3D normal = xRotate(SCTheta, SCGama, p, xSin, xCos, mesh->mNormals[t]);
            mesh->mNormals[t].Set(normal.x, normal.y, normal.z);
            mesh->mNormals[t].Normalize();
		}
    }

    for (n = 0; n < nd->mNumChildren; ++n) {
        freeRecursiveRotation(sc, nd->mChildren[n], SCTheta, SCGama, p, xSin, xCos);
    }
}

void ModelLoader::freeRotation(aiVector2D SCTheta, aiVector2D SCGama, aiVector3D p, float xSin, float xCos)
{
    freeRecursiveRotation(scene, scene->mRootNode, SCTheta, SCGama, p, xSin, xCos);
    glDeleteLists(scene_list,1);
    scene_list = 0;
    glDeleteLists(mesh_list,1);
    mesh_list = 0;
}

aiVector2D getSinAndCos(aiVector3D vec1, aiVector3D vec2)
{
    // deal with special case
    // if vec1 or vec2 is a dot (0,0,0), then rotate 0;
    if (vec1 == aiVector3D(0,0,0) || vec2 == aiVector3D(0,0,0))
        return aiVector2D(0,1);

    // from vec1 to vec2 and the direction of axis is settled
    float cosTheta = dotProduct(vec1, vec2) / (vec1.Length()*vec2.Length());

    aiVector3D cross = crossProduct(vec1, vec2);
    float sinTheta = cross.Length() / (vec1.Length()*vec2.Length());
    if (cross.x < 0 || cross.y < 0 || cross.z < 0)
        sinTheta = -sinTheta;
    return aiVector2D(sinTheta, cosTheta);
}

aiVector2D xGetSinAndCos(aiVector3D vec1, aiVector3D vec2)
{
    // from vec1 to vec2 and the direction of axis is unsettled
    float cosTheta = dotProduct(vec1, vec2) / (vec1.Length()*vec2.Length());

    aiVector3D cross = crossProduct(vec1, vec2);
    float sinTheta = cross.Length() / (vec1.Length()*vec2.Length());
    return aiVector2D(sinTheta, cosTheta);
}

aiVector3D nRotate(aiVector3D from, float nSin, float nCos, char axis)
{
    float nx, ny, nz;
    float x = from.x;
    float y = from.y;
    float z = from.z;
    if (axis == 'x' || axis == 'X'){
        // x
        nx = x;
        ny = y*nCos - z*nSin;
        nz = y*nSin + z*nCos;
    }
    else if (axis == 'y' || axis == 'Y'){
        // y
        nx = z*nSin + x*nCos;
        ny = y;
        nz = z*nCos - x*nSin;
    }
    else if (axis == 'z' || axis == 'Z'){
        // z
        nx = x*nCos - y*nSin;
        ny = x*nSin + y*nCos;
        nz = z;
    }
    else{
        return from;
    }

    return aiVector3D(nx,ny,nz);
}

aiVector3D xRotate(aiVector2D SCTheta, aiVector2D SCGama, aiVector3D p, float xSin, float xCos, aiVector3D in)
{
    aiVector3D out(0,0,0);

    // translation to origin.
    out = in - p;

    // rotate to z axis
    // theta is the angle of the projections of the two vector in x-y plane
    // gama is the angle of the projections of the two vector in y-z plane
    out = nRotate(out, SCTheta.x, SCTheta.y, 'z');
    out = nRotate(out, SCGama.x, SCGama.y, 'x');

    // rotate @angle to z axis
    out = nRotate(out, xSin, xCos, 'z');

    // back using theta, gama, and translation
    out = nRotate(out, -SCGama.x, SCGama.y, 'x');
    out = nRotate(out, -SCTheta.x, SCTheta.y, 'z');
    out += p;

    return out;
}

//Vector calculations:
float dotProduct(aiVector3D a, aiVector3D b){
    return a.x*b.x + a.y*b.y + a.z*b.z;
}

aiVector3D crossProduct(aiVector3D a, aiVector3D b){
    aiVector3D res;
    res.Set(a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x);
    return res;
}

//Duplicate:
//Override the constructor of aiScene:
using namespace Assimp;
aiScene::aiScene(){}
aiScene::~aiScene(){}

ModelLoader* ModelLoader::duplicate(){
    ModelLoader* dest = new ModelLoader();
    dest->loaded = true;
    dest->isDuplicated = true;

    dest->scene_max = scene_max;
    dest->scene_min = scene_min;
    dest->geometry_center = geometry_center;
    dest->box_center = box_center;
    dest->total_num = total_num;

    //Modify dest->scene. DANGEROUS!
    aiScene *dest_scene = new aiScene();
    model_copy(scene, dest_scene);
    dest->scene = dest_scene;

    return dest;
}

aiVector3D ModelLoader::getSelectedNormal(){
    if(meshFaceIndex == -1) //No face is selected.
        return aiVector3D();
    if(sceneMeshIndex == -999 || sceneMeshIndex ==-1)
        return aiVector3D();
    return selectedNormal;
}

aiVector3D ModelLoader::getSelectedCenter(){
    /* Just use the average number of the vertices of the selected faces as the center.*/
    if(meshFaceIndex == -1) //No face is selected.
        return aiVector3D();
    if(sceneMeshIndex == -999 || sceneMeshIndex ==-1)
        return aiVector3D();

    aiVector3D result;
    for(int i=0;i<selection->size();i++)
        for(int j=0;j<3;j++)
            result += selection->at(i).vertices[j];
    result /= (selection->size() * 3.0f);
    return result;
}

//Alignment:
void ModelLoader::alignTo(aiVector3D normAlignTo){
    aiVector3D norm = -getSelectedNormal(); //Remember to add the MINUS symbol.

    // Calculate axis vector with direction and the sin and cos of rotate angle.
    aiVector3D axis = crossProduct(norm, normAlignTo);
    aiVector2D CS;
    // If vectors are parallel
    if(axis.Length()==0){
        if (dotProduct(norm,normAlignTo) >= 0) // If vectors point to same direction.
            return;
        else{ //Vectors point to the opposite direction.
            aiVector3D tmp;
            if (norm.x == 0 || norm.y == 0 || norm.z == 0)
                tmp.Set(1,1,1);
            else
                tmp.Set(0,0,1);
            axis = crossProduct(norm, tmp);
            CS = aiVector2D(0,-1);
        }
    }else{
        CS = xGetSinAndCos(norm, normAlignTo);
    }
    axis = axis.Normalize();

    aiVector2D SCTheta = getSinAndCos(aiVector3D(axis.x, axis.y, 0), aiVector3D(0, 1, 0));
    aiVector2D SCGama = getSinAndCos(aiVector3D(0, sqrt(axis.x*axis.x + axis.y*axis.y), axis.z), aiVector3D(0, 0, 1));

    // Apply the rotation to every vertex
    freeRotation(SCTheta,SCGama, geometry_center, CS.x, CS.y);

    // Remember to recalculate selectionNormal and reset selection!
    setSelectionIndex(sceneMeshIndex, meshFaceIndex);
}

void ModelLoader::stickTo(aiVector3D normAlignTo, aiVector3D point){
    aiVector3D norm = -getSelectedNormal(); //Remember to add the MINUS symbol.

    // Calculate axis vector with direction and the sin and cos of rotate angle.
    aiVector3D axis = crossProduct(norm, normAlignTo);
    aiVector2D CS;
    // If vectors are parallel
    if(axis.Length()==0){
        if (dotProduct(norm,normAlignTo) >= 0){ // If vectors point to same direction.
            // Translate:
            aiVector3D center = getSelectedCenter();
            point -= center;
            position(point.x, point.y, point.z);

            // Remember to recalculate selectionNormal and reset selection!
            setSelectionIndex(sceneMeshIndex, meshFaceIndex);
            return;
        }else{ //Vectors point to the opposite direction.
            aiVector3D tmp;
            if (norm.x == 0 || norm.y == 0 || norm.z == 0)
                tmp.Set(1,1,1);
            else
                tmp.Set(0,0,1);
            axis = crossProduct(norm, tmp);
            CS = aiVector2D(0,-1);
        }
    }else{
        CS = xGetSinAndCos(norm, normAlignTo);
    }
    axis = axis.Normalize();

    aiVector2D SCTheta = getSinAndCos(aiVector3D(axis.x, axis.y, 0), aiVector3D(0, 1, 0));
    aiVector2D SCGama = getSinAndCos(aiVector3D(0, sqrt(axis.x*axis.x + axis.y*axis.y), axis.z), aiVector3D(0, 0, 1));

    // Apply the rotation to every vertex
    freeRotation(SCTheta,SCGama, geometry_center, CS.x, CS.y);

    // Remember to recalculate selectionNormal and reset selection!
    setSelectionIndex(sceneMeshIndex, meshFaceIndex);

    // Translate:
    aiVector3D center = getSelectedCenter();
    point -= center;
    position(point.x, point.y, point.z);

    // Remember to recalculate selectionNormal and reset selection!
    setSelectionIndex(sceneMeshIndex, meshFaceIndex);
}
