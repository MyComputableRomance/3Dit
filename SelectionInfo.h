#ifndef SELECTIONINFO_H
#define SELECTIONINFO_H

#include "meshinfo.h"

struct SelectionInfo{
    MeshInfo* mesh;
    int sceneMeshIndex;
    int meshFaceIndex;

    SelectionInfo(MeshInfo* i, int smi, int mfi){
        mesh = i;
        sceneMeshIndex = smi;
        meshFaceIndex = mfi;
    }
};

#endif // SELECTIONINFO_H
