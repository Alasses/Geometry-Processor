#ifndef OBJECTMODEL_H
#define OBJECTMODEL_H

#include <object.h>

class ObjectModel : public Object
{
public:
    ObjectModel();

    //The function used to update the loaded mesh
    //
    //@Para QString "path" : The path to the target mesh object
    //
    //@Result : Update the mesh saved in:
    //  -"vertices"
    //  -"uvs"
    //  -"nomals"
    float LoadMesh(QString path, int ID);

private:

    /***** Some OpenGL Parameters and functions *****/


    //The mesh file that will be loaded into the scene
    QString meshFilePath;

};

#endif // OBJECTMODEL_H
