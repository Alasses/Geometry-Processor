#ifndef OBJECTPOLY_H
#define OBJECTPOLY_H

#include <objectsilhouette.h>

class ObjectPoly : public Object
{
public:
    ObjectPoly(QString filePath);
    ~ObjectPoly();

    float ProcessData(int ID);

    void Subdivision(int mode);

    void SurfaceSmooth(int mode);

    void HeatDiffusion(int mode);

    void CurvatureCalc(int mode);

    void PrincipalCurvatureCalc();
    void PrincipalCurvatureSmooth();

    void CalcMaxPrincipalLine();
    void CalcMinPrincipalLine();
    void CalcStreamLine(int mode);

    void SphericalParameterization();

    void Draw(mat4 &Translation, mat4 &mvp, mat4 &m, mat4 &v, vec3 lightColor,
              vec3 lightPos, vec3 lightPos2);

private:
    //The stored polyhedron
    Polyhedron poly;

    //Silhouette
    ObjectSilhouette * silhouette;
    ObjectSilhouette * principalLine;

    //Generate silhouette
    void GenSilhouette(mat4 &Translation, mat4 &mvp, mat4 &m, mat4 &v);

    //Clear up the arrays
    void ClearData();

    //Update the vertex, color, and normal info
    void UpdateData();

    //Update the color
    void UpdateColorDiffusion();

    //Update the color based on curvature result
    void UpdateColorCurvature();

    vec3 calc_heat_color(float heat, float max);

    vec3 calc_curvature_color(float cur, float max, float min, float avg);
};

#endif // OBJECTPOLY_H
