#ifndef _OPENGL_BASE_H_
#define _OPENGL_BASE_H_
#include "stdafx.h"
#include <GL/glew.h>
#include <GL/freeglut.h>

namespace openglRendering
{
	extern "C" void DrawingCylinder(GLfloat topvertpt[3], GLfloat bottomvertpt[3], GLfloat puncvector[3], float radius, float halfLength, int slices);
	extern "C" void DrawingCylinderDirectly(GLfloat *topvertpt, GLfloat *bottomvertpt, GLfloat cylindercenter[3], int Slice);
	extern "C" void DrawingCircle();
	extern "C" void DrawingCone(GLfloat vertpt[3], GLfloat colorvec[4], GLfloat angle, GLfloat slopelen, GLuint axisflag);
	extern "C" void DrawingConeDirectly(GLfloat *conecircle, GLfloat conetop[3], int Slice);
	extern "C" void DrawingPuncNeedle(GLfloat *topvertpt, GLfloat *bottomvertpt, GLfloat cylindercenter[3], GLfloat *conecircle, GLfloat conetop[3], int Slice);
}
#endif // !_OPENGL_BASE_H_
