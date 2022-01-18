#include "objectcomplex.h"

ComplexObject::ComplexObject()
{

}

ComplexObject::~ComplexObject()
{
    for(int i = 0; i < pointList.size(); i++)
    {
        free(pointList[i]);
    }
    for(int i = 0; i < edgeList.size(); i++)
    {
        free(edgeList[i]);
    }
    for(int i = 0; i < faceList.size(); i++)
    {
        free(faceList[i]);
    }
}

bool ComplexObject::ObjLoader(const char * path)
{
    vector< unsigned int > vertexIndices, uvIndices, normalIndices;
    //vector< glm::vec3 > tempVertices;
    vector< glm::vec2 > tempUvs;
    vector< glm::vec3 > tempNormals;

    FILE * file = fopen(path, "r");
    if( file == NULL ){
        qInfo("File open fail\n");
        return false;
    }
    else{
        qInfo("Mesh file open succeed!");
    }

    //Read the file
    while(true)
    {
        char lineHeader[128];
        //Read the first word of the line
        int res = fscanf(file, "%s", lineHeader);

        //Quit the loop if meet the end of file
        if (res == EOF){
            break;
        }
        else
        {
            //3 float of vertices informations
            if ( strcmp( lineHeader, "v" ) == 0 )
            {
                glm::vec3 vertex;
                fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z );

                OPoint * newPoint = new OPoint;     //Create a new vertex
                newPoint->position = vertex;        //Store the position
                pointList.push_back(newPoint);
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
                int matches = fscanf(file, "%d//%d %d//%d %d//%d\n",
                                    &vertexIndex[0],
                                    &normalIndex[0],
                                    &vertexIndex[1],
                                    &normalIndex[1],
                                    &vertexIndex[2],
                                    &normalIndex[2] );
                if (matches != 6){
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

                //Create edge
                OEdge * newEdge1, * newEdge2, * newEdge3;

                bool exist = false;
                //Check if the edge already exist, if it exist then one of the point must linked
                if(pointList[vertexIndex[0]-1]->edges.size() != 0)
                {
                    //Check each edge, if the current edge already exist
                    for(int i = 0; i < pointList[vertexIndex[0]-1]->edges.size(); i++)
                    {
                        //The terminal points of target edge
                        OPoint * p1 = pointList[vertexIndex[0]-1]->edges[i]->points[0];
                        OPoint * p2 = pointList[vertexIndex[0]-1]->edges[i]->points[1];

                        //Check both order
                        if((p1 == pointList[vertexIndex[0]-1] && p2 == pointList[vertexIndex[1]-1]) ||
                            (p1 == pointList[vertexIndex[1]-1] && p2 == pointList[vertexIndex[0]-1]))
                        {
                            exist = true;
                            //If the edge already exist, reference it
                            //qInfo("Same Edge\n");
                            newEdge1 = pointList[vertexIndex[0]-1]->edges[i];
                        }
                    }
                }
                //If a new edge, add edge and bind points
                if(!exist)
                {
                    newEdge1 = new OEdge;
                    //Add point to edge
                    newEdge1->points.push_back(pointList[vertexIndex[0]-1]);
                    newEdge1->points.push_back(pointList[vertexIndex[1]-1]);
                    //Link the edge to both point
                    pointList[vertexIndex[0]-1]->edges.push_back(newEdge1);
                    pointList[vertexIndex[1]-1]->edges.push_back(newEdge1);
                    edgeList.push_back(newEdge1);
                }

                exist = false;
                if(pointList[vertexIndex[1]-1]->edges.size() != 0)
                {
                    //Check each edge, if the current edge already exist
                    for(int i = 0; i < pointList[vertexIndex[1]-1]->edges.size(); i++)
                    {
                        OPoint * p1 = pointList[vertexIndex[1]-1]->edges[i]->points[0];
                        OPoint * p2 = pointList[vertexIndex[1]-1]->edges[i]->points[1];

                        if((p1 == pointList[vertexIndex[1]-1] && p2 == pointList[vertexIndex[2]-1]) ||
                            (p1 == pointList[vertexIndex[2]-1] && p2 == pointList[vertexIndex[1]-1]))
                        {
                            exist = true;
                            //If the edge already exist, reference it
                            //qInfo("Same Edge\n");
                            newEdge2 = pointList[vertexIndex[1]-1]->edges[i];
                        }
                    }
                }
                if(!exist)
                {
                    newEdge2 = new OEdge;
                    //Add point to edge
                    newEdge2->points.push_back(pointList[vertexIndex[1]-1]);
                    newEdge2->points.push_back(pointList[vertexIndex[2]-1]);
                    //Link the edge to both point
                    pointList[vertexIndex[1]-1]->edges.push_back(newEdge2);
                    pointList[vertexIndex[2]-1]->edges.push_back(newEdge2);
                    edgeList.push_back(newEdge2);
                }

                exist = false;
                if(pointList[vertexIndex[2]-1]->edges.size() != 0)
                {
                    //Check each edge, if the current edge already exist
                    for(int i = 0; i < pointList[vertexIndex[2]-1]->edges.size(); i++)
                    {
                        OPoint * p1 = pointList[vertexIndex[2]-1]->edges[i]->points[0];
                        OPoint * p2 = pointList[vertexIndex[2]-1]->edges[i]->points[1];

                        if((p1 == pointList[vertexIndex[2]-1] && p2 == pointList[vertexIndex[0]-1]) ||
                            (p1 == pointList[vertexIndex[0]-1] && p2 == pointList[vertexIndex[2]-1]))
                        {
                            exist = true;
                            //If the edge already exist, reference it
                            //qInfo("Same Edge\n");
                            newEdge3 = pointList[vertexIndex[2]-1]->edges[i];
                        }
                    }
                }
                if(!exist)
                {
                    newEdge3 = new OEdge;
                    newEdge3->points.push_back(pointList[vertexIndex[2]-1]);
                    newEdge3->points.push_back(pointList[vertexIndex[0]-1]);
                    //Link the edge to both point
                    pointList[vertexIndex[2]-1]->edges.push_back(newEdge3);
                    pointList[vertexIndex[0]-1]->edges.push_back(newEdge3);
                    edgeList.push_back(newEdge3);
                }

                /*--- New Face ---*/

                //Each face must only exist once in obj file
                OFace * newFace = new OFace;

                //Save the edges
                newFace->edges.push_back(newEdge1);
                newFace->edges.push_back(newEdge2);
                newFace->edges.push_back(newEdge3);

                //Bind the edges
                newEdge1->faces.push_back(newFace);
                newEdge2->faces.push_back(newFace);
                newEdge3->faces.push_back(newFace);

                //Save the points
                newFace->points.push_back(pointList[vertexIndex[0]-1]);
                newFace->points.push_back(pointList[vertexIndex[1]-1]);
                newFace->points.push_back(pointList[vertexIndex[2]-1]);

                vec3 v1 = pointList[vertexIndex[2]-1]->position - pointList[vertexIndex[1]-1]->position;
                vec3 v2 = pointList[vertexIndex[0]-1]->position - pointList[vertexIndex[1]-1]->position;
                newFace->normal = normalize(cross(v1, v2));

                //Bind the points
                pointList[vertexIndex[0]-1]->faces.push_back(newFace);
                pointList[vertexIndex[1]-1]->faces.push_back(newFace);
                pointList[vertexIndex[2]-1]->faces.push_back(newFace);

                faceList.push_back(newFace);

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
        vertices.push_back(pointList[ vertexIndex-1 ]->position);     //To the final referenced vertices

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

void ComplexObject::GenExtra()
{
    //
    /***** Cur VAO VBO and attribute pointer *****/
    //

    if(curVertices.size() != 0)
    {
        //Generate the vertex array object and start store information
        glGenVertexArrays(1, &curArrayID);

        //Bind the VAO
        glBindVertexArray(curArrayID);

        //Generate the vertex buffer object for the surface mesh
        glGenBuffers(1, &curBufferID);

        //Bind the current buffer
        glBindBuffer(GL_ARRAY_BUFFER, curBufferID);

        //Load information into the buffer
        glBufferData(GL_ARRAY_BUFFER, curVertices.size() * sizeof(vec3),
                     &curVertices[0], GL_STATIC_DRAW);

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
    }
}

void ComplexObject::DrawExtra(GLuint MatrixID, GLuint uniTrans, mat4 &mvp, mat4 &Translation)
{
    /***** Draw cur *****/

    if(curVertices.size() != 0 && ifShowCurLine)
    {
        //Bind the vertex array
        glBindVertexArray(curArrayID);

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
        glDrawArrays(GL_LINES, 0, curVertices.size()); //Starting from vertex 0

        /* Clean up */
        glUseProgram(0);

        //Detach the VAO
        glBindVertexArray(0);
    }

}

void ComplexObject::GenCurvature()
{
    OFace * currentFace = faceList[12465];
    vec3 currentPosition = (currentFace->points[0]->position + currentFace->points[1]->position + currentFace->points[2]->position) / 3.0f;
    int iteration = 0;
    vec3 lastCastVec = vec3(0, 0, 0);

    while(iteration < 100)
    {
        /*
         * dot = x1*x2 + y1*y2 + z1*z2    #between [x1, y1, z1] and [x2, y2, z2]
            lenSq1 = x1*x1 + y1*y1 + z1*z1
            lenSq2 = x2*x2 + y2*y2 + z2*z2
            angle = acos(dot/sqrt(lenSq1 * lenSq2))
        */

        //Calculate the surface area
        float area = 0.5 * sqrt(pow(currentFace->normal.x,2) +
                                pow(currentFace->normal.y,2) +
                                pow(currentFace->normal.z,2));

        //Check the error
        if(currentFace->edges.size() != 3)
            qInfo("Edge num for face incorrect!\n");

        //Create the summation matrix
        Eigen::MatrixXd M = Eigen::MatrixXd::Zero(3, 3);

        //Iterate through each side
        for(int i = 0; i < currentFace->edges.size(); i++)
        {
            qInfo("-----\n");
            OEdge * currentEdge = currentFace->edges[i];
            //Check
            if(currentEdge->points.size() != 2)
                qInfo("Point num for edge incorrect!\n");

            //Calculate angle
            vec3 currentNorm = currentFace->normal;
            vec3 otherNorm;
            if(currentEdge->faces.size() == 1)
            {
                qInfo("Single face edge\n");
                otherNorm = currentNorm;
            }
            else
            {
                if(currentEdge->faces[0] == currentFace)
                    otherNorm = currentEdge->faces[1]->normal;
                else if(currentEdge->faces[1] == currentFace)
                    otherNorm = currentEdge->faces[0]->normal;
            }
            float dot = currentNorm.x * otherNorm.x + currentNorm.y * otherNorm.y + currentNorm.z * otherNorm.z;
            float angle;
            if(abs(dot - 1.0f) > 0.000001)
                angle = acos(dot);
            else
                angle = 0;
            //qInfo("c Norm: %f, %f, %f\n", currentNorm.x, currentNorm.y, currentNorm.z);
            //qInfo("o Norm: %f, %f, %f\n", otherNorm.x, otherNorm.y, otherNorm.z);
            //qInfo("Dot: %f, Angle: %f\n", dot, angle);

            //Calculate v
            vec3 v = currentEdge->points[0]->position - currentEdge->points[1]->position;

            //Calculate t
            vec3 t = normalize(cross(currentNorm, v)) * float(sqrt(pow(v.x, 2) + pow(v.y, 2) + pow(v.z, 2)));
            //qInfo("t%d: %f, %f, %f\n", i, t.x, t.y, t.z);

            //Build the t tensor product matrix
            Eigen::MatrixXd m(3,3);
            m(0, 0) = t.x * t.x; m(0, 1) = t.x * t.y; m(0, 2) = t.x * t.z;
            m(1, 0) = t.y * t.x; m(1, 1) = t.y * t.y; m(1, 2) = t.y * t.z;
            m(2, 0) = t.z * t.x; m(2, 1) = t.z * t.y; m(2, 2) = t.z * t.z;

            /*
            qInfo("m before:\n");
            for(int i = 0; i < 3; i++)
            {
                qInfo("%f, %f, %f\n", m(i, 0), m(i, 1), m(i, 2));
            }
            qInfo("\n");
            */

            //Sum up
            //qInfo("%f\n", angle);
            m = m * angle * 0.5 / (area * sqrt(pow(v.x, 2) + pow(v.y, 2) + pow(v.z, 2)));

            /*
            qInfo("m after:\n");
            for(int i = 0; i < 3; i++)
            {
                qInfo("%f, %f, %f\n", m(i, 0), m(i, 1), m(i, 2));
            }
            qInfo("\n");
            */

            M += m;
        }

        for(int i = 0; i < 3; i++)
        {
            qInfo("%f, %f, %f\n", M(i, 0), M(i, 1), M(i, 2));
        }
        qInfo("\n");

        Eigen::EigenSolver<Eigen::MatrixXd> es(M);

        int position;
        float max = -9999;
        for(int i = 0; i < 3; i++)
        {
            //qInfo("Eigen value: %f\n", es.eigenvalues().col(0)[i].real());
            if(es.eigenvalues().col(0)[i].real() > max)
            {
                max = es.eigenvalues().col(0)[i].real();
                position = i;
            }
        }
        //qInfo("Largest value: %f\n\n", es.eigenvalues().col(0)[position].real());
        /*
        qInfo("%f\n", es.eigenvalues().col(0)[position].real());
        qInfo("%f %f %f\n", es.eigenvectors().col(position)[0].real(),
                            es.eigenvectors().col(position)[1].real(),
                            es.eigenvectors().col(position)[2].real());
                            */


        //Find eigen vector and cast onto the triangle
        vec3 eigenVector = vec3(es.eigenvectors().col(position)[0].real(),
                                es.eigenvectors().col(position)[1].real(),
                                es.eigenvectors().col(position)[2].real());

        vec3 castVec;
        if(abs(dot(eigenVector, currentFace->normal)-0.0) > 0.000001)
        {
            qInfo("EigenVector not tangent! %f", dot(eigenVector, currentFace->normal));
            vec3 norm = currentFace->normal;
            vec3 tempVec = normalize(cross(eigenVector, norm));
            castVec = normalize(cross(norm, tempVec));
            castVec = dot(eigenVector, castVec) * castVec;
        }
        else
            castVec = eigenVector;

        if(iteration == 0)
            lastCastVec = castVec;
        else
        {
            if(dot(lastCastVec, castVec) < 0)
                castVec = castVec * -1.0f;
        }

        //qInfo("EigenVec: %f, %f, %f\n\n", castVec.x, castVec.y, castVec.z);

        //Find which edge pointing
        int maxPos = 0;
        max = -9999;
        for(int i = 0; i < currentFace->edges.size(); i++)
        {
            OEdge * currentEdge = currentFace->edges[i];

            //Calculate v
            vec3 v = currentEdge->points[0]->position - currentEdge->points[1]->position;

            //Calculate t
            vec3 currentNorm = currentFace->normal;
            vec3 t = normalize(cross(currentNorm, v));

            if(dot(t, castVec) > max)
            {
                max = dot(t, castVec);
                maxPos = i;
            }
            else if(dot(castVec, t) == max) //If pointing to a vertex or something
            {
                qInfo("Nani?\n");
            }
        }

        vec3 v = castVec;
        vec3 p = currentPosition;
        vec3 x2 = currentFace->edges[maxPos]->points[1]->position;
        vec3 x1 = currentFace->edges[maxPos]->points[0]->position;
        vec3 edge = x2 - x1;
        float k = (1 - ((p.x - x1.x) * (x2.x - x1.x) + (p.y - x1.y) * (x2.y - x1.y) + (p.z - x1.z) * (x2.z - x1.z))) /
                    (v.x * (x2.x - x1.x) + v.y * (x2.y - x1.y) + v.z * (x2.z - x1.z));
        //qInfo("K: %f\n", k);

        //Update current position
        curVertices.push_back(currentPosition);
        currentPosition += castVec * 3.0f;
        curVertices.push_back(currentPosition);

        //Get next face
        OEdge * targetEdge = currentFace->edges[maxPos];
        if(targetEdge->faces.size() == 1)
            break;
        else if( targetEdge->faces.size() == 2)
        {
            if(targetEdge->faces[0] == currentFace)
                currentFace = targetEdge->faces[1];
            else
                currentFace = targetEdge->faces[0];
        }

        currentPosition = (currentFace->points[0]->position + currentFace->points[1]->position + currentFace->points[2]->position) / 3.0f;

        iteration ++;
        //break;
    }
}









