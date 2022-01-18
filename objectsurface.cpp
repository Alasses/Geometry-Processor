#include "objectsurface.h"

ObjectSurface::ObjectSurface(QOpenGLContext * targetContext)
{
    currentContext = targetContext;
}

void ObjectSurface::GenCurve()
{
    float u,v;

    //Initialize the expression evaluator
    typedef exprtk::symbol_table<float> symbol_table_t;
    typedef exprtk::expression<float>     expression_t;
    typedef exprtk::parser<float>             parser_t;

    //Create a symbol table
    symbol_table_t sTable;
    expression_t expX, expY, expZ;      //X Y Z
    expression_t Xu, Yu, Zu;            //Ru
    expression_t Xv, Yv, Zv;            //Rv
    expression_t Xuu, Yuu, Zuu;         //Ruu
    expression_t Xuv, Yuv, Zuv;         //Ruv / Rvu
    expression_t Xvv, Yvv, Zvv;         //Rvv
    expression_t expIncU, expIncV;      //du dv
    expression_t umi, uma, vmi, vma;    //U min/max, V min/max
    parser_t parser;

    //Add variable to table
    sTable.add_constants();
    sTable.add_variable("u", u);
    sTable.add_variable("v", v);

    //Register the variable
    expX.register_symbol_table(sTable);
    expY.register_symbol_table(sTable);
    expZ.register_symbol_table(sTable);
    //
    Xu.register_symbol_table(sTable);
    Yu.register_symbol_table(sTable);
    Zu.register_symbol_table(sTable);
    //
    Xv.register_symbol_table(sTable);
    Yv.register_symbol_table(sTable);
    Zv.register_symbol_table(sTable);
    //
    Xuu.register_symbol_table(sTable);
    Yuu.register_symbol_table(sTable);
    Zuu.register_symbol_table(sTable);
    //
    Xuv.register_symbol_table(sTable);
    Yuv.register_symbol_table(sTable);
    Zuv.register_symbol_table(sTable);
    //
    Xvv.register_symbol_table(sTable);
    Yvv.register_symbol_table(sTable);
    Zvv.register_symbol_table(sTable);
    //
    umi.register_symbol_table(sTable);
    uma.register_symbol_table(sTable);
    vmi.register_symbol_table(sTable);
    vma.register_symbol_table(sTable);
    //
    expIncU.register_symbol_table(sTable);
    expIncV.register_symbol_table(sTable);

    //Compile the expression
    parser.compile(equationX.toLocal8Bit().constData(), expX);
    parser.compile(equationY.toLocal8Bit().constData(), expY);
    parser.compile(equationZ.toLocal8Bit().constData(), expZ);
    //
    parser.compile(equationXu.toLocal8Bit().constData(), Xu);
    parser.compile(equationYu.toLocal8Bit().constData(), Yu);
    parser.compile(equationZu.toLocal8Bit().constData(), Zu);
    //
    parser.compile(equationXv.toLocal8Bit().constData(), Xv);
    parser.compile(equationYv.toLocal8Bit().constData(), Yv);
    parser.compile(equationZv.toLocal8Bit().constData(), Zv);
    //
    parser.compile(equationXuu.toLocal8Bit().constData(), Xuu);
    parser.compile(equationYuu.toLocal8Bit().constData(), Yuu);
    parser.compile(equationZuu.toLocal8Bit().constData(), Zuu);
    //
    parser.compile(equationXuv.toLocal8Bit().constData(), Xuv);
    parser.compile(equationYuv.toLocal8Bit().constData(), Yuv);
    parser.compile(equationZuv.toLocal8Bit().constData(), Zuv);
    //
    parser.compile(equationXvv.toLocal8Bit().constData(), Xvv);
    parser.compile(equationYvv.toLocal8Bit().constData(), Yvv);
    parser.compile(equationZvv.toLocal8Bit().constData(), Zvv);
    //
    parser.compile(umin.toLocal8Bit().constData(), umi);
    parser.compile(umax.toLocal8Bit().constData(), uma);
    parser.compile(vmin.toLocal8Bit().constData(), vmi);
    parser.compile(vmax.toLocal8Bit().constData(), vma);
    //
    parser.compile(evaU.toLocal8Bit().constData(), expIncU);
    parser.compile(evaV.toLocal8Bit().constData(), expIncV);

    //Evaluate the boundary
    uMin = umi.value();
    uMax = uma.value();
    vMin = vmi.value();
    vMax = vma.value();

    float kMaxcuru = point.x;
    float kMaxcurv = point.y;
    float kMincuru = point.x;
    float kMincurv = point.y;

    int iteration = 0;
    while(iteration < 100000)
    {
        /***** k max *****/

        u = kMaxcuru;
        v = kMaxcurv;

        //Calculate fundamental form
        vec3 ru = vec3(Xu.value(),
                       Yu.value(),
                       Zu.value());

        vec3 rv = vec3(Xv.value(),
                       Yv.value(),
                       Zv.value());

        vec3 ruu = vec3(Xuu.value(),
                        Yuu.value(),
                        Zuu.value());

        vec3 ruv = vec3(Xuv.value(),
                        Yuv.value(),
                        Zuv.value());

        vec3 rvv = vec3(Xvv.value(),
                        Yvv.value(),
                        Zvv.value());

        vec3 norm = normalize(cross(ru, rv));

        vec3 curPos = vec3(expX.value(), expY.value(), expZ.value());

        //Calculate first fundamental form
        float E, F, G;
        E = dot(ru, ru);
        F = dot(ru, rv);
        G = dot(rv, rv);

        float L, M, N;
        L = dot(ruu, norm);
        M = dot(ruv, norm);
        N = dot(rvv, norm);

        float H = (E*N+G*L-2*F*M) / (2*(E*G-F*F));
        float K = (L*N-M*M) / (E*G-F*F);

        float kmax = H + sqrt(H*H - K);
        //qInfo("curve k: %f", kmax);

        float lambda = -(M-kmax*F)/(N-kmax*G);
        float du = 0.001;
        float dv = lambda * du;

        kMaxcuru = kMaxcuru + du;
        kMaxcurv = kMaxcurv + dv;
        u = kMaxcuru;
        v = kMaxcurv;

        vec3 nxtPos = vec3(expX.value(), expY.value(), expZ.value());

        curVertices.push_back(curPos);
        curVertices.push_back(nxtPos);


        /***** k min *****/

        u = kMincuru;
        v = kMincurv;

        //Calculate fundamental form
        ru = vec3(Xu.value(),
                       Yu.value(),
                       Zu.value());

        rv = vec3(Xv.value(),
                       Yv.value(),
                       Zv.value());

        ruu = vec3(Xuu.value(),
                        Yuu.value(),
                        Zuu.value());

        ruv = vec3(Xuv.value(),
                        Yuv.value(),
                        Zuv.value());

        rvv = vec3(Xvv.value(),
                        Yvv.value(),
                        Zvv.value());

        norm = normalize(cross(ru, rv));

        curPos = vec3(expX.value(), expY.value(), expZ.value());

        //Calculate first fundamental form
        E = dot(ru, ru);
        F = dot(ru, rv);
        G = dot(rv, rv);

        L = dot(ruu, norm);
        M = dot(ruv, norm);
        N = dot(rvv, norm);

        H = (E*N+G*L-2*F*M) / (2*(E*G-F*F));
        K = (L*N-M*M) / (E*G-F*F);

        kmax = H - sqrt(H*H - K);
        lambda = -(M-kmax*F)/(N-kmax*G);
        du = -0.001;
        dv = lambda * du;

        kMincuru = kMincuru + du;
        kMincurv = kMincurv + dv;
        u = kMincuru;
        v = kMincurv;

        nxtPos = vec3(expX.value(), expY.value(), expZ.value());

        curVertices.push_back(curPos);
        curVertices.push_back(nxtPos);


        iteration++;
    }

    qInfo("Iterations: %d", curVertices.size());
}

void ObjectSurface::SetInfo(QString equa[20], QString info[4], int ID)
{
    id = ID;

    //Copy and store the information
    equationX = equa[0];
    equationY = equa[1];
    equationZ = equa[2];

    equationXu = equa[3];
    equationYu = equa[4];
    equationZu = equa[5];

    equationXv = equa[6];
    equationYv = equa[7];
    equationZv = equa[8];

    equationXuu = equa[9];
    equationYuu = equa[10];
    equationZuu = equa[11];

    equationXuv = equa[12];
    equationYuv = equa[13];
    equationZuv = equa[14];

    equationXvv = equa[15];
    equationYvv = equa[16];
    equationZvv = equa[17];

    evaU = equa[18];
    evaV = equa[19];

    umin = info[0];
    umax = info[1];
    vmin = info[2];
    vmax = info[3];

    //***** Start Processing the data *****

    //Initialize the u v
    float u, v;

    //Initialize the expression evaluator
    typedef exprtk::symbol_table<float> symbol_table_t;
    typedef exprtk::expression<float>     expression_t;
    typedef exprtk::parser<float>             parser_t;

    //Create a symbol table
    symbol_table_t sTable;
    expression_t expX, expY, expZ;      //X Y Z
    expression_t Xu, Yu, Zu;            //Ru
    expression_t Xv, Yv, Zv;            //Rv
    expression_t Xuu, Yuu, Zuu;         //Ruu
    expression_t Xuv, Yuv, Zuv;         //Ruv / Rvu
    expression_t Xvv, Yvv, Zvv;         //Rvv
    expression_t expIncU, expIncV;      //du dv
    expression_t umi, uma, vmi, vma;    //U min/max, V min/max
    parser_t parser;

    //Add variable to table
    sTable.add_constants();
    sTable.add_variable("u", u);
    sTable.add_variable("v", v);

    //Register the variable
    expX.register_symbol_table(sTable);
    expY.register_symbol_table(sTable);
    expZ.register_symbol_table(sTable);
    //
    Xu.register_symbol_table(sTable);
    Yu.register_symbol_table(sTable);
    Zu.register_symbol_table(sTable);
    //
    Xv.register_symbol_table(sTable);
    Yv.register_symbol_table(sTable);
    Zv.register_symbol_table(sTable);
    //
    Xuu.register_symbol_table(sTable);
    Yuu.register_symbol_table(sTable);
    Zuu.register_symbol_table(sTable);
    //
    Xuv.register_symbol_table(sTable);
    Yuv.register_symbol_table(sTable);
    Zuv.register_symbol_table(sTable);
    //
    Xvv.register_symbol_table(sTable);
    Yvv.register_symbol_table(sTable);
    Zvv.register_symbol_table(sTable);
    //
    umi.register_symbol_table(sTable);
    uma.register_symbol_table(sTable);
    vmi.register_symbol_table(sTable);
    vma.register_symbol_table(sTable);
    //
    expIncU.register_symbol_table(sTable);
    expIncV.register_symbol_table(sTable);

    //Compile the expression
    parser.compile(equationX.toLocal8Bit().constData(), expX);
    parser.compile(equationY.toLocal8Bit().constData(), expY);
    parser.compile(equationZ.toLocal8Bit().constData(), expZ);
    //
    parser.compile(equationXu.toLocal8Bit().constData(), Xu);
    parser.compile(equationYu.toLocal8Bit().constData(), Yu);
    parser.compile(equationZu.toLocal8Bit().constData(), Zu);
    //
    parser.compile(equationXv.toLocal8Bit().constData(), Xv);
    parser.compile(equationYv.toLocal8Bit().constData(), Yv);
    parser.compile(equationZv.toLocal8Bit().constData(), Zv);
    //
    parser.compile(equationXuu.toLocal8Bit().constData(), Xuu);
    parser.compile(equationYuu.toLocal8Bit().constData(), Yuu);
    parser.compile(equationZuu.toLocal8Bit().constData(), Zuu);
    //
    parser.compile(equationXuv.toLocal8Bit().constData(), Xuv);
    parser.compile(equationYuv.toLocal8Bit().constData(), Yuv);
    parser.compile(equationZuv.toLocal8Bit().constData(), Zuv);
    //
    parser.compile(equationXvv.toLocal8Bit().constData(), Xvv);
    parser.compile(equationYvv.toLocal8Bit().constData(), Yvv);
    parser.compile(equationZvv.toLocal8Bit().constData(), Zvv);
    //
    parser.compile(umin.toLocal8Bit().constData(), umi);
    parser.compile(umax.toLocal8Bit().constData(), uma);
    parser.compile(vmin.toLocal8Bit().constData(), vmi);
    parser.compile(vmax.toLocal8Bit().constData(), vma);
    //
    parser.compile(evaU.toLocal8Bit().constData(), expIncU);
    parser.compile(evaV.toLocal8Bit().constData(), expIncV);

    //Evaluate the boundary
    uMin = umi.value();
    uMax = uma.value();
    vMin = vmi.value();
    vMax = vma.value();

    qInfo("%s\n", umax.toLocal8Bit().constData());
    qInfo("%f\n", uma.value());

    /*** Start Generate vertices ***/

    int rowNum = 0;
    int colNum = 0;

    //v = M_PI/4;
    //qInfo("expVeqa: %s; expV: %f\n", evaV.toLocal8Bit().constData(), expIncV.value());
    //qInfo("expXeqa: %s; expX: %f\n", equationX.toLocal8Bit().constData(), expX.value());

    //i.e. :
    //U - 2 * FM_PI / 500.0f
    //V - 2 * FM_PI / 500.0f
    vector<vec3> rawVertices;
    vector<vec3> rawNormals;
    vector<float> rawCurvature;

    minK = 9999999;
    maxK = -9999999;
    for(v = vMin; v <= vMax; v += expIncV.value())
    {
        rowNum = 0;
        for(u = uMin; u <= uMax; u += expIncU.value())
        {
            //i.e. :
            //1 * qSin(v)
            //(2 + (1 * qCos(v))) * qSin(u)
            //(2 + (1 * qCos(v))) * qCos(u)

            //Calculate the normal vector
            vec3 ru = vec3(Xu.value(),
                           Yu.value(),
                           Zu.value());

            vec3 rv = vec3(Xv.value(),
                           Yv.value(),
                           Zv.value());

            vec3 ruu = vec3(Xuu.value(),
                            Yuu.value(),
                            Zuu.value());

            vec3 ruv = vec3(Xuv.value(),
                            Yuv.value(),
                            Zuv.value());

            vec3 rvv = vec3(Xvv.value(),
                            Yvv.value(),
                            Zvv.value());

            vec3 norm = normalize(cross(ru, rv));
            rawNormals.push_back(norm);

            //Calculate first fundamental form
            float E, F, G;
            E = dot(ru, ru);
            F = dot(ru, rv);
            G = dot(rv, rv);

            float L, M, N;
            L = dot(ruu, norm);
            M = dot(ruv, norm);
            N = dot(rvv, norm);

            float H = (E*N+G*L-2*F*M) / (2*(E*G-F*F));
            float K = (L*N-M*M) / (E*G-F*F);

            float kmax = H + sqrt(H*H - K);
            float kmin = H - sqrt(H*H - K);
            float k = kmax * kmin;
            //qInfo("k: %f\n", k);
            //float k = H;

            if(k < minK)
                minK = k;
            if(k > maxK)
                maxK = k;

            rawCurvature.push_back(k);

            vec3 pos = vec3(expX.value(), expY.value(), expZ.value());
            rawVertices.push_back(pos);

            rowNum++;
        }
        colNum++;
    }
    qInfo("kmin: %f, kmax: %f", minK, maxK);

    //The base index of the vertices

    for(int i = 0; i < rowNum - 1; i++)
    {
        for(int j = 0; j < colNum - 1; j++)
        {
            vec3 color;
            float k;
            int power = 1;

            vec3 range = vec3(0, 0, 0.5) - vec3(0.5, 0, 0);
            vec3 base = vec3(0.5, 0, 0.5);
            float minus = 0;
            float divide;
            if((maxK + minK) == 0)
                divide = 1;
            else
                divide = (maxK + minK) / 2;


            //Vert 1
            vertices.push_back(rawVertices[(i + 1) * rowNum + j]);
            gausCurvature.push_back(rawCurvature[(i + 1) * rowNum + j]);
            normals.push_back(rawNormals[(i + 1) * rowNum + j]);
            k = rawCurvature[(i + 1) * rowNum + j];
            color = float(pow((k - minus)/divide,power)) * range + base;
            colors.push_back(color);
            //Vert 2
            vertices.push_back(rawVertices[i * rowNum + j]);
            gausCurvature.push_back(rawCurvature[i * rowNum + j]);
            normals.push_back(rawNormals[i * rowNum + j]);
            k = rawCurvature[i * rowNum + j];
            color = float(pow((k - minus)/divide,power)) * range + base;
            colors.push_back(color);
            //Vert 3
            vertices.push_back(rawVertices[i * rowNum + j + 1]);
            gausCurvature.push_back(rawCurvature[i * rowNum + j + 1]);
            normals.push_back(rawNormals[i * rowNum + j + 1]);
            k = rawCurvature[i * rowNum + j + 1];
            color = float(pow((k - minus)/divide,power)) * range + base;
            colors.push_back(color);

            //Draw the back side
            //Vert 1
            vertices.push_back(rawVertices[(i + 1) * rowNum + j]);
            gausCurvature.push_back(rawCurvature[(i + 1) * rowNum + j]);
            normals.push_back(rawNormals[(i + 1) * rowNum + j]);
            k = rawCurvature[(i + 1) * rowNum + j];
            color = float(pow((k - minus)/divide,power)) * range + base;
            colors.push_back(color);
            //Vert 3
            vertices.push_back(rawVertices[i * rowNum + j + 1]);
            gausCurvature.push_back(rawCurvature[i * rowNum + j + 1]);
            normals.push_back(rawNormals[i * rowNum + j + 1]);
            k = rawCurvature[i * rowNum + j + 1];
            color = float(pow((k - minus)/divide,power)) * range + base;
            colors.push_back(color);
            //Vert 2
            vertices.push_back(rawVertices[i * rowNum + j]);
            gausCurvature.push_back(rawCurvature[i * rowNum + j]);
            normals.push_back(rawNormals[i * rowNum + j]);
            k = rawCurvature[i * rowNum + j];
            color = float(pow((k - minus)/divide,power)) * range + base;
            colors.push_back(color);

            //Vert 1
            vertices.push_back(rawVertices[(i + 1) * rowNum + j]);
            gausCurvature.push_back(rawCurvature[(i + 1) * rowNum + j]);
            normals.push_back(rawNormals[(i + 1) * rowNum + j]);
            k = rawCurvature[(i + 1) * rowNum + j];
            color = float(pow((k - minus)/divide,power)) * range + base;
            colors.push_back(color);
            //Vert 3
            vertices.push_back(rawVertices[i * rowNum + j + 1]);
            gausCurvature.push_back(rawCurvature[i * rowNum + j + 1]);
            normals.push_back(rawNormals[i * rowNum + j + 1]);
            k = rawCurvature[i * rowNum + j + 1];
            color = float(pow((k - minus)/divide,power)) * range + base;
            colors.push_back(color);
            //Vert 4
            vertices.push_back(rawVertices[(i + 1) * rowNum + j + 1]);
            gausCurvature.push_back(rawCurvature[(i + 1) * rowNum + j + 1]);
            normals.push_back(rawNormals[(i + 1) * rowNum + j + 1]);
            k = rawCurvature[(i + 1) * rowNum + j + 1];
            color = float(pow((k - minus)/divide,power)) * range + base;
            colors.push_back(color);

            //Vert 1
            vertices.push_back(rawVertices[(i + 1) * rowNum + j]);
            gausCurvature.push_back(rawCurvature[(i + 1) * rowNum + j]);
            normals.push_back(rawNormals[(i + 1) * rowNum + j]);
            k = rawCurvature[(i + 1) * rowNum + j];
            color = float(pow((k - minus)/divide,power)) * range + base;
            colors.push_back(color);
            //Vert 4
            vertices.push_back(rawVertices[(i + 1) * rowNum + j + 1]);
            gausCurvature.push_back(rawCurvature[(i + 1) * rowNum + j + 1]);
            normals.push_back(rawNormals[(i + 1) * rowNum + j + 1]);
            k = rawCurvature[(i + 1) * rowNum + j + 1];
            color = float(pow((k - minus)/divide,power)) * range + base;
            colors.push_back(color);
            //Vert 3
            vertices.push_back(rawVertices[i * rowNum + j + 1]);
            gausCurvature.push_back(rawCurvature[i * rowNum + j + 1]);
            normals.push_back(rawNormals[i * rowNum + j + 1]);
            k = rawCurvature[i * rowNum + j + 1];
            color = float(pow((k - minus)/divide,power)) * range + base;
            colors.push_back(color);
        }
    }

    qInfo("Finish generating the vertices\n Line: %d, Surface: %d\n", indicesLines.size(), vertices.size());

    point = vec2(uMin + (uMax - uMin) / 1.75, vMin + (vMax - vMin) / 1.23);

    GenCurve();

    //Generate the object after setting the info
    GenObject();
}
