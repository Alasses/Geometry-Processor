#include "objectmodel.h"

ObjectModel::ObjectModel()
{

}

float ObjectModel::LoadMesh(QString path, int ID)
{
    meshFilePath = path;

    id = ID;

    ObjLoader(meshFilePath.toLocal8Bit().data());

    return GenObject();
}
