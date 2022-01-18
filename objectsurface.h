#ifndef OBJECTSURFACE_H
#define OBJECTSURFACE_H

#include <object.h>

//The class for the parameter surface, used to store the single surface
class ObjectSurface : public Object
{
public:
    float uMin, uMax, vMin, vMax;
    vec2 point;
    float minK, maxK;

    ObjectSurface(QOpenGLContext * targetContext);

    void GenCurve();

    void SetInfo(QString equa[20], QString info[4], int ID);

protected:

    vector<vec3>    curVertices;       //Curve vertices
    vector<float>   gausCurvature;     //Curvature list

    //The math equation
    QString equationX;
    QString equationY;
    QString equationZ;

    QString equationXu;
    QString equationYu;
    QString equationZu;

    QString equationXv;
    QString equationYv;
    QString equationZv;

    QString equationXuu;
    QString equationYuu;
    QString equationZuu;

    QString equationXuv;
    QString equationYuv;
    QString equationZuv;

    QString equationXvv;
    QString equationYvv;
    QString equationZvv;

    QString evaU, evaV;

    //The uv limit
    QString umin, umax, vmin, vmax;


    /***** Some OpenGL Parameters and functions *****/

    //Vertices information
    vector<vec3> indicesLines;
};

#endif // OBJECTSURFACE_H
