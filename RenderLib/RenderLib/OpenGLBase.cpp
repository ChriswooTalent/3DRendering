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
			glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
			glVertex3f(conetop[0], conetop[1], conetop[2]);
			glVertex3f(conecircle[i * 3], conecircle[i * 3 + 1], conecircle[i * 3 + 2]);
		}
		glEnd();
	}

	void DrawingPuncNeedle(GLfloat *topvertpt, GLfloat *bottomvertpt, GLfloat cylindercenter[3], GLfloat *conecircle, GLfloat conetop[3], int Slice)
	{
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		GLfloat light_ambient[] = { 0.0, 0.0, 0.0, 1.0 };
		GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
		GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
		GLfloat light_position0[] = { 100.0, 100.0, 100.0, 0 };
		glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient); //环境光
		glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse); //漫射光
		glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular); //镜面反射
		glLightfv(GL_LIGHT0, GL_POSITION, light_position0); //光照位置
		GLfloat mat_ambient[] = { 0.192250, 0.192250, 0.192250, 1.000000, };
		GLfloat mat_diffuse[] = { 0.507540, 0.507540, 0.507540, 1.000000 };
		GLfloat mat_specular[] = { 0.508273, 0.508273, 0.508273, 1.000000 };
		GLfloat mat_shininess[] = { 51.200001 }; //材质RGBA镜面指数，数值在0～128范围内

		/*GLfloat mat_ambient[] = { 0.231250, 0.231250, 0.231250, 1.000000 };
		GLfloat mat_diffuse[] = { 0.277500, 0.277500, 0.277500, 1.000000 };
		GLfloat mat_specular[] = { 0.773911, 0.773911, 0.773911, 1.000000 };
		GLfloat mat_shininess[] = { 89.599998 }; //材质RGBA镜面指数，数值在0～128范围内*/


		glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
		glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
		glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
		glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
		glEnable(GL_LIGHTING); //启动光照
		glEnable(GL_LIGHT0); //使第一盏灯有效
		DrawingCylinderDirectly(topvertpt, bottomvertpt, cylindercenter, Slice);
		DrawingConeDirectly(conecircle, conetop, Slice);
		glDisable(GL_LIGHTING);
		glDisable(GL_LIGHT0); //使第一盏灯有效
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