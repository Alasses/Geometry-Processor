#ifndef MYWIDGET_H
#define MYWIDGET_H

#include <QSurfaceFormat>
#include <QMouseEvent>
#include <QTimer>
#include <QKeyEvent>
#include <QWheelEvent>

#include <object.h>
#include <objectmodel.h>
#include <objectsurface.h>
#include <objectpoly.h>
#include <objectcomplex.h>

using namespace glm;
using namespace std;

class myWidget : public QOpenGLWidget, public QOpenGLFunctions_4_4_Core
{
public:
    //The object constructor, also used for initialize some variable
    myWidget(QWidget *parent = nullptr);

    //The function used to update the loaded mesh
    int UpdateMeshFilePath(QString path, int mode);

    //The function used to generate the surfaces
    //*All the parameters was specifies by the user
    //*The opengl default draw order is CCW (counter clock wise)
    int GenerateSurfaceVertices(QString equa[5], QString info[4]);

    Object * FindObject(int type, int id);

    vec3 GetLightPos();
    void SetLightPos(vec3 increase);

    //Set if current rotating action is towarding the light
    void SetRotateLight(bool status);

    vec3 GetColor(int type, int id);
    bool ChangeColor(int type, int id, vec3 newColor);

    //Update the information of current object
    void SetCurrentObject(int type, int id);

    //Control display mode for face or line
    bool GetDisplayMode(int type, int id);
    void DisplayMode(int type, int id, bool mode);

    //Control if show bound box or not
    bool GetBoundMode(int type, int id);
    void BoundBoxMode(int type, int id, bool mode);

    //Control if show object or not (render)
    bool GetRenderMode(int type, int id);
    void RenderMode(int type, int id, bool mode);

    //Control if use check board texture, only for ply object
    bool GetCheckBoardMode(int id);
    void CheckBoardMode(int id, bool mode);

    //Reset the object's rotation and position
    void ResetPosition(int type, int id);

    //Subdivide the ply object currently use
    void SubdividePly(int id, int mode);

    //Smooth the surface of ply object
    void SmoothPly(int id, int mode);

    //Running the heat diffussion
    void HeatDiffusionPly(int id, int mode);

    //Running curvature calculation
    void CurvaturePly(int id, int mode);

    //Running the spherical parameterization
    void SphericalParaPly(int id);

    bool DeleteObject(int type, int id);

protected:
    void initializeGL() override;
    void paintGL() override;
    //void resizeGL(int width, int height) override;

    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

    void wheelEvent(QWheelEvent * event) override;

    void keyPressEvent(QKeyEvent *event) override;

    void mousePressEvent(QMouseEvent *event) override;

    //The function used to store the mouse motion use for rotation
    void mouseMoveEvent(QMouseEvent *event) override;

    //A subfunction used to solve the quaternion rotation
    quat QuatSolver(int dx, int dy, quat targetQ);

public slots:
    // slots for xyz-rotation slider
    void setScale(double sFactor);

private:
    int drawFlag;
    int size = 30;
    int initValue = -15;

    float oscale;

    QPoint lastPos;

    int curObjId;
    int curObjType;

    bool ifRotatingLight;

    /***** Some OpenGL Parameters *****/

    /*** Camera Info ***/
    vec3 camPos;
    vec3 camLookAt;
    vec3 camUp;
    vec3 camLeft;

    /*** Lighting Info ***/
    quat lightRotation;
    vec3 lightPos, lightPos2;
    vec3 lightColor;

    /*** Object List ***/
    vector<ObjectSurface *>    surfaceList;
    vector<ObjectModel *>       modelList;
    vector<ObjectPoly *>     polyList;

    //The quaternion represents the rotation angle
    quat sceneRotation;

    vec3 objMovement;

    QOpenGLContext *currentContext = nullptr;
};

bool ObjLoader(const char * path, vector<vec3> &vertices, vector <vec2> &uvs, vector<vec3> &normals);

#endif // MYWIDGET_H

