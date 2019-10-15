//  CarbonMed3DRecon:  
//  An Interface class that include the 3D Rendering Object  
//  often call by other models as the interface  
//  Created by Chriswoo on 2017-08-02.
//  Contact: wsscx01@aliyun.com
//  
#ifndef _CARBONMED3DRECON_H_
#define _CARBONMED3DRECON_H_

#include <GL/glew.h>
#include <GL/freeglut.h>

#include <CL/cl.h>

#include <stdlib.h>
#include <stdio.h>
#include <memory.h>

#include <iostream>
#include <fstream>
#include <sstream>

#include "DataTypes.h"
#include "RenderBase.h"
#include "Window.h"
#include "Matrices.h"

/*#ifdef RENDERLIB_EXPORTS  
#define CARBONMED3DRECONCLASS_API __declspec(dllexport)  
#else  
#define CARBONMED3DRECONCLASS_API __declspec(dllimport)  
#endif  */

using namespace Win;
class  CCarbonMed3DRecon //CARBONMED3DRECONCLASS_API
{
public:
	CCarbonMed3DRecon() {}
	virtual ~CCarbonMed3DRecon();
	static int InitialFunctorTest(float *dicominfo, float *dicomdata, int framecount, HWND cwnd[3], int *_3dwindowinfo, int *slicewindowinfow, int *uswindowinfo);
	// initial Functor resource
	static int InitialFunctor();
	// release Functor resource
	static void ReleaseFunctor();
	// functor
	static void Functor(float *maskdata, float *maskinfo);
	static void RenderingInit(float *maskdata, float *maskinfo);
	static void Showing3D();
	static bool Get3DWindowActiveFlag();
	static void ShowingSlice();
	static bool GetSliceWindowActiveFlag();
	static void ShowingUltrasound();
	static bool GetUSWindowActiveFlag();
	static void grab(int index);
//C# UI Interface start
public:
	//UI Mouse Event
	static void UILeftButtonDown(float x, float y);
	//UI MouseMove Event
	static void UIMouseMoveInterface(float x, float y);
	//UI KeyBoard Event
	static void UIKeyEvent(int key);
	//UI Get slice
	static void UIGetSliceData(float *buf, int len);
	//UI Set Ultrasound
	static void UISetUltraSoundBuf(float *buf, int len);
	//UI Get NDI Result
	static void UIGetNDIResult(float &x, float &y, float &z, float &azimuth, float &elevation, float &roll);
	//UI Draw Result
	static void UIDrawOpenGLResult();
//C# UI Interface end

	//Registration
	static void InitRegRefPts(Point3f refpt1, Point3f refpt2, Point3f refpt3, Point3f refpt4);
	static void InitPunctureLine(float ang, float plength, float refdis, float radius, float axis[3], int ind);
	static void Registration();
	static void SetRegistrationPt(int step, int ind);
	//Waiting for the registration flag
	static void WaitingForRegFlag();

	// Rendering Object
	static RenderBase *render_obj;

	//data Interface
	//slice data
	static void GetVolumeSliceData(cl_mem slicedata);
	//slice data
	static void GetNormalizedSliceData(float *slicedata);
	//NDI data
	static void GetNDITrackerRecord(void *precord);
	//Registration coordinate info
	static void GetRegistrationCorInfo(float *rmat, float *tvec);

	//win window reshape
	static void reshapeWgl(int w, int h);
	//WGL CreateContext
	static bool WincreateContext(HWND handle, int colorBits, int depthBits, int stencilBits);
	static bool WincreateContextWithHandle(HWND handle, HDC &windc, HGLRC &winRc, int colorBits, int depthBits, int stencilBits);
	static void closeContext(HWND handle, HDC &windc, HGLRC &winRc);
	static void WinswapBuffers(HDC &windc);
	static void MakeCurrentWin(HDC &windc, HGLRC &winRc);
	// pixel Format
	static bool setPixelFormat(HDC hdc, int colorBits, int depthBits, int stencilBits);
	static int findPixelFormat(HDC hdc, int colorbits, int depthBits, int stencilBits); // return best matched format ID
    // InitModelGL
	static void InitModelGL();
protected:
	// Initial OpenCL  components
	static void OpenCLini();
	// Release OpenCL  components
	static void OpenCLRelease();
	// Initial OpenGL components
	static bool InitGL();
	static bool InitWGL(HWND cwnd[3], int *_3dwindowinfo, int *slicewindowinfo, int *uswindowinfo);
	// Initial Volume data;
	static void GetVolumeInfoFromFile(const char *filename, _DICOMInfo *dicomobj, int *slicenum);
	static void VolumeInit();
	static void SlicePBOInit();
	static void SliceInit();
	static void NdiDeviceInit();
	//opengl
	static void motion(int x, int y);
	static void reshape(int w, int h);

	static void mouse(int button, int state, int x, int y);
	static void keyboard(unsigned char key, int x, int y);
	static void idle();
	static void ImageRender();
	static void ImageRenderWgl();
	static void computeFPS();
	// Check error for CL kernel
	inline static void check_CL_error(cl_int error, char* error_messgae);
	// run time
	static double shrDeltaT(int iCounterID);
public:
	//3D Win
	static Win::Window *m_GlWin;
	//Slice Win
	static Win::Window *m_GlSliceWin;
	//Ultrasound Win
	static Win::Window *m_GlUltrasoundWin;
	static BOOL winactiveflag;
	static BOOL slicewinactiveflag;
	static BOOL uswinactiveflag;
public:
	//Window Event
	static int command(int id, int cmd, LPARAM msg);   // for WM_COMMAND
	static int destroy();                                // close the RC and destroy OpenGL window
	static int paint();
	static int keyDown(int key, LPARAM lParam);
	static int keyUp(int key, LPARAM lParam);
	static int lButtonDown(WPARAM state, int x, int y);
	static int lButtonUp(WPARAM state, int x, int y);
	static int rButtonDown(WPARAM state, int x, int y);
	static int rButtonUp(WPARAM state, int x, int y);
	static int mButtonDown(WPARAM wParam, int x, int y);
	static int mButtonUp(WPARAM wParam, int x, int y);
	static int mouseMove(WPARAM state, int x, int y);
	static int mouseWheel(int state, int delta, int x, int y); // for WM_MOUSEWHEEL:state, delta, x, y
	static int size(int w, int h, WPARAM wParam);      // for WM_SIZE: width, height, type(SIZE_MAXIMIZED...)
public:
	//OpenGL position param
	static void setCamera(float posX, float posY, float posZ, float targetX, float targetY, float targetZ);

	static void setMousePosition(int x, int y);
	static void setWindowSize(int width, int height);
	static void setViewMatrix(float x, float y, float z, float pitch, float heading, float roll);
	static void setModelMatrix(float x, float y, float z, float rx, float ry, float rz);

	static void setCameraX(float x);
	static void setCameraY(float y);
	static void setCameraZ(float z);
	static void setCameraAngleX(float p);
	static void setCameraAngleY(float h);
	static void setCameraAngleZ(float r);
	static float getCameraX();
	static float getCameraY();
	static float getCameraZ();
	static float getCameraAngleX();
	static float getCameraAngleY();
	static float getCameraAngleZ();

	static void setModelX(float x);
	static void setModelY(float y);
	static void setModelZ(float z);
	static void setModelAngleX(float a);
	static void setModelAngleY(float a);
	static void setModelAngleZ(float a);
	static float getModelX();
	static float getModelY();
	static float getModelZ();
	static float getModelAngleX();
	static float getModelAngleY();
	static float getModelAngleZ();

	// return 16 elements of  target matrix
	static const float* getViewMatrixElements();
	static const float* getModelMatrixElements();
	static const float* getModelViewMatrixElements();
	static const float* getProjectionMatrixElements();

	static void rotateCamera(int x, int y);
	static void zoomCamera(int dist);
	static void zoomCameraDelta(int delta);        // for mousewheel
	static void ZoomScale(int flag);

protected:
	static int ReadInteger(LPCTSTR szSection, LPCTSTR szKey, int iDefaultValue);
	static float ReadFloat(LPCTSTR szSection, LPCTSTR szKey, float fltDefaultValue);
	static bool ReadBoolean(LPCTSTR szSection, LPCTSTR szKey, bool bolDefaultValue);
	static string ReadString(LPCTSTR szSection, LPCTSTR szKey, LPCTSTR szDefaultValue);
	static void ReadArray(LPCTSTR szSection, LPCTSTR szKey, float *resultArray, int alen);
	static string TCHAR2STRING(TCHAR *STR);

private:
	static void draw(int winwidth, int winheight);
	static void drawSlice(int slicewinwidth, int slicewinheight);
	static void RenderWinInfo(int width, int height);
	static void drawString(const char* str, int len, GLfloat strcolor[4], float drawstartx, float drawstarty, float drawstartz);
	static void setViewport(int x, int y, int width, int height);
	static void setViewportSub(int left, int bottom, int width, int height, float nearPlane, float farPlane);
	static void drawGrid(float size, float step);          // draw a grid on XZ plane
	static void drawAxis(float size);                      // draw 3 axis
	static void drawAxisVBO(float size);
	static void drawAxisInterface();
	static void draw3DReconstruction(int winwidth, int winheight);
	//slice window
	static void drawSliceImage();                                                        
	static void setFrustum(float l, float r, float b, float t, float n, float f);
	static void setFrustum(float fovy, float ratio, float n, float f);
	static void setOrthoFrustum(float l, float r, float b, float t, float n = -1, float f = 1);
	static void updateModelMatrix();
	static void updateViewMatrix();
	static void animation();

private:
	static TCHAR m_ConfigFileName[255];
	static const char m_RegistrationDirectionPath[255];

	//devices
	static bool _NDIDeviceConnect;

	//OpenGL win
	static int m_WinWidth;
	static int m_WinHeight;
	static int m_SliceWinW;
	static int m_SliceWinH;
	static int m_UltrasoundWinW;
	static int m_UltrasoundWinH;

	// global sizes and local sizes
	static size_t _GlobalWorkSize[2];
	static size_t _LocalWorkSize[2];

	// Functor config info
	static double a;
	static double b;

	// cl variables
	static cl_int error;
	static cl_uint num;

	// Data on GPU
	static cl_mem d_ImagIn;
	static cl_mem d_ImagOut;

	static string m_Graphicardinfo;

	//CT Info
	static float _Windowcenter;
	static float _Windowwidth;
	static float _Pixelspacex;
	static float _Pixelspacey;
	static float _UpperLeft_X;
	static float _UpperLeft_Y;
	static float _UpperLeft_Z[MAXSLICENUM];
	static int _SliceNum;
	string m_RenderMethod;

	//texture order flag
	static int m_TextureOrderFlag;

	//openGL
	static bool OpenGLWinFlag;
	static double curxpos;
	static double curypos;

	static double refxpos;
	static double refypos;

	static double offsetx;
	static double offsety;
	static double totalTime;
	static float isoValue;
	static int mouse_buttons;

	static bool wireframe;
	static bool animate;
	static bool lighting;
	static bool render;
	static bool compute;
	static UINT32 frameCount;
	static UINT32 fpsLimit;
	static cl_bool g_glInterop;
	static int TotalVerts;

	static cl_float clrotate[4];
	static cl_float translate[4];
	//GL Matrix
	// 4x4 transform matrices
	static Matrix4 matrixView;
	static Matrix4 matrixModel;
	static Matrix4 matrixModelView;
	static Matrix4 matrixProjection;

	//ModelGL param
	static float cameraPosition[3];
	static float cameraAngle[3];
	static float modelPosition[3];
	static float modelAngle[3];

	// these are for 3rd person view
	static float cameraAngleX;
	static float cameraAngleY;
	static float cameraDistance;
	static float bgColor[4];

	//Zoom Aspect
	static float m_ZoomAspect;

	//window extra info
	static bool m_IsGetFrameRate;
	static bool m_IsGetErrorRate;
	static string strFrameRate;
	static string strErrorRate;
};


#endif//_CARBONMED3DRECON_H_
