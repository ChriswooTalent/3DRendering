#include "OpenGLBase.h"

namespace openglRendering
{
	void DrawingCylinder(GLfloat topvertpt[3], GLfloat bottomvertpt[3], GLfloat puncvector[3], float radius, float halfLength, int slices)
	{
		GLfloat middlevertpt[3] = { 0.0f };
		middlevertpt[0] = (topvertpt[0] + bottomvertpt[0]) / 2.0f;
		middlevertpt[1] = (topvertpt[1] + bottomvertpt[1]) / 2.0f;
		middlevertpt[2] = (topvertpt[2] + bottomvertpt[2]) / 2.0f;
		for (int i = 0; i < slices; i++)
		{
			float theta = ((float)i)*2.0*PI;
			float nextTheta = ((float)i + 1)*2.0*PI;
			glBegin(GL_TRIANGLE_STRIP);
			/*vertex at middle of end */ 
			glVertex3f(topvertpt[0], topvertpt[1], topvertpt[2]);
			/*vertices at edges of circle*/ 
			glVertex3f(radius*cos(theta), halfLength, radius*sin(theta));
			glVertex3f(radius*cos(nextTheta), halfLength, radius*sin(nextTheta));
			/* the same vertices at the bottom of the cylinder*/
			glVertex3f(radius*cos(nextTheta), -halfLength, radius*sin(nextTheta));
			glVertex3f(radius*cos(theta), -halfLength, radius*sin(theta));
			glVertex3f(bottomvertpt[0], bottomvertpt[1], bottomvertpt[2]);
			glEnd();
		}
	}

	void DrawingCylinderDirectly(GLfloat *topvertpt, GLfloat *bottomvertpt, GLfloat cylindercenter[3],int Slice)
	{
		int i = 0;
		for (i = 0; i < Slice-1; i++)
		{
			glBegin(GL_TRIANGLE_STRIP);
			glVertex3f(topvertpt[i * 3], topvertpt[i * 3 + 1], topvertpt[i * 3 + 2]);
			glVertex3f(bottomvertpt[i * 3], bottomvertpt[i * 3 + 1], bottomvertpt[i * 3 + 2]);
			glVertex3f(cylindercenter[0], cylindercenter[1], cylindercenter[2]);

			glVertex3f(topvertpt[(i + 1) * 3], topvertpt[(i + 1) * 3 + 1], topvertpt[(i + 1) * 3 + 2]);	
			glVertex3f(bottomvertpt[(i + 1) * 3], bottomvertpt[(i + 1) * 3 + 1], bottomvertpt[(i + 1) * 3 + 2]);
			glVertex3f(cylindercenter[0], cylindercenter[1], cylindercenter[2]);
			glEnd();
		}
	}

	void DrawingConeDirectly(GLfloat *conecircle, GLfloat conetop[3], int Slice)
	{
		glBegin(GL_QUAD_STRIP);//连续填充四边形串
		int i = 0;
		for (i = 0; i < Slice - 1; i++)
		{
			glVertex3f(conetop[0], conetop[1], conetop[2]);
			glVertex3f(conecircle[i * 3], conecircle[i * 3 + 1], conecircle[i * 3 + 2]);
		}
		glEnd();
	}

	void DrawingPuncNeedle(GLfloat *topvertpt, GLfloat *bottomvertpt, GLfloat cylindercenter[3], GLfloat *conecircle, GLfloat conetop[3], int Slice)
	{
		DrawingCylinderDirectly(topvertpt, bottomvertpt, cylindercenter, Slice);
		DrawingConeDirectly(conecircle, conetop, Slice);
	}

	void DrawingCircle()
	{
		glBegin(GL_TRIANGLE_FAN);//扇形连续填充三角形串  
		glVertex3f(0.0f, 0.0f, 0.0f);
		int i = 0;
		for (i = 0; i <= 390; i += 15)
		{
			float p = i * 3.14 / 180;
			glVertex3f(sin(p), cos(p), 0.0f);
		}
		glEnd();
	}

	void DrawingCone(GLfloat vertpt[3], GLfloat colorvec[4], GLfloat angle, GLfloat slopelen, GLuint axisflag)
	{
		glBegin(GL_QUAD_STRIP);//连续填充四边形串  
		int i = 0;
		for (i = 0; i <= 720; i += 6)
		{
			float p = i * PI / 180;
			glColor4f(colorvec[0], colorvec[1], colorvec[2], colorvec[3]);
			glVertex3f(vertpt[0], vertpt[1], vertpt[2]);
			if (axisflag == 0)
			{
				glVertex3f(vertpt[0] - slopelen*cos(angle), slopelen*sin(angle)*sin(p), slopelen*sin(angle)*cos(p));
			}
			if (axisflag == 1)
			{
				glVertex3f(slopelen*sin(angle)*sin(p), vertpt[1] - slopelen*cos(angle), slopelen*sin(angle)*cos(p));
			}
			if (axisflag == 2)
			{
				glVertex3f(slopelen*sin(angle)*sin(p), slopelen*sin(angle)*cos(p), vertpt[2] - slopelen*cos(angle));
			}
		}
		glEnd();
	}
}