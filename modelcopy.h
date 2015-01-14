#ifndef MODELCOPY_H
#define MODELCOPY_H

/*
 * Just copy the model information that used to render in opengl.
 * Material and texture information is not copied.
 *
 * Usage:
 *  @src is the exist aiScene.
 *
 *  aiScene *scene = new aiScene();
 *  model_copy(src, scene);
 *
 */

#include <assimp/scene.h>

bool model_copy(const aiScene *src, aiScene *dst);

// helper functions
void mesh_copy(aiMesh *src, aiMesh *dst);

void recursive_node_copy(aiNode *src, aiNode *dst, aiNode *parent);

// this is the functions for developers to print model information using qDebug();

void print_scene(aiScene *scene);

void print_mesh(aiMesh *mesh);

void print_face(aiFace face);

void print_node(aiNode *node);

#endif // MODELCOPY_H
