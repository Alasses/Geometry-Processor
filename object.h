#ifndef SHADERLOADER_H
#define SHADERLOADER_H
#define GLEW_STATIC

//Qt libraries
#include <QDebug>
#include <QOpenGLWidget>
#include <QOpenGLFunctions_4_4_Core>
#include <QOpenGLShader>
#include <QOpenGLShaderProgram>
#include <QtMath>
#include <QVariant>
#include <QTextStream>

//System libraries
#include <fstream>
#include <sstream>
#include <vector>

//External libraries
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtx/quaternion.hpp>
#include <exprtk.hpp>
#include <Eigen/Dense>
#include <learnply.h>

using namespace glm;
using namespace std;

class Object : public QOpenGLFunctions_4_4_Core
{
public:

    bool ifLineMode;
    bool ifShowBoundBox;
    bool ifShowCurLine;
    bool ifRender;

    bool ifVertexColor;

    bool ifTexture = false;

    float L;

    //Object position on imagine plane
    vec3 objMovement;

    quat objectRotation;

    Object();
    ~Object();

    int GetID();

    vec3 GetColor();
    void SetColor(vec3 newColor);

    void CalcCheckBoardTexture();

    //Render the object
    void Draw(mat4 &Translation, mat4 &mvp, mat4 &m, mat4 &v, vec3 lightColor,
              vec3 lightPos, vec3 lightPos2);

    void ResetPosition();

protected:

    /*** Parameters ***/

    int id;

    //Object color
    vec3 color;

    //Vertex information
    vector<vec3> boxVertices;       //Bounding box
    vector<vec3> vertices;          //Main render vertices
    vector<vec3> testvers;          //Test draw set?

    vector<vec3> normals;
    vector<vec2> uvs;
    vector<vec3> colors;            //The buffer save object color, originally
    vector<vec3> checkBoardColors;  //The buffer save checkboard color
    vector<vec3> heatColors;        //The heat diffusion color
    vector<vec3> curvatureColors;   //The curvature color

    //Shader program
    GLuint shaderProgID;
    GLuint boxShaderProgID;

    //Vertex array object
    GLuint vertexArrayID;
    GLuint boxArrayID;

    //Textures
    GLuint textureID;

    //Vertex buffer object
    GLuint surfaceBufferID, lineBufferID;
    GLuint boxBufferID;

    //Normal buffer and color buffer
    GLuint normalBufferID, colorBufferID;

    //Stored context
    QOpenGLContext * currentContext;


    /*** Functions ***/

    //Load the texture
    GLuint LoadTexture(const char * imagepath);

    //Load the shader
    GLuint LoadShader(const char * vertex_file_path, const char * fragment_file_path);

    //The overloadable obj file reader
    bool ObjLoader(const char * path);

    //Generate the bounding box
    float GenBoundBox();

    //Generate VAO and VBO
    void GenBuffer();

    //Update all vertex information
    float UpdateObject();
    void WriteColorBuffer(int mode);

    //Generate OpenGL object
    float GenObject();

    //Overload function, generate the extra component
    void GenExtra();

    //Overload function, draw the extra component
    void DrawExtra(GLuint MatrixID, GLuint uniTrans, mat4 &mvp, mat4 &Translation);
};

#endif // SHADERLOADER_H
