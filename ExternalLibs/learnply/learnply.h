/*

Data structures for learnply

Eugene Zhang 2005

*/

#ifndef LEARNPLY_H
#define LEARNPLY_H

#define M_PI           3.14159265358979323846

#include <ply.h>
#include <vectorField.h>
#include <icVector.H>
#include <vector>
#include <time.h>

#include <glm/glm.hpp>

#include <Eigen/Eigen>
#include <Eigen/Dense>
#include <Eigen/src/Core/DenseCoeffsBase.h>
#include <Eigen/Sparse>

#include <QDebug>

//using namespace std;

/* forward declarations */
class Vertex;
class Triangle;
class Corner;
class CornerTable;
class Polyhedron;
class Edge;
class Quad;

struct curvatureInfo
{
    float Curvature;

    float l,m,n;

    glm::vec3 base1, base2, normal; //Base of face's system

    glm::vec2 v[3];   //3 vertex of a face on 2D plane

    glm::vec3 basePoint;    //Base point of face's system

    Eigen::MatrixXf T;  //T of the face, 2*2 matrix

    Eigen::MatrixXf M, Mt, Tg, Tgtemp;  //3*3 matrix

    glm::vec2 max2DPrincipal, min2DPrincipal;   //For face
    glm::vec3 maxPrincipal, minPrincipal;   //For vertex
};

struct subFlag
{
    bool ifOld = false;
    bool ifNeedRemove = false;

    //New vertex info
    double x,y,z;

    //Position of 0 level set of the edge
    bool if0Pos = false;
    bool if0Whol = false;
    glm::vec3 lPos;

    //New heat info
    float newHeat = 0.0f;

    //Min max saddle indicator
    //1 for min
    //2 for max
    //3 for saddle
    int typeIdentity = -1;
    int nSaddle = 0;

    //Track the new edge
    Edge* subEdges[2];

    //Track the new vert
    Vertex* newVert = NULL;

    //Position in vlist
    int i;
};

class Vertex {
public:
    double x,y,z;
    int index;

    double vx, vy, vz;
    double scalar = 0;

    float heat = 0.0;
    bool lockHeat = false;
    int minOrMax = 0;

    double R = 0, G = 0, B = 0;

    int nedges;
    std::vector<Edge*>edges;
    int valence;

    int ntris;
    std::vector<Triangle*> tris;
    int max_tris;

    int nquads;
    std::vector<Quad *> quads;
    int max_quads;

    int ncorners;
    std::vector<Corner*>corners;
    double totalAngle;

    icVector3 normal;
    void *other_props;

    subFlag sflag;

    curvatureInfo cInfo;

public:
	Vertex(double xx, double yy, double zz) { x = xx; y = yy; z = zz; }
};

class Edge {
public:
    int index;
    Vertex *verts[2];

    int ntris;
    std::vector<Triangle*> tris;

    int nconers;
    std::vector<Corner*> corners;

    int nquads;
    std::vector<Quad *> quads;

    double length;

    subFlag sflag;
};

class Triangle {
public:
    int index;
    int nverts;
    Vertex* verts[3];
    Edge*	edges[3];
    Corner* corners[3];

    double	angle[3];
    float	area;

    icVector3	normal;
    void*		other_props;

    subFlag sflag;
    curvatureInfo cInfo;
};

class Quad {
public:
	int index;

	int nverts;
	Vertex *verts[4];
	Edge *edges[4];

	double angle[4];
	float area;

	icVector3 normal;
	void *other_props;
};

class Corner
{
public:
    int index;

    Corner* p;		//Previous corner
    Corner* n;		//Next corner
    Corner* o;		//Opposite corner

    Vertex* v;		//Corresponding vertex
    Edge* e;		//Opposite edge
    Triangle* t;	//Corresponding triangle

    double angle;

    int min, max;	//The min and max for vertex index

    subFlag sflag;

    Corner();
    void Copy(Corner* targetC);
};

class CornerTable
{
public:
    int num_corners;

    std::vector<Corner*> corners;

    CornerTable();
    bool ClearTable();
    bool GenerateTable(Polyhedron* tpoly);
};

class Polyhedron {
public:

	int index;

    std::vector<Vertex*> vlist;		/* list of vertices */
    int nverts;
    int max_verts;

    std::vector<Edge*> elist;		/* list of edges */
    int nedges;
    int max_edges;

    std::vector<Quad*> qlist;  /* list of quads */
    int nquads;
    int max_quads;

    std::vector<Triangle*> tlist;	/* list of triangles */
    int ntris;
    int max_tris;

	icVector3 center;
	double radius;
	double area;

	int seed;

    PlyFile *in_ply;

	PlyOtherProp *vert_other,*face_other;


    /* some extra function */

    CornerTable ctable;
    int ncorners;
    int max_corners;

    double check_valence();
    void check_vlist();

    bool irregular_divide_triangle(Triangle*, int newVert);
    bool regular_divide_triangle(Triangle*);
    bool regular_subdivide();
    bool irregular_subdivide();
    bool subdivide_finalize();

    float smooth_weight_calc(int, Vertex *, Edge *);
    bool general_smooth(int mode, float dt);
    bool surface_smooth_finalize();
    bool surface_smooth(int mode);

    float maxHeat = 1.0f;
    bool heatInited = false;
    int check_type();
    bool initialize_heat();
    bool general_diffusion(int mode, float dt, float lambda);
    bool diffusion_finalize();
    bool heat_diffusion(int mode);

    float find_Amix(Vertex * vert);     //Calculate the Amix for one vertex
    void gaussian_curvature();
    void mean_curvature();

    glm::vec3 iv2vec(Vertex * vert);
    float find_normal_curvature(Vertex * vsource, Vertex * vtarget);
    Vertex * find_another_point_on_edge(Vertex * vert, int i);
    glm::vec3 find_edge_vector(Vertex * vert, int i);

    std::vector<glm::vec2> mapping_to_tangent_plane(Vertex * vert, glm::vec3&, glm::vec3&);
    void find_principal_curvature();

    void map_curvature_tensor();
    void map_curvature_tensor_back();

    void curvature_tensor_smooth();
    void smooth_principal_curvature();
    void calc_face_base();
    void interpolate_face_cur_tensor();
    void transform_face_coord();
    std::vector<glm::vec3> calc_stream_line(Triangle *, int iteration, int mode, int direction);

    void spherical_parameterize();

    /* original poly function */

	void average_normals();
	void create_edge(Vertex *, Vertex *);
	void create_edges();
    int face_to_vertex_ref(Triangle *, Vertex *);
    void order_vertex_to_tri_ptrs(Vertex *);
    void vertex_to_tri_ptrs();
	Quad *find_common_edge(Quad *, Vertex *, Vertex *);
	Quad *other_quad(Edge *, Quad *);
    Triangle *find_common_edge(Triangle *, Vertex *, Vertex *);
    Triangle *other_triangle(Edge *, Triangle *);
	void calc_bounding_sphere();
	void calc_face_normals_and_area();
	void calc_edge_length();

    Polyhedron();
    void PolyhedronCreate(FILE *);
	void write_file(FILE *);

	void create_pointers();

	// initialization and finalization
	void initialize();
	void finalize();
};

#endif /* LEARNPLY_H */

