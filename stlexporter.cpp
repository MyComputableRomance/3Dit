#include "stlexporter.h"
#include <QString>

void recursiveAsciiWrite(ofstream & ofs, const aiScene *sc, const aiNode *nd)
{
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
				
                // write the normal infomation
                ofs << QString("  facet normal %1 %2 %3").arg(normal.x).arg(normal.y).arg(normal.z).toStdString() << endl;

                // write the vertices
                ofs << "    outer loop" << endl;
                ofs << QString("      vertex %1 %2 %3").arg(a.x).arg(a.y).arg(a.z).toStdString() << endl;
                ofs << QString("      vertex %1 %2 %3").arg(b.x).arg(b.y).arg(b.z).toStdString() << endl;
                ofs << QString("      vertex %1 %2 %3").arg(c.x).arg(c.y).arg(c.z).toStdString() << endl;
                ofs << "    endloop" << endl;
                ofs << "  endfacet" << endl;
            }
        }
    }

    // write all children
    for (unsigned int n = 0; n < nd->mNumChildren; ++n) {
        recursiveAsciiWrite(ofs, sc, nd->mChildren[n]);
    }
}

bool AsciiStlExporter(string filename, vector<const aiScene *> & scs)
{
    //if (scs.size() == 0)
    //    return false;
    if (filename.empty())
        filename = TEMPORARY_STL_FILE;
    ofstream ofs(filename);
    if (!ofs)
        return false;

    /*
     * Write data into file
     *
     * Sample stl ascii file
     *
     * solid testTriangle
     *   facet normal 0.0 0.0 1.0
     *     outer loop
     *       vertex 1.0 1.0 0.0
     *       vertex -1.0 1.0 0.0
     *       vertex 0.0 -1.0 0.0
     *     endloop
     *   endfacet
     * endsolid
     */

    // Write header
    ofs << "solid model produced by 3dit\n";

    // Write the content
    for (unsigned int i=0;i<scs.size();i++){
        recursiveAsciiWrite(ofs, scs[i], scs[i]->mRootNode);
    }

    // Write tail
    ofs << "endsolid\n";

    ofs.close();
    return true;
}
