#include "objectsilhouette.h"

ObjectSilhouette::ObjectSilhouette()
{
    //Disable the bound box option
    ifShowBoundBox = false;

    //Force the line mode
    ifLineMode = true;

    //Set the flag
    ifInitialized = false;

    //Set color
    color = vec3(0, 0, 0);
}

void ObjectSilhouette::CreateSilhouette(vector<glm::vec3> data, vec3 ocolor, int mode)
{
    //Set data
    ReceiveData(data);

    GenObject();

    color = ocolor;

    principalMode = mode;
}

void ObjectSilhouette::Draw(mat4 &Translation, mat4 &mvp, mat4 &m, mat4 &v, vec3 &lightPos)
{
    /***** Draw Silhouette *****/

    //Bind the vertex array
    glBindVertexArray(vertexArrayID);

    //Bind the shader
    glUseProgram(shaderProgID);

    //MVP matrix
    GLuint MatrixID = glGetUniformLocation(shaderProgID, "MVP");
    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &mvp[0][0]);

    //Object color
    GLuint colorID = glGetUniformLocation(shaderProgID, "objectColor");
    glUniform3f(colorID, color.x, color.y, color.z);

    //trans matrix
    GLint uniTrans = glGetUniformLocation(shaderProgID, "trans");
    glUniformMatrix4fv(uniTrans, 1, GL_FALSE, value_ptr(Translation));

    glLineWidth(2.0f);
    glDrawArrays(GL_LINES, 0, vertices.size()); //Starting from vertex 0

    /* Clean up */
    glUseProgram(0);

    //Detach the VAO
    glBindVertexArray(0);
}

void ObjectSilhouette::GenBuffer()
{
    //Generate the vertex array object and start store information
    glGenVertexArrays(1, &vertexArrayID);

    //Generate the vertex buffer object for the surface mesh
    glGenBuffers(1, &surfaceBufferID);

    ifInitialized = true;
}

void ObjectSilhouette::UpdateObject()
{
    //for(int i = 0; i < vertices.size(); i++)
    {

        //qInfo("== Vert %d's coor: %f,%f,%f\n", i,
                      //vertices[i].x,
                      //vertices[i].y,
                      //vertices[i].z);
    }

    //Bind the VAO
    glBindVertexArray(vertexArrayID);

    //qInfo("Create silhouette vertex buffer");

    //
    /***** Vertex buffer and attribute pointer *****/
    //

    //Bind the current buffer
    glBindBuffer(GL_ARRAY_BUFFER, surfaceBufferID);

    //Load information into the buffer
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vec3),
                 &vertices[0], GL_STATIC_DRAW);

    //Set the vertex pointer to the VAO
    glVertexAttribPointer(
       0,                  //Attribute 0. No particular reason for 0, but must match the layout in the shader.
       3,                  //Size
       GL_FLOAT,           //Type
       GL_FALSE,           //Normalized?
       0,  //Stride
       (void*)0            //Array buffer offset
    );

    //Enable the attribute array
    glEnableVertexAttribArray(0);
}

void ObjectSilhouette::GenObject()
{
    /*** Generate Object ***/

    qInfo("Start generating the silhouette obejct");

    //if(!ifInitialized)
    GenBuffer();

    //Write to buffer, bind VAO with VBO
    UpdateObject();

    //Create the shader program
    shaderProgID = LoadShader("shaders/Mesh.vert", "shaders/Mesh.frag");

    qInfo("Object created, Shader ID: %d", shaderProgID);
}

void ObjectSilhouette::ReceiveData(vector<glm::vec3> newVerts)
{
    qInfo("Silhouette data received: %d.\n", newVerts.size());

    //Clear the old data
    vertices.clear();

    //Copy the target data
    vertices = newVerts;
}
