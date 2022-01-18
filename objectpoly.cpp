#include "objectpoly.h"

ObjectPoly::ObjectPoly(QString filePath)
{
    initializeOpenGLFunctions();

    qInfo("Pre-Processing the poly object.\n");

    poly.PolyhedronCreate(fopen(filePath.toLocal8Bit().data(), "r"));

    qInfo("Finish Pre-Processing the poly!\n");

    poly.initialize();
    poly.calc_bounding_sphere();
    poly.calc_face_normals_and_area();
    poly.average_normals();

    //initialize the object
    silhouette = NULL;
    principalLine = NULL;
}

ObjectPoly::~ObjectPoly()
{
    poly.finalize();

    //free(poly);
}

void ObjectPoly::GenSilhouette(mat4 &Translation, mat4 &mvp, mat4 &m, mat4 &v)
{
    //Check edge
    if(poly.nedges != poly.elist.size())
        poly.nedges = poly.elist.size();

    int mode = 2;

    //Loop through each
    for(int i = 0; i < poly.elist.size(); i++)
    {
        Edge * currentE = poly.elist[i];

        Vertex *v1, *v2;
        Triangle *f1, *f2;

        if(mode == 1)
        {
            v1 = currentE->verts[0];
            v2 = currentE->verts[1];
        }
        else if(mode == 2)
        {
            if(currentE->tris.size() == 2)
            {
                f1 = currentE->tris[0];
                f2 = currentE->tris[1];
            }
            else
                continue;
        }

        //Check each vert
        float v_0[2] = {0, 0};

        for(int j = 0; j < 2; j++)
        {
            Vertex * cV;
            if(mode == 1)
                cV = currentE->verts[j];
            else if(mode == 2)
                cV = currentE->tris[j]->verts[0];

            vec4 Vc = ( v * m * Translation * vec4(vec3(cV->x, cV->y, cV->z),1));
            vec3 vertexPosition_cameraspace = vec3(Vc.x, Vc.y, Vc.z);
            vec3 E = normalize(vec3(0,0,0) - vertexPosition_cameraspace);

            vec3 vertexNormal;
            if(mode == 1)
                vertexNormal = vec3(cV->normal.x, cV->normal.y, cV->normal.z);
            else if(mode == 2)
                vertexNormal = vec3(currentE->tris[j]->normal.x,
                                    currentE->tris[j]->normal.y,
                                    currentE->tris[j]->normal.z);

            vec4 Nc = (v * m * Translation * vec4(vertexNormal ,0));
            vec3 N = normalize(vec3(Nc.x, Nc.y, Nc.z));

            float d = dot(E, N);

            v_0[j] = d;
        }

        //Check if sign different or 0
        if(v_0[0] * v_0[1] < 0)
        {
            if(mode == 1)
            {
                //Linear interpolate to find the 0 position
                float d = (0 - v_0[0]) / (v_0[1] - v_0[0]);

                vec3 Pos = d * (vec3(v2->x, v2->y, v2->z) - vec3(v1->x, v1->y, v1->z)) + vec3(v1->x, v1->y, v1->z);
                currentE->sflag.if0Pos = true;
                currentE->sflag.lPos = Pos;
            }
            else if(mode == 2)
                currentE->sflag.if0Pos = true;
        }
        else if(v_0[0] * v_0[1] == 0 && mode == 1)
        {
            currentE->sflag.if0Pos = true;

            if(v_0[0] == 0)
                currentE->sflag.lPos = vec3(v1->x, v1->y, v1->z);
            else
                currentE->sflag.lPos = vec3(v2->x, v2->y, v2->z);

            if(v_0[0] == 0 && v_0[1] == 0)
                currentE->sflag.if0Whol = true;
        }
    }

    //Loop through the triangles
    vector<vec3> sverts;

    if(mode == 1)
    {
        for(int i = 0; i < poly.tlist.size(); i++)
        {
            vector<vec3> lvlSet;

            Triangle * currentT = poly.tlist[i];

            //Loop through each edge to look for levelset
            int pNum = 0;
            int pos = 0;
            for(int j = 0; j < 3; j++)
            {
                //Check the flag of the edge
                if(currentT->edges[j]->sflag.if0Pos)
                {
                    pNum++;
                    if(currentT->edges[j]->sflag.if0Whol)
                    {
                        vec3 v1 = vec3(currentT->edges[j]->verts[0]->x, currentT->edges[j]->verts[0]->y, currentT->edges[j]->verts[0]->z);
                        vec3 v2 = vec3(currentT->edges[j]->verts[1]->x, currentT->edges[j]->verts[1]->y, currentT->edges[j]->verts[1]->z);
                        lvlSet.push_back(v1);
                        lvlSet.push_back(v2);
                    }
                    else
                        lvlSet.push_back(currentT->edges[j]->sflag.lPos);

                    //Reset the flag (clear data)
                    currentT->edges[j]->sflag.if0Pos = false;
                    pos = j;
                }
            }
            if(pNum == 1)
            {
                vec3 v1 = vec3(currentT->edges[pos]->verts[0]->x, currentT->edges[pos]->verts[0]->y, currentT->edges[pos]->verts[0]->z);
                vec3 v2 = vec3(currentT->edges[pos]->verts[1]->x, currentT->edges[pos]->verts[1]->y, currentT->edges[pos]->verts[1]->z);
                sverts.push_back(v1);
                sverts.push_back(v2);
            }
            //Save the silhouette segment
            if(pNum == 2)
            {
                sverts.push_back(lvlSet[0]);
                sverts.push_back(lvlSet[1]);
            }
            else if(pNum == 3)
            {
                sverts.push_back(lvlSet[0]);
                sverts.push_back(lvlSet[1]);

                sverts.push_back(lvlSet[1]);
                sverts.push_back(lvlSet[2]);

                sverts.push_back(lvlSet[2]);
                sverts.push_back(lvlSet[0]);
            }
        }
    }
    else if(mode == 2)
    {
        //Loop through the edge list
        for(int i = 0; i < poly.elist.size(); i++)
        {
            if(poly.elist[i]->sflag.if0Pos)
            {
                vec3 v1 = vec3(poly.elist[i]->verts[0]->x,
                               poly.elist[i]->verts[0]->y,
                               poly.elist[i]->verts[0]->z);
                vec3 v2 = vec3(poly.elist[i]->verts[1]->x,
                               poly.elist[i]->verts[1]->y,
                               poly.elist[i]->verts[1]->z);

                //Save the edge
                sverts.push_back(v1);
                sverts.push_back(v2);

                //Reset the flag
                poly.elist[i]->sflag.if0Pos = false;
            }
        }
    }

    //Create the silhouette object, only first time
    if(silhouette == NULL)
        silhouette = new ObjectSilhouette();

    //Create new data dor silhouette object
    silhouette->CreateSilhouette(sverts, vec3(0, 0, 0), 0);
}

void ObjectPoly::Draw(mat4 &Translation, mat4 &mvp, mat4 &m, mat4 &v, vec3 lightColor,
                  vec3 lightPos, vec3 lightPos2)
{
    /***** Calculate the silhouette *****/

    //GenSilhouette(Translation, mvp, m, v);

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
        //glDrawArrays(GL_LINE_STRIP, 0, vertices.size()); //Starting from vertex 0
        glDrawArrays(GL_LINES, 0, vertices.size()); //Starting from vertex 0
    }
    else
        glDrawArrays(GL_TRIANGLES, 0, vertices.size());

    /* Clean up */
    glUseProgram(0);

    //Detach the VAO
    glBindVertexArray(0);

    /***** Draw silhouette *****/

    if(silhouette != NULL)
    {
        silhouette->UpdateObject();
        silhouette->Draw(Translation, mvp, m, v, lightPos);
    }

    if(principalLine != NULL)
    {
        principalLine->UpdateObject();
        principalLine->Draw(Translation, mvp, m, v, lightPos);
    }


    /***** Draw box *****/

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

        /* Clean up */
        glUseProgram(0);

        //Detach the VAO
        glBindVertexArray(0);
    }
}

float ObjectPoly::ProcessData(int ID)
{
    id = ID;

    qInfo("Creating the poly's vertex data with %d tris.\n", poly.tlist.size());

    UpdateData();

    qInfo("Finish generating the poly vertexs!\n");

    return GenObject();
}

void ObjectPoly::ClearData()
{
    vertices.clear();
    normals.clear();
    colors.clear();
    boxVertices.clear();
}

vec3 ObjectPoly::calc_heat_color(float heat, float max)
{
    int rgbTotal = (255 * 3) * heat / max;

    //return vec3(heat, heat, heat);

    if(rgbTotal < 255)
        return ((vec3(0, 1, 1) - vec3(0, 0, 0)) *
                (rgbTotal / 255.0f) + vec3(0, 0, 0));
    else if(rgbTotal < 255 * 2)
        return ((vec3(1, 0, 1) - vec3(0, 1, 1)) *
                ((rgbTotal - 255) / 255.0f) + vec3(0, 1, 1));
    else if(rgbTotal <= 255 * 3)
        return ((vec3(1, 1, 1) - vec3(1, 0, 1)) *
                ((rgbTotal - 255 * 2) / 255.0f) + vec3(1, 0, 1));
    else
        return vec3(1, 1, 1);

}

vec3 ObjectPoly::calc_curvature_color(float cur, float max, float min, float avg)
{
    vec3 vcolor;
    float portion;

    float tMax = avg + (max - avg) / 20.0f;
    float tMin = avg - (avg - min) / 20.0f;


    if((cur >= tMin) && (cur <= avg))
    {
        portion = (avg - cur) / (avg - tMin);
        vcolor = float(pow(portion, 2)) * (vec3(0, 1, 0) - vec3(0, 0, 1)) + vec3(0, 0, 1);
    }
    else if((cur > avg) && (cur <= tMax))
    {
        portion = (cur - avg) / (tMax - avg);
        vcolor = float(pow(portion, 2)) * (vec3(1, 0, 0) - vec3(0, 1, 0)) + vec3(0, 1, 0);
    }
    else if (cur > tMax)
    {
        vcolor = vec3(1, 0, 0);
    }
    else if(cur < tMin)
    {
        vcolor = vec3(0, 0, 1);
    }

    //if(cur == 0)
        //vcolor = vec3(0, 1, 0);
    //else if(cur > 0)
        //vcolor = vec3(1, 0, 0);
    //else if(cur < 0)
        //vcolor = vec3(0, 0, 1);


    //vcolor = vec3(1, 0, 1);

    return vcolor;
}

void ObjectPoly::UpdateColorCurvature()
{
    qInfo("== Heat color calculating with %d verts.\n", poly.vlist.size());

    //Find the min and max
    float min = 999;
    float max = -999;
    float avg = 0.0;
    for(int i = 0; i < poly.vlist.size(); i++)
    {
        if(poly.vlist[i]->cInfo.Curvature > max)
            max = poly.vlist[i]->cInfo.Curvature;
        if(poly.vlist[i]->cInfo.Curvature < min)
            min = poly.vlist[i]->cInfo.Curvature;

        avg += poly.vlist[i]->cInfo.Curvature / poly.vlist.size();

        //qInfo("-- Curvature: %f\n", poly->vlist[i]->sflag.gaussianCurvature);
    }

    //Process the vertex to actual vertices
    for(int i = 0; i < poly.tlist.size(); i++)
    {
        Triangle* curT = poly.tlist[i];

        vec3 color1, color2, color3;
        //Front
        color1 = calc_curvature_color(curT->verts[0]->cInfo.Curvature, max, min, avg);
        color2 = calc_curvature_color(curT->verts[1]->cInfo.Curvature, max, min, avg);
        color3 = calc_curvature_color(curT->verts[2]->cInfo.Curvature, max, min, avg);

        //Back
        curvatureColors.push_back(color1);
        curvatureColors.push_back(color3);
        curvatureColors.push_back(color2);
    }

    qInfo("-- Curvature max: %f, min: %f, avg: %f\n", max, min, avg);

    qInfo("== Curvature color calculated.\n");
}

void ObjectPoly::UpdateColorDiffusion()
{    
    qInfo("== Heat color calculating with %d verts.\n", poly.vlist.size());

    //Process the vertex to actual vertices
    for(int i = 0; i < poly.tlist.size(); i++)
    {
        Triangle* curT = poly.tlist[i];

        vec3 color1, color2, color3;
        //Front
        color1 = calc_heat_color(curT->verts[0]->heat, poly.maxHeat);
        color2 = calc_heat_color(curT->verts[1]->heat, poly.maxHeat);
        color3 = calc_heat_color(curT->verts[2]->heat, poly.maxHeat);
        //heatColors.push_back(color1);
        //heatColors.push_back(color2);
        //heatColors.push_back(color3);

        //Back
        heatColors.push_back(color1);
        heatColors.push_back(color3);
        heatColors.push_back(color2);
    }

    qInfo("== Heat color calculated.\n");
}

void ObjectPoly::UpdateData()
{
    //Process the vertex to actual vertices
    for(int i = 0; i < poly.tlist.size(); i++)
    {
        Triangle* curT = poly.tlist[i];

        vec3 v1 = vec3(curT->verts[0]->x, curT->verts[0]->y, curT->verts[0]->z);
        vec3 n1 = vec3(curT->verts[0]->normal.x, curT->verts[0]->normal.y,curT->verts[0]->normal.z);

        vec3 v2 = vec3(curT->verts[1]->x, curT->verts[1]->y, curT->verts[1]->z);
        vec3 n2 = vec3(curT->verts[1]->normal.x, curT->verts[1]->normal.y,curT->verts[1]->normal.z);

        vec3 v3 = vec3(curT->verts[2]->x, curT->verts[2]->y, curT->verts[2]->z);
        vec3 n3 = vec3(curT->verts[2]->normal.x, curT->verts[2]->normal.y,curT->verts[2]->normal.z);

        //Front
        //
        vertices.push_back(v1);
        normals.push_back(n1);
        colors.push_back(vec3(0, 0, 1));
        vertices.push_back(v2);
        normals.push_back(n2);
        colors.push_back(vec3(0, 0, 1));
        vertices.push_back(v3);
        normals.push_back(n3);
        colors.push_back(vec3(0, 0, 1));
        //

        //Back
        vertices.push_back(v1);
        normals.push_back(n1);
        colors.push_back(vec3(0, 0, 1));
        vertices.push_back(v3);
        normals.push_back(n3);
        colors.push_back(vec3(0, 0, 1));
        vertices.push_back(v2);
        normals.push_back(n2);
        colors.push_back(vec3(0, 0, 1));
    }
}

void ObjectPoly::Subdivision(int mode)
{
    if(mode == 1)
        poly.regular_subdivide();
    else
        poly.irregular_subdivide();

    //Clear the info storage
    ClearData();

    //Regenerate the vertex and other info
    UpdateData();

    //Rewrite buffer info
    UpdateObject();
}

void ObjectPoly::SurfaceSmooth(int mode)
{
    poly.surface_smooth(mode);

    //Clear the info storage
    ClearData();

    //Regenerate the vertex and other info
    UpdateData();

    //If need check board texture
    if(ifVertexColor)
        CalcCheckBoardTexture();

    //Rewrite buffer info
    UpdateObject();
}

void ObjectPoly::HeatDiffusion(int mode)
{
    bool check = poly.heat_diffusion(mode);

    //Only update the color
    heatColors.clear();
    UpdateColorDiffusion();

    //Bind the VAO
    glBindVertexArray(vertexArrayID);

    //Write new color buffer
    WriteColorBuffer(3);

    //Enable the pervertex color
    ifVertexColor = true;

    //Detach the VAO
    glBindVertexArray(0);
}

void ObjectPoly::CurvatureCalc(int mode)
{
    qInfo("== Start calculating the curvature\n");

    //Calculate the curvature
    if(mode == 1)
        poly.gaussian_curvature();
    else
        poly.mean_curvature();

    qInfo("== Start generating color\n");

    //Update the curvature color buffer
    curvatureColors.clear();
    UpdateColorCurvature();

    //Bind the VAO
    glBindVertexArray(vertexArrayID);

    //Write new color buffer
    WriteColorBuffer(4);

    //Enable the pervertex color
    ifVertexColor = true;

    //Detach the VAO
    glBindVertexArray(0);
}

float cportion = 0.3f;
float nportion = 0.01f;

void ObjectPoly::PrincipalCurvatureCalc()
{
    qInfo("== Start calculate the principal curvature\n");

    poly.find_principal_curvature();

    //Prepare the data
    vector<vec3> vdata, tdata;

    for(int i = 0; i < poly.vlist.size(); i++)
    {
        Vertex * vert = poly.vlist[i];

        vec3 n = vec3(vert->normal.x, vert->normal.y, vert->normal.z);

        vec3 vorigin = vec3(vert->x, vert->y, vert->z);

        vec3 start = vorigin - vert->cInfo.maxPrincipal * cportion + n * nportion;
        vec3 vend = vorigin + vert->cInfo.maxPrincipal * cportion + n * nportion;

        vdata.push_back(start);
        vdata.push_back(vend);

        start = vorigin - vert->cInfo.minPrincipal * cportion + n * nportion;
        vend = vorigin + vert->cInfo.minPrincipal * cportion + n * nportion;

        vdata.push_back(start);
        vdata.push_back(vend);
    }

    //Create the silhouette object, only first time
    if(silhouette == NULL)
    {
        qInfo("== Creating curvature line\n");
        silhouette = new ObjectSilhouette();
    }

    //Create new data dor silhouette object
    silhouette->CreateSilhouette(vdata, vec3(1, 0, 0), 0);
}

void ObjectPoly::PrincipalCurvatureSmooth()
{

    poly.smooth_principal_curvature();

    //Prepare the data
    vector<vec3> vdata, tdata;

    for(int i = 0; i < poly.vlist.size(); i++)
    {
        Vertex * vert = poly.vlist[i];

        vec3 n = vec3(vert->normal.x, vert->normal.y, vert->normal.z);

        vec3 vorigin = vec3(vert->x, vert->y, vert->z);

        vec3 start = vorigin - vert->cInfo.maxPrincipal * cportion + n * nportion;
        vec3 vend = vorigin + vert->cInfo.maxPrincipal * cportion + n * nportion;

        vdata.push_back(start);
        vdata.push_back(vend);

        start = vorigin - vert->cInfo.minPrincipal * cportion + n * nportion;
        vend = vorigin + vert->cInfo.minPrincipal * cportion + n * nportion;

        vdata.push_back(start);
        vdata.push_back(vend);
    }

    //Create the silhouette object, only first time

    silhouette = new ObjectSilhouette();

    //Create new data dor silhouette object
    silhouette->CreateSilhouette(vdata, vec3(1, 0, 0), 0);
}

void ObjectPoly::CalcMaxPrincipalLine()
{
    srand(time(NULL));

    poly.calc_face_base();
    poly.transform_face_coord();
    poly.interpolate_face_cur_tensor();

    vector<vec3> vdata, data;
    for(int i = 0; i < poly.tlist.size(); i++)
    {
        //int random = rand() % 10 + 5;
        //if(i % 2 == 0)
        {
            data = poly.calc_stream_line(poly.tlist[i], 20, 1, 1);
            for(int j = 0; j < data.size(); j++)
            {
                vdata.push_back(data[j]);
            }
        }
    }

    //Create the silhouette object, only first time
    silhouette = new ObjectSilhouette();

    //Create new data dor silhouette object
    silhouette->CreateSilhouette(vdata, vec3(0, 0, 0), 1);

    vdata.clear();
    for(int i = 0; i < poly.tlist.size(); i++)
    {
        //int random = rand() % 10 + 5;
        //if(i % 2 == 0)
        {
            data = poly.calc_stream_line(poly.tlist[i], 20, 1, 2);
            for(int j = 0; j < data.size(); j++)
            {
                vdata.push_back(data[j]);
            }
        }
    }

    //Create the silhouette object, only first time
    principalLine = new ObjectSilhouette();

    //Create new data dor silhouette object
    principalLine->CreateSilhouette(vdata, vec3(0, 0, 0), 2);
}

void ObjectPoly::SphericalParameterization()
{
    //Apply the parameterization
    poly.spherical_parameterize();

    //Clear the info storage
    ClearData();

    //Regenerate the vertex and other info
    UpdateData();

    //Rewrite buffer info
    UpdateObject();
}










