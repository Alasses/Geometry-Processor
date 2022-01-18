#ifndef COMPLEXOBJECT_H
#define COMPLEXOBJECT_H

#include <object.h>

class OEdge;
class OFace;

class OPoint
{
public:
    //The corresponding vertices index
    vec3 position;

    vector<OEdge *> edges;
    vector<OFace *> faces;
};

class OEdge
{
public:
    vector<OPoint *> points;
    vector<OFace *> faces;
};

class OFace
{
public:
    vec3 normal;

    vector<OEdge *> edges;
    vector<OPoint *> points;
};

class ComplexObject : public Object
{
public:
    ComplexObject();
    ~ComplexObject();

    vector<OPoint *>    pointList;
    vector<OEdge *>     edgeList;
    vector<OFace *>     faceList;

protected:
    vector<vec3>    curVertices;       //Curvature vertices
    vector<float>   gausCurvature;     //Curvature list

    //Vertex array object
    GLuint curArrayID;

    //Vertex buffer object
    GLuint curBufferID;

    //Overload function, to read vertices into data structure.
    bool ObjLoader(const char * path);

    //Overload function
    void GenExtra();

    //Overload function, to render the extra information of complex object.
    void DrawExtra(GLuint MatrixID, GLuint uniTrans, mat4 &mvp, mat4 &Translation);

    //Generate the curvature streamline
    void GenCurvature();
};

#endif // COMPLEXOBJECT_H
