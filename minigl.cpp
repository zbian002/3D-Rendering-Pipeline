/**
 * minigl.cpp
 * -------------------------------
 * Implement miniGL here.
 *
 * You may include minigl.h and any of the standard C++ libraries.
 * No other includes are permitted.  Other preprocessing directives
 * are also not permitted.  These requirements are strictly
 * enforced.  Be sure to run a test grading to make sure your file
 * passes the sanity tests.
 *
 * The behavior of the routines your are implenting is documented here:
 * https://www.opengl.org/sdk/docs/man2/
 * Note that you will only be implementing a subset of this.  In particular,
 * you only need to implement enough to pass the tests in the suite.
 */

#include "minigl.h"
#include "vec.h"
#include "mat.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <vector>
#include <cstdio>

using namespace std;

/**
 * Useful data types
 */
typedef mat<MGLfloat,4,4> mat4; //data structure storing a 4x4 matrix, see mat.h
typedef mat<MGLfloat,3,3> mat3; //data structure storing a 3x3 matrix, see mat.h
typedef vec<MGLfloat,4> vec4;   //data structure storing a 4 dimensional vector, see vec.h
typedef vec<MGLfloat,3> vec3;   //data structure storing a 3 dimensional vector, see vec.h
typedef vec<MGLfloat,2> vec2;   //data structure storing a 2 dimensional vector, see vec.h

struct Vertex {
    vec4 position;
    vec3 color;
};

struct Triangle {
    Vertex A, B, C;
};

vector<Vertex> vertices;
vector<Triangle> triangles;
vec3 cur_color;
MGLpoly_mode type;
MGLmatrix_mode matrix_mode;
vector<mat4> ModelMatrices(1, {{1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1}});
vector<mat4> ProjMatrices(1, {{1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1}});
vector<float> zBuffer;

float area(float ax, float ay, float bx, float by, float cx, float cy) {
    return ax * (by - cy) + ay * (cx - bx) + (bx * cy - by * cx);
}

float max(float a, float b, float c) {
    float max;
    max = (a > b) ? a : b;
    max = (max > c) ? max : c;
    return max;
}

float min(float a, float b, float c) {
    float min;
    min = (a < b) ? a : b;
    min = (min < c) ? min : c;
    return min;
}

void Rasterize_Triangle(const Triangle& tri, int width, int height, MGLpixel* data) {
    float ax = ((tri.A.position[0] / tri.A.position[3] + 1) * width / 2) - 0.5;
    float ay = ((tri.A.position[1] / tri.A.position[3] + 1) * height / 2) - 0.5;
    float bx = ((tri.B.position[0] / tri.B.position[3] + 1) * width / 2) - 0.5;
    float by = ((tri.B.position[1] / tri.B.position[3] + 1) * height / 2) - 0.5;
    float cx = ((tri.C.position[0] / tri.C.position[3] + 1) * width / 2) - 0.5;
    float cy = ((tri.C.position[1] / tri.C.position[3] + 1) * height / 2) - 0.5;
    float min_x = min(max(min(ax, bx, cx), (float)0), (float)(width - 1));
    float min_y = min(max(min(ay, by, cy), (float)0), (float)(height - 1));
    float max_x = min(max(max(ax, bx, cx), (float)0), (float)(width - 1));
    float max_y = min(max(max(ay, by, cy), (float)0), (float)(height - 1));
    float abc_area = area(ax, ay, bx, by, cx, cy);
    for(int i = min_x; i <= max_x; ++i) {
        for(int j = min_y; j <= max_y; ++j) {
            float pbc_area = area(i, j, bx, by, cx, cy);
            float apc_area = area(ax, ay, i, j, cx, cy);
            float abp_area = area(ax, ay, bx, by, i, j);
            float alpha = pbc_area / abc_area;
            float beta = apc_area / abc_area;
            float gamma = abp_area / abc_area;
            if(alpha >= 0 && beta >= 0 && gamma >= 0) {
                float newz = alpha * tri.A.position[2] / abs(tri.A.position[3]) + beta * tri.B.position[2] / abs(tri.B.position[3]) + gamma * tri.C.position[2] / abs(tri.C.position[3]);
                if(newz < zBuffer.at(i + j * width) && newz > -1 && newz < 1) {
                    float k = alpha / tri.A.position[3] + beta / tri.B.position[3] + gamma / tri.C.position[3];
                    float new_alpha = alpha / (tri.A.position[3] * k);
                    float new_beta = beta / (tri.B.position[3] * k);
                    float new_gamma = gamma / (tri.C.position[3] * k);
                    float first = new_alpha * 255 * tri.A.color[0] + new_beta * 255 * tri.B.color[0] + new_gamma * 255 * tri.C.color[0];
                    float second = new_alpha * 255 * tri.A.color[1] + new_beta * 255 * tri.B.color[1] + new_gamma * 255 * tri.C.color[1];
                    float third = new_alpha * 255 * tri.A.color[2] + new_beta * 255 * tri.B.color[2] + new_gamma * 255 * tri.C.color[2];
                    data[i + j * width] = Make_Pixel(first, second, third);
                    zBuffer.at(i + j * width) = newz;
                }
            }
        }
    }
}

/**
 * Standard macro to report errors
 */
inline void MGL_ERROR(const char* description) {
    printf("%s\n", description);
    exit(1);
}

/**
 * Read pixel data starting with the pixel at coordinates
 * (0, 0), up to (width,  height), into the array
 * pointed to by data.  The boundaries are lower-inclusive,
 * that is, a call with width = height = 1 would just read
 * the pixel at (0, 0).
 *
 * Rasterization and z-buffering should be performed when
 * this function is called, so that the data array is filled
 * with the actual pixel values that should be displayed on
 * the two-dimensional screen.
 */
void mglReadPixels(MGLsize width,
                   MGLsize height,
                   MGLpixel *data)
{
    zBuffer.assign(width * height, 2.0);
    for(unsigned i = 0; i < width; ++i) {
        for(unsigned j = 0; j < height; ++j) 
            data[i + j * width] = Make_Pixel(0, 0, 0);
    }
    for(unsigned i = 0; i < triangles.size(); i++)
        Rasterize_Triangle(triangles.at(i), width, height, data);
    triangles.clear();
}

/**
 * Start specifying the vertices for a group of primitives,
 * whose type is specified by the given mode.
 */
void mglBegin(MGLpoly_mode mode)
{
    type = mode;
}

/**
 * Stop specifying the vertices for a group of primitives.
 */
void mglEnd()
{
    if(type == MGL_TRIANGLES) {
        for(unsigned i = 0; i < vertices.size(); i += 3) {
            Triangle t;
            t.A = vertices.at(i);
            t.B = vertices.at(i + 1);
            t.C = vertices.at(i + 2);
            triangles.push_back(t);
        }
    }
    else if(type == MGL_QUADS){
        for(unsigned i = 0; i < vertices.size(); i += 4) {
            Triangle t1, t2;
            t1.A = vertices.at(i);
            t1.B = vertices.at(i + 1);
            t1.C = vertices.at(i + 2);
            t2.A = vertices.at(i);
            t2.B = vertices.at(i + 2);
            t2.C = vertices.at(i + 3);
            triangles.push_back(t1);
            triangles.push_back(t2);
        }
    }
    vertices.clear();
}

/**
 * Specify a two-dimensional vertex; the x- and y-coordinates
 * are explicitly specified, while the z-coordinate is assumed
 * to be zero.  Must appear between calls to mglBegin() and
 * mglEnd().
 */
void mglVertex2(MGLfloat x,
                MGLfloat y)
{
    mglVertex3(x, y, 0);
}

/**
 * Specify a three-dimensional vertex.  Must appear between
 * calls to mglBegin() and mglEnd().
 */
void mglVertex3(MGLfloat x,
                MGLfloat y,
                MGLfloat z)
{
    Vertex vertex;
    vertex.position = ProjMatrices.back() * ModelMatrices.back() * vec4(x, y, z, 1);
    vertex.color = cur_color;
    vertices.push_back(vertex);
}

/**
 * Set the current matrix mode (modelview or projection).
 */
void mglMatrixMode(MGLmatrix_mode mode)
{
    matrix_mode = mode;
}

/**
 * Push a copy of the current matrix onto the stack for the
 * current matrix mode.
 */
void mglPushMatrix()
{
    if(matrix_mode == MGL_MODELVIEW && !ModelMatrices.empty())
		ModelMatrices.push_back(ModelMatrices.back());
	else if(matrix_mode == MGL_PROJECTION && !ProjMatrices.empty())
		ProjMatrices.push_back(ProjMatrices.back());
}

/**
 * Pop the top matrix from the stack for the current matrix
 * mode.
 */
void mglPopMatrix()
{
    if(matrix_mode == MGL_MODELVIEW && !ModelMatrices.empty()) {
		if(!ModelMatrices.empty())
			ModelMatrices.pop_back();
	}
	else if(matrix_mode == MGL_PROJECTION && !ProjMatrices.empty()) {
		if(!ProjMatrices.empty())
			ProjMatrices.pop_back();
	}
}

/**
 * Replace the current matrix with the identity.
 */
void mglLoadIdentity()
{
    if(matrix_mode == MGL_MODELVIEW && !ModelMatrices.empty())
		ModelMatrices.back() = {{1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1}};
	else if(matrix_mode == MGL_PROJECTION && !ProjMatrices.empty())
	    ProjMatrices.back() = {{1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1}};
}

/**
 * Replace the current matrix with an arbitrary 4x4 matrix,
 * specified in column-major order.  That is, the matrix
 * is stored as:
 *
 *   ( a0  a4  a8  a12 )
 *   ( a1  a5  a9  a13 )
 *   ( a2  a6  a10 a14 )
 *   ( a3  a7  a11 a15 )
 *
 * where ai is the i'th entry of the array.
 */
void mglLoadMatrix(const MGLfloat *matrix)
{
	mat4 tmp = {{matrix[0], matrix[1], matrix[2], matrix[3], matrix[4], matrix[5], matrix[6], matrix[7], matrix[8], matrix[9], matrix[10], matrix[11], matrix[12], matrix[13], matrix[14], matrix[15]}};
	if(matrix_mode == MGL_MODELVIEW && !ModelMatrices.empty())
		ModelMatrices.back() = ModelMatrices.back() * tmp;
	else if(matrix_mode == MGL_PROJECTION && !ProjMatrices.empty())
		ProjMatrices.back() = ProjMatrices.back() * tmp;
}

/**
 * Multiply the current matrix by an arbitrary 4x4 matrix,
 * specified in column-major order.  That is, the matrix
 * is stored as:
 *
 *   ( a0  a4  a8  a12 )
 *   ( a1  a5  a9  a13 )
 *   ( a2  a6  a10 a14 )
 *   ( a3  a7  a11 a15 )
 *
 * where ai is the i'th entry of the array.
 */
void mglMultMatrix(const MGLfloat *matrix)
{
    mat4 tmp = {{matrix[0], matrix[1], matrix[2], matrix[3], matrix[4], matrix[5], matrix[6], matrix[7], matrix[8], matrix[9], matrix[10], matrix[11], matrix[12], matrix[13], matrix[14], matrix[15]}};
    if(matrix_mode == MGL_MODELVIEW && !ModelMatrices.empty())
		ModelMatrices.back() = ModelMatrices.back() * tmp;
	else if(matrix_mode == MGL_PROJECTION && !ProjMatrices.empty())
		ProjMatrices.back() = ProjMatrices.back() * tmp;
}

/**
 * Multiply the current matrix by the translation matrix
 * for the translation vector given by (x, y, z).
 */
void mglTranslate(MGLfloat x,
                  MGLfloat y,
                  MGLfloat z)
{
    mat4 tmp = {{1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, x, y, z, 1}};
    if(matrix_mode == MGL_MODELVIEW && !ModelMatrices.empty())
		ModelMatrices.back() = ModelMatrices.back() * tmp;
	else if(matrix_mode == MGL_PROJECTION && !ProjMatrices.empty())
		ProjMatrices.back() = ProjMatrices.back() * tmp;
}

/**
 * Multiply the current matrix by the rotation matrix
 * for a rotation of (angle) degrees about the vector
 * from the origin to the point (x, y, z).
 */
void mglRotate(MGLfloat angle,
               MGLfloat x,
               MGLfloat y,
               MGLfloat z)
{
    vec3 vetmp = (vec3(x, y, z)).normalized();
    MGLfloat s = sin(angle * M_PI / 180);
	MGLfloat c = cos(angle * M_PI / 180);	
    mat4 tmp = {{vetmp[0] * vetmp[0] * (1 - c) + c, vetmp[1] * vetmp[0] * (1 - c) + vetmp[2] * s, vetmp[0] * vetmp[2] * (1 - c) - vetmp[1] * s, 0, vetmp[0] * vetmp[1] * (1 - c) - vetmp[2] * s, vetmp[1] * vetmp[1] * (1 - c) + c, vetmp[1] * vetmp[2] * (1 - c) + vetmp[0] * s, 0, vetmp[0] * vetmp[2] * (1 - c) + vetmp[1] * s, vetmp[1] * vetmp[2] * (1 - c) - vetmp[0] * s, vetmp[2] * vetmp[2] * (1 - c) + c, 0, 0, 0, 0, 1}};
    if(matrix_mode == MGL_MODELVIEW && !ModelMatrices.empty())
		ModelMatrices.back() = ModelMatrices.back() * tmp;
	else if(matrix_mode == MGL_PROJECTION && !ProjMatrices.empty())
		ProjMatrices.back() = ProjMatrices.back() * tmp;
}

/**
 * Multiply the current matrix by the scale matrix
 * for the given scale factors.
 */
void mglScale(MGLfloat x,
              MGLfloat y,
              MGLfloat z)
{
    mat4 tmp = {{x, 0, 0, 0, 0, y, 0, 0, 0, 0, z, 0, 0, 0, 0, 1}};
    if(matrix_mode == MGL_MODELVIEW && !ModelMatrices.empty())
		ModelMatrices.back() = ModelMatrices.back() * tmp;
	else if(matrix_mode == MGL_PROJECTION && !ProjMatrices.empty())
		ProjMatrices.back() = ProjMatrices.back() * tmp;
}

/**
 * Multiply the current matrix by the perspective matrix
 * with the given clipping plane coordinates.
 */
void mglFrustum(MGLfloat left,
                MGLfloat right,
                MGLfloat bottom,
                MGLfloat top,
                MGLfloat near,
                MGLfloat far)
{
    mat4 tmp = {{2 * near / (right - left), 0, 0, 0, 0, 2 * near / (top - bottom), 0, 0, (right + left) / (right - left), (top + bottom) / (top - bottom), - (far + near) / (far - near), -1, 0, 0, - 2 * far * near / (far - near), 0}};
    if(matrix_mode == MGL_MODELVIEW && !ModelMatrices.empty())
		ModelMatrices.back() = tmp * ModelMatrices.back();
	else if(matrix_mode == MGL_PROJECTION && !ProjMatrices.empty())
		ProjMatrices.back() = tmp * ProjMatrices.back();
}

/**
 * Multiply the current matrix by the orthographic matrix
 * with the given clipping plane coordinates.
 */
void mglOrtho(MGLfloat left,
              MGLfloat right,
              MGLfloat bottom,
              MGLfloat top,
              MGLfloat near,
              MGLfloat far)
{
    mat4 tmp = {{2 / (right - left), 0, 0, 0, 0, 2 / (top - bottom), 0, 0, 0, 0, - 2 / (far - near), 0, - (right + left) / (right - left), - (top + bottom) / (top - bottom), - (far + near) / (far - near), 1}};
    if(matrix_mode == MGL_MODELVIEW && !ModelMatrices.empty())
		ModelMatrices.back() = tmp * ModelMatrices.back();
	else if(matrix_mode == MGL_PROJECTION && !ProjMatrices.empty())
		ProjMatrices.back() = tmp * ProjMatrices.back();
}

/**
 * Set the current color for drawn shapes.
 */
void mglColor(MGLfloat red,
              MGLfloat green,
              MGLfloat blue)
{
    cur_color = vec3(red, green, blue);
}
