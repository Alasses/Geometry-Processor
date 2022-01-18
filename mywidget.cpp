#define _CRT_SECURE_NO_WARNINGS

#include "mywidget.h"

#define FM_PI (float)M_PI

myWidget::myWidget(QWidget *parent) : QOpenGLWidget( parent )
{
    oscale = 1.0;
    drawFlag = -1;

    ifRotatingLight = false;

    //Initialize the quaternion rotation
    sceneRotation = quat(1, 0, 0, 0);

    //Initialize the light position and light color
    lightPos = vec3(0, 10, 0);
    lightPos2 = vec3(0, -10, 0);
    lightColor = vec3(1, 1, 1);
    lightRotation = quat(1, 0, 0, 0);

    camPos = vec3(0,0,30);          // Camera is at (0,0,30), in World Space
    camLookAt = vec3(0,0,0);        // and looks at the origin
    camUp = vec3(0,1,0);            // Head is up (set to 0,-1,0 to look upside-down)
    camLeft = vec3(-1, 0, 0);

    objMovement = vec3(0, 0, 0);

    setFocus();
}

QSize myWidget::minimumSizeHint() const
{
    return QSize(50, 50);
}

QSize myWidget::sizeHint() const
{
    return QSize(1800, 960);
}

quat myWidget::QuatSolver(int dx, int dy, quat targetQ)
{
    dx = -dx;
    //dy = -dy;

    //Create projection sphere
    float r = 100;
    vec3 rPos = vec3(0, 0, -1 * r);

    //Create relative vector
    vec3 zAxis = vec3(0, 0, 0) - rPos;

    //Then creat screen coordinate, relate to last position
    vec3 sPos = vec3(dx, dy, 0) - rPos;

    //Get the distance, to find the angle
    float k = sqrt(dx * dx + dy * dy);

    //Find the sin(theta / 2)
    float thetaBy2 = asin(k / 2 / r);

    //Find the rotation axis relate to current system
    vec3 rotateAxis = normalize(cross(sPos, zAxis));

    //Find the true order of current system, since the rotation applied already
    vec4 tempVec = vec4(rotateAxis, 0) * toMat4(targetQ);

    //Retrive the axis
    rotateAxis = normalize(vec3(tempVec.x, tempVec.y, tempVec.z));

    //Create quaternion [w, x, y, z]
    quat result = quat(cos(thetaBy2),
                       rotateAxis.x * sin(thetaBy2),
                       rotateAxis.y * sin(thetaBy2),
                       rotateAxis.z * sin(thetaBy2));

    return result;
}

void myWidget::wheelEvent(QWheelEvent * event)
{
    float dy = -1 * event->angleDelta().y() / 4;
    setScale(dy);
}

void myWidget::mousePressEvent(QMouseEvent *event)
{
    makeCurrent();

    lastPos = event->pos();

    update();
}

void myWidget::keyPressEvent(QKeyEvent * event)
{
    makeCurrent();

    //vec3 movement = vec3(0, 0, 0);

    //camPos += movement;
    //camLookAt += movement;

    update();
}

void myWidget::mouseMoveEvent(QMouseEvent *event)
{
    makeCurrent();

    //First get the mouse motion
    int dx = event->x() - lastPos.x();
    int dy = event->y() - lastPos.y();
    //qInfo("X pos: %d, Y pos: %d\n", event->x(), event->y());

    if(dx != 0 || dy != 0)
    {
        if (((event->buttons() & Qt::RightButton)) && ((event->buttons() & Qt::LeftButton)))
        {
            //Rotate the light
            lightPos = vec3((vec4(lightPos, 1) * toMat4(QuatSolver(dx, dy, lightRotation))).x,
                            (vec4(lightPos, 1) * toMat4(QuatSolver(dx, dy, lightRotation))).y,
                            (vec4(lightPos, 1) * toMat4(QuatSolver(dx, dy, lightRotation))).z);
            lightPos2 = vec3((vec4(lightPos2, 1) * toMat4(QuatSolver(dx, dy, lightRotation))).x,
                            (vec4(lightPos2, 1) * toMat4(QuatSolver(dx, dy, lightRotation))).y,
                            (vec4(lightPos2, 1) * toMat4(QuatSolver(dx, dy, lightRotation))).z);
        }
        else if((event->buttons() & Qt::RightButton))
        {
            FindObject(curObjType, curObjId)->objMovement += -0.001f * (float)dx * camLeft;
            FindObject(curObjType, curObjId)->objMovement += -0.001f * (float)dy * camUp;
            //FindObject(curObjType, curObjId)->objectRotation *= QuatSolver(dx, dy);
        }
        else if((event->buttons() & Qt::LeftButton))
        {
            FindObject(curObjType, curObjId)->objectRotation *=
                    QuatSolver(dx, dy, FindObject(curObjType, curObjId)->objectRotation);
            //sceneRotation *= QuatSolver(dx, dy);
        }
        else if(event->buttons() & Qt::MiddleButton)
        {
            //setScale(dy);
        }
    }

    lastPos = event->pos();

    update();
}

void myWidget::setScale(double scales)
{
    makeCurrent();

    oscale -= (scales / 100.0);
    if(oscale > 20.0)
        oscale = 20.0;
    else if(oscale < 0)
        oscale = 0;

    update();
}

void myWidget::initializeGL()
{
    initializeOpenGLFunctions();

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    //glDisable(GL_CULL_FACE);
    //glCullFace(GL_FRONT);
    //glFrontFace(GL_CCW);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
}

void myWidget::paintGL()
{
    makeCurrent();

    /***** Matrix relate to viewport *****/

    //Create the project matrix
    mat4 Projection = ortho(-(float)this->width() / (40 * oscale),
                            (float)this->width() / (40 * oscale),
                            -(float)this->height() / (40 * oscale),
                            (float)this->height() / (40 * oscale),
                            0.001f, 1000.0f);
    //qInfo("oscale: %f", oscale);
    vec3 zoomMat = vec3(1, 1, 1);

    //Create the view matrix
    mat4 View = lookAt(
                camPos, // Camera is at (0,0,30), in World Space
                camLookAt, // and looks at the origin
                camUp  // Head is up (set to 0,1,0 to look upside-down)
                );

    mat4 Model = mat4(1.0f);

    mat4 mvp = Projection * View * Model;


    /***** Actual rendering *****/

    //Enable z buffer testing
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if(surfaceList.size() != 0)
    {
        for(int i = 0; i < (int)surfaceList.size(); i++)
        {
            //Create the translation matrix
            mat4 Translation = glm::translate(glm::mat4(1.0f), surfaceList[i]->objMovement);
            Translation *= toMat4(surfaceList[i]->objectRotation);
            Translation = scale(Translation, zoomMat);

            if(surfaceList[i]->ifRender)
                surfaceList[i]->Draw(Translation, mvp, Model, View, lightColor, lightPos, lightPos2);
        }
    }
    if(modelList.size() != 0)
    {
        for(int i = 0; i < (int)modelList.size(); i++)
        {
            //Create the translation matrix
            mat4 Translation = glm::translate(glm::mat4(1.0f), modelList[i]->objMovement);
            Translation *= toMat4(modelList[i]->objectRotation);
            Translation = scale(Translation, zoomMat);

            if(modelList[i]->ifRender)
                modelList[i]->Draw(Translation, mvp, Model, View, lightColor, lightPos, lightPos2);
        }
    }
    if(polyList.size() != 0)
    {
        for(int i = 0; i < (int)polyList.size(); i++)
        {
            //Create the translation matrix
            mat4 Translation = glm::translate(glm::mat4(1.0f), polyList[i]->objMovement);
            Translation *= toMat4(polyList[i]->objectRotation);
            Translation = scale(Translation, zoomMat);

            if(polyList[i]->ifRender)
                polyList[i]->Draw(Translation, mvp, Model, View, lightColor, lightPos, lightPos2);
        }
    }
    //qInfo("Current context: %d\n", context());
}

int myWidget::UpdateMeshFilePath(QString path, int mode)
{
    makeCurrent();

    qInfo("Current context: %d\n", context());

    float maxRadius;

    int id = -1;
    if(mode == 1)
    {
        //Generate a new object
        ObjectModel* newObject = new ObjectModel();

        if(modelList.size() != 0)
            id = modelList[modelList.size()-1]->GetID() + 1;
        else
            id = 100 + 1;

        //Read in the object
        maxRadius = newObject->LoadMesh(path, id);

        //Store the new object
        modelList.push_back(newObject);
    }
    else if(mode == 2)
    {
        ObjectPoly* newPoly = new ObjectPoly(path);

        if(polyList.size() != 0)
            id = polyList[polyList.size()-1]->GetID() + 1;
        else
            id = 300 + 1;

        maxRadius = newPoly->ProcessData(id);

        //Store
        polyList.push_back(newPoly);
    }

    oscale = 45.0f / maxRadius;

    update();

    return id;
}

int myWidget::GenerateSurfaceVertices(QString equa[5], QString info[4])
{
    makeCurrent();

    //Create the new surface instance
    ObjectSurface* newSurface = new ObjectSurface(context());

    int id;
    if(surfaceList.size() != 0)
        id = surfaceList[surfaceList.size()-1]->GetID() + 1;
    else
        id = 200 + 1;

    //Pass the information into the surface instance
    newSurface->SetInfo(equa, info, id);

    //Store the new surface
    surfaceList.push_back(newSurface);

    currentContext = context();
    qInfo("Current widget context: %d\n", currentContext);

    update();

    return id;
}

vec3 myWidget::GetLightPos()
{
    return lightPos;
}

void myWidget::SetLightPos(vec3 increase)
{
    lightPos = increase;
    update();
}

void myWidget::SetRotateLight(bool status)
{
    ifRotatingLight = status;
}

Object* myWidget::FindObject(int type, int id)
{
    //Target list based on type
    if(type == 1)
    {
        for(int i = 0; i < (int)surfaceList.size(); i++)
        {
            //Find the target object
            if(surfaceList[i]->GetID() == id)
            {
                return surfaceList[i];
            }
        }
    }
    else if(type == 2)
    {
        for(int i = 0; i < (int)modelList.size(); i++)
        {
            //Find the target object
            if(modelList[i]->GetID() == id)
            {
                return modelList[i];
            }
        }
    }
    else if(type == 3)
    {
        for(int i = 0; i < (int)polyList.size(); i++)
        {
            //Find the target object
            if(polyList[i]->GetID() == id)
            {
                return polyList[i];
            }
        }
    }

    return NULL;
}

vec3 myWidget::GetColor(int type, int id)
{
    Object * target  = FindObject(type, id);
    if(target != NULL)
        return target->GetColor();
    else
        return vec3(-1, -1, -1);
}

bool myWidget::ChangeColor(int type, int id, vec3 newColor)
{
    makeCurrent();

    Object * target  = FindObject(type, id);
    if(target != NULL)
    {
        target->SetColor(newColor);
        update();

        return true;
    }
    else
        return false;
}

void myWidget::SetCurrentObject(int type, int id)
{
    curObjId = id;
    curObjType = type;
}

bool myWidget::GetDisplayMode(int type, int id)
{
    Object * target = FindObject(type, id);
    if(target)
        return target->ifLineMode;
    return false;
}

void myWidget::DisplayMode(int type, int id, bool mode)
{
    Object * target = FindObject(type, id);
    if(target)
        target->ifLineMode = mode;

    update();
}

bool myWidget::GetBoundMode(int type, int id)
{
    Object * target = FindObject(type, id);
    if(target)
        return target->ifShowBoundBox;
    return false;
}

void myWidget::BoundBoxMode(int type, int id, bool mode)
{
    Object * target = FindObject(type, id);
    if(target)
        target->ifShowBoundBox = mode;

    update();
}

bool myWidget::GetRenderMode(int type, int id)
{
    Object * target = FindObject(type, id);
    if(target)
        return target->ifRender;
    return false;
}

void myWidget::RenderMode(int type, int id, bool mode)
{
    Object * target = FindObject(type, id);
    if(target)
        target->ifRender = mode;

    update();
}

bool myWidget::GetCheckBoardMode(int id)
{
    Object * target = FindObject(3, id);
    if(target)
        return target->ifVertexColor;
    return false;
}

void myWidget::CheckBoardMode(int id, bool mode)
{
    Object * target = FindObject(3, id);
    if(target)
    {
        target->ifVertexColor = mode;

        if(mode)
            target->CalcCheckBoardTexture();
    }

    update();
}

void myWidget::ResetPosition(int type, int id)
{
    Object * target = FindObject(type, id);
    target->ResetPosition();

    update();
}

void myWidget::SubdividePly(int id, int mode)
{
    ObjectPoly * targetObj;
    for(int i = 0; i < (int)polyList.size(); i++)
    {
        //Find the target object
        if(polyList[i]->GetID() == id)
        {
            targetObj = polyList[i];

            //Regular or irregular subdivision
            if(mode == 1)
            {
                targetObj->Subdivision(1);
            }
            else if(mode == 2)
            {
                targetObj->Subdivision(2);
            }
        }
    }

    update();
}

void myWidget::SmoothPly(int id, int mode)
{
    ObjectPoly * targetObj;
    for(int i = 0; i < (int)polyList.size(); i++)
    {
        //Find the target object
        if(polyList[i]->GetID() == id)
        {
            targetObj = polyList[i];

            targetObj->SurfaceSmooth(mode);
        }
    }

    update();
}

void myWidget::HeatDiffusionPly(int id, int mode)
{
    ObjectPoly * targetObj;
    for(int i = 0; i < (int)polyList.size(); i++)
    {
        //Find the target object
        if(polyList[i]->GetID() == id)
        {
            targetObj = polyList[i];

            targetObj->HeatDiffusion(mode);
        }
    }

    update();
}

void myWidget::CurvaturePly(int id, int mode)
{
    ObjectPoly * targetObj;
    for(int i = 0; i < (int)polyList.size(); i++)
    {
        //Find the target object
        if(polyList[i]->GetID() == id)
        {
            targetObj = polyList[i];

            if(mode == 1 || mode == 2)
                targetObj->CurvatureCalc(mode);
            else if(mode == 3)
                targetObj->PrincipalCurvatureCalc();
            else if(mode == 4)
                targetObj->PrincipalCurvatureSmooth();
            else if(mode == 5)
                targetObj->CalcMaxPrincipalLine();
        }
    }

    update();
}

void myWidget::SphericalParaPly(int id)
{
    ObjectPoly * targetObj;
    for(int i = 0; i < (int)polyList.size(); i++)
    {
        //Find the target object
        if(polyList[i]->GetID() == id)
        {
            targetObj = polyList[i];

            targetObj->SphericalParameterization();
        }
    }

    update();
}

bool myWidget::DeleteObject(int type, int id)
{
    //Target list based on type
    if(type == 1)
    {
        for(int i = 0; i < (int)surfaceList.size(); i++)
        {
            //Find the target object
            if(surfaceList[i]->GetID() == id)
            {
                //Free and delete the object
                free(surfaceList[i]);
                surfaceList.erase(surfaceList.begin() + i);

                return true;
            }
        }
    }
    else if(type == 2)
    {
        for(int i = 0; i < (int)modelList.size(); i++)
        {
            //Find the target object
            if(modelList[i]->GetID() == id)
            {
                //Free and delete the object
                free(modelList[i]);
                modelList.erase(modelList.begin() + i);

                return true;
            }
        }
    }
    else if(type == 3)
    {
        for(int i = 0; i < (int)polyList.size(); i++)
        {
            //Find the target object
            if(polyList[i]->GetID() == id)
            {
                //Free and delete the object
                free(polyList[i]);
                polyList.erase(polyList.begin() + i);

                return true;
            }
        }
    }

    return false;
}

