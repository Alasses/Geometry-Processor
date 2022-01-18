#define FM_PI (float)M_PI

#include "object.h"

bool Object::ObjLoader(const char * path)
{
    vector< unsigned int > vertexIndices, uvIndices, normalIndices;
    vector< glm::vec3 > tempVertices;
    vector< glm::vec2 > tempUvs;
    vector< glm::vec3 > tempNormals;

    FILE * file = fopen(path, "r");
    if( file == NULL ){
        qInfo("File open fail\n");
        return false;
    }
    else
    {
        qInfo("Mesh file open succeed!");
    }

    //Read the file
    while(true)
    {
        char lineHeader[128];
        //Read the first word of the line
        int res = fscanf(file, "%s", lineHeader);

        //Quit the loop if meet the end of file
        if (res == EOF)
        {
            break;
        }
        else
        {
            //3 float of vertices informations
            if ( strcmp( lineHeader, "v" ) == 0 )
            {
                glm::vec3 vertex;
                fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z );
                tempVertices.push_back(vertex);
            }
            //2 float of uv coordinates
            else if ( strcmp( lineHeader, "vt" ) == 0 )
            {
                glm::vec2 uv;
                fscanf(file, "%f %f\n", &uv.x, &uv.y );
                tempUvs.push_back(uv);
            }
            //3 float of normals
            else if ( strcmp( lineHeader, "vn" ) == 0 )
            {
                glm::vec3 normal;
                fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z );
                tempNormals.push_back(normal);
            }
            //Face information
            else if ( strcmp( lineHeader, "f" ) == 0 )
            {
                std::string vertex1, vertex2, vertex3;
                unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
                int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n",
                                    &vertexIndex[0],
                                    &uvIndex[0],
                                    &normalIndex[0],
                                    &vertexIndex[1],
                                    &uvIndex[1],
                                    &normalIndex[1],
                                    &vertexIndex[2],
                                    &uvIndex[2],
                                    &normalIndex[2] );
                if (matches != 9){
                    qInfo("Having problem reading faces");
                    return false;
                }


                /***/
                /***** The Front Face *****/
                /***/

                vertexIndices.push_back(vertexIndex[0]);
                vertexIndices.push_back(vertexIndex[1]);
                vertexIndices.push_back(vertexIndex[2]);
                uvIndices    .push_back(uvIndex[0]);
                uvIndices    .push_back(uvIndex[1]);
                uvIndices    .push_back(uvIndex[2]);
                normalIndices.push_back(normalIndex[0]);
                normalIndices.push_back(normalIndex[1]);
                normalIndices.push_back(normalIndex[2]);

                /***/
                /***** The Back Face *****/
                /***/

                //Back face
                vertexIndices.push_back(vertexIndex[0]);
                vertexIndices.push_back(vertexIndex[2]);
                vertexIndices.push_back(vertexIndex[1]);
                uvIndices    .push_back(uvIndex[0]);
                uvIndices    .push_back(uvIndex[1]);
                uvIndices    .push_back(uvIndex[2]);
                normalIndices.push_back(normalIndex[0]);
                normalIndices.push_back(normalIndex[2]);
                normalIndices.push_back(normalIndex[1]);

            }
        }
    }

    //Start processing the informaitons
    for( unsigned int i=0; i<vertexIndices.size(); i++ )
    {
        unsigned int vertexIndex = vertexIndices[i];
        vertices.push_back(tempVertices[ vertexIndex-1 ]);     //To the final referenced vertices

        //Color setting
        colors.push_back(vec3(0, 0, 1));
    }

    for( unsigned int i=0; i<uvIndices.size(); i++ )
    {
        unsigned int uvIndex = uvIndices[i];
        uvs.push_back(tempUvs[ uvIndex-1 ]);     //To the final referenced vertices
    }

    for( unsigned int i=0; i<normalIndices.size(); i++ )
    {
        unsigned int normalIndex = normalIndices[i];
        normals.push_back(tempNormals[ normalIndex-1 ]);     //To the final referenced vertices
    }

    return true;
}

GLuint Object::LoadShader(const char * vertex_file_path, const char * fragment_file_path)
{
        qInfo("Start loading shader\n");

        // Create the shaders
        //QOpenGLShader vertShader(QOpenGLShader::Vertex);
        //QOpenGLShader fragShader(QOpenGLShader::Fragment);
        GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
        GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

        // Read the Vertex Shader code from the file
        std::string VertexShaderCode;
        std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
        if(VertexShaderStream.is_open()){
            std::stringstream sstr;
            sstr << VertexShaderStream.rdbuf();
            VertexShaderCode = sstr.str();
            VertexShaderStream.close();
        }else{
            qInfo("Impossible to open %s. Are you in the right directory ?\n", vertex_file_path);
            getchar();
            return 0;
        }

        // Read the Fragment Shader code from the file
        std::string FragmentShaderCode;
        std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
        if(FragmentShaderStream.is_open()){
            std::stringstream sstr;
            sstr << FragmentShaderStream.rdbuf();
            FragmentShaderCode = sstr.str();
            FragmentShaderStream.close();
        }else{
            qInfo("Impossible to open %s. Are you in the right directory ?\n", fragment_file_path);
            getchar();
            return 0;
        }

        GLint Result = GL_FALSE;
        int InfoLogLength;

        // Compile Vertex Shader
        qInfo("Compiling shader : %s\n", vertex_file_path);
        char const * VertexSourcePointer = VertexShaderCode.c_str();
        //vertShader.compileSourceCode(VertexSourcePointer);
        glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
        glCompileShader(VertexShaderID);

        // Check Vertex Shader
        /*
        glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
        glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
        if ( InfoLogLength > 0 ){
            std::vector<char> VertexShaderErrorMessage(InfoLogLength+1);
            glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
            qInfo("%s\n", &VertexShaderErrorMessage[0]);
        }
        */

        // Compile Fragment Shader
        qInfo("Compiling shader : %s\n", fragment_file_path);
        char const * FragmentSourcePointer = FragmentShaderCode.c_str();
        //fragShader.compileSourceCode(FragmentSourcePointer);
        glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
        glCompileShader(FragmentShaderID);

        // Check Fragment Shader
        /*
        glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
        glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
        if ( InfoLogLength > 0 ){
            std::vector<char> FragmentShaderErrorMessage(InfoLogLength+1);
            glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
            qInfo("%s\n", &FragmentShaderErrorMessage[0]);
        }
        */

        // Link the program
        /*
        qInfo("Linking program with context: %d\n", currentContext);
        QOpenGLShaderProgram shaderProgram(currentContext);
        GLuint ProgramID = shaderProgram.programId();
        shaderProgram.addShader(&vertShader);
        shaderProgram.addShader(&fragShader);
        if(shaderProgram.link())
        {
            qInfo("Link succeed.\n");
        }
        else
        {
            qInfo("Link Failed.\n");
            shaderProgram.log();
        }
        */

        GLuint ProgramID = glCreateProgram();
        glAttachShader(ProgramID, VertexShaderID);
        glAttachShader(ProgramID, FragmentShaderID);
        glLinkProgram(ProgramID);

        // Check the program
        /*
        glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
        glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
        if ( InfoLogLength > 0 ){
            std::vector<char> ProgramErrorMessage(InfoLogLength+1);
            glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
            qInfo("%s\n", &ProgramErrorMessage[0]);
        }
        */

        //Bind the current shader program
        /*
        if(shaderProgram.bind())
        {
            qInfo("Bind succeed.\n");
            shaderProgram.release();
        }
        else
        {
            qInfo("Bind Failed.\n");
            shaderProgram.log();
        }
        */

        glDetachShader(ProgramID, VertexShaderID);
        glDetachShader(ProgramID, FragmentShaderID);

        glDeleteShader(VertexShaderID);
        glDeleteShader(FragmentShaderID);

        qInfo("Returning shader program ID, compile finish.\n");
        return ProgramID;
}

GLuint Object::LoadTexture(const char * imagepath)
{
    // Data read from the header of the BMP file
    unsigned char header[54]; // Each BMP file begins by a 54-bytes header
    unsigned int dataPos;     // Position in the file where the actual data begins
    unsigned int width, height;
    unsigned int imageSize;   // = width*height*3

    // Actual RGB data
    unsigned char * data;

    // Open the file
    FILE * file = fopen(imagepath,"rb");
    if (!file){qInfo("== Image could not be opened\n"); return 0;}

    if ( fread(header, 1, 54, file)!=54 ){ // If not 54 bytes read : problem
        qInfo("== Not a correct BMP file\n");
        return false;
    }

    if ( header[0]!='B' || header[1]!='M' ){
        qInfo("== Not a correct BMP file\n");
        return 0;
    }

    // Read ints from the byte array
    dataPos    = *(int*)&(header[0x0A]);
    imageSize  = *(int*)&(header[0x22]);
    width      = *(int*)&(header[0x12]);
    height     = *(int*)&(header[0x16]);

    // Some BMP files are misformatted, guess missing information
    if (imageSize==0)    imageSize=width*height*3; // 3 : one byte for each Red, Green and Blue component
    if (dataPos==0)      dataPos=54; // The BMP header is done that way

    // Create a buffer
    data = new unsigned char [imageSize];

    // Read the actual data from the file into the buffer
    fread(data,1,imageSize,file);

    //Everything is in memory now, the file can be closed
    fclose(file);

    // Create one OpenGL texture
    GLuint textureID;
    glGenTextures(1, &textureID);

    // "Bind" the newly created texture : all future texture functions will modify this texture
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Give the image to OpenGL
    glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    return textureID;
}

Object::Object()
{
    initializeOpenGLFunctions();

    color = vec3(0, 0, 1);

    objMovement = vec3(0, 0, 0);

    objectRotation = quat(1, 0, 0, 0);

    ifShowBoundBox = false;
    ifLineMode = false;
    ifShowCurLine = false;
    ifRender = true;
    ifVertexColor = false;

    L = 0.1f;
}

Object::~Object()
{

}

int Object::GetID()
{
    return id;
}

vec3 Object::GetColor()
{
    return color;
}

void Object::SetColor(vec3 newColor)
{
    color = newColor;
}

void Object::Draw(mat4 &Translation, mat4 &mvp, mat4 &m, mat4 &v, vec3 lightColor,
                  vec3 lightPos, vec3 lightPos2)
{
    //qInfo("Drawing\n");

    /***** Draw mesh *****/

    //Bind the vertex array
    glBindVertexArray(vertexArrayID);

    //glBindVertexArray(vi);

    //Bind the shader
    glUseProgram(shaderProgID);

    //MVP matrix
    GLuint MatrixID = glGetUniformLocation(shaderProgID, "MVP");
    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &mvp[0][0]);

    //M matrix
    GLuint MMatrixID = glGetUniformLocation(shaderProgID, "M");
    glUniformMatrix4fv(MMatrixID, 1, GL_FALSE, &m[0][0]);

    //V matrix
    GLuint VMatrixID = glGetUniformLocation(shaderProgID, "V");
    glUniformMatrix4fv(VMatrixID, 1, GL_FALSE, &v[0][0]);

    //Light position
    GLuint lishgPosID = glGetUniformLocation(shaderProgID, "lightPos");
    glUniform3f(lishgPosID, lightPos.x, lightPos.y, lightPos.z);

    //Light position2
    GLuint lishgPos2ID = glGetUniformLocation(shaderProgID, "lightPos2");
    glUniform3f(lishgPos2ID, lightPos2.x, lightPos2.y, lightPos2.z);

    //Light color
    GLuint lishgColID = glGetUniformLocation(shaderProgID, "lightColor");
    glUniform3f(lishgColID, lightColor.x, lightColor.y, lightColor.z);

    //Object color
    GLuint colorID = glGetUniformLocation(shaderProgID, "objectColor");
    glUniform3f(colorID, color.x, color.y, color.z);

    //Using object or per vertex color
    GLint checkBoardID = glGetUniformLocation(shaderProgID, "ifCheckBoard");
    glUniform1i(checkBoardID, ifVertexColor);

    //trans matrix
    GLint uniTrans = glGetUniformLocation(shaderProgID, "trans");
    glUniformMatrix4fv(uniTrans, 1, GL_FALSE, value_ptr(Translation));

    //Draw the scene
    if(ifLineMode)
    {
        glLineWidth(1);
        glDrawArrays(GL_LINES, 0, vertices.size()); //Starting from vertex 0
    }
    else
        glDrawArrays(GL_TRIANGLES, 0, vertices.size());

    /* Clean up */
    glUseProgram(0);

    //Detach the VAO
    glBindVertexArray(0);


    /***** Draw box *****/

    /*
    if(ifShowBoundBox)
    {
        //Bind the vertex array
        glBindVertexArray(boxArrayID);

        //glBindVertexArray(vi);

        //Bind the shader
        glUseProgram(boxShaderProgID);

        //MVP matrix
        MatrixID = glGetUniformLocation(boxShaderProgID, "MVP");
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &mvp[0][0]);

        //trans matrix
        uniTrans = glGetUniformLocation(boxShaderProgID, "trans");
        glUniformMatrix4fv(uniTrans, 1, GL_FALSE, value_ptr(Translation));

        //Draw the scene
        glLineWidth(5);
        glDrawArrays(GL_LINES, 0, boxVertices.size()); //Starting from vertex 0

        /* Clean up *
        glUseProgram(0);

        //Detach the VAO
        glBindVertexArray(0);
    }
    */


    /***** Draw extra content *****/
    DrawExtra(MatrixID, uniTrans, mvp, Translation);

    //qInfo("Finish drawing\n");
}

void Object::ResetPosition()
{
    objMovement = vec3(0, 0, 0);

    objectRotation = quat(1, 0, 0, 0);
}

float Object::GenBoundBox()
{
    float xMin = 999;
    float xMax = -999;
    float yMin = 999;
    float yMax = -999;
    float zMin = 999;
    float zMax = -999;

    //Iterate through the vertices list to find the bound box
    for(int i = 0; i < (int)vertices.size(); i++)
    {
        if(vertices[i].x > xMax)
            xMax = vertices[i].x;
        if(vertices[i].x < xMin)
            xMin = vertices[i].x;

        if(vertices[i].y > yMax)
            yMax = vertices[i].y;
        if(vertices[i].y < yMin)
            yMin = vertices[i].y;

        if(vertices[i].z > zMax)
            zMax = vertices[i].z;
        if(vertices[i].z < zMin)
            zMin = vertices[i].z;
    }

    qInfo("Box information: %f, %f, %f, %f, %f, %f\n",
          xMin, xMax,
          yMin, yMax,
          zMin, zMax);

    //Append the bound box vertices into the list
    //    8________7
    //   /        /
    //  /        /
    // 5________6
    // |
    // |  4_______3
    // | /       /
    // |/       /
    // 1_______2

    vec3 vert1 = vec3(xMin, yMin, zMin);
    vec3 vert2 = vec3(xMax, yMin, zMin);
    vec3 vert3 = vec3(xMax, yMin, zMax);
    vec3 vert4 = vec3(xMin, yMin, zMax);

    vec3 vert5 = vec3(xMin, yMax, zMin);
    vec3 vert6 = vec3(xMax, yMax, zMin);
    vec3 vert7 = vec3(xMax, yMax, zMax);
    vec3 vert8 = vec3(xMin, yMax, zMax);

    //Lower face
    boxVertices.push_back(vert1);
    boxVertices.push_back(vert2);

    boxVertices.push_back(vert2);
    boxVertices.push_back(vert3);

    boxVertices.push_back(vert3);
    boxVertices.push_back(vert4);

    boxVertices.push_back(vert4);
    boxVertices.push_back(vert1);

    //Upper face
    boxVertices.push_back(vert5);
    boxVertices.push_back(vert6);

    boxVertices.push_back(vert6);
    boxVertices.push_back(vert7);

    boxVertices.push_back(vert7);
    boxVertices.push_back(vert8);

    boxVertices.push_back(vert8);
    boxVertices.push_back(vert5);

    //Verticle line
    boxVertices.push_back(vert5);
    boxVertices.push_back(vert1);

    boxVertices.push_back(vert6);
    boxVertices.push_back(vert2);

    boxVertices.push_back(vert7);
    boxVertices.push_back(vert3);

    boxVertices.push_back(vert8);
    boxVertices.push_back(vert4);

    //Find and return the maximum radius, to adjust the viewport
    if(distance(vert1, vert7) > distance(vert2, vert8))
        return distance(vert1, vert7);
    else
        return distance(vert2, vert8);
}

void Object::GenExtra()
{

}

void Object::DrawExtra(GLuint MatrixID, GLuint uniTrans, mat4 &mvp, mat4 &Translation)
{

}

void Object::GenBuffer()
{
    //Generate the vertex array object and start store information
    glGenVertexArrays(1, &vertexArrayID);

    //Generate the vertex buffer object for the surface mesh
    glGenBuffers(1, &surfaceBufferID);

    //Generate the vertex buffer object for the surface mesh
    glGenBuffers(1, &colorBufferID);

    //Generate the vertex buffer object for the surface mesh
    glGenBuffers(1, &normalBufferID);

    //Generate the vertex array object and start store information
    glGenVertexArrays(1, &boxArrayID);

    //Generate the vertex buffer object for the surface mesh
    glGenBuffers(1, &boxBufferID);
}

void Object::WriteColorBuffer(int mode)
{
    qInfo("== Writing color buffer.\n");

    //Bind the current buffer
    glBindBuffer(GL_ARRAY_BUFFER, colorBufferID);

    if(mode == 1)       //Using the original color of object
    {
        //Load information into the buffer
        glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(vec3),
                     &colors[0], GL_STATIC_DRAW);
    }
    else if(mode == 2)  //Using the check board color
    {
        //Load information into the buffer
        glBufferData(GL_ARRAY_BUFFER, checkBoardColors.size() * sizeof(vec3),
                     &checkBoardColors[0], GL_STATIC_DRAW);
    }
    else if(mode == 3)  //Using the heat diffusion color
    {
        //Load information into the buffer
        glBufferData(GL_ARRAY_BUFFER, heatColors.size() * sizeof(vec3),
                     &heatColors[0], GL_STATIC_DRAW);
    }
    else if(mode == 4)  //Using the heat diffusion color
    {
        //Load information into the buffer
        glBufferData(GL_ARRAY_BUFFER, curvatureColors.size() * sizeof(vec3),
                     &curvatureColors[0], GL_STATIC_DRAW);
    }

    //Set the vertex pointer to the VAO
    glVertexAttribPointer(
       1,                  //Attribute 1. No particular reason for 0, but must match the layout in the shader.
       3,                  //Size
       GL_FLOAT,           //Type
       GL_FALSE,           //Normalized?
       0,  //Stride
       (void*)0            //Array buffer offset
    );

    //Enable the attribute array
    glEnableVertexAttribArray(1);

    qInfo("== Color buffer mode %d wrote.\n", mode);
}

float Object::UpdateObject()
{
    //Bind the VAO
    glBindVertexArray(vertexArrayID);

    qInfo("Create vertex buffer");

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


    //
    /***** Color buffer and attribute pointer *****/
    //

    WriteColorBuffer(1);


    //
    /***** Normal buffer and attribute pointer *****/
    //

    //Bind the current buffer
    glBindBuffer(GL_ARRAY_BUFFER, normalBufferID);

    //Load information into the buffer
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(vec3),
                 &normals[0], GL_STATIC_DRAW);

    //Set the vertex pointer to the VAO
    glVertexAttribPointer(
       2,                  //Attribute 0. No particular reason for 0, but must match the layout in the shader.
       3,                  //Size
       GL_FLOAT,           //Type
       GL_FALSE,           //Normalized?
       0,  //Stride
       (void*)0            //Array buffer offset
    );

    //Enable the attribute array
    glEnableVertexAttribArray(2);


    //
    /***** Textures *****/
    //

    textureID = LoadTexture("./textures/checkerboard.bmp");

    //Detach the VAO
    glBindVertexArray(0);

    //
    /***** Box VAO VBO and attribute pointer *****/
    //

    //Generate bounding box info
    float maxRadius = GenBoundBox();

    //Bind the VAO
    glBindVertexArray(boxArrayID);

    //Bind the current buffer
    glBindBuffer(GL_ARRAY_BUFFER, boxBufferID);

    //Load information into the buffer
    glBufferData(GL_ARRAY_BUFFER, boxVertices.size() * sizeof(vec3),
                 &boxVertices[0], GL_STATIC_DRAW);

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

    //Detach the VAO
    glBindVertexArray(0);

    return maxRadius;
    //return 1;
}

float Object::GenObject()
{
    /*** Generate Object ***/

    qInfo("Start generating the obejct");

    GenBuffer();

    //Write to buffer, bind VAO with VBO
    float maxRadius = UpdateObject();

    /***** Generate extra object *****/
    GenExtra();

    //Create the shader program
    shaderProgID = LoadShader("shaders/Light.vert", "shaders/Light.frag");

    boxShaderProgID = LoadShader("shaders/Box.vert", "shaders/Box.frag");

    qInfo("Object created, Shader ID: %d and %d", shaderProgID, boxShaderProgID);
    qInfo("Max Radius: %f\n", maxRadius);

    return maxRadius;
}

int funcF(int val)
{
    if (val % 2 == 0)
        return 1;
    else
        return 0;
}

void Object::CalcCheckBoardTexture()
{
    //Clear the color buffer
    checkBoardColors.clear();

    //Check for each vertex
    for(int i = 0; i < vertices.size(); i++)
    {
        checkBoardColors.push_back(vec3(funcF(vertices[i].x / L),
                                        funcF(vertices[i].y / L),
                                        funcF(vertices[i].z / L)));
    }

    //Bind the VAO
    glBindVertexArray(vertexArrayID);

    //Write new color buffer
    WriteColorBuffer(2);

    //Detach the VAO
    glBindVertexArray(0);
}

































