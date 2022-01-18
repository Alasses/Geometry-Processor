/*

Functions for learnply

Eugene Zhang, 2005
*/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <ctime>
#include <stdio.h>
#include <string.h>
#include <fstream>
#include <vector>
#include <iostream>
#include <vector>
#include <learnply.h>
#include "ply.h"
#include "icVector.H"
#include "icMatrix.H"
#include "learnply_io.h"
#include "tmatrix.h"
#include "trackball.h"
//#include "GL/glui.h"

using namespace std;
using namespace glm;
using namespace Eigen;

Polyhedron * poly;

float lengthOfvec3(vec3 t);

bool checkIfBetween(vec3 t, vec3 p1, vec3 p2);

//0=ccw, 1=cw
unsigned char orientation;

Corner::Corner()
{
    p = NULL;
    n = NULL;
    o = NULL;
    v = NULL;
    e = NULL;
    t = NULL;
    angle = -999;
}

void Corner::Copy(Corner* targetC)
{
    p = targetC->p;
    n = targetC->n;
    o = targetC->o;

    v = targetC->v;
    e = targetC->e;
    t = targetC->t;

    angle = targetC->angle;
}

CornerTable::CornerTable()
{
    num_corners = 0;
}

bool CornerTable::GenerateTable(Polyhedron * tpoly)
{
    //Check if the poly is valid
    if (tpoly->tlist.size() == 0)
    {
        qInfo("== The target poly was not initialized!\n");
        return false;
    }
    else
    {
        //Loop through the triangle list to build the temp corner table
        for (int i = 0; i < tpoly->ntris; i++)
        {
            Triangle* currentT = tpoly->tlist[i];

            //Check if the triangle is valid
            if (currentT->nverts != 3)
            {
                qInfo("== Triangle %d is not correct!\n", i);
                return false;
            }
            else
            {
                //Check each of three corners
                //		 1
                //	  e0
                //	0       e1
                //       e2
                //			   2

                /***Basic corner info***/

                Vertex* v0 = currentT->verts[0];
                Vertex* v1 = currentT->verts[1];
                Vertex* v2 = currentT->verts[2];

                Corner* c0 = new Corner;
                c0->index = num_corners;
                currentT->corners[0] = c0;
                Corner* c1 = new Corner;
                c1->index = num_corners + 1;
                currentT->corners[1] = c1;
                Corner* c2 = new Corner;
                c2->index = num_corners + 2;
                currentT->corners[2] = c2;

                corners.push_back(c0);
                corners.push_back(c1);
                corners.push_back(c2);
                num_corners += 3;

                c0->p = c2;		//Previous corner
                c1->p = c0;
                c2->p = c1;

                c0->n = c1;		//Next corner
                c1->n = c2;
                c2->n = c0;

                c0->v = v0;		//Corres vert
                v0->corners.push_back(c0);
                v0->ncorners = v0->corners.size();
                c1->v = v1;
                v1->corners.push_back(c1);
                v1->ncorners = v1->corners.size();
                c2->v = v2;
                v2->corners.push_back(c2);
                v2->ncorners = v2->corners.size();

                c0->e = currentT->edges[1];     //Corres edge, and connect edge with corners
                currentT->edges[1]->corners.push_back(c0);
                currentT->edges[1]->nconers++;

                c1->e = currentT->edges[2];
                currentT->edges[2]->corners.push_back(c1);
                currentT->edges[2]->nconers++;

                c2->e = currentT->edges[0];
                currentT->edges[0]->corners.push_back(c2);
                currentT->edges[0]->nconers++;

                c0->t = c1->t = c2->t = currentT;	//Triangle contained


                /***Calculate angles***/

                double x1 = v1->x - v0->x;
                double y1 = v1->y - v0->y;
                double z1 = v1->z - v0->z;

                double x2 = v2->x - v0->x;
                double y2 = v2->y - v0->y;
                double z2 = v2->z - v0->z;

                double dot = x1 * x2 + y1 * y2 + z1 * z2;
                double lsq1 = x1 * x1 + y1 * y1 + z1 * z1;
                double lsq2 = x2 * x2 + y2 * y2 + z2 * z2;
                currentT->angle[0] = c0->angle = acos(dot / sqrt(lsq1 * lsq2));

                x1 = v2->x - v1->x;
                y1 = v2->y - v1->y;
                z1 = v2->z - v1->z;

                x2 = v0->x - v1->x;
                y2 = v0->y - v1->y;
                z2 = v0->z - v1->z;

                dot = x1 * x2 + y1 * y2 + z1 * z2;
                lsq1 = x1 * x1 + y1 * y1 + z1 * z1;
                lsq2 = x2 * x2 + y2 * y2 + z2 * z2;
                currentT->angle[1] = c1->angle = acos(dot / sqrt(lsq1 * lsq2));

                x1 = v0->x - v2->x;
                y1 = v0->y - v2->y;
                z1 = v0->z - v2->z;

                x2 = v1->x - v2->x;
                y2 = v1->y - v2->y;
                z2 = v1->z - v2->z;

                dot = x1 * x2 + y1 * y2 + z1 * z2;
                lsq1 = x1 * x1 + y1 * y1 + z1 * z1;
                lsq2 = x2 * x2 + y2 * y2 + z2 * z2;
                currentT->angle[2] = c2->angle = acos(dot / sqrt(lsq1 * lsq2));
            }
        }

        /***Start to set up the opposite corner***/

        for (int i = 0; i < corners.size(); i++)
        {
            //Get the target edge first
            Edge* targetEdge = corners[i]->e;
            if (targetEdge == NULL)
            {
                qInfo("== Opposite error! Target edge is NULL.\n");
                return false;
            }

            //Check if target edge contains another corner
            if (targetEdge->nconers == 2)
            {
                //Get another corner
                for (int j = 0; j < 2; j++)
                {
                    if (targetEdge->corners[j] != corners[i])
                    {
                        corners[i]->o = targetEdge->corners[j];
                        //printf("Corner %d's opposite corner is: corner %d\n",
                        //	corners[i]->index,
                        //	corners[i]->o->index);
                    }
                }
            }

            //Calculate min and max for each corner
            int temparr[3];
            temparr[0] = corners[i]->p->v->index;
            temparr[1] = corners[i]->n->v->index;
            temparr[2] = corners[i]->v->index;

            int min = 999;
            int max = -999;
            for (int j = 0; j < 3; j++)
            {
                if (temparr[j] > max)
                    max = temparr[j];
                if (temparr[j] < min)
                    min = temparr[j];
            }
            corners[i]->max = max;
            corners[i]->min = min;
        }
    }

    return true;
}

bool CornerTable::ClearTable()
{
    //Check if there error left behind
    if (num_corners != corners.size())
    {
        qInfo("== Corner amount not correct when clearing table!\n");
        return false;
    }

    //Loop through all the corners
    for (int i = 0; i < num_corners; i++)
    {
        Corner* curCorner = corners[i];

        //Clear the vertex's corner list
        curCorner->v->corners.clear();
        curCorner->v->ncorners = 0;

        //Clear the edge's corner list
        curCorner->e->corners.clear();
        curCorner->e->nconers = 0;

        //Clear the tri list, reset to NULL
        for (int j = 0; j < 3; j++)
            curCorner->t->corners[j] = NULL;
    }

    return true;
}

/******************************************************************************
Read in a polyhedron from a file.
******************************************************************************/

void Polyhedron::PolyhedronCreate(FILE *file)
{
    int i,j;
    int elem_count;
    char *elem_name;

    /*** Read in the original PLY object ***/
    in_ply = read_ply (file);

    for (i = 0; i < in_ply->num_elem_types; i++) {

        /* prepare to read the i'th list of elements */
        elem_name = setup_element_read_ply (in_ply, i, &elem_count);

        if (equal_strings ((char *)"vertex", elem_name)) {

            //Initialize the vlist and nverts
            nverts = max_verts = elem_count;

            /* set up for getting vertex elements */

            setup_property_ply (in_ply, &vert_props[0]);
            setup_property_ply (in_ply, &vert_props[1]);
            setup_property_ply (in_ply, &vert_props[2]);
            vert_other = get_other_properties_ply (in_ply,
                                offsetof(Vertex_io,other_props));

            /* grab all the vertex elements */
            qInfo("== nverts: %d\n", nverts);
            for (j = 0; j < nverts; j++)
            {
                Vertex_io vert;
                get_element_ply (in_ply, (void *) &vert);

                /* copy info from the "vert" structure */
                Vertex* v = new Vertex(vert.x, vert.y, vert.z);
                //qInfo("vert: %f, %f, %f\n", vert.x, vert.y, vert.z);
                vlist.push_back(v);
                v->other_props = vert.other_props;
            }
        }
        else if (equal_strings ((char *)"face", elem_name)) {

            /* create a list to hold all the face elements */
            ntris = max_tris = elem_count;

            /* set up for getting face elements */
            setup_property_ply (in_ply, &face_props[0]);
            face_other = get_other_properties_ply (in_ply, offsetof(Face_io,other_props));

            /* grab all the face elements */
            for (j = 0; j < elem_count; j++) {
            Face_io face;
            get_element_ply (in_ply, (void *) &face);

            if (face.nverts != 3) {
                fprintf (stderr, "Face has %d vertices (should be three).\n",
                        face.nverts);
                exit (-1);
            }

            /* copy info from the "face" structure */
            tlist.push_back(new Triangle);
            tlist[j]->nverts = 3;
            tlist[j]->verts[0] = (Vertex *) face.verts[0];
            tlist[j]->verts[1] = (Vertex *) face.verts[1];
            tlist[j]->verts[2] = (Vertex *) face.verts[2];
            tlist[j]->other_props = face.other_props;
            }
        }
        else
            get_other_element_ply (in_ply);
    }

    /* set ntris */
    ntris = tlist.size();

    /* close the file */

    close_ply (in_ply);

    /* fix up vertex pointers in triangles */
    for (i = 0; i < ntris; i++)
    {
        tlist[i]->verts[0] = vlist[(int) tlist[i]->verts[0]];
        tlist[i]->verts[1] = vlist[(int) tlist[i]->verts[1]];
        tlist[i]->verts[2] = vlist[(int) tlist[i]->verts[2]];
    }

    //printf("tlist 0: %f, %f, %f\n", tlist[0]->verts[0]->x, tlist[0]->verts[0]->y, tlist[0]->verts[0]->z);

    /* get rid of triangles that use the same vertex more than once */

    for (i = ntris-1; i >= 0; i--)
    {
        Triangle *tri = tlist[i];
        Vertex *v0 = tri->verts[0];
        Vertex *v1 = tri->verts[1];
        Vertex *v2 = tri->verts[2];

        if (v0 == v1 || v1 == v2 || v2 == v0)
        {
            tlist.erase(tlist.begin() + i);
            ntris--;
            //tlist[i] = tlist[ntris];
        }
    }

    /* set ntris */
    ntris = tlist.size();
}

/******************************************************************************
Write out a polyhedron to a file.
******************************************************************************/

void Polyhedron::write_file(FILE *file)
{
    int i;
    PlyFile *ply;
    char **elist;
    int num_elem_types;

    /*** Write out the transformed PLY object ***/

    elist = get_element_list_ply (in_ply, &num_elem_types);
    ply = write_ply (file, num_elem_types, elist, in_ply->file_type);

    /* describe what properties go into the vertex elements */

    describe_element_ply (ply, (char *)"vertex", nverts);
    describe_property_ply (ply, &vert_props[0]);
    describe_property_ply (ply, &vert_props[1]);
    describe_property_ply (ply, &vert_props[2]);
    //  describe_other_properties_ply (ply, vert_other, offsetof(Vertex_io,other_props));

    describe_element_ply (ply, (char *)"face", ntris);
    describe_property_ply (ply, &face_props[0]);

    //  describe_other_properties_ply (ply, face_other,
    //                                offsetof(Face_io,other_props));

    //  describe_other_elements_ply (ply, in_ply->other_elems);

    copy_comments_ply (ply, in_ply);
    char mm[1024];
    sprintf(mm, "modified by learnply");
    //  append_comment_ply (ply, "modified by simvizply %f");
        append_comment_ply (ply, mm);
    copy_obj_info_ply (ply, in_ply);

    header_complete_ply (ply);

    /* set up and write the vertex elements */
    put_element_setup_ply (ply, (char *)"vertex");
    for (i = 0; i < nverts; i++)
    {
        Vertex_io vert;

        /* copy info to the "vert" structure */
        vert.x = vlist[i]->x;
        vert.y = vlist[i]->y;
        vert.z = vlist[i]->z;
        vert.other_props = vlist[i]->other_props;

        put_element_ply (ply, (void *) &vert);
    }

    /* index all the vertices */
    for (i = 0; i < nverts; i++)
        vlist[i]->index = i;

    /* set up and write the face elements */
    put_element_setup_ply (ply, (char *)"face");

    Face_io face;
    face.verts = new int[3];

    for (i = 0; i < ntris; i++) {

    /* copy info to the "face" structure */
    face.nverts = 3;
    face.verts[0] = tlist[i]->verts[0]->index;
    face.verts[1] = tlist[i]->verts[1]->index;
    face.verts[2] = tlist[i]->verts[2]->index;
    face.other_props = tlist[i]->other_props;

    put_element_ply (ply, (void *) &face);
    }
    put_other_elements_ply (ply);

    close_ply (ply);
    free_ply (ply);
}

void Polyhedron::initialize() {
	icVector3 v1, v2;

	create_pointers();
	calc_edge_length();
	seed = -1;
}

void Polyhedron::finalize() {
	int i;

	for (i = 0; i < nquads; i++) {
		free(qlist[i]->other_props);
		free(qlist[i]);
	}
	for (i = 0; i < nedges; i++) {
		free(elist[i]);
	}
	for (i = 0; i < nverts; i++) {
		free(vlist[i]->other_props);
		free(vlist[i]);
	}

	if (!vert_other)
		free(vert_other);
	if (!face_other)
		free(face_other);
}

/******************************************************************************
Find out if there is another face that shares an edge with a given face.

Entry:
  f1    - face that we're looking to share with
  v1,v2 - two vertices of f1 that define edge

Exit:
  return the matching face, or NULL if there is no such face
******************************************************************************/

Triangle *Polyhedron::find_common_edge(Triangle *f1, Vertex *v1, Vertex *v2)
{
    int i,j;
    Triangle *f2;
    Triangle *adjacent = NULL;

    /* look through all faces of the first vertex */

    for (i = 0; i < v1->ntris; i++) {
        f2 = v1->tris[i];
        if (f2 == f1)
          continue;
        /* examine the vertices of the face for a match with the second vertex */
        for (j = 0; j < f2->nverts; j++) {

          /* look for a match */
          if (f2->verts[j] == v2) {

        #if 0
        /* watch out for triple edges */

            if (adjacent != NULL) {

          fprintf (stderr, "model has triple edges\n");

          fprintf (stderr, "face 1: ");
          for (k = 0; k < f1->nverts; k++)
            fprintf (stderr, "%d ", f1->iverts[k]);
          fprintf (stderr, "\nface 2: ");
          for (k = 0; k < f2->nverts; k++)
            fprintf (stderr, "%d ", f2->iverts[k]);
          fprintf (stderr, "\nface 3: ");
          for (k = 0; k < adjacent->nverts; k++)
            fprintf (stderr, "%d ", adjacent->iverts[k]);
          fprintf (stderr, "\n");

        }

        /* if we've got a match, remember this face */
            adjacent = f2;
        #endif

        #if 1
        /* if we've got a match, return this face */
            return (f2);
        #endif

          }
        }
    }

    return (adjacent);
}

/******************************************************************************
Create an edge.

Entry:
  v1,v2 - two vertices of f1 that define edge
******************************************************************************/

void Polyhedron::create_edge(Vertex *v1, Vertex *v2)
{
    int i,j;
    Triangle *f;

    /* create the edge */

    Edge *e = new Edge;
    elist.push_back(e);
    e->index = nedges;
    e->verts[0] = v1;
    e->verts[1] = v2;
    nedges++;

    /* count all triangles that will share the edge, and do this */
    /* by looking through all faces of the first vertex */

    e->ntris = 0;

    e->nconers = 0;		//A fake initialization

    for (i = 0; i < v1->ntris; i++)
    {
        f = v1->tris[i];
        /* examine the vertices of the face for a match with the second vertex */
        for (j = 0; j < 3; j++)
        {
            /* look for a match */
            if (f->verts[j] == v2)
            {
                e->ntris++;
                break;
            }
        }
    }

    /* create pointers from edges to faces and vice-versa */

    e->ntris = 0; /* start this out at zero again for creating ptrs to tris */

    for (i = 0; i < v1->ntris; i++) {

        f = v1->tris[i];

        /* examine the vertices of the face for a match with the second vertex */
        for (j = 0; j < 3; j++)
        {
            if (f->verts[j] == v2)
            {
                e->tris.push_back(f);
                e->ntris++;

                if (f->verts[(j + 1) % 3] == v1)
                    f->edges[j] = e;
                else if (f->verts[(j + 2) % 3] == v1)
                    f->edges[(j + 2) % 3] = e;
                else
                {
                    fprintf(stderr, "Non-recoverable inconsistancy in create_edge()\n");
                    exit(-1);
                }

                break;  /* we'll only find one instance of v2 */
            }
        }
    }
}

/******************************************************************************
Create edges.
******************************************************************************/

void Polyhedron::create_edges()
{
    int i,j;
    Triangle *f;
    Vertex *v1,*v2;
    double count = 0;

    //Initialize edge list and nedges

    nedges = 0;

    /* zero out all the pointers from faces to edges */

    for (i = 0; i < ntris; i++)
        for (j = 0; j < 3; j++)
            tlist[i]->edges[j] = NULL;

    /* create all the edges by examining all the triangles */

    for (i = 0; i < ntris; i++)
    {
        f = tlist[i];
        for (j = 0; j < 3; j++)
        {
            /* skip over edges that we've already created */
            if (f->edges[j])
                continue;
            v1 = f->verts[j];
            v2 = f->verts[(j+1) % f->nverts];
            create_edge (v1, v2);

            if (f->edges[j] == NULL)
                break;
        }
    }
}

/******************************************************************************
Create pointers from vertices to faces.
******************************************************************************/

void Polyhedron::vertex_to_tri_ptrs()
{
    int i, j, k;
    Triangle *f;
    Vertex *v;

    /* initialize the face counter */

    for (i = 0; i < nverts; i++)
    {
        vlist[i]->ntris = 0;
    }

    /* now actually create the face pointers */

    for (i = 0; i < ntris; i++)
    {
        f = tlist[i];
        for (j = 0; j < f->nverts; j++)
        {
            v = f->verts[j];

            //Check if vertex contain the face already
            bool contain = false;
            for (k = 0; k < v->ntris; k++)
            {
                if (v->tris[k] == f)
                    contain = true;
            }

            //Add tri if not contain
            if (!contain)
            {
                v->tris.push_back(f);
                v->ntris++;
            }
        }
    }
}

/******************************************************************************
Find the other quad that is incident on an edge, or NULL if there is
no other.
******************************************************************************/

Triangle *Polyhedron::other_triangle(Edge *edge, Triangle *tri)
{
  /* search for any other triangle */

  for (int i = 0; i < edge->tris.size(); i++)
    if (edge->tris[i] != tri)
      return (edge->tris[i]);

  /* there is no such other triangle if we get here */
  return (NULL);
}

/******************************************************************************
Order the pointers to faces that are around a given vertex.

Entry:
  v - vertex whose face list is to be ordered
******************************************************************************/

void Polyhedron::order_vertex_to_tri_ptrs(Vertex *v)
{
    int i,j;
    Triangle *f;
    Triangle *fnext;
    int nf;
    int vindex;
    int boundary;
    int count;

    v->ntris = v->tris.size();
    nf = v->ntris;
    f = v->tris[0];

    /* go backwards (clockwise) around faces that surround a vertex */
    /* to find out if we reach a boundary */

    boundary = 0;

    for (i = 1; i <= nf; i++)
    {
        /* find reference to v in f */
        vindex = -1;
        for (j = 0; j < f->nverts; j++)
        {
            if (f->verts[j] == v) {
                vindex = j;
                break;
            }
        }

        /* error check */
        if (vindex == -1)
        {
            fprintf (stderr, "can't find vertex #1\n");
            exit (-1);
        }

        /* corresponding face is the previous one around v */
        fnext = other_triangle (f->edges[vindex], f);

        /* see if we've reached a boundary, and if so then place the */
        /* current face in the first position of the vertice's face list */

        if (fnext == NULL)
        {
            /* find reference to f in v */
            for (j = 0; j < v->ntris; j++)
            {
                if (v->tris[j] == f)
                {
                    v->tris[j] = v->tris[0];
                    v->tris[0] = f;
                    break;
                }
            }
            boundary = 1;
            break;
        }

        f = fnext;
    }

    /* now walk around the faces in the forward direction and place */
    /* them in order */

    f = v->tris[0];
    count = 0;

    for (i = 1; i < nf; i++)
    {

        /* find reference to vertex in f */
        vindex = -1;
        for (j = 0; j < f->nverts; j++)
        {
            if (f->verts[(j+1) % f->nverts] == v)
            {
                vindex = j;
                break;
            }
        }

        /* error check */
        if (vindex == -1)
        {
            fprintf (stderr, "can't find vertex #2\n");
            exit (-1);
        }

        /* corresponding face is next one around v */
        fnext = other_triangle (f->edges[vindex], f);

        /* break out of loop if we've reached a boundary */
        count = i;
        if (fnext == NULL)
            break;

        /* swap the next face into its proper place in the face list */
        for (j = 0; j < v->ntris; j++)
        {
            if (v->tris[j] == fnext)
            {
                v->tris[j] = v->tris[i];
                v->tris[i] = fnext;
                break;
            }
        }
        f = fnext;
    }
}

/******************************************************************************
Find the index to a given vertex in the list of vertices of a given face.

Entry:
  f - face whose vertex list is to be searched
  v - vertex to return reference to

Exit:
  returns index in face's list, or -1 if vertex not found
******************************************************************************/

int Polyhedron::face_to_vertex_ref(Triangle *f, Vertex *v)
{
    int j;
    int vindex = -1;

    for (j = 0; j < f->nverts; j++)
    {
        if (f->verts[j] == v)
        {
            vindex = j;
            break;
        }
    }

    return (vindex);
}

/******************************************************************************
Create various face and vertex pointers.
******************************************************************************/

void Polyhedron::create_pointers()
{
    int i;

        /* index the vertices and triangles */

        if (nverts != vlist.size())
        {
            qInfo("== Vertex amount not correct!\nnverts: %d, vlist size: %d\n", nverts, vlist.size());
        }
        for (i = 0; i < nverts; i++)
            vlist[i]->index = i;

        if (ntris != tlist.size())
        {
            qInfo("== Triangle amount not correct!\nntris: %d, tlist size: %d\n", ntris, tlist.size());
        }
        for (i = 0; i < ntris; i++)
            tlist[i]->index = i;

        /* create pointers from vertices to triangles */
        vertex_to_tri_ptrs();

        /* double check the vlist */
        check_vlist();

        /* make edges */
        create_edges();

        /* order the pointers from vertices to faces */
        for (i = 0; i < nverts; i++)
        {
            order_vertex_to_tri_ptrs(vlist[i]);
        }

        /* Index the edge */
        for (i = 0; i < nedges; i++)
        {
            elist[i]->index = i;
        }

        /* Clear and Create corner table */
        bool result = ctable.ClearTable();
        ctable.corners.clear();
        if (!result)
            qInfo("== Error when clearing corner table!\n");
        result = ctable.GenerateTable(this);
        if (!result)
            printf("Error when generating corner table!\n");

        /* Check valence */
        double irr = check_valence();
        qInfo("== Total num of valence deficit are: %f\n", irr);
}

void Polyhedron::calc_bounding_sphere()
{
	unsigned int i;
	icVector3 min, max;

	for (i = 0; i < nverts; i++) {
		if (i == 0) {
			min.set(vlist[i]->x, vlist[i]->y, vlist[i]->z);
			max.set(vlist[i]->x, vlist[i]->y, vlist[i]->z);
		}
		else {
			if (vlist[i]->x < min.entry[0])
				min.entry[0] = vlist[i]->x;
			if (vlist[i]->x > max.entry[0])
				max.entry[0] = vlist[i]->x;
			if (vlist[i]->y < min.entry[1])
				min.entry[1] = vlist[i]->y;
			if (vlist[i]->y > max.entry[1])
				max.entry[1] = vlist[i]->y;
			if (vlist[i]->z < min.entry[2])
				min.entry[2] = vlist[i]->z;
			if (vlist[i]->z > max.entry[2])
				max.entry[2] = vlist[i]->z;
		}
	}
	center = (min + max) * 0.5;
	radius = length(center - min);
}

void Polyhedron::calc_edge_length()
{
	int i;
	icVector3 v1, v2;

	for (i = 0; i < nedges; i++) {
		v1.set(elist[i]->verts[0]->x, elist[i]->verts[0]->y, elist[i]->verts[0]->z);
		v2.set(elist[i]->verts[1]->x, elist[i]->verts[1]->y, elist[i]->verts[1]->z);
		elist[i]->length = length(v1 - v2);
	}
}

void Polyhedron::calc_face_normals_and_area()
{
    unsigned int i, j;
    icVector3 v0, v1, v2;
    Triangle *temp_t;
    double length[3];

    area = 0.0;
    //qInfo("ntris: %d\n", ntris);
    for (i=0; i<ntris; i++){
        for (j=0; j<3; j++)
            length[j] = tlist[i]->edges[j]->length;
        double temp_s = (length[0] + length[1] + length[2]) / 2.0;
        tlist[i]->area = sqrt(temp_s * (temp_s - length[0]) * (temp_s - length[1]) * (temp_s - length[2]));

        area += tlist[i]->area;
        temp_t = tlist[i];
        v1.set(tlist[i]->verts[0]->x, tlist[i]->verts[0]->y, tlist[i]->verts[0]->z);
        v2.set(tlist[i]->verts[1]->x, tlist[i]->verts[1]->y, tlist[i]->verts[1]->z);
        v0.set(tlist[i]->verts[2]->x, tlist[i]->verts[2]->y, tlist[i]->verts[2]->z);
        tlist[i]->normal = cross((v0-v1), (v2-v1));
        normalize(tlist[i]->normal);
    }

    double signedvolume = 0.0;
    icVector3 test = center;
    for (i=0; i<ntris; i++){
        icVector3 cent(vlist[tlist[i]->verts[0]->index]->x, vlist[tlist[i]->verts[0]->index]->y, vlist[tlist[i]->verts[0]->index]->z);
        signedvolume += dot(test-cent, tlist[i]->normal)*tlist[i]->area;
    }
    signedvolume /= area;
    if (signedvolume<0)
        orientation = 0;
    else {
        orientation = 1;
        for (i=0; i<ntris; i++)
            tlist[i]->normal *= -1.0;
    }
}

double Polyhedron::check_valence()
{
    int nirre = 0;

    //First create the valence map
    for (int i = 0; i < nedges; i++)
    {
        //Chekc if vertex contains edge already
        Vertex* v1 = elist[i]->verts[0];
        Vertex* v2 = elist[i]->verts[1];

        bool contain = false;
        if(v1->edges.size() != 0)
            for (int j = 0; j < v1->edges.size(); j++)
            {
                if (v1->edges[j] == elist[i])
                    contain = true;
            }
        if (!contain)
        {
            v1->edges.push_back(elist[i]);
            v1->nedges++;
        }

        contain = false;
        if(v2->edges.size() != 0)
            for (int j = 0; j < v2->edges.size(); j++)
            {
                if (v2->edges[j] == elist[i])
                    contain = true;
            }
        if (!contain)
        {
            v2->edges.push_back(elist[i]);
            v2->nedges++;
        }
    }

    //Then check for all valence
    double totalAngle = 0;
    for (int i = 0; i < nverts; i++)
    {
        vlist[i]->valence = vlist[i]->edges.size();
        //printf("Vertex %d's valence is: %d\n", i, vlist[i]->valence);
        if (vlist[i]->valence != 6)
            nirre += (vlist[i]->valence - 6);

        double total = 0;
        for (int j = 0; j < vlist[i]->ncorners; j++)
        {
            total += vlist[i]->corners[j]->angle;
        }
        vlist[i]->totalAngle = total;
        totalAngle += (total - M_PI * 2);
        //printf("%f\n", total);
    }
    qInfo("== Total angle is: %f\n", totalAngle);

    return nirre;
}

bool Polyhedron::irregular_divide_triangle(Triangle* currentTri, int newVert)
{
    //The current three edge
    Edge* e1 = currentTri->edges[0];
    Edge* e2 = currentTri->edges[1];
    Edge* e3 = currentTri->edges[2];

    icVector3 iv1, iv2, iv3, inv1, inv2, inv3;

    //The current three vertex of triangle
    Vertex* v1 = currentTri->verts[0];
    Vertex* v2 = currentTri->verts[1];
    Vertex* v3 = currentTri->verts[2];
    iv1.set(v1->x, v1->y, v1->z);
    iv2.set(v2->x, v2->y, v2->z);
    iv3.set(v3->x, v3->y, v3->z);

    //The three new vertex
    Vertex* nv1 = e1->sflag.newVert;
    Vertex* nv2 = e2->sflag.newVert;
    Vertex* nv3 = e3->sflag.newVert;
    if(nv1)
        inv1.set(nv1->x, nv1->y, nv1->z);
    if(nv2)
        inv2.set(nv2->x, nv2->y, nv2->z);
    if(nv3)
        inv3.set(nv3->x, nv3->y, nv3->z);

    Triangle* nt[3];		//ntc - the center new triangle

    if (newVert == 2)
    {
        if (nv1 != NULL && nv2 != NULL)
        {
            nt[0] = new Triangle;

            //Vertex of new triangle
            nt[0]->nverts = 3;
            nt[0]->verts[0] = nv1;
            nt[0]->verts[1] = v2;
            nt[0]->verts[2] = nv2;

            //Find shortest
            if (length(inv1 - iv3) < length(inv2 - iv1))
            {
                nt[1] = new Triangle;

                //Vertex of new triangle
                nt[1]->nverts = 3;
                nt[1]->verts[0] = nv1;
                nt[1]->verts[1] = nv2;
                nt[1]->verts[2] = v3;

                nt[2] = new Triangle;

                //Vertex of new triangle
                nt[2]->nverts = 3;
                nt[2]->verts[0] = v1;
                nt[2]->verts[1] = nv1;
                nt[2]->verts[2] = v3;
            }
            else
            {
                nt[1] = new Triangle;

                //Vertex of new triangle
                nt[1]->nverts = 3;
                nt[1]->verts[0] = nv1;
                nt[1]->verts[1] = nv2;
                nt[1]->verts[2] = v1;

                nt[2] = new Triangle;

                //Vertex of new triangle
                nt[2]->nverts = 3;
                nt[2]->verts[0] = v1;
                nt[2]->verts[1] = nv2;
                nt[2]->verts[2] = v3;
            }
        }
        else if (nv1 != NULL && nv3 != NULL)
        {
            nt[0] = new Triangle;

            //Vertex of new triangle
            nt[0]->nverts = 3;
            nt[0]->verts[0] = v1;
            nt[0]->verts[1] = nv1;
            nt[0]->verts[2] = nv3;

            //Find shortest
            if (length(inv1 - iv3) < length(inv3 - iv2))
            {
                nt[1] = new Triangle;

                //Vertex of new triangle
                nt[1]->nverts = 3;
                nt[1]->verts[0] = nv1;
                nt[1]->verts[1] = v3;
                nt[1]->verts[2] = nv3;

                nt[2] = new Triangle;

                nt[2]->nverts = 3;
                nt[2]->verts[0] = nv1;
                nt[2]->verts[1] = v2;
                nt[2]->verts[2] = v3;
            }
            else
            {
                nt[1] = new Triangle;

                //Vertex of new triangle
                nt[1]->nverts = 3;
                nt[1]->verts[0] = nv1;
                nt[1]->verts[1] = v2;
                nt[1]->verts[2] = nv3;

                nt[2] = new Triangle;

                //Vertex of new triangle
                nt[2]->nverts = 3;
                nt[2]->verts[0] = v2;
                nt[2]->verts[1] = v3;
                nt[2]->verts[2] = nv3;
            }
        }
        else if (nv2 != NULL && nv3 != NULL)
        {
            nt[0] = new Triangle;

            //Vertex of new triangle
            nt[0]->nverts = 3;
            nt[0]->verts[0] = v3;
            nt[0]->verts[1] = nv3;
            nt[0]->verts[2] = nv2;

            //Find shortest
            if (length(inv3 - iv2) < length(inv2 - iv1))
            {
                nt[1] = new Triangle;

                //Vertex of new triangle
                nt[1]->nverts = 3;
                nt[1]->verts[0] = nv3;
                nt[1]->verts[1] = v2;
                nt[1]->verts[2] = nv2;

                nt[2] = new Triangle;

                //Vertex of new triangle
                nt[2]->nverts = 3;
                nt[2]->verts[0] = v1;
                nt[2]->verts[1] = v2;
                nt[2]->verts[2] = nv3;
            }
            else
            {
                nt[1] = new Triangle;

                //Vertex of new triangle
                nt[1]->nverts = 3;
                nt[1]->verts[0] = nv3;
                nt[1]->verts[1] = v1;
                nt[1]->verts[2] = nv2;

                nt[2] = new Triangle;

                //Vertex of new triangle
                nt[2]->nverts = 3;
                nt[2]->verts[0] = v1;
                nt[2]->verts[1] = v2;
                nt[2]->verts[2] = nv2;
            }
        }
        else
        {
            qInfo("== Get error when irregular subdivide triangle.\n");
            return false;
        }

        //Link vert with new triangle and push all triangle to the tlist
        for (int j = 0; j < 3; j++)
        {
            for (int k = 0; k < 3; k++)
            {
                if (nt[j]->verts[k] == NULL)
                {
                    qInfo("== Assigned NULL vertex to triangle!\n");
                    return false;
                }
                nt[j]->verts[k]->tris.push_back(nt[j]);
                nt[j]->verts[k]->ntris++;
            }
            tlist.push_back(nt[j]);
            ntris++;
        }
    }
    else if (newVert == 1)
    {
        if (nv1 != NULL)
        {
            nt[0] = new Triangle;

            //Vertex of new triangle
            nt[0]->nverts = 3;
            nt[0]->verts[0] = v1;
            nt[0]->verts[1] = nv1;
            nt[0]->verts[2] = v3;

            nt[1] = new Triangle;

            //Vertex of new triangle
            nt[1]->nverts = 3;
            nt[1]->verts[0] = nv1;
            nt[1]->verts[1] = v2;
            nt[1]->verts[2] = v3;
        }
        else if (nv2 != NULL)
        {
            nt[0] = new Triangle;

            //Vertex of new triangle
            nt[0]->nverts = 3;
            nt[0]->verts[0] = v1;
            nt[0]->verts[1] = v2;
            nt[0]->verts[2] = nv2;

            nt[1] = new Triangle;

            //Vertex of new triangle
            nt[1]->nverts = 3;
            nt[1]->verts[0] = v1;
            nt[1]->verts[1] = nv2;
            nt[1]->verts[2] = v3;
        }
        else if (nv3 != NULL)
        {
            nt[0] = new Triangle;

            //Vertex of new triangle
            nt[0]->nverts = 3;
            nt[0]->verts[0] = v1;
            nt[0]->verts[1] = v2;
            nt[0]->verts[2] = nv3;

            nt[1] = new Triangle;

            //Vertex of new triangle
            nt[1]->nverts = 3;
            nt[1]->verts[0] = nv3;
            nt[1]->verts[1] = v2;
            nt[1]->verts[2] = v3;
        }
        else
        {
            qInfo("== Get error when irregular subdivide triangle.\n");
            return false;
        }

        //Link vert with new triangle and push all triangle to the tlist
        for (int j = 0; j < 2; j++)
        {
            for (int k = 0; k < 3; k++)
            {
                if (nt[j]->verts[k] == NULL)
                {
                    qInfo("== Assigned NULL vertex to triangle!\n");
                    return false;
                }
                nt[j]->verts[k]->tris.push_back(nt[j]);
                nt[j]->verts[k]->ntris++;
            }
            tlist.push_back(nt[j]);
            ntris++;
        }
    }

    //Set old triangle and edge to be delete
    currentTri->sflag.ifOld = true;
    currentTri->sflag.ifNeedRemove = true;

    return true;
}

bool Polyhedron::regular_divide_triangle(Triangle* currentTri)
{
    //The current three edge
    Edge* e1 = currentTri->edges[0];
    Edge* e2 = currentTri->edges[1];
    Edge* e3 = currentTri->edges[2];

    //The current three vertex of triangle
    Vertex* v1 = currentTri->verts[0];
    Vertex* v2 = currentTri->verts[1];
    Vertex* v3 = currentTri->verts[2];

    //The three new vertex
    Vertex* nv1 = e1->sflag.newVert;
    Vertex* nv2 = e2->sflag.newVert;
    Vertex* nv3 = e3->sflag.newVert;

    Triangle* nt1, * nt2, * nt3, * ntc;		//ntc - the center new triangle

    /*** New tri 1 ***/

    nt1 = new Triangle;
    nt1->index = tlist.back()->index + 1;

    //Vertex of new triangle
    nt1->nverts = 3;
    nt1->verts[0] = v1;
    nt1->verts[1] = nv1;
    nt1->verts[2] = nv3;

    //Link vert with new triangle
    v1->tris.push_back(nt1);
    v1->ntris++;
    nv1->tris.push_back(nt1);
    nv1->ntris++;
    nv3->tris.push_back(nt1);
    nv3->ntris++;

    /*** New tri 2 ***/

    nt2 = new Triangle;
    nt2->index = tlist.back()->index + 2;

    //Vertex of new triangle
    nt2->nverts = 3;
    nt2->verts[0] = v2;
    nt2->verts[1] = nv2;
    nt2->verts[2] = nv1;

    //Link vert with new triangle
    nv1->tris.push_back(nt2);
    nv1->ntris++;
    v2->tris.push_back(nt2);
    v2->ntris++;
    nv2->tris.push_back(nt2);
    nv2->ntris++;

    /*** New tri 3 ***/

    nt3 = new Triangle;
    nt3->index = tlist.back()->index + 3;

    //Vertex of new triangle
    nt3->nverts = 3;
    nt3->verts[0] = v3;
    nt3->verts[1] = nv3;
    nt3->verts[2] = nv2;

    //Link vert with new triangle
    nv3->tris.push_back(nt3);
    nv3->ntris++;
    nv2->tris.push_back(nt3);
    nv2->ntris++;
    v3->tris.push_back(nt3);
    v3->ntris++;

    /*** New tri center ***/

    ntc = new Triangle;
    ntc->index = tlist.back()->index + 4;

    //Vertex of new triangle
    ntc->nverts = 3;
    ntc->verts[0] = nv2;
    ntc->verts[1] = nv3;
    ntc->verts[2] = nv1;

    //Link vert with new triangle
    ntc->verts[0]->tris.push_back(ntc);
    ntc->verts[0]->ntris++;
    ntc->verts[1]->tris.push_back(ntc);
    ntc->verts[1]->ntris++;
    ntc->verts[2]->tris.push_back(ntc);
    ntc->verts[2]->ntris++;

    //Push new triangle onto the list
    tlist.push_back(nt1);
    ntris++;
    tlist.push_back(nt2);
    ntris++;
    tlist.push_back(nt3);
    ntris++;
    tlist.push_back(ntc);
    ntris++;

    //Set old triangle and edge to be delete
    currentTri->sflag.ifOld = true;
    currentTri->sflag.ifNeedRemove = true;

    return true;
}

bool Polyhedron::irregular_subdivide()
{
    qInfo("== Start subdivide the mesh, running with irregular method.\n");

    double avgLen = 0.0f;
    int totalEdge = nedges;		//Here need to track to old total number of edges
    if (totalEdge != elist.size())
        qInfo("== Edge number not correct before create new edge!\n");
    for (int i = 0; i < totalEdge; i++)
    {
        avgLen += elist[i]->length;
    }
    avgLen = avgLen / totalEdge;
    qInfo("== The average edge length is: %f\n", avgLen);

    //Loop through the edges to subdivide them
    for (int i = 0; i < totalEdge; i++)
    {
        Edge* currentEdge = elist[i];

        if (currentEdge->length > avgLen)
        {
            //Get two vert
            Vertex* v1 = currentEdge->verts[0];
            Vertex* v2 = currentEdge->verts[1];

            //Find the new mid vertex and create new vert
            Vertex* newVert = new Vertex((v1->x + v2->x) / 2.0f, (v1->y + v2->y) / 2.0f, (v1->z + v2->z) / 2.0f);

            //Save the new vert (both vlist and flag info)
            vlist.push_back(newVert);
            nverts++;

            currentEdge->sflag.newVert = newVert;
        }
    }
    if (nedges != elist.size())
        qInfo("== Edge number not correct after create new edge!\n");

    //Clear all the vert to triangle pointer
    for (int i = 0; i < vlist.size(); i++)
    {
        vlist[i]->tris.clear();
        vlist[i]->ntris = 0;
        vlist[i]->edges.clear();
        vlist[i]->nedges = 0;
        vlist[i]->corners.clear();
        vlist[i]->ncorners = 0;
    }

    //Loop through the whole triangle list to generate new triangles
    int totalTri = ntris;		//Same idea as edge
    if (totalTri != tlist.size())
        qInfo("== Triangle number not correct before create new triangle!\n");
    for (int i = 0; i < totalTri; i++)
    {
        Triangle* currentTri = tlist[i];

        //Check which situation is this
        int newVerts = 0;
        for (int j = 0; j < 3; j++)
        {
            if (currentTri->edges[j]->sflag.newVert != NULL)
                newVerts++;
        }

        if (newVerts == 3)
            regular_divide_triangle(currentTri);
        else if (newVerts == 2 || newVerts == 1)
            irregular_divide_triangle(currentTri, newVerts);
        else if (newVerts == 0)
        {
            //Add the triangle back and link together
            currentTri->sflag.ifOld = false;
            currentTri->sflag.ifNeedRemove = false;
            for (int j = 0; j < 3; j++)
            {
                currentTri->verts[j]->tris.push_back(currentTri);
                currentTri->verts[j]->ntris++;
            }
        }
        else
            printf("WTF!\n");
    }
    if (ntris != tlist.size())
        qInfo("== Triangle number not correct after create new triangle!\n");

    qInfo("== Subdivision generation finished, deleting garbage tri and edge.\n");

    bool result = subdivide_finalize();
    if (!result)
    {
        qInfo("== Subdivision logic error when finalize!\n");
        return false;
    }

    qInfo("== nverts: %d, nedges: %d, ntris: %d\n", nverts, nedges, ntris);

    return true;
}

bool Polyhedron::regular_subdivide()
{
    qInfo("== Start subdivide the mesh, running with regular method.\n");

    //Loop through the edges to subdivide them
    int totalEdge = nedges;		//Here need to track to old total number of edges
    if (totalEdge != elist.size())
        qInfo("== Edge number not correct before create new edge!\n");
    for (int i = 0; i < totalEdge; i++)
    {
        Edge* currentEdge = elist[i];

        //Get two vert
        Vertex* v1 = currentEdge->verts[0];
        Vertex* v2 = currentEdge->verts[1];

        //Find the new mid vertex and create new vert
        Vertex* newVert = new Vertex((v1->x + v2->x) / 2.0f, (v1->y + v2->y) / 2.0f, (v1->z + v2->z) / 2.0f );

        //Save the new vert (both vlist and flag info)
        vlist.push_back(newVert);
        nverts++;

        currentEdge->sflag.newVert = newVert;
    }
    if (nedges != elist.size())
        qInfo("== Edge number not correct after create new edge!\n");

    //Clear all the vert to triangle pointer
    for (int i = 0; i < vlist.size(); i++)
    {
        vlist[i]->tris.clear();
        vlist[i]->ntris = 0;
        vlist[i]->edges.clear();
        vlist[i]->nedges = 0;
        vlist[i]->corners.clear();
        vlist[i]->ncorners = 0;
    }

    //Loop through the whole triangle list to generate new triangles
    int totalTri = ntris;		//Same idea as edge
    if (totalTri != tlist.size())
        qInfo("== Triangle number not correct before create new triangle!\n");
    for (int i = 0; i < totalTri; i++)
    {
        regular_divide_triangle(tlist[i]);
    }
    if (ntris != tlist.size())
        qInfo("== Triangle number not correct after create new triangle!\n");

    qInfo("== Subdivision generation finished, deleting garbage tri and edge.\n");

    bool result = subdivide_finalize();
    if (!result)
    {
        qInfo("== Subdivision logic error when finalize!\n");
        return false;
    }

    qInfo("== nverts: %d, nedges: %d, ntris: %d\n", nverts, nedges, ntris);

    return true;
}

bool Polyhedron::subdivide_finalize()
{
    /***** Finalize the process *****/

    qInfo("== Subdivide finish, start finalize.\n");

    /* get rid of triangles that use the same vertex more than once */

    for (int i = 0; i < tlist.size(); i++)
    {
        Triangle* tri = tlist[i];
        Vertex* v0 = tri->verts[0];
        Vertex* v1 = tri->verts[1];
        Vertex* v2 = tri->verts[2];

        if (v0 == v1 || v1 == v2 || v2 == v0)
        {
            qInfo("== Find same triangle\n");
            tlist.erase(tlist.begin() + i);
            ntris--;
            tlist[i] = tlist[ntris];
        }
    }

    //Loop through the triangle list to remove all the old triangles
    for (int i = 0; i < tlist.size(); i++)
    {
        if (tlist[i]->sflag.ifNeedRemove)
        {
            tlist.erase(tlist.begin() + i);
            ntris--;
            i = 0;
        }
    }

    for (int i = 0; i < tlist.size(); i++)
    {
        if (tlist[i]->sflag.ifNeedRemove)
        {
            tlist.erase(tlist.begin() + i);
            ntris--;
        }
    }

    for (int i = 0; i < tlist.size(); i++)
    {
        if (tlist[i]->sflag.ifNeedRemove)
        {
            qInfo("== There still triangle need clear!\n");
        }
    }

    //Clear the edge list
    elist.clear();

    /* create pointers from vertices to triangles */

    create_pointers();

    /* Calculate all necessary properties */
    calc_edge_length();
    calc_bounding_sphere();
    calc_face_normals_and_area();
    average_normals();

    /***Check valence***/

    double irr = check_valence();
    qInfo("== Total num of valence deficit are: %f\n", irr);

    return true;
}

float Polyhedron::smooth_weight_calc(int mode, Vertex * currentV, Edge * targetE)
{
    float w = 0;

    if(mode == 1)       //Uniform
        w= 1.0f;
    else if(mode == 2)      //Cord
    {
        if(targetE->length > 1)
            w = 1.0f / targetE->length;
        else
            w = targetE->length;
    }
    else if(mode == 3)      //Mean curvature
    {
        //Only apply if a regular edge of a pan
        if(targetE->corners.size() == 2)
        {
            float th1 = targetE->corners[0]->angle;
            float th2 = targetE->corners[1]->angle;

            float t1, t2;

            if(sin(th1 == 0))
                t1 = 1;
            else
                t1 = cos(th1) / sin(th1);

            if(sin(th2 == 0))
                t2 = 1;
            else
                t2 = cos(th2) / sin(th2);
            w = (t1 + t2) / 2.0f;
        }
    }
    else if(mode == 4)      //Mean value
    {
        if(targetE->tris.size() == 2)   //Should be right, right?
        {
            float th1 = 0.0f;
            float th2 = 0.0f;

            //Find two corners
            for(int i = 0; i < 3; i++)
            {
                //First tri check
                if((targetE->tris[0]->edges[i]->verts[0] != currentV) &&
                       (targetE->tris[0]->edges[i]->verts[0] != currentV))
                {
                    th1 = targetE->tris[0]->edges[i]->corners[0]->angle;
                    if(targetE->tris[0]->edges[i]->corners[0]->v != currentV)
                        th1 = targetE->tris[0]->edges[i]->corners[1]->angle;
                }

                //First tri check
                if((targetE->tris[1]->edges[i]->verts[0] != currentV) &&
                       (targetE->tris[1]->edges[i]->verts[0] != currentV))
                {
                    th1 = targetE->tris[1]->edges[i]->corners[0]->angle;
                    if(targetE->tris[1]->edges[i]->corners[0]->v != currentV)
                        th1 = targetE->tris[1]->edges[i]->corners[1]->angle;
                }
            }

            w = (tan(th1 / 2.0f) + tan(th2 / 2.0f)) / 2.0f;
        }
    }

    return w;
}

bool Polyhedron::general_smooth(int mode, float dt)
{
    //qInfo("== Running uniform surface smooth\n");

    //Security check
    if(nverts != vlist.size())
    {
        qInfo("== Vertex list number incorrect!\n");
        return false;
    }

    float wt = 0;

    //Loop through each vertex
    for(int i = 0; i < nverts; i++)
    {
        Vertex * currentVert = vlist[i];
        currentVert->sflag.x = currentVert->x;
        currentVert->sflag.y = currentVert->y;
        currentVert->sflag.z = currentVert->z;

        //edge check
        if(currentVert->nedges != currentVert->edges.size())
        {
            currentVert->nedges = currentVert->edges.size();
        }

        //corner check
        if(currentVert->nedges != currentVert->ncorners)
        {
            qInfo("==%d v: nedge %d - ncorner %d\n", i, currentVert->nedges, currentVert->ncorners);
        }

        //Access each vertex in pan by loop through elist
        for(int j = 0; j < currentVert->nedges; j++)
        {
            //Get another vertex
            Vertex * targetV = targetV = currentVert->edges[j]->verts[0];
            if(currentVert->edges[j]->verts[0] == currentVert)
                targetV = currentVert->edges[j]->verts[1];

            float w;

            w = smooth_weight_calc(mode, currentVert, currentVert->edges[j]);

            //w /= currentVert->nedges;
            if(mode == 3 || mode == 4)
            {
                if(w > 2)
                    w = 2;
                else if(w < -2)
                    w = -2;

                if(w < 1 && w > -1)
                    w = 0;
            }

            //qInfo("%f\n", w);

            currentVert->sflag.x += dt * w * (targetV->x - currentVert->x);
            currentVert->sflag.y += dt * w * (targetV->y - currentVert->y);
            currentVert->sflag.z += dt * w * (targetV->z - currentVert->z);
        }

        currentVert->sflag.ifOld = true;
    }

    //qInfo("== Uniform Smooth finish. w avg: %f\n", wt / nverts);

    return true;
}

bool Polyhedron::surface_smooth_finalize()
{
    qInfo("== Start finalize surface smooth process.\n");

    for(int i = 0; i < nverts; i++)
    {
        if(!vlist[i]->sflag.ifOld)
        {
            qInfo("== Vertex no updated!\n");
            return false;
        }

        vlist[i]->x = vlist[i]->sflag.x;
        vlist[i]->y = vlist[i]->sflag.y;
        vlist[i]->z = vlist[i]->sflag.z;

        vlist[i]->sflag.ifOld = false;

        vlist[i]->sflag.x = 0;
        vlist[i]->sflag.y = 0;
        vlist[i]->sflag.z = 0;
    }

    /* Calculate all necessary properties */
    calc_edge_length();
    calc_bounding_sphere();
    calc_face_normals_and_area();
    average_normals();

    /***Check valence***/

    double irr = check_valence();
    qInfo("== Total num of valence deficit are: %f\n", irr);

    return true;
}

bool Polyhedron::surface_smooth(int mode)
{
    bool check;
    for(int i = 0; i < 1; i++)
    {
        check = false;

        //qInfo("== Iteration %d\n", i);
        check = general_smooth(mode, 0.1);

        if(check)
            surface_smooth_finalize();
        else
        {
            qInfo("== Something wrong with surface smooth.\n");
            return false;
        }
    }

    return true;
}

int Polyhedron::check_type()
{
    vector<int> indicator;
    int M = 0;
    for(int i = 0; i < vlist.size(); i++)
    {
        Vertex * currentVert = vlist[i];

        //Access each vertex in pan by loop through elist
        for(int j = 0; j < currentVert->nedges; j++)
        {
            //Get another vertex
            Vertex * targetV = targetV = currentVert->edges[j]->verts[0];
            if(currentVert->edges[j]->verts[0] == currentVert)
                targetV = currentVert->edges[j]->verts[1];

            if(targetV->heat > currentVert->heat)
                indicator.push_back(1);
            else if(targetV->heat < currentVert->heat)
                indicator.push_back(-1);
            else
                indicator.push_back(0);
        }

        int nSwitch = 0;
        int currentI = indicator[0];
        for(int j = 1; j < indicator.size(); j++)
        {
            if(currentI * indicator[i] < 1)
                nSwitch++;
            currentI = indicator[i];
        }

        if(nSwitch == 0 && indicator[0] == 1)
        {
            currentVert->sflag.typeIdentity = 1;        //Minimum
            M++;
        }
        else if(nSwitch == 0 && indicator[0] == -1)
        {
            currentVert->sflag.typeIdentity = 2;        //Maximum
            M++;
        }
        else if(nSwitch != 0)
        {
            currentVert->sflag.typeIdentity = 3;        //Saddle
            currentVert->sflag.nSaddle = nSwitch;
            M -= nSwitch;
        }
    }

    return M;
}

bool Polyhedron::initialize_heat()
{
    if(heatInited)
       return true;

    int vNum = vlist.size();

    //Randomly pick up one vertex to be min and max
    srand(time(NULL));
    int max, min;
    float maxx = -999;
    float minx = 999;
    for(int i = 0; i < vlist.size(); i++)
    {
        if(vlist[i]->x > maxx)
        {
            maxx = vlist[i]->x;
            max = i;
        }
        if(vlist[i]->x < minx)
        {
            minx = vlist[i]->x;
            min = i;
        }
    }
    max = rand() % vlist.size();
    min = rand() % vlist.size();
    vlist[max]->heat = maxHeat;
    vlist[max]->sflag.newHeat = maxHeat;
    vlist[max]->lockHeat = true;
    vlist[max]->minOrMax = 1;
    vlist[min]->lockHeat = true;
    vlist[min]->minOrMax = -1;

    //Loop through the vertex list to assign the heat
    for(int i = 0; i < vlist.size(); i++)
    {
        //Also initialize the list index btw
        vlist[i]->sflag.i = i;

        //Initialize the heat value
        if(!vlist[i]->lockHeat)
            vlist[i]->heat = 0.0f;
    }

    heatInited = true;

    qInfo("== Heat diffusion calc initialized.\n");

    return true;
}

bool Polyhedron::general_diffusion(int mode, float dt, float lambda)
{
    //qInfo("== Heat diffusion with mode %d\n", mode);

    /*** Prepare for mode 2 the implicit ***/

    typedef Eigen::SparseMatrix<double> SpMat;
    typedef Eigen::COLAMDOrdering<int> Sint;
    typedef Eigen::SparseLU<SpMat, Sint> SpC;
    //typedef Eigen::SimplicialLDLT<SpMat> SpC ;
    //typedef Eigen::SimplicialCholesky<SpMat> SpC ;
    typedef Eigen::VectorXd VecX;

    int d = vlist.size();
    SpMat A(d, d);
    VecX b(d);

    /*** Prepare for mode 2 the implicit ***/


    //Security check
    if(nverts != vlist.size())
    {
        qInfo("== Vertex list number incorrect!\n");
        return false;
    }

    //Loop through each vertex of vertex list
    for(int i = 0; i < nverts; i++)
    {
        Vertex * currentVert = vlist[i];
        float newHeat = currentVert->heat;

        //Initialize the current u vector b
        b[i] = currentVert->heat;

        //If the max or min vert the pass,
        //but for sparse matrix need coe 1 set
        if(currentVert->lockHeat)
        {
            //Initialize the A for current vert
            if(mode == 2)
                //Should be (i, i)
                A.coeffRef(i, currentVert->sflag.i) = 1.0f;
            continue;
        }

        //edge check
        if(currentVert->nedges != currentVert->edges.size())
        {
            currentVert->nedges = currentVert->edges.size();
        }
        float numE = currentVert->nedges;

        //Initialize the A for current vert
        if(mode == 2)
            //A.coeffRef(i, currentVert->sflag.i) = 0;
            A.coeffRef(i, currentVert->sflag.i) = 1 + 1 * dt * lambda;

        //Access each vertex in pan by loop through elist
        for(int j = 0; j < currentVert->nedges; j++)
        {
            //Get another vertex
            Vertex * targetV = targetV = currentVert->edges[j]->verts[0];
            if(currentVert->edges[j]->verts[0] == currentVert)
                targetV = currentVert->edges[j]->verts[1];

            if(mode != 2)
            {
                //Calculate the relate heat transfer
                newHeat += (dt * (targetV->heat - currentVert->heat) / numE);
            }
            else
            {
                //Mean curvature weight
                //float w = smooth_weight_calc(3, dt, currentVert, targetV, currentVert->edges[j]);

                //Update the sparse matrix
                //
                //for i'th iteration, corresponding i'th row in matrix (in-list order)
                //for k'th target vertex index in list, it's k'th column
                //
                //According to formula, each other v's heat has 1 / totalV * dt coe
                A.coeffRef(i, targetV->sflag.i) = -1 * dt * lambda / numE;
            }
        }

        if(newHeat > maxHeat)
            newHeat = maxHeat;
        else if(newHeat < 0.0)
            newHeat = 0.0;

        if(mode == 1)   //Explicit update
            currentVert->sflag.newHeat = newHeat;
        else if(mode == 3)  //Gauss update
        {
            currentVert->heat = newHeat;
            currentVert->sflag.newHeat = newHeat;
        }
    }

    /*** Solve for A x = b ***/
    if(mode == 2)
    {
        //qInfo("== Solving for sparse result\n");

        //Build the solver, for LU method solver
        SpC chol;
        chol.analyzePattern(A);
        chol.factorize(A);

        //Solver for LDLT
        //SpC chol;
        //chol.compute(A);

        //Regular SpC
        //SpC chol(A);

        //Solve the result
        VecX x = chol.solve(b);

        //Check the size first
        if(x.size() != vlist.size())
        {
            qInfo("== x size: %d, vlist size: %d\n", x.size(), vlist.size());
            return false;
        }

        for(int i = 0; i < x.size(); i++)
        {
            float result = x[i];

            //Clamp
            if(result > maxHeat)
                result = maxHeat;
            else if(result < 0.0)
                result = 0.0;

            if(vlist[i]->minOrMax == 1)
                result = maxHeat;
            else if(vlist[i]->minOrMax == -1)
                result = 0.0f;

            vlist[i]->sflag.newHeat = result;
        }
    }

    //qInfo("== Heat diffusion calc finish.\n");

    return true;
}

bool Polyhedron::diffusion_finalize()
{
    for(int i = 0; i < vlist.size(); i++)
    {
        vlist[i]->heat = vlist[i]->sflag.newHeat;
    }

    //qInfo("== Heat info updated.\n");

    return true;
}

bool Polyhedron::heat_diffusion(int mode)
{
    initialize_heat();

    bool check;
    int iterations;

    if(mode != 2)
        iterations = 1000;
    else
        iterations = 1;

    for(int i = 0; i < iterations; i++)
    {
        check = false;

        qInfo("Iteration: %d\n", i);

        //1 100 for explicit and gauss
        //5 100 for implicit
        if(mode != 2)
            check = general_diffusion(mode, 1.0f, 100.0f);
        else
            check = general_diffusion(mode, 5.0f, 100.0f);

        if(check)
            diffusion_finalize();
        else
        {
            qInfo("== Something wrong with surface smooth.\n");
            return false;
        }
    }

    int M = check_type();

    int Eu = vlist.size() - elist.size() + tlist.size();

    qInfo("== The Multiplicity M is: %d\n", M);
    qInfo("== The Eu is: %d\n", Eu);


    return true;
}

float Polyhedron::find_Amix(Vertex *tvert)
{
    float aMix = 0.0f;

    //Loop through the corners (looking for triangle)
    for(int i = 0; i < tvert->corners.size(); i++)
    {
        //qInfo("Corner %d\n", i);

        //Access the corner
        Corner * cor = tvert->corners[i];
        Corner * c1 = NULL;
        Corner * c2 = NULL;      //preparation

        //Access the triangle
        Triangle * tri = cor->t;

        //First check if the triangle is obtuse
        bool ifObtuse = false;
        for(int j = 0; j < 3; j++)
        {
            if(tri->corners[j]->angle > (M_PI / 2))
                ifObtuse = true;

            //Check if current corner, save for later
            if(tri->corners[j] != cor)
            {
                if(c1 == NULL)
                    c1 = tri->corners[j];
                else
                    c2 = tri->corners[j];
            }
        }

        if(c1 == NULL || c2 == NULL)
            qInfo("Nani?\n");
        //qInfo("Here?\n");

        //If the obtuse triangle
        if(ifObtuse)
        {
            //Check the current corner
            if(cor->angle > (M_PI / 2))
                aMix += tri->area / 2.0f;
            else
                aMix += tri->area / 4.0f;

            continue;
        }

        //If the regular triangle, calculate the voronoi area
        float cot1;
        float tan1;
        if(sin(c1->angle) == 0)
            cot1 = 1;
        else
            cot1 = cos(c1->angle) / sin(c1->angle);

        float cot2;
        float tan2;
        if(sin(c2->angle) == 0)
            cot2 = 1;
        else
            cot2 = cos(c2->angle) / sin(c2->angle);

        aMix += (pow((c1->e->length), 2) * cot1 + pow((c2->e->length), 2) * cot2) / 8.0f;
    }

    //qInfo("Finish amix\n");

    return aMix;
}

vec3 Polyhedron::iv2vec(Vertex * vert)
{
    return vec3(vert->x, vert->y, vert->z);
}

float Polyhedron::find_normal_curvature(Vertex * vert, Vertex * vtarget)
{
    //Get the normal
    vec3 n = vec3(vert->normal.x, vert->normal.y, vert->normal.z);

    //Convert to vec3
    vec3 source = vec3(vert->x, vert->y, vert->z);
    vec3 target = vec3(vtarget->x, vtarget->y, vtarget->z);

    vec3 point = target - source;
    float d = point.x * point.x +
            point.y * point.y +
            point.z * point.z;

    return 2 * dot(point, n) / d;
}

Vertex * Polyhedron::find_another_point_on_edge(Vertex *vert, int pos)
{
    for(int i = 0; i < 2; i++)
        if(vert->edges[pos]->verts[i] != vert)
            return vert->edges[pos]->verts[i];

    return NULL;
}

vec3 Polyhedron::find_edge_vector(Vertex * vert, int pos)
{
    Vertex * target = find_another_point_on_edge(vert, pos);

    return iv2vec(target) - iv2vec(vert);
}

vector<vec2> Polyhedron::mapping_to_tangent_plane(Vertex * vert, vec3 & b1, vec3 & b2)
{
    vector<vec2> result;

    //Get the normal
    vec3 n = vec3(vert->normal.x, vert->normal.y, vert->normal.z);

    int pos = 0;
    /*
    float max = -999;
    for(int i = 0; i < vert->edges.size(); i++)
    {
        if(vert->edges[i]->length > max)
        {
            pos = i;
            max = vert->edges[i]->length;
        }
    }
    */

    //Get the first edge direction's tangent plane projection
    vec3 edgevReal = find_edge_vector(vert, 0);
    vec3 base1 = edgevReal - dot(edgevReal, n) * n;
    b1 = base1;

    //get the second base
    vec3 base2 = cross(base1, n);
    b2 = base2;

    //Loop through each edge
    for(int i = 0; i < vert->edges.size(); i++)
    {
        //Get the edge vector
        edgevReal = find_edge_vector(vert, i);
        vec3 edgev = edgevReal - dot(edgevReal, n) * n;

        /*** Using x and y to solve for a and b ***/

        double k = base1.y / base1.x;

        double q = (edgev.x * k - edgev.y) / (base2.x * k - base2.y);

        double p = (edgev.x - q * base2.x) / base1.x;

        //Check if pass the test
        /*
        vec3 resultv = (float)p * base1 + (float)q * base2;
        if(resultv != edgev)
        {
            qInfo("== Projection fail!\n");
            qInfo("== Result: %f,%f,%f Target: %f,%f,%f",
                  resultv.x, resultv.y, resultv.z,
                  edgev.x, edgev.y, edgev.z);
            result.clear();
            break;
        }
        */

        //qInfo("== Projection success!\n");

        //Save the 2D vector
        result.push_back(vec2(p, q));
    }

    return result;
}

void Polyhedron::gaussian_curvature()
{    
    qInfo("-- Start calculating gaussian curvature\n");

    //Loop through all the vertex
    for(int i = 0; i < vlist.size(); i++)
    {
        Vertex * vert = vlist[i];

        //Clear the pointer first
        vert->cInfo.Curvature = 0.0f;

        //Loop through the corner, finding sum of angle
        float totalA = 0.0f;
        for(int j = 0; j < vert->corners.size(); j++)
        {
            totalA += vert->corners[j]->angle;
        }

        vert->cInfo.Curvature = (2 * M_PI - totalA) / find_Amix(vert);

        //qInfo("%d\n", i);
    }
}

void Polyhedron::mean_curvature()
{
    qInfo("-- Start calculating mean curvature\n");

    //Loop through all the vertex
    for(int i = 0; i < vlist.size(); i++)
    {
        Vertex * vert = vlist[i];

        //Clear the pointer first
        vert->cInfo.Curvature = 0.0f;

        //Loop the edge list, calculate the toatal
        for(int j = 0; j < vert->edges.size(); j++)
        {
            //Calculate the cotangent
            Edge * targetE = vert->edges[j];

            float th1 = targetE->corners[0]->angle;
            float th2 = targetE->corners[1]->angle;

            float t1, t2;

            if(sin(th1 == 0))
                t1 = 1;
            else
                t1 = cos(th1) / sin(th1);

            if(sin(th2 == 0))
                t2 = 1;
            else
                t2 = cos(th2) / sin(th2);
            float w = (t1 + t2);

            //Calculate the normal curvature
            vec3 edgeV = iv2vec(targetE->verts[1]) - iv2vec(targetE->verts[0]);
            vec3 n = vec3(vert->normal.x, vert->normal.y, vert->normal.z);
            float edgeL = lengthOfvec3(edgeV);

            float kN = 2 * dot(edgeV, n) / edgeL;

            //Calculate the mean curvature
            float kH = w * edgeL * edgeL * kN / 4.0f;

            vert->cInfo.Curvature += kH / (2 * find_Amix(vert));
        }
    }
}

void Polyhedron::find_principal_curvature()
{
    //Loop through the vertex list
    for(int i = 0; i < vlist.size(); i++)
    {
        //Find the center

        Vertex * vert = vlist[i];

        //Get the normal
        vec3 n = vec3(vert->normal.x, vert->normal.y, vert->normal.z);

        //Get the projection, follow the edge order
        vec3 base1, base2;
        vector<vec2> projection = mapping_to_tangent_plane(vert, base1, base2);

        //Save the base
        vert->cInfo.base1 = base1;
        vert->cInfo.base2 = base2;

        //Get edge num
        int nedge = vert->edges.size();

        //Form the 2D vector matrix dim(nedge, 3)
        MatrixXf A(nedge, 3);

        //Form the 2D target
        VectorXf b(nedge);

        //Fill the matrix and vector
        for(int j = 0; j < nedge; j++)
        {
            //Matrix element
            A(j, 0) = projection[j].x * projection[j].x;
            A(j, 1) = 2 * projection[j].x * projection[j].y;
            A(j, 2) = projection[j].y * projection[j].y;

            //Vector element
            b(j) = find_normal_curvature(vert, find_another_point_on_edge(vert,j));
        }

        //Solve the least square system
        //JacobiSVD<MatrixXf> svd(A, ComputeThinU | ComputeThinV);
        //VectorXf x = svd.solve(b);
        VectorXf x = A.colPivHouseholderQr().solve(b);

        if(x.size() != 3)
            qInfo("== Matrix error!!!Ahhhhhh\n");

        //Form the curvature tensor
        MatrixXf T(2, 2);
        T(0, 0) = vert->cInfo.l = x(0);
        T(0, 1) =T(1, 0) = vert->cInfo.m = x(1);
        T(1, 1) = vert->cInfo.n = x(2);

        EigenSolver<MatrixXf> eigensolver(T);

        //Find two eigen value position
        int minPos = 0;
        int maxPos = 0;
        if(eigensolver.eigenvalues()[0].real() > eigensolver.eigenvalues()[1].real())
        {
            maxPos = 0;
            minPos = 1;
        }
        //else
        {
            minPos = 0;
            maxPos = 1;
        }

        //Pos is max curvature
        vert->cInfo.maxPrincipal = eigensolver.eigenvectors().col(maxPos).x().real() * base1 +
                 eigensolver.eigenvectors().col(maxPos).y().real() * base2;

        vert->cInfo.minPrincipal = eigensolver.eigenvectors().col(minPos).x().real() * base1 +
                 eigensolver.eigenvectors().col(minPos).y().real() * base2;
    }
}

void Polyhedron::map_curvature_tensor()
{
    //Loop through the vertex list to map the tensor field into R3
    for(int i = 0; i < vlist.size(); i++)
    {
        Vertex * vert = vlist[i];

        //Form the tensor matrix T, dim(3, 3)
        MatrixXf T = MatrixXf::Zero(3, 3);
        T(0, 0) = vert->cInfo.l;
        T(0, 1) = T(1, 0) = vert->cInfo.m;
        T(1, 1) = vert->cInfo.n;

        //Form the matrix mapping matrix M, dim(3, 3)
        MatrixXf Mt(3, 3);
        Mt(0, 0) = vert->cInfo.base1.x;
        Mt(0, 1) = vert->cInfo.base1.y;
        Mt(0, 2) = vert->cInfo.base1.z;
        Mt(1, 0) = vert->cInfo.base2.x;
        Mt(1, 1) = vert->cInfo.base2.y;
        Mt(1, 2) = vert->cInfo.base2.z;
        Mt(2, 0) = vert->normal.x;
        Mt(2, 1) = vert->normal.y;
        Mt(2, 2) = vert->normal.z;
        vert->cInfo.Mt = Mt;

        //Form the transpose matrix of M, Mt, dim(3, 3)
        MatrixXf M(3, 3);
        M = Mt.transpose();
        vert->cInfo.M = M;

        //Find the T global
        MatrixXf Tg;
        Tg = M * T * Mt;
        vert->cInfo.Tg = Tg;
    }
}

void Polyhedron::map_curvature_tensor_back()
{
    //Loop through the vertex list to map the tensor field into R3
    for(int i = 0; i < vlist.size(); i++)
    {
        Vertex * vert = vlist[i];

        //Get the smoothed Tg matrix
        MatrixXf Tg = vert->cInfo.Tg;

        Tg = vert->cInfo.Mt * Tg * vert->cInfo.M;

        vert->cInfo.l = Tg(0, 0);
        vert->cInfo.m = Tg(0, 1);
        vert->cInfo.n = Tg(1, 1);

        //Form the curvature tensor
        MatrixXf T(2, 2);
        T(0, 0) = vert->cInfo.l;
        T(0, 1) =T(1, 0) = vert->cInfo.m;
        T(1, 1) = vert->cInfo.n;

        vec3 base1 = vert->cInfo.base1;
        vec3 base2 = vert->cInfo.base2;

        EigenSolver<MatrixXf> eigensolver(T);

        //Find two eigen value position
        int minPos = 0;
        int maxPos = 0;
        if(eigensolver.eigenvalues()[0].real() > eigensolver.eigenvalues()[1].real())
        {
            maxPos = 0; minPos = 1;
        }
        else
        {
            minPos = 0; maxPos = 1;
        }

        //Pos is max curvature
        vert->cInfo.maxPrincipal = eigensolver.eigenvectors().col(maxPos).x().real() * base1 +
                 eigensolver.eigenvectors().col(maxPos).y().real() * base2;

        vert->cInfo.minPrincipal = eigensolver.eigenvectors().col(minPos).x().real() * base1 +
                 eigensolver.eigenvectors().col(minPos).y().real() * base2;
    }
}

void Polyhedron::curvature_tensor_smooth()
{
    //qInfo("== Curvature tensor smoothed!\n");

    float dt = 1;

    //Smooth the tensor
    for(int i = 0; i < vlist.size(); i++)
    {
        Vertex * vert = vlist[i];

        int mode = 4;

        //Calculate the weight base
        float wb = 0.0;
        for(int j = 0; j < vert->edges.size(); j++)
        {
            Vertex * target = find_another_point_on_edge(vert, j);
            wb += smooth_weight_calc(mode, vert, vert->edges[j]);
        }

        MatrixXf Tg = vert->cInfo.Tg;
        for(int j = 0; j < vert->edges.size(); j++)
        {
            Vertex * target = find_another_point_on_edge(vert, j);

            float w = smooth_weight_calc(mode, vert, vert->edges[j]);
            float weight = w / wb;

            Tg += weight * dt * (target->cInfo.Tg - vert->cInfo.Tg);
        }
        vert->cInfo.Tgtemp = Tg;
    }

    //Load up
    for(int i = 0; i < vlist.size(); i++)
    {
        vlist[i]->cInfo.Tg = vlist[i]->cInfo.Tgtemp;
    }
}

void Polyhedron::smooth_principal_curvature()
{
    map_curvature_tensor();

    int iteration = 30;
    for(int i = 0; i < iteration; i++)
    {
        //map_curvature_tensor();

        curvature_tensor_smooth();
        qInfo("== Principal smooth finish\n");

        //map_curvature_tensor_back();
    }

    map_curvature_tensor_back();
}

//After running the function, the 3 base, midpoint, and M, Mt will be computed
void Polyhedron::calc_face_base()
{
    //Loop through all the faces
    for(int i = 0; i < tlist.size(); i++)
    {
        //Get the triangle
        Triangle * tri = tlist[i];

        //Calculate the basepoint
        vec3 basePoint = (iv2vec(tri->verts[0]) + iv2vec(tri->verts[1])) / 2.0f;
        basePoint = (basePoint + iv2vec(tri->verts[2])) / 2.0f;
        tri->cInfo.basePoint = basePoint;

        //Get the first edge and normal
        vec3 base1 = normalize((iv2vec(tri->verts[0]) + iv2vec(tri->verts[1])) / 2.0f - basePoint);
        vec3 n = vec3(tri->normal.x, tri->normal.y, tri->normal.z);

        //Calc the second base
        vec3 base2 = cross(n, base1);

        tri->cInfo.base1 = base1;
        tri->cInfo.base2 = base2;
        tri->cInfo.normal = n;

        //Build up the matrix
        //Form the matrix mapping matrix M, dim(3, 3)
        MatrixXf Mt(3, 3);
        Mt(0, 0) = tri->cInfo.base1.x;
        Mt(0, 1) = tri->cInfo.base1.y;
        Mt(0, 2) = tri->cInfo.base1.z;
        Mt(1, 0) = tri->cInfo.base2.x;
        Mt(1, 1) = tri->cInfo.base2.y;
        Mt(1, 2) = tri->cInfo.base2.z;
        Mt(2, 0) = tri->normal.x;
        Mt(2, 1) = tri->normal.y;
        Mt(2, 2) = tri->normal.z;
        tri->cInfo.Mt = Mt;

        //Form the transpose matrix of M, Mt, dim(3, 3)
        MatrixXf M(3, 3);
        M = Mt.transpose();
        tri->cInfo.M = M;
    }
}

//After running the function there will be a 2*2 tensor matrix for each face
void Polyhedron::interpolate_face_cur_tensor()
{
    //Loop through all the faces
    for(int i = 0; i < tlist.size(); i++)
    {
        Triangle * tri = tlist[i];

        //Interpolate the tensor field
        tri->cInfo.Tg = MatrixXf::Zero(3, 3);
        //MatrixXf t = MatrixXf::Zero(3, 3);
        //tri->cInfo.maxPrincipal = vec3(0, 0, 0);
        //tri->cInfo.minPrincipal = vec3(0 ,0 ,0);
        for(int j = 0; j < 3; j++)
        {
            //Initialize the matrix, of current vertex
            //t(0, 0) = c[j]->v->cInfo.l;
            //t(1, 0) = t(0, 1) = c[j]->v->cInfo.m;
            //t(1, 1) = c[j]->v->cInfo.n;
            //t = c[j]->v->cInfo.M * t * c[j]->v->cInfo.Mt;

            //t = (w[j] / (w[0] + w[1] + w[2])) * t;
            //T += t;

            //Interpolate
            tri->cInfo.Tg += (tri->verts[j]->cInfo.Tg / 3.0f);
            //tri->cInfo.Tg += (w[j] / (w[0] + w[1] + w[2])) * c[j]->v->cInfo.Tg;
        }


        MatrixXf Tg = tri->cInfo.Mt * tri->cInfo.Tg * tri->cInfo.M;
        MatrixXf T = MatrixXf::Zero(2, 2);
        T(0, 0) = Tg(0, 0);
        T(0, 1) = Tg(0, 1);
        T(1, 0) = Tg(1, 0);
        T(1, 1) = Tg(1, 1);
        //qInfo("== Current T: %f, %f, %f, %f\n", T(0, 0), T(0, 1), T(1, 0), T(1, 1));
        //qInfo("== Current w: %f, %f, %f\n", w[0], w[1], w[2]);

        //Solve for face eigen
        EigenSolver<MatrixXf> eigensolver(T);

        //Find two eigen value position
        int minPos = 0;
        int maxPos = 0;
        if(eigensolver.eigenvalues()[0].real() > eigensolver.eigenvalues()[1].real())
        {
            maxPos = 0;
            minPos = 1;
        }
        else
        {
            minPos = 0;
            maxPos = 1;
        }

        //2D
        tri->cInfo.max2DPrincipal = vec2(eigensolver.eigenvectors().col(maxPos).x().real(),
                                         eigensolver.eigenvectors().col(maxPos).y().real());

        tri->cInfo.min2DPrincipal = vec2(eigensolver.eigenvectors().col(minPos).x().real(),
                                         eigensolver.eigenvectors().col(minPos).y().real());

        //3D
        tri->cInfo.maxPrincipal = eigensolver.eigenvectors().col(maxPos).x().real() * tri->cInfo.base1 +
                                         eigensolver.eigenvectors().col(maxPos).y().real() * tri->cInfo.base2;

        tri->cInfo.minPrincipal = eigensolver.eigenvectors().col(minPos).x().real() * tri->cInfo.base1 +
                                         eigensolver.eigenvectors().col(minPos).y().real() * tri->cInfo.base2;

    }
}

//Convert all the face vertex into fave plane's coordinate
void Polyhedron::transform_face_coord()
{
    for(int i = 0; i < tlist.size(); i++)
    {
        Triangle * tri = tlist[i];

        vec3 base1 = tri->cInfo.base1;
        vec3 base2 = tri->cInfo.base2;

        for(int j = 0; j < 3; j++)
        {
            Vertex * vert = tri->verts[j];

            //Calculate the relative position of the vertex
            vec3 edgev = iv2vec(vert) - tri->cInfo.basePoint;


            /*** Using x and y to solve for a and b ***/

            double k = base1.y / base1.x;

            double q = (edgev.x * k - edgev.y) / (base2.x * k - base2.y);
            if((base2.x * k - base2.y) == 0)
                q = 1;

            double p = (edgev.x - q * base2.x) / base1.x;

            //qInfo("== Projection success!\n");

            //Save the 2D vector
            tri->cInfo.v[j] = vec2(p, q);
        }
    }
}

float lengthOfvec3(vec3 t)
{
    return sqrt(t.x * t.x + t.y * t.y + t.z * t.z);
}

bool checkIfBetween(vec3 t, vec3 p1, vec3 p2)
{
    vec3 lint = t - p1;
    vec3 line = p2 - p1;
    //If same direction
    if(dot(lint, line) > 0)
    {
        //If shorter
        if(lengthOfvec3(lint) < lengthOfvec3(line))
            return true;
    }


    return false;
}

vec3 calcInterSection(vec3 p1, vec3 p2, vec3 p3, vec3 p4)
{
    //Calculate the intersection
    vec3 p13,p43,p21, pr;
    float d1343,d4321,d1321,d4343,d2121;
    float numer,denom;

    p13 = p1 - p3;
    p43 = p4 - p3;
    p21 = p2 - p1;

    d1343 = p13.x * p43.x + p13.y * p43.y + p13.z * p43.z;
    d4321 = p43.x * p21.x + p43.y * p21.y + p43.z * p21.z;
    d1321 = p13.x * p21.x + p13.y * p21.y + p13.z * p21.z;
    d4343 = p43.x * p43.x + p43.y * p43.y + p43.z * p43.z;
    d2121 = p21.x * p21.x + p21.y * p21.y + p21.z * p21.z;

    denom = d2121 * d4343 - d4321 * d4321;
    numer = d1343 * d4321 - d1321 * d4343;

    double mua = numer / denom;
    double mub = (d1343 + d4321 * (mua)) / d4343;

    pr.x = p3.x + mub * p43.x;
    pr.y = p3.y + mub * p43.y;
    pr.z = p3.z + mub * p43.z;

    return pr;
}

vector<vec3> Polyhedron::calc_stream_line(Triangle * start, int iteration, int mode, int direction)
{
    vector<vec3> data;

    //Change tensor everytime or just go
    int dIteration = 1;
    if(mode == 2)       //Unfold method
        dIteration = 5;

    Triangle * currentT;

    MatrixXf Tg;

    int iterate, distance;

    vec3 p1, p2, p3, p4, pInte, direct;

    for(int pn = 0; pn < 2; pn++)
    {

        iterate = 0;    //Total iteration of single line
        distance = 0;   //Go how far to change principal direction
        //MatrixXf T, Tin2;   //Curvature Tensor and its R2 version


        currentT = start;            //Current iteration's triangle

        p1 = currentT->cInfo.basePoint;    //Used to find intersection

        direct = currentT->cInfo.maxPrincipal;       //The principal curvature direction in 2D space

        bool conti = true;

        while((iterate < iteration) && conti)
        {
            /*** Current Triangle ***/

            //If need to get new principal direction
            if(distance == 0)
            {
                //Get the principal direction, check the direction
                //Make sure don't go back
                //if(dot(direct, currentT->cInfo.maxPrincipal) > 0)
                float diii = -1.0f;
                if(pn % 2 == 0)
                    diii = 1.0f;

                //Max or min?
                if(direction == 1)
                    direct = diii * normalize(currentT->cInfo.maxPrincipal);
                else if(direction == 2)
                    direct = diii * normalize(currentT->cInfo.minPrincipal);
                //else
                    //direct = -1.0f * currentT->cInfo.maxPrincipal;

                //Curvature tensor
                Tg = currentT->cInfo.Tg;
            }

            //Temporary use direct as end point
            p2 = p1 + direct;

            //Find towarding which edge
            vec3 towardE1 = ((iv2vec(currentT->verts[0]) + iv2vec(currentT->verts[1])) / 2.0f) - p1;
            vec3 towardE2 = ((iv2vec(currentT->verts[1]) + iv2vec(currentT->verts[2])) / 2.0f) - p1;
            vec3 towardE3 = ((iv2vec(currentT->verts[2]) + iv2vec(currentT->verts[0])) / 2.0f) - p1;

            float d1 = dot(normalize(towardE1), normalize(direct));
            float d2 = dot(normalize(towardE2), normalize(direct));
            float d3 = dot(normalize(towardE3), normalize(direct));
            //qInfo("== d: %f, %f, %f\n", d1, d2, d3);

            float max = d1;
            int pos[2] = {0, 1};
            if(d2 > max)
            {
                max = d2; pos[0] = 1; pos[1] = 2;
            }
            if(d3 > max)
            {
                max = d3; pos[0] = 2; pos[1] = 0;
            }

            //Get p3 and p4
            p3 = iv2vec(currentT->verts[pos[0]]); p4 = iv2vec(currentT->verts[pos[1]]);

            pInte = calcInterSection(p1, p2, p3, p4);

            if(!checkIfBetween(pInte, p3, p4))
            {
                conti = false;
                //pInte = dot(pInte - p3, p4 - p3) * (p4 - p3) + p3;
            }

            //Save the vec3
            vec3 cn = vec3(currentT->normal.x, currentT->normal.y, currentT->normal.z);

            if(conti)
            {
                data.push_back(p1); data.push_back(pInte);
            }

            //data.push_back(currentT->cInfo.basePoint - normalize(currentT->cInfo.maxPrincipal) * 0.03f + 0.01f * cn);
            //data.push_back(currentT->cInfo.basePoint + normalize(currentT->cInfo.maxPrincipal) * 0.03f + 0.01f * cn);

            //data.push_back(iv2vec(currentT->verts[0])); data.push_back(iv2vec(currentT->verts[1]));
            //data.push_back(iv2vec(currentT->verts[2])); data.push_back(iv2vec(currentT->verts[1]));
            //data.push_back(iv2vec(currentT->verts[0])); data.push_back(iv2vec(currentT->verts[2]));


            /*** Prepare for the next triangle ***/

            //Use current end point as start point and do the calc based on next triangle
            Vertex * currentV1 = currentT->verts[pos[0]];
            Vertex * currentV2 = currentT->verts[pos[1]];

            //Find next triangle
            Triangle * nextTri;
            for(int i = 0; i < 3; i++)
                if((currentT->edges[i]->verts[0] == currentV1 && currentT->edges[i]->verts[1] == currentV2) ||
                        (currentT->edges[i]->verts[0] == currentV2 && currentT->edges[i]->verts[1] == currentV1))
                    for(int j = 0; j < 2; j++)
                        if(currentT->edges[i]->tris[j] != currentT)
                            nextTri = currentT->edges[i]->tris[j];

            //Set the end point to be next initial point
            p1 = pInte;

            //If draw line a long way, need to convert the current tensor to next system
            if(mode == 2)
            {
                MatrixXf tg = nextTri->cInfo.Mt * Tg * nextTri->cInfo.M;
                MatrixXf T = MatrixXf::Zero(2, 2);
                T(0, 0) = tg(0, 0);
                T(0, 1) = tg(0, 1);
                T(1, 0) = tg(1, 0);
                T(1, 1) = tg(1, 1);
                //qInfo("== Current T: %f, %f, %f, %f\n", T(0, 0), T(0, 1), T(1, 0), T(1, 1));
                //qInfo("== Current w: %f, %f, %f\n", w[0], w[1], w[2]);

                //Solve for face eigen
                EigenSolver<MatrixXf> eigensolver(T);

                //Find two eigen value position
                int minPos = 0;
                int maxPos = 0;
                if(eigensolver.eigenvalues()[0].real() > eigensolver.eigenvalues()[1].real())
                {
                    maxPos = 0;
                    minPos = 1;
                }
                else
                {
                    minPos = 0;
                    maxPos = 1;
                }

                //Max or min?
                int epos = maxPos;
                if(direction == 2)
                    epos = minPos;

                direct = eigensolver.eigenvectors().col(epos).x().real() * nextTri->cInfo.base1 +
                        eigensolver.eigenvectors().col(epos).y().real() * nextTri->cInfo.base2;

                float diii = -1.0f;
                if(pn % 2 == 0)
                    diii = 1.0f;
                direct = diii * normalize(direct);
            }

            //Move to next triangle
            currentT = nextTri;

            //Track how long already been
            distance++;
            if(distance == dIteration)
                distance = 0;

            iterate++;
        }
    }

    return data;
}

void Polyhedron::spherical_parameterize()
{
    int iteration = 0;

    while(iteration < 500)
    {
        //qInfo("iteration: %d\n", iteration);
        int weightMode = 1;

        //Loop through the vertex list
        for(int i = 0; i < vlist.size(); i++)
        {
            Vertex * currentV = vlist[i];

            //Set the weight, to use Tutte Laplacian weight
            float w = 0.0f;
            for(int j = 0; j < currentV->edges.size(); j++)
            {
                w += smooth_weight_calc(weightMode, currentV, currentV->edges[j]);
            }

            //Loop through all its neighbours to solve for alpha
            float ai = 0;
            float Lwx = 0; float Lwy = 0; float Lwz = 0;
            for(int j = 0; j < currentV->edges.size(); j++)
            {
                Vertex * targetV = find_another_point_on_edge(currentV, j);

                float sw = smooth_weight_calc(weightMode, currentV, currentV->edges[j]) / w;

                //Sum up (Lw * xyz) ^ 2
                Lwx += (sw * targetV->x);
                Lwy += (sw * targetV->y);
                Lwz += (sw * targetV->z);
            }

            //Adding current V part for I - W's I
            //Lwx -= currentV->x;
            //Lwy -= currentV->y;
            //Lwz -= currentV->z;

            //Taking sqrt since its alpha square now
            //ai = sqrt((Lwx * Lwx + Lwy * Lwy + Lwz * Lwz) / 1);

            //Calculate new vertex position
            currentV->sflag.x = Lwx;
            currentV->sflag.y = Lwy;
            currentV->sflag.z = Lwz;
        }

        for(int i = 0; i < vlist.size(); i++)
        {
            Vertex * currentV = vlist[i];

            vec3 newPos = vec3(currentV->sflag.x, currentV->sflag.y, currentV->sflag.z);

            currentV->x = newPos.x;
            currentV->y = newPos.y;
            currentV->z = newPos.z;

            vec3 Pos = vec3(currentV->x, currentV->y, currentV->z);

            //Cast onto sphere
            float d = lengthOfvec3(Pos);
            Pos *= 1 / d;

            currentV->x = Pos.x;
            currentV->y = Pos.y;
            currentV->z = Pos.z;
        }

        if(weightMode > 2)
        {
            ctable.ClearTable();
            ctable.GenerateTable(this);
        }

        iteration++;
    }
}

void Polyhedron::check_vlist()
{
    for (int i = 0; i < vlist.size(); i++)
    {
        Vertex* v = vlist[i];

        //Check if there any over lapped triangle inside tris, remove them
        if (v->tris.size() != 0)
        {
            for (int j = 0; j < v->tris.size(); j++)
            {
                for (int k = 0; j < v->tris.size(); j++)
                {
                    if (v->tris[j] == v->tris[k] && j != k)
                    {
                        v->tris.erase(v->tris.begin() + k);
                        k = 0;
                    }
                }
            }

            v->ntris = v->tris.size();
        }
    }
}

Polyhedron::Polyhedron()
{
	nverts = nedges = nquads = 0;
	max_verts = max_quads = 50;
}

void Polyhedron::average_normals()
{
    int i, j;

    for (i = 0; i < nverts; i++)
    {
        vlist[i]->normal = icVector3(0.0);

        for (j = 0; j<vlist[i]->ntris; j++)
            vlist[i]->normal += vlist[i]->tris[j]->normal;
        normalize(vlist[i]->normal);
    }
}



























