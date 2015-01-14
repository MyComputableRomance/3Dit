#ifndef STLEXPORTER_H
#define STLEXPORTER_H

#include <fstream>
#include <vector>
#include <assimp/scene.h>

#define TEMPORARY_STL_FILE "_temp_stl_file_for_printer_5407_.stl"

using namespace std;

void recursiveAsciiWrite(ofstream & ofs, const aiScene *sc, const aiNode *nd);

bool AsciiStlExporter(string filename, vector<const aiScene *> & scs);

#endif // STLEXPORTER_H
