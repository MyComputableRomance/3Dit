#include "modelcopy.h"
#include <QDebug>

bool model_copy(const aiScene *src, aiScene *dst)
{
    if(dst == NULL || src == NULL)
        return false;

    // animations
    dst->mAnimations = NULL;

    // camera
    dst->mCameras = NULL;

    // flags
    dst->mFlags = src->mFlags;

    // light
    dst->mLights = NULL;

    // materials
    dst->mMaterials = NULL;

    // texture
    dst->mTextures = NULL;

    // mesh
    dst->mMeshes = (aiMesh **) malloc(sizeof(aiMesh*) * src->mNumMeshes);

    for (unsigned int i=0;i<src->mNumMeshes;i++){
        (dst->mMeshes)[i] = new aiMesh();
        mesh_copy((src->mMeshes)[i], (dst->mMeshes)[i]);
    }

    // numbers
    dst->mNumAnimations = 0;
    dst->mNumCameras = 0;
    dst->mNumLights = 0;
    dst->mNumMaterials = 0;
    dst->mNumTextures = 0;
    dst->mNumMeshes = src->mNumMeshes;

    // private
    dst->mPrivate = NULL;

    // rootnode
    dst->mRootNode = new aiNode(src->mRootNode->mName.C_Str());
    recursive_node_copy(src->mRootNode, dst->mRootNode, NULL);

    return true;
}

void mesh_copy(aiMesh *src, aiMesh *dst)
{
    if(dst == NULL || src == NULL)
        return;

    // anim mesh NOT CURRENTLY IN USE
    dst->mAnimMeshes = NULL;

    // bitangents
    dst->mBitangents = NULL;

    // bone
    dst->mBones = NULL;

    // faces
    dst->mFaces = (aiFace *)malloc(sizeof(aiFace) * src->mNumFaces);
    for (unsigned int i = 0; i < src->mNumFaces; i++){
        (dst->mFaces)[i] = aiFace((src->mFaces)[i]);
    }

    // name
    dst->mName = aiString(src->mName);

    // normals
    dst->mNormals = (aiVector3D *)malloc(sizeof(aiVector3D) * src->mNumVertices);
    for (unsigned int i = 0; i < src->mNumVertices; i++){
        (dst->mNormals)[i].Set((src->mNormals)[i].x, (src->mNormals)[i].y, (src->mNormals)[i].z);
    }

    // tangents
    dst->mTangents = NULL;

    // vertex
    dst->mVertices = (aiVector3D *)malloc(sizeof(aiVector3D) * src->mNumVertices);
    for (unsigned int i = 0; i < src->mNumVertices; i++){
        (dst->mVertices)[i].Set((src->mVertices)[i].x, (src->mVertices)[i].y, (src->mVertices)[i].z);
    }

    // numbers
    dst->mNumFaces = src->mNumFaces;
    dst->mNumVertices = src->mNumVertices;
    dst->mNumAnimMeshes = 0;
    dst->mNumBones = 0;
}

void recursive_node_copy(aiNode *src, aiNode *dst, aiNode *parent)
{
    if(src == NULL){
        dst = NULL;
        return;
    }

    // numbers
    dst->mNumChildren = src->mNumChildren;
    dst->mNumMeshes = src->mNumMeshes;

    // name
    dst->mName = src->mName;

    // meshes
    dst->mMeshes = (unsigned int *)malloc(sizeof(unsigned int) * src->mNumMeshes);
    for (unsigned int i=0;i<src->mNumMeshes;i++){
        (dst->mMeshes)[i] = (src->mMeshes)[i];
    }

    // parent
    dst->mParent = parent;

    // children
    if (src->mNumChildren != 0){
        dst->mChildren = (aiNode **)malloc(sizeof(aiNode *) * src->mNumChildren);
        for (unsigned int i=0;i<src->mNumChildren;i++){
            (dst->mChildren)[i] = new aiNode((src->mChildren)[i]->mName.C_Str());
            recursive_node_copy((src->mChildren)[i], (dst->mChildren)[i], dst);
        }
    }
}

void print_scene(aiScene *scene)
{
    qDebug() << "\n[scene]";
	
    // flags
    qDebug() << "flags : " << scene->mFlags;

    qDebug() << "meshnums : " << scene->mNumMeshes;
	
    // mesh
    for (unsigned int i=0;i<scene->mNumMeshes;i++)
        print_mesh((scene->mMeshes)[i]);
	
    // rootnode
    print_node(scene->mRootNode);
}

void print_mesh(aiMesh *mesh)
{
    qDebug() << "[mesh]" << "[" << mesh->mName.C_Str() << "]";

    qDebug() << "[normals]" << mesh->mNumVertices;
    // normals
    for (unsigned int i = 0; i < mesh->mNumVertices; i++){
        qDebug() << (mesh->mNormals)[i].x << ", " << (mesh->mNormals)[i].y << ", " << (mesh->mNormals)[i].z;
    }

    qDebug() << "[vertexes]" << mesh->mNumVertices;
    // vertex
    for (unsigned int i = 0; i < mesh->mNumVertices; i++){
        qDebug() << (mesh->mVertices)[i].x << ", " << (mesh->mVertices)[i].y << ", " << (mesh->mVertices)[i].z;
    }
}

void print_face(aiFace face)
{
    qDebug() << "[face]" << face.mNumIndices;

    for (unsigned int i=0;i<face.mNumIndices;i++){
        qDebug() << (face.mIndices)[i];
    }
}

void print_node(aiNode *node)
{
    qDebug() << "[node]" << "[" << node->mName.C_Str() << "]" << node->mNumChildren;
    qDebug() << "(mesh index)" << node->mNumMeshes;
    for (unsigned int i=0;i<node->mNumMeshes;i++){
        qDebug() << (node->mMeshes)[i];
    }
    for (unsigned int i=0;i<node->mNumChildren;i++){
        print_node((node->mChildren)[i]);
    }
}
