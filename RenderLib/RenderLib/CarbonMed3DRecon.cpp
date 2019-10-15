//  CarbonMed3DRecon:  
//  An Interface class that include the 3D Rendering Object  
//  often call by other models as the interface  
//  Created by Chriswoo on 2017-08-02.
//  Contact: wsscx01@aliyun.com
//  
#include "CarbonMed3DRecon.h"
#include "MarchingCube.h"
#include "VolumeRendering.h"
#include "OpenGLBase.h"
#include "ImageBasicProcess.h"

using namespace std;
using namespace openglRendering;

extern "C" __declspec(dllimport) void OpenLog();
extern "C" __declspec(dllimport) void CloseLog();
extern "C" __declspec(dllimport) void PrintLog(const char * sysmessage);

//GL Param
//ModelGL param
float CCarbonMed3DRecon::cameraPosition[3] = {0.0f};
float CCarbonMed3DRecon::cameraAngle[3] = {0.0f};
float CCarbonMed3DRecon::modelPosition[3] = {0.0f};
float CCarbonMed3DRecon::modelAngle[3] = {0.0f};

// these are for 3rd person view
float CCarbonMed3DRecon::cameraAngleX = 0.0f;
float CCarbonMed3DRecon::cameraAngleY = 0.0f;
float CCarbonMed3DRecon::cameraDistance = 0.0;
float CCarbonMed3DRecon::bgColor[4] = {0.0f};

//window resource
float CCarbonMed3DRecon::m_ZoomAspect = 0.0f;
int CCarbonMed3DRecon::m_WinWidth = WIDTH;
int CCarbonMed3DRecon::m_WinHeight = HEIGHT;
int CCarbonMed3DRecon::m_SliceWinW = WIDTH;
int CCarbonMed3DRecon::m_SliceWinH = HEIGHT;
int CCarbonMed3DRecon::m_UltrasoundWinW = WIDTH;
int CCarbonMed3DRecon::m_UltrasoundWinH = HEIGHT;
BOOL CCarbonMed3DRecon::winactiveflag = TRUE;
BOOL CCarbonMed3DRecon::slicewinactiveflag = TRUE;
BOOL CCarbonMed3DRecon::uswinactiveflag = TRUE;
Window *CCarbonMed3DRecon::m_GlWin = NULL;
Window *CCarbonMed3DRecon::m_GlSliceWin = NULL;
Window *CCarbonMed3DRecon::m_GlUltrasoundWin = NULL;
Matrix4 CCarbonMed3DRecon::matrixView;
Matrix4 CCarbonMed3DRecon::matrixModel;
Matrix4 CCarbonMed3DRecon::matrixModelView;
Matrix4 CCarbonMed3DRecon::matrixProjection;
// devices
bool CCarbonMed3DRecon::_NDIDeviceConnect = false;
// global sizes and local sizes
size_t CCarbonMed3DRecon::_GlobalWorkSize[2] = { 0, 0 };
size_t CCarbonMed3DRecon::_LocalWorkSize[2] = { 0, 0 };
// this functor parameter from config file
double CCarbonMed3DRecon::a = 0.0;
double CCarbonMed3DRecon::b = 0.0;
// cl variables
cl_int CCarbonMed3DRecon::error = 0;
cl_uint CCarbonMed3DRecon::num = 0;
// data on gpu
cl_mem CCarbonMed3DRecon::d_ImagIn = NULL;
cl_mem CCarbonMed3DRecon::d_ImagOut = NULL;
// para for kernel
RenderBase *CCarbonMed3DRecon::render_obj = NULL;
string CCarbonMed3DRecon::m_Graphicardinfo = " ";

float CCarbonMed3DRecon::_Windowcenter = 0.0f;
float CCarbonMed3DRecon::_Windowwidth = 0.0f;
float CCarbonMed3DRecon::_Pixelspacex = 0.0f;
float CCarbonMed3DRecon::_Pixelspacey = 0.0f;
float CCarbonMed3DRecon::_UpperLeft_X = 0.0f;
float CCarbonMed3DRecon::_UpperLeft_Y = 0.0f;
float CCarbonMed3DRecon::_UpperLeft_Z[MAXSLICENUM] = { 0.0f };
int CCarbonMed3DRecon::_SliceNum = 0;
TCHAR CCarbonMed3DRecon::m_ConfigFileName[255] = _T("D:/CarbonMed/Config/Global/3DRenderingParam.ini");
const char CCarbonMed3DRecon::m_RegistrationDirectionPath[255] = "D:/CarbonMed/Config/Global/";

//texture order flag
int CCarbonMed3DRecon::m_TextureOrderFlag = 0;

//openGL
bool   CCarbonMed3DRecon::OpenGLWinFlag = true;
double CCarbonMed3DRecon::curxpos = 0;
double CCarbonMed3DRecon::curypos = 0;

double CCarbonMed3DRecon::refxpos = 0;
double CCarbonMed3DRecon::refypos = 0;

double CCarbonMed3DRecon::offsetx = 0;
double CCarbonMed3DRecon::offsety = 0;

double CCarbonMed3DRecon::totalTime = 0.0;

cl_float CCarbonMed3DRecon::clrotate[4];
cl_float CCarbonMed3DRecon::translate[4];

int CCarbonMed3DRecon::mouse_buttons = 0;

bool CCarbonMed3DRecon::wireframe = false;
bool CCarbonMed3DRecon::animate = true;
UINT32 CCarbonMed3DRecon::frameCount = 0;
UINT32 CCarbonMed3DRecon::fpsLimit = 100;

//window extra info
bool CCarbonMed3DRecon::m_IsGetFrameRate = false;
bool CCarbonMed3DRecon::m_IsGetErrorRate = false;
string CCarbonMed3DRecon::strFrameRate = "Frame Rate:";
string CCarbonMed3DRecon::strErrorRate = "Error Rate:";

const float glhalfworldsize = 1.414f / 2;
const float DEG2RAD = 3.141593f / 180;
const float FOV_Y = 60.0f;              // vertical FOV in degree
const float NEAR_PLANE = -300.0f;
const float FAR_PLANE = 300.0f;
const float CAMERA_ANGLE_X = 45.0f;     // pitch in degree
const float CAMERA_ANGLE_Y = -45.0f;    // heading in degree
const float CAMERA_DISTANCE = 50.0f;    // camera distance
//3d Window info
HDC             CarbonhDC = 0;
HGLRC           CarbonhRC = 0;
HWND            CarbonhWnd = NULL;
//slice Window info
HDC             CarbonSlicehDC = 0;
HGLRC           CarbonSlicehRC = 0;
HWND            CarbonSlicehWnd = NULL;
//ultrasound Window info
HDC             CarbonUltrasoundhDC = 0;
HGLRC           CarbonUltrasoundhRC = 0;
HWND            CarbonUltrasoundhWnd = NULL;

LRESULT CALLBACK windowProcedure(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LRESULT returnValue = 0;        // return value

	// route messages to the associated controller
	switch (msg)
	{
	case WM_ACTIVATE:
		if (!HIWORD(wParam))
		{
			CCarbonMed3DRecon::winactiveflag = TRUE;
		}
		else
		{
			CCarbonMed3DRecon::winactiveflag = FALSE;
		}
		returnValue = 0;
		break;

	case WM_CREATE:
		if(!CCarbonMed3DRecon::WincreateContextWithHandle(hwnd, CarbonhDC, CarbonhRC, 32, 24, 8))
		{
			return -1;
		}
		returnValue = 0;
		break;

	case WM_SIZE:
		returnValue = 0;
		break;

	case WM_ENABLE:
		returnValue = 0;
		break;

	case WM_COMMAND:
		returnValue = CCarbonMed3DRecon::command(LOWORD(wParam), HIWORD(wParam), lParam);
		break;

	case WM_MOUSEMOVE:
		returnValue = CCarbonMed3DRecon::mouseMove(wParam, LOWORD(lParam), HIWORD(lParam));
		break;

	case WM_CLOSE:
		returnValue = 0;
		break;

	case WM_DESTROY:
		returnValue = CCarbonMed3DRecon::destroy();
		break;

	case WM_SYSCOMMAND:
		returnValue = ::DefWindowProc(hwnd, msg, wParam, lParam);
		break;

	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		returnValue = CCarbonMed3DRecon::keyDown((int)wParam, lParam);
		break;

	case WM_LBUTTONDOWN:
		returnValue = CCarbonMed3DRecon::lButtonDown(wParam, LOWORD(lParam), HIWORD(lParam));
		break;

	case WM_LBUTTONUP:
		returnValue = CCarbonMed3DRecon::lButtonUp(wParam, LOWORD(lParam), HIWORD(lParam));
		break;

	case WM_RBUTTONDOWN:
		returnValue = CCarbonMed3DRecon::rButtonDown(wParam, LOWORD(lParam), HIWORD(lParam));
		break;

	case WM_RBUTTONUP:
		returnValue = CCarbonMed3DRecon::rButtonUp(wParam, LOWORD(lParam), HIWORD(lParam));
		break;

	case WM_MBUTTONDOWN:
		returnValue = CCarbonMed3DRecon::mButtonDown(wParam, LOWORD(lParam), HIWORD(lParam));
		break;

	case WM_MBUTTONUP:
		returnValue = CCarbonMed3DRecon::mButtonUp(wParam, LOWORD(lParam), HIWORD(lParam));
		break;

	case WM_MOUSEWHEEL:
		returnValue = CCarbonMed3DRecon::mouseWheel((short)LOWORD(wParam), (short)HIWORD(wParam) / WHEEL_DELTA, (short)LOWORD(lParam), (short)HIWORD(lParam));   // state, delta, x, y
		break;

	default:
		returnValue = ::DefWindowProc(hwnd, msg, wParam, lParam);
	}

	return returnValue;
}

LRESULT CALLBACK SlicewindowProcedure(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LRESULT returnValue = 0;
	switch (msg)
	{
	case WM_ACTIVATE:
		if (!HIWORD(wParam))
		{
			CCarbonMed3DRecon::slicewinactiveflag = TRUE;
		}
		else
		{
			CCarbonMed3DRecon::slicewinactiveflag = FALSE;
		}
		returnValue = 0;
		break;

	case WM_CREATE:
		if (!CCarbonMed3DRecon::WincreateContextWithHandle(hwnd, CarbonSlicehDC, CarbonSlicehRC, 32, 24, 8))
		{
			return -1;
		}
		returnValue = 0;
		break;

	case WM_SIZE:
		returnValue = 0;
		break;

	case WM_ENABLE:
		returnValue = 0;
		break;

	case WM_COMMAND:
		returnValue = CCarbonMed3DRecon::command(LOWORD(wParam), HIWORD(wParam), lParam);
		break;

	case WM_MOUSEMOVE:
		//returnValue = CCarbonMed3DRecon::mouseMove(wParam, LOWORD(lParam), HIWORD(lParam));
		returnValue = 0;
		break;

	case WM_CLOSE:
		returnValue = 0;
		break;

	case WM_DESTROY:
		returnValue = CCarbonMed3DRecon::destroy();
		break;

	case WM_SYSCOMMAND:
		returnValue = ::DefWindowProc(hwnd, msg, wParam, lParam);
		break;

	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		returnValue = CCarbonMed3DRecon::keyDown((int)wParam, lParam);
		break;

	case WM_LBUTTONDOWN:
		returnValue = 0;
		break;

	case WM_LBUTTONUP:
		returnValue = 0;
		break;

	case WM_RBUTTONDOWN:
		returnValue = 0;
		break;

	case WM_RBUTTONUP:
		returnValue = 0;
		break;

	case WM_MBUTTONDOWN:
		returnValue = 0;
		break;

	case WM_MBUTTONUP:
		returnValue = 0;
		break;

	case WM_MOUSEWHEEL:
		returnValue = CCarbonMed3DRecon::mouseWheel((short)LOWORD(wParam), (short)HIWORD(wParam) / WHEEL_DELTA, (short)LOWORD(lParam), (short)HIWORD(lParam));
		break;

	default:
		returnValue = ::DefWindowProc(hwnd, msg, wParam, lParam);
	}

	return returnValue;
}

LRESULT CALLBACK UltrasoundwindowProcedure(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LRESULT returnValue = 0;
	switch (msg)
	{
	case WM_ACTIVATE:
		if (!HIWORD(wParam))
		{
			CCarbonMed3DRecon::uswinactiveflag = TRUE;
		}
		else
		{
			CCarbonMed3DRecon::uswinactiveflag = FALSE;
		}
		returnValue = 0;
		break;

	case WM_CREATE:
		if (!CCarbonMed3DRecon::WincreateContextWithHandle(hwnd, CarbonUltrasoundhDC, CarbonUltrasoundhRC, 32, 24, 8))
		{
			return -1;
		}
		returnValue = 0;
		break;

	case WM_SIZE:
		returnValue = 0;
		break;

	case WM_ENABLE:
		returnValue = 0;
		break;

	case WM_COMMAND:
		returnValue = CCarbonMed3DRecon::command(LOWORD(wParam), HIWORD(wParam), lParam);
		break;

	case WM_MOUSEMOVE:
		//returnValue = CCarbonMed3DRecon::mouseMove(wParam, LOWORD(lParam), HIWORD(lParam));
		returnValue = 0;
		break;

	case WM_CLOSE:
		returnValue = 0;
		break;

	case WM_DESTROY:
		returnValue = CCarbonMed3DRecon::destroy();
		break;

	case WM_SYSCOMMAND:
		returnValue = ::DefWindowProc(hwnd, msg, wParam, lParam);
		break;

	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		returnValue = CCarbonMed3DRecon::keyDown((int)wParam, lParam);
		break;

	case WM_LBUTTONDOWN:
		returnValue = 0;
		break;

	case WM_LBUTTONUP:
		returnValue = 0;
		break;

	case WM_RBUTTONDOWN:
		returnValue = 0;
		break;

	case WM_RBUTTONUP:
		returnValue = 0;
		break;

	case WM_MBUTTONDOWN:
		returnValue = 0;
		break;

	case WM_MBUTTONUP:
		returnValue = 0;
		break;

	case WM_MOUSEWHEEL:
		returnValue = CCarbonMed3DRecon::mouseWheel((short)LOWORD(wParam), (short)HIWORD(wParam) / WHEEL_DELTA, (short)LOWORD(lParam), (short)HIWORD(lParam));
		break;

	default:
		returnValue = ::DefWindowProc(hwnd, msg, wParam, lParam);
	}

	return returnValue;
}

CCarbonMed3DRecon::~CCarbonMed3DRecon()
{
	ReleaseFunctor();
}

void CCarbonMed3DRecon::GetVolumeInfoFromFile(const char *filename, _DICOMInfo *dicomobj, int *slicenum)
{
	int filelength = 0;
	FILE *fp = fopen(filename, "r");

	if (!fp)
	{
		return;
	}
	int line_count = 0;
	char c;
	do
	{
		c = fgetc(fp);
		if (c == '\n')
			line_count++;
	} while (c != EOF);
	fclose(fp);
	*slicenum = line_count;
	float *volumeinfodata = (float *)malloc(7 * sizeof(float));
	fp = fopen(filename, "r");
	for (int i = 0; i < line_count; i++)
	{
		int valueinfile = 0;
		for (int j = 0; j < 7; j++)
		{
			float value = 0;
			fscanf(fp, "%f", &value);
			volumeinfodata[j] = value;
		}
		dicomobj[i].PixelSpacing_X = volumeinfodata[0];
		dicomobj[i].PixelSpacing_Y = volumeinfodata[1];
		dicomobj[i].UpperLeft_X = volumeinfodata[2];
		dicomobj[i].UpperLeft_Y = volumeinfodata[3];
		dicomobj[i].UpperLeft_Z = volumeinfodata[4];
		dicomobj[i].WindowCenter = volumeinfodata[5];
		dicomobj[i].WindowWidth = volumeinfodata[6];
	}
	fclose(fp);

	free(volumeinfodata);
}

//slice data
void CCarbonMed3DRecon::GetVolumeSliceData(cl_mem slicedata)
{
	render_obj->GetSliceDataU8GPU(slicedata);
}

//normalized slice data
void CCarbonMed3DRecon::GetNormalizedSliceData(float *slicedata)
{
	render_obj->GetNomalizedSliceData(slicedata);
}

//NDI data
void CCarbonMed3DRecon::GetNDITrackerRecord(void *precord)
{
	render_obj->GetNDIRecord(precord);
}

void CCarbonMed3DRecon::GetRegistrationCorInfo(float *rmat, float *tvec)
{
	render_obj->GetRegistrationCorInfo(rmat, tvec);
}

bool CCarbonMed3DRecon::WincreateContext(HWND handle, int colorBits, int depthBits, int stencilBits)
{
	// retrieve a handle to a display device context
	CarbonhDC = ::GetDC(handle);

	PIXELFORMATDESCRIPTOR pfd;
	GLuint pixelFormat;
	//使用openGL GlutCreateWindow创建窗口后获取的窗口的pixelFormat为33
	pixelFormat = 33;
	DescribePixelFormat(CarbonhDC, pixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pfd);
	if (!::SetPixelFormat(CarbonhDC, pixelFormat, &pfd))
		return false;

	// create a new OpenGL rendering context
	CarbonhRC = wglCreateContext(CarbonhDC);
	wglMakeCurrent(CarbonhDC, CarbonhRC);
	return true;
}

bool CCarbonMed3DRecon::WincreateContextWithHandle(HWND handle, HDC &windc, HGLRC &winRc, int colorBits, int depthBits, int stencilBits)
{
	// retrieve a handle to a display device context
	windc = ::GetDC(handle);

	PIXELFORMATDESCRIPTOR pfd;
	GLuint pixelFormat;
	//使用openGL GlutCreateWindow创建窗口后获取的窗口的pixelFormat为33
	pixelFormat = 33;
	DescribePixelFormat(windc, pixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pfd);
	if (!::SetPixelFormat(windc, pixelFormat, &pfd))
		return false;

	// create a new OpenGL rendering context
	winRc = wglCreateContext(windc);
	//wglMakeCurrent(windc, winRc);
	return true;
}

void  CCarbonMed3DRecon::MakeCurrentWin(HDC &windc, HGLRC &winRc)
{
	wglMakeCurrent(windc, winRc);
}

///////////////////////////////////////////////////////////////////////////////
// choose pixel format
// By default, pdf.dwFlags is set PFD_DRAW_TO_WINDOW, PFD_DOUBLEBUFFER and PFD_SUPPORT_OPENGL.
///////////////////////////////////////////////////////////////////////////////
#define DESFLAGS PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER | PFD_SWAP_EXCHANGE
bool CCarbonMed3DRecon::setPixelFormat(HDC hdc, int colorBits, int depthBits, int stencilBits)
{
	PIXELFORMATDESCRIPTOR pfd;
	GLuint testflag = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER | PFD_SWAP_EXCHANGE;
	GLuint pixelFormat;
	GLuint maxPixelFormat = DescribePixelFormat(hdc, 1, sizeof(PIXELFORMATDESCRIPTOR), NULL);  //获取所支持的像素最大索引

	for (GLuint iPixelFormat = 1; iPixelFormat <= maxPixelFormat; iPixelFormat++)
	{

		DescribePixelFormat(hdc, iPixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pfd);
		if (iPixelFormat == 8)
		{
			pixelFormat = iPixelFormat;

			break;
		}

	}

	// set the fixel format
	if (!::SetPixelFormat(hdc, pixelFormat, &pfd))
		return false;

	return true;
}

///////////////////////////////////////////////////////////////////////////////
// find the best pixel format
///////////////////////////////////////////////////////////////////////////////
int CCarbonMed3DRecon::findPixelFormat(HDC hdc, int colorBits, int depthBits, int stencilBits)
{
	int bestMode = 0;                           // return value, best pixel format
	int currScore = 0;                          // points of current mode
	int bestScore = 0;                          // points of best candidate
	PIXELFORMATDESCRIPTOR pfd;
	static  PIXELFORMATDESCRIPTOR pfd11 =                 // /pfd 告诉窗口我们所希望的东东，即窗口使用的像素格式
	{
		sizeof(PIXELFORMATDESCRIPTOR),                  // 上述格式描述符的大小
		1,                              // 版本号
		PFD_DRAW_TO_WINDOW |                        // 格式支持窗口
		PFD_SUPPORT_OPENGL |                        // 格式必须支持OpenGL
		PFD_DOUBLEBUFFER,                       // 必须支持双缓冲
		PFD_TYPE_RGBA,                          // 申请 RGBA 格式
		32,                             // 选定色彩深度
		0, 0, 0, 0, 0, 0,                       // 忽略的色彩位
		0,                              // 无Alpha缓存
		0,                              // 忽略Shift Bit
		0,                              // 无累加缓存
		0, 0, 0, 0,                         // 忽略聚集位
		32,                             // 16位 Z-缓存 (深度缓存)
		0,                              // 无蒙板缓存
		0,                              // 无辅助缓存
		PFD_MAIN_PLANE,                         // 主绘图层
		0,                              // Reserved
		0, 0, 0                             // 忽略层遮罩
	};
	pfd = pfd11;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER | PFD_SWAP_EXCHANGE;
	pfd.cColorBits = 32;
	pfd.cRedBits = 8;
	pfd.cRedShift = 16;
	pfd.cGreenBits = 8;
	pfd.cGreenShift = 8;
	pfd.cBlueBits = 8;
	pfd.cBlueShift = 0;
	pfd.cAlphaBits = 0;
	pfd.cAlphaShift = 0;
	pfd.cAccumBits = 64;
	pfd.cAccumRedBits = 16;
	pfd.cAccumGreenBits = 16;
	pfd.cAccumBlueBits = 16;
	pfd.cAccumAlphaBits = 16;
	pfd.cDepthBits = 32;

	bestMode = ChoosePixelFormat(hdc, &pfd);

	return bestMode;
}

void CCarbonMed3DRecon::InitModelGL()
{
	glShadeModel(GL_SMOOTH);                        // shading mathod: GL_SMOOTH or GL_FLAT

	// enable/disable features
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_POINT_SMOOTH);
	glEnable(GL_POLYGON_SMOOTH);
	glEnable(GL_MULTISAMPLE);

	// track material ambient and diffuse from surface color, call it before glEnable(GL_COLOR_MATERIAL)
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	glEnable(GL_COLOR_MATERIAL);

	glClearColor(0, 0, 0, 0);   // background color
	glClearStencil(0);                              // clear stencil buffer
	glClearDepth(1.0f);                             // 0 is near, 1 is far
}

///////////////////////////////////////////////////////////////////////////////
// swap OpenGL frame buffers
///////////////////////////////////////////////////////////////////////////////
void CCarbonMed3DRecon::WinswapBuffers(HDC &windc)
{
	::SwapBuffers(windc);
}

void CCarbonMed3DRecon::closeContext(HWND handle, HDC &windc, HGLRC &winRc)
{
	if (!windc || !winRc)
		return;

	// delete DC and RC
	::wglMakeCurrent(0, 0);
	::wglDeleteContext(winRc);
	::ReleaseDC(handle, windc);

	windc = 0;
	winRc = 0;
}

bool CCarbonMed3DRecon::InitWGL(HWND cwnd[3], int *_3dwindowinfo, int *slicewindowinfo, int *uswindowinfo)
{
	int ps = 0;
	char *cmd[128] = { 0 };
	glutInit(&ps, cmd);
	cameraAngleX = CAMERA_ANGLE_X;
	cameraAngleY = CAMERA_ANGLE_Y;
	cameraDistance = CAMERA_DISTANCE;
	cameraPosition[0] = cameraPosition[1] = cameraPosition[2] = 0;
	cameraAngle[0] = cameraAngle[1] = cameraAngle[2] = 0;
	modelPosition[0] = modelPosition[1] = modelPosition[2] = 0;
	modelAngle[0] = modelAngle[1] = modelAngle[2] = 0;
	bgColor[0] = bgColor[1] = bgColor[2] = bgColor[3] = 0;

	matrixView.identity();
	matrixModel.identity();
	matrixModelView.identity();
	matrixProjection.identity();

	if (cwnd == NULL)
	{
		//3d Window
		m_GlWin = new Window(windowProcedure, L"3DGL", L"3DGL");
		m_GlWin->setWindowStyle(WS_VISIBLE | WS_CLIPSIBLINGS | WS_OVERLAPPEDWINDOW);
		m_GlWin->setWidth(_3dwindowinfo[2]);
		m_GlWin->setHeight(_3dwindowinfo[3]);
		m_GlWin->setWindowStyleEx(WS_EX_WINDOWEDGE);
		m_GlWin->setClassStyle(CS_OWNDC);
		CarbonhWnd = m_GlWin->create();
		MakeCurrentWin(CarbonhDC, CarbonhRC);
		render_obj->SetRenderingWindowSize(_3dwindowinfo[2], _3dwindowinfo[3]);
		m_WinWidth = _3dwindowinfo[2];
		m_WinHeight = _3dwindowinfo[3];
		m_GlWin->show();
		//Slice Window
		m_GlSliceWin = new Window(SlicewindowProcedure, L"SliceGL", L"SliceGL");
		m_GlSliceWin->setWindowStyle(WS_VISIBLE | WS_CLIPSIBLINGS | WS_OVERLAPPEDWINDOW);
		m_GlSliceWin->setWidth(slicewindowinfo[2]);
		m_GlSliceWin->setHeight(slicewindowinfo[3]);
		m_GlSliceWin->setWindowStyleEx(WS_EX_WINDOWEDGE);
		m_GlSliceWin->setClassStyle(CS_OWNDC);
		CarbonSlicehWnd = m_GlSliceWin->create();

		render_obj->SetRenderingWindowSize(slicewindowinfo[2], slicewindowinfo[3]);
		m_SliceWinW = slicewindowinfo[2];
		m_SliceWinH = slicewindowinfo[3];
	    //m_GlSliceWin->show();
        //Us window
		m_GlUltrasoundWin = new Window(UltrasoundwindowProcedure, L"UltrasoundGL", L"UltrasoundGL");
		m_GlUltrasoundWin->setWindowStyle(WS_VISIBLE | WS_CLIPSIBLINGS | WS_OVERLAPPEDWINDOW);
		m_GlUltrasoundWin->setWidth(uswindowinfo[2]);
		m_GlUltrasoundWin->setHeight(uswindowinfo[3]);
		m_GlUltrasoundWin->setWindowStyleEx(WS_EX_WINDOWEDGE);
		m_GlUltrasoundWin->setClassStyle(CS_OWNDC);
		CarbonSlicehWnd = m_GlUltrasoundWin->create();

		render_obj->SetRenderingWindowSize(uswindowinfo[2], uswindowinfo[3]);
		m_UltrasoundWinW = uswindowinfo[2];
		m_UltrasoundWinH = uswindowinfo[3];
	}
	else
	{
		CarbonhWnd = cwnd[0];
		WincreateContextWithHandle(CarbonhWnd, CarbonhDC, CarbonhRC, 32, 24, 8);
		m_WinWidth = _3dwindowinfo[2];
		m_WinHeight = _3dwindowinfo[3];
		render_obj->SetRenderingWindowSize(m_WinWidth, m_WinHeight);
		CarbonSlicehWnd = cwnd[1];
		WincreateContextWithHandle(CarbonSlicehWnd, CarbonSlicehDC, CarbonSlicehRC, 32, 24, 8);
		m_SliceWinW = _3dwindowinfo[2];
		m_SliceWinH = _3dwindowinfo[3];
		render_obj->SetRenderingWindowSize(m_SliceWinW, m_SliceWinH);
		CarbonUltrasoundhWnd = cwnd[2];
		WincreateContextWithHandle(CarbonUltrasoundhWnd, CarbonUltrasoundhDC, CarbonUltrasoundhRC, 32, 24, 8);
		m_UltrasoundWinW = _3dwindowinfo[2];
		m_UltrasoundWinH = _3dwindowinfo[3];
		render_obj->SetRenderingWindowSize(m_UltrasoundWinW, m_UltrasoundWinH);
	}

	if (!CarbonhWnd)
	{
		exit(EXIT_SUCCESS);
	}
	
	glewInit();

	if (!glewIsSupported("GL_VERSION_2_0 GL_ARB_pixel_buffer_object"))
	{
		PrintLog("Required OpenGL extensions missing.");
		exit(EXIT_SUCCESS);
	}
	return true;
}

bool CCarbonMed3DRecon::InitGL()
{
	// initialize GLUT callback functions
	int ps = 0;
	char *cmd[128] = { 0 };
	glutInit(&ps, cmd);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_MULTISAMPLE);
	glutInitWindowSize(WIDTH, HEIGHT);
	render_obj->SetRenderingWindowSize(WIDTH, HEIGHT);
	glutCreateWindow("CUDA volume rendering");
	//HWND hwnd = GetActiveWindow();
	//CarbonhWnd = hwnd;
	glewInit();

	if (!glewIsSupported("GL_VERSION_2_0 GL_ARB_pixel_buffer_object"))
	{
		printf("Required OpenGL extensions missing.");
		exit(EXIT_SUCCESS);
	}
	return true;
}

//volume init
void CCarbonMed3DRecon::VolumeInit()
{
	render_obj->VolumeDataInit();
	render_obj->TextureInit();
	render_obj->OpenGLBufferInit();
}

//slice PBO init
void CCarbonMed3DRecon::SlicePBOInit()
{
	render_obj->SlicePBOInit();
}

void CCarbonMed3DRecon::NdiDeviceInit()
{
	SliceInit();
}
//slice init
void CCarbonMed3DRecon::SliceInit()
{
	char *errormessage = NULL;
	render_obj->StartInternalThread(errormessage);
	//render_obj->GetThreadData();
}

// -------------------------------------------------- Functions ------------------------------------------------------------
void CCarbonMed3DRecon::OpenCLini()
{
	PrintLog("begin to initialize OpenCL components for this functor.");

	render_obj->InitCL(m_Graphicardinfo);
	render_obj->KernelInit();

	PrintLog("initialize OpenCL components for this functor successfully.");
}

void CCarbonMed3DRecon::OpenCLRelease()
{
	// release resources
	if (render_obj)
	{
		delete render_obj;
		render_obj = NULL;
	}

	PrintLog("release OpenCL components for this functor successfully.");
}

string CCarbonMed3DRecon::TCHAR2STRING(TCHAR *STR)
{
	int iLen = WideCharToMultiByte(CP_ACP, 0, STR, -1, NULL, 0, NULL, NULL);
	char* chRtn = new char[iLen*sizeof(char)];
	WideCharToMultiByte(CP_ACP, 0, STR, -1, chRtn, iLen, NULL, NULL);
	std::string str(chRtn);
	return str;
}

int CCarbonMed3DRecon::ReadInteger(LPCTSTR szSection, LPCTSTR szKey, int iDefaultValue)
{
	int iResult = GetPrivateProfileInt(szSection, szKey, iDefaultValue, m_ConfigFileName);
	return iResult;
}
float CCarbonMed3DRecon::ReadFloat(LPCTSTR szSection, LPCTSTR szKey, float fltDefaultValue)
{
	TCHAR szResult[255];
	TCHAR szDefault[255];
	float fltResult;
	_stprintf_s(szDefault, 255, TEXT("%f"), fltDefaultValue);
	GetPrivateProfileString(szSection, szKey, szDefault, szResult, 255, m_ConfigFileName);
	fltResult = (float)_tstof(szResult);
	return fltResult;
}
bool CCarbonMed3DRecon::ReadBoolean(LPCTSTR szSection, LPCTSTR szKey, bool bolDefaultValue)
{
	TCHAR szResult[255];
	TCHAR szDefault[255];
	bool bolResult;
	_stprintf_s(szDefault, 255, TEXT("%s"), bolDefaultValue ? TEXT("True") : TEXT("False"));
	GetPrivateProfileString(szSection, szKey, szDefault, szResult, 255, m_ConfigFileName);
	bolResult = (_tcscmp(szResult, TEXT("True")) == 0 ||
		_tcscmp(szResult, TEXT("true")) == 0) ? true : false;
	return bolResult;
}
string CCarbonMed3DRecon::ReadString(LPCTSTR szSection, LPCTSTR szKey, LPCTSTR szDefaultValue)
{
	LPTSTR szResult = new TCHAR[255];
	memset(szResult, 0x00, sizeof(szResult));
	GetPrivateProfileString(szSection, szKey, szDefaultValue, szResult, 255, m_ConfigFileName);
	string resultstr = TCHAR2STRING(szResult);
	return resultstr;
}
void CCarbonMed3DRecon::ReadArray(LPCTSTR szSection, LPCTSTR szKey, float *resultArray, int alen)
{
	LPTSTR szResult = new TCHAR[255];
	LPCTSTR szDefaultValue = _T(" ");
	memset(szResult, 0x00, sizeof(szResult));
	GetPrivateProfileString(szSection, szKey, szDefaultValue, szResult, 255, m_ConfigFileName);
	string resultstr = TCHAR2STRING(szResult);
	string temp = "";
	char *sch = const_cast<char*>(resultstr.c_str());
	int base_index = 0;
	for (int i = 0; i < resultstr.length(); i++)
	{
		if (sch[i] != ',')
		{
			temp += resultstr[i];
		}
		else
		{
			resultArray[base_index++] = atof(temp.c_str());
			temp = "";
		}
	}
	if (base_index >= alen)
	{
		return;
	}
	resultArray[base_index++] = atof(temp.c_str());
}


int CCarbonMed3DRecon::InitialFunctorTest(float *dicominfo, float *dicomdata, int framecount, HWND cwnd[3], int *_3dwindowinfo, int *slicewindowinfow, int *uswindowinfo)
{
	OpenLog();

	PrintLog("begin to initiate this functor resource.");
	//Graph Card info
	m_Graphicardinfo = ReadString(_T("device"), _T("GraphCardInfo"), _T(" "));
	bool Amd_flag = ReadBoolean(_T("device"), _T("AMD"), true);
	int MaskNum = ReadInteger(_T("Param"), _T("MaskNum"), 1);
	RenderParam rParam;
	rParam.IsoValue = ReadFloat(_T("Param"), _T("IsoValue"), -63.0f);
	rParam.IsoValueBone = ReadFloat(_T("Param"), _T("IsoValueBone"), 130.0f);
	rParam.IsoValueMask = ReadFloat(_T("Param"), _T("IsoValueMask"), -50.0f);
	rParam.IsoUp = ReadFloat(_T("Param"), _T("IsoUp"), -62.0f);
	rParam.IsoBot = ReadFloat(_T("Param"), _T("IsoBot"), -64.0f);
	rParam.DisoValue = ReadFloat(_T("Param"), _T("DisoValue"), -0.5f);
	rParam.MaskStartSlice = ReadFloat(_T("Param"), _T("MaskStartNO"), 0);
	rParam.MaskEndSlice = ReadFloat(_T("Param"), _T("MaskEndNO"), 1);
	rParam.Density = ReadFloat(_T("Param"), _T("Density"), 0.1f);
	rParam.Brightness = ReadFloat(_T("Param"), _T("Brightness"), 8.0f);
	rParam.TransferOffset = ReadFloat(_T("Param"), _T("TransferOffset"), 0.0f);
	rParam.TransferScale = ReadFloat(_T("Param"), _T("TransferScale"), 1.0f);
	string renderMethod = ReadString(_T("Param"), _T("RenderMethod"), _T(" "));
	int InterpNum = ReadInteger(_T("Param"), _T("InterpSliceNum"), 1);
	float VoxelZsize = ReadFloat(_T("Param"), _T("InterpVoxelZsize"), 7.0f);
	OpenGLWinFlag = ReadBoolean(_T("Control"), _T("OpenGLWinFlag"), true);
	m_TextureOrderFlag = ReadInteger(_T("Control"), _T("RenderingOrderFlag"), 0);
	m_ZoomAspect = ReadFloat(_T("Control"), _T("RenderingZoomAspect"), 500.0f);
	bool SkinMeshFilteringFlag = ReadBoolean(_T("Control"), _T("SkinMeshFilteringFlag"), true);
	bool BoneMeshFilteringFlag = ReadBoolean(_T("Control"), _T("BoneMeshFilteringFlag"), true);
	bool MaskMeshFilteringFlag = ReadBoolean(_T("Control"), _T("MaskMeshFilteringFlag"), true);
	int SkinFilteringIterTimes = ReadInteger(_T("Param"), _T("SkinFilteringIterTimes"), 1);
	int BoneFilteringIterTimes = ReadInteger(_T("Param"), _T("BoneFilteringIterTimes"), 1);
	int MaskFilteringIterTimes = ReadInteger(_T("Param"), _T("MaskFilteringIterTimes"), 1);
	bool SkinRenderFlag = ReadBoolean(_T("Control"), _T("SkinRenderFlag"), true);
	bool BoneRenderFlag = ReadBoolean(_T("Control"), _T("BoneRenderFlag"), true);
	bool SliceRenderFlag = ReadBoolean(_T("Control"), _T("SliceRenderFlag"), true);
	bool OrganRenderFlag[8] = { false };
	for (int i = 0; i < MaskNum; i++)
	{
		string tempstr = "OrganRenderFlag" + to_string(i);
		std::wstring wstr(tempstr.begin(), tempstr.end());
		_TCHAR* Tstr = (_TCHAR*)(&wstr[0]);
		OrganRenderFlag[i] = ReadBoolean(_T("Control"), Tstr, true);
	}
	bool PboSWFlag = ReadBoolean(_T("Control"), _T("PboSWFlag"), true);
	float skincolor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	float bonecolor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	float maskcolor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	float punclinecolor[DEFAULTSENSORCOUNT*4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	float probecolor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	ReadArray(_T("Render"), _T("SkinColor"), skincolor, 4);
	ReadArray(_T("Render"), _T("BoneColor"), bonecolor, 4);
	ReadArray(_T("Render"), _T("MaskColor"), maskcolor, 4);
	ReadArray(_T("Render"), _T("ProbeColor"), probecolor, 4);
	ReadArray(_T("Render"), _T("PuncLineColor1"), &punclinecolor[0], 4);
	ReadArray(_T("Render"), _T("PuncLineColor2"), &punclinecolor[DEFAULTSENSORCOUNT], 4);
	ReadArray(_T("Render"), _T("PuncLineColor3"), &punclinecolor[2 * DEFAULTSENSORCOUNT], 4);
	ReadArray(_T("Render"), _T("PuncLineColor4"), &punclinecolor[3 * DEFAULTSENSORCOUNT], 4);
	//Reg RealWorld Param
	float PuncNeedleRadius = 0.0f;
	float PuncLineAngle[DEFAULTSENSORCOUNT] = { 0.0f };
	float PuncLineLength[DEFAULTSENSORCOUNT] = { 0.0f };
	float Refdistance[DEFAULTSENSORCOUNT] = { 0.0f };
	float RefAxis[DEFAULTSENSORCOUNT * 3] = { 0.0f };
	bool  PuncLineRenderFlag[DEFAULTSENSORCOUNT] = { false };
	PuncNeedleRadius = ReadFloat(_T("Registor"), _T("PuncneedleRadius"), 1.0f);//default radius is 1.5mm 
	PuncLineAngle[0] = ReadFloat(_T("Registor"), _T("PuncLineAngle1"), 20.0f); //default angle is 20 degree
	PuncLineAngle[1] = ReadFloat(_T("Registor"), _T("PuncLineAngle2"), 20.0f); //default angle is 20 degree
	PuncLineAngle[2] = ReadFloat(_T("Registor"), _T("PuncLineAngle3"), 20.0f); //default angle is 20 degree
	PuncLineAngle[3] = ReadFloat(_T("Registor"), _T("PuncLineAngle4"), 20.0f); //default angle is 20 degree
	PuncLineLength[0] = ReadFloat(_T("Registor"), _T("PuncLineLength1"), 120.0f); //default trace length is 12cm
	PuncLineLength[1] = ReadFloat(_T("Registor"), _T("PuncLineLength2"), 120.0f); //default trace length is 12cm
	PuncLineLength[2] = ReadFloat(_T("Registor"), _T("PuncLineLength3"), 120.0f); //default trace length is 12cm
	PuncLineLength[3] = ReadFloat(_T("Registor"), _T("PuncLineLength4"), 120.0f); //default trace length is 12cm
	Refdistance[0] = ReadFloat(_T("Registor"), _T("RefDistance1"), 33.0f);//refdistance is probabely equal to the radius of the us probe
	Refdistance[1] = ReadFloat(_T("Registor"), _T("RefDistance2"), 33.0f);//refdistance is probabely equal to the radius of the us probe
	Refdistance[2] = ReadFloat(_T("Registor"), _T("RefDistance3"), 33.0f);//refdistance is probabely equal to the radius of the us probe
	Refdistance[3] = ReadFloat(_T("Registor"), _T("RefDistance4"), 33.0f);//refdistance is probabely equal to the radius of the us probe
	PuncLineRenderFlag[0] = ReadBoolean(_T("Control"), _T("PuncLineRenderFlag1"), true);
	PuncLineRenderFlag[1] = ReadBoolean(_T("Control"), _T("PuncLineRenderFlag2"), true);
	PuncLineRenderFlag[2] = ReadBoolean(_T("Control"), _T("PuncLineRenderFlag3"), true);
	PuncLineRenderFlag[3] = ReadBoolean(_T("Control"), _T("PuncLineRenderFlag4"), true);
	ReadArray(_T("Registor"), _T("RefAxis1"), &RefAxis[0], 3);
	ReadArray(_T("Registor"), _T("RefAxis2"), &RefAxis[3], 3);
	ReadArray(_T("Registor"), _T("RefAxis3"), &RefAxis[2 * 3], 3);
	ReadArray(_T("Registor"), _T("RefAxis4"), &RefAxis[3 * 3], 3);
	_DICOMInfo dicomobj[MAXSLICENUM];
	for (int i = 0; i < framecount; i++)
	{
		dicomobj[i].PixelSpacing_X = dicominfo[i * 7 + 0];
		dicomobj[i].PixelSpacing_Y = dicominfo[i * 7 + 1];
		dicomobj[i].UpperLeft_X = dicominfo[i * 7 + 2];
		dicomobj[i].UpperLeft_Y = dicominfo[i * 7 + 3];
		dicomobj[i].UpperLeft_Z = dicominfo[i * 7 + 4];
		dicomobj[i].WindowCenter = dicominfo[i * 7 + 5];
		dicomobj[i].WindowWidth = dicominfo[i * 7 + 6];
	}

	if (renderMethod == "RayCasting")
	{
		render_obj = new VolumeRender();
	}
	else if (renderMethod == "MarchingCubes")
	{
		render_obj = new MarchingCube();
	}
	NdiDeviceInit();
	render_obj->SetZoomParam(m_ZoomAspect);
	render_obj->SetAMDFlag(Amd_flag);
	render_obj->SetMeshFilteringParam(SkinFilteringIterTimes, BoneFilteringIterTimes, MaskFilteringIterTimes, SkinMeshFilteringFlag, BoneMeshFilteringFlag, MaskMeshFilteringFlag);
	render_obj->SetEndSliceNo(rParam.MaskEndSlice);
	render_obj->SetDicomInfo(dicomobj, framecount);
	render_obj->SetRenderingFeature(rParam);
	render_obj->SetGridSize(WIDTH, HEIGHT, framecount);
	render_obj->SetRoiInfo(0, 0, 0, WIDTH, HEIGHT, framecount);
	render_obj->SetOrganNum((int)MaskNum);
	render_obj->SetDpRenderingFlag(SkinRenderFlag, BoneRenderFlag, SliceRenderFlag, PuncLineRenderFlag, OrganRenderFlag);
	render_obj->SetPboSWFlag(PboSWFlag);
	render_obj->SetDicomData(dicomdata);
	render_obj->SetHintDataPath(m_RegistrationDirectionPath);
	render_obj->SetRenderingColors(skincolor, bonecolor, maskcolor, punclinecolor, probecolor, 4);
	for (int i = 0; i < DEFAULTSENSORCOUNT; i++)
	{
		render_obj->InitPunctureLine(PuncLineAngle[i], PuncLineLength[i], Refdistance[i], PuncNeedleRadius, &RefAxis[i*3], i);
	}
	if (OpenGLWinFlag)
	{
		if (false == InitGL())
		{
			PrintLog("OpenGL init failed!\n");
			return -1;
		}
	}
	else
	{
		if (false == InitWGL(cwnd, _3dwindowinfo, slicewindowinfow, uswindowinfo))
		{
			PrintLog("OpenGL init failed!\n");
			return -1;
		}
	}

	OpenCLini();
	//等待NDI初始化完毕
	render_obj->WaitForInitOfNDI();
	render_obj->StopInternalThread();
	// CPU running grid
	_LocalWorkSize[0] = 16;
	_LocalWorkSize[1] = 16;
	_GlobalWorkSize[0] = HEIGHT;
	_GlobalWorkSize[1] = WIDTH;
	PrintLog("initiate this functor resource successfully.");

	return 1;
}

int CCarbonMed3DRecon::InitialFunctor()
{
	return 1;
}

void CCarbonMed3DRecon::ReleaseFunctor()
{
	OpenCLRelease();

	if (!OpenGLWinFlag)
	{
		closeContext(CarbonhWnd, CarbonhDC, CarbonhRC);
		closeContext(CarbonSlicehWnd, CarbonSlicehDC, CarbonSlicehRC);
	}
	if (!m_GlWin)
	{
		delete m_GlWin;
	}
	PrintLog("release this functor resource successfully.");
}

inline void CCarbonMed3DRecon::check_CL_error(cl_int error, char* error_messgae)
{
	if (error != CL_SUCCESS) {
		PrintLog(error_messgae);
		exit(error);
	}
}

#define BMP_Header_Length 54
void CCarbonMed3DRecon::grab(int index)
{
	char dummyimagename[256] = { 0 };
	char grabimagename[256] = {0};
	sprintf(dummyimagename, "%sdummy_test.bmp", m_RegistrationDirectionPath);
	sprintf(grabimagename, "D:/GrabFile/GrabImage%d.bmp", index);
	FILE*    pDummyFile = NULL;  //指向另一bmp文件，用于复制它的文件头和信息头数据
	FILE*    pWritingFile = NULL;  //指向要保存截图的bmp文件
	GLubyte* pPixelData;    //指向新的空的内存，用于保存截图bmp文件数据
	GLubyte  BMP_Header[BMP_Header_Length];
	GLint    i, j;
	GLint    PixelDataLength;   //BMP文件数据总长度

	// 计算像素数据的实际长度
	i = m_WinWidth * 4;   // 得到每一行的像素数据长度
	while (i % 4 != 0)      // 补充数据，直到i是的倍数
		++i;
	PixelDataLength = i * m_WinHeight;  //补齐后的总位数

	// 分配内存和打开文件
	pPixelData = (GLubyte*)malloc(PixelDataLength);
	if (pPixelData == 0)
		exit(0);

	fopen_s(&pDummyFile,dummyimagename, "rb");//只读形式打开
	if (pDummyFile == 0)
		exit(0);

	fopen_s(&pWritingFile, grabimagename, "wb"); //只写形式打开
	if (pWritingFile == 0)
		exit(0);

	//把读入的bmp文件的文件头和信息头数据复制，并修改宽高数据
	fread(BMP_Header, sizeof(BMP_Header), 1, pDummyFile);  //读取文件头和信息头，占据54字节
	fwrite(BMP_Header, sizeof(BMP_Header), 1, pWritingFile);
	fseek(pWritingFile, 0x0012, SEEK_SET); //移动到0X0012处，指向图像宽度所在内存
	i = m_WinWidth;
	j = m_WinHeight;
	fwrite(&i, sizeof(i), 1, pWritingFile);
	fwrite(&j, sizeof(j), 1, pWritingFile);

	// 读取当前画板上图像的像素数据
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);  //设置4位对齐方式
	glReadPixels(0, 0, m_WinWidth, m_WinHeight, GL_BGRA_EXT, GL_UNSIGNED_BYTE, pPixelData);

	// 写入像素数据
	fseek(pWritingFile, 0, SEEK_END);
	//把完整的BMP文件数据写入pWritingFile
	fwrite(pPixelData, PixelDataLength, 1, pWritingFile);

	// 释放内存和关闭文件
	fclose(pDummyFile);
	fclose(pWritingFile);
	free(pPixelData);
}

void CCarbonMed3DRecon::Functor(float *maskdata, float *maskinfo)
{
	Point3f refpt1(260.0f, 368.0f, 35.0f, 0.0f);
	Point3f refpt2(260.0f, 368.0f, 35.0f, 0.0f);
	Point3f refpt3(260.0f, 368.0f, 35.0f, 0.0f);
	Point3f refpt4(260.0f, 368.0f, 35.0f, 0.0f);
	InitRegRefPts(refpt1, refpt2, refpt3, refpt4);
	render_obj->SetInterpVoxelInfo((int)maskinfo[2], maskinfo[1]);
	render_obj->SetStartSliceNo(maskinfo[0]);
	render_obj->SetOrganData(maskdata);
	render_obj->SetMaskGridSize(WIDTH, HEIGHT, maskinfo[2]);
	render_obj->SetMaskRoiInfo(0, 0, 0, WIDTH, HEIGHT, maskinfo[0]);

	PrintLog("begin to run this Functor.");
	int count = 0;
	if (OpenGLWinFlag)
	{
		// register callbacks
		glutDisplayFunc(ImageRender);
		glutKeyboardFunc(keyboard);
		glutMouseFunc(mouse);
		glutMotionFunc(motion);
		glutIdleFunc(idle);
		glutReshapeFunc(reshape);

		VolumeInit();

		glutMainLoop();
	}
	else
	{
		bool done = false;

		VolumeInit();

		WaitingForRegFlag();
		PrintLog("WaitingForRegFlag end.");
		MSG msg;                                // Windowsx消息结构
		while (!done)
		{
			if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))           // 有消息在等待吗?
			{
				if (msg.message == WM_QUIT)               // 收到退出消息?
				{
					done = TRUE;                  // 是，则done=TRUE
				}
				else                            // 不是，处理窗口消息
				{
					TranslateMessage(&msg);             // 翻译消息
					DispatchMessage(&msg);              // 发送消息
				}
			}
			else
			{
				if (winactiveflag)                     // 程序激活的么?
				{
					wglMakeCurrent(0, 0);
					MakeCurrentWin(CarbonhDC, CarbonhRC);
					draw(m_WinWidth, m_WinHeight);
					m_GlWin->show();
					WinswapBuffers(CarbonhDC);
					wglMakeCurrent(0, 0);
					MakeCurrentWin(CarbonSlicehDC, CarbonSlicehRC);
					drawSlice(m_SliceWinW, m_SliceWinH);
					m_GlSliceWin->show();
					WinswapBuffers(CarbonSlicehDC);
					//grab(count);
					count++;
				}
			}
		}
		wglMakeCurrent(0, 0);             // unset RC
	}
	PrintLog("run this functor successfully.");
}

void CCarbonMed3DRecon::RenderingInit(float *maskdata, float *maskinfo)
{
	Point3f refpt1(260.0f, 368.0f, 35.0f, 0.0f);
	Point3f refpt2(260.0f, 368.0f, 35.0f, 0.0f);
	Point3f refpt3(260.0f, 368.0f, 35.0f, 0.0f);
	Point3f refpt4(260.0f, 368.0f, 35.0f, 0.0f);
	InitRegRefPts(refpt1, refpt2, refpt3, refpt4);
	render_obj->SetInterpVoxelInfo((int)maskinfo[2], maskinfo[1]);
	render_obj->SetStartSliceNo(maskinfo[0]);
	render_obj->SetOrganData(maskdata);
	render_obj->SetMaskGridSize(WIDTH, HEIGHT, maskinfo[2]);
	render_obj->SetMaskRoiInfo(0, 0, 0, WIDTH, HEIGHT, maskinfo[0]);

	PrintLog("Mask Rendering Setting Finished!");
	VolumeInit();
	PrintLog("3D Rendering Setting Finished!");
	WaitingForRegFlag();
	PrintLog("WaitingForRegFlag end.");
}

void CCarbonMed3DRecon::Showing3D()
{
	wglMakeCurrent(0, 0);
	MakeCurrentWin(CarbonhDC, CarbonhRC);
	draw(m_WinWidth, m_WinHeight);
	m_GlWin->show();
	WinswapBuffers(CarbonhDC);
}

bool CCarbonMed3DRecon::Get3DWindowActiveFlag()
{
	return CCarbonMed3DRecon::winactiveflag;
}


void CCarbonMed3DRecon::ShowingSlice()
{
	wglMakeCurrent(0, 0);
	MakeCurrentWin(CarbonSlicehDC, CarbonSlicehRC);
	drawSlice(m_SliceWinW, m_SliceWinH);
	m_GlSliceWin->show();
	WinswapBuffers(CarbonSlicehDC);
}

bool CCarbonMed3DRecon::GetSliceWindowActiveFlag()
{
	return CCarbonMed3DRecon::slicewinactiveflag;
}

void CCarbonMed3DRecon::ShowingUltrasound()
{

}

bool CCarbonMed3DRecon::GetUSWindowActiveFlag()
{
	return CCarbonMed3DRecon::uswinactiveflag;
}

void CCarbonMed3DRecon::InitRegRefPts(Point3f refpt1, Point3f refpt2, Point3f refpt3, Point3f refpt4)
{
	render_obj->InitRegRefPts(refpt1, refpt2, refpt3, refpt4);
}
void CCarbonMed3DRecon::InitPunctureLine(float ang, float plength, float refdis, float radius, float axis[3], int ind)
{
	render_obj->InitPunctureLine(ang, plength, refdis, radius, axis, ind);
}
void CCarbonMed3DRecon::Registration()
{
	render_obj->Registration();
}
void CCarbonMed3DRecon::SetRegistrationPt(int step, int ind)
{
	render_obj->SetRegistrationPt(step, ind);
}

void CCarbonMed3DRecon::WaitingForRegFlag()
{
	render_obj->WaitingForRegFlag();
}


//计算帧率
void CCarbonMed3DRecon::computeFPS()
{
	frameCount++;

	if (frameCount == fpsLimit)
	{
		char fps[256];
		float ifps = (float)frameCount / (float)(totalTime);
		sprintf(fps, "OPENCL Marching Cubes: %3.1f fps", ifps);

		glutSetWindowTitle(fps);

		frameCount = 0;
		totalTime = 0.0;
	}
}

// Helper function to return precision delta time for 3 counters since last call based upon host high performance counter
// *********************************************************************
double CCarbonMed3DRecon::shrDeltaT(int iCounterID = 0)
{
	// local var for computation of microseconds since last call
	double DeltaT;

	// Windows version of precision host timer
	// Variables that need to retain state between calls
	static LARGE_INTEGER liOldCount[3] = { { 0, 0 }, { 0, 0 }, { 0, 0 } };

	// locals for new count, new freq and new time delta 
	LARGE_INTEGER liNewCount, liFreq;
	if (QueryPerformanceFrequency(&liFreq))
	{
		// Get new counter reading
		QueryPerformanceCounter(&liNewCount);

		if (iCounterID >= 0 && iCounterID <= 2)
		{
			// Calculate time difference for timer 0.  (zero when called the first time) 
			DeltaT = liOldCount[iCounterID].LowPart ? (((double)liNewCount.QuadPart - (double)liOldCount[iCounterID].QuadPart) / (double)liFreq.QuadPart) : 0.0;
			// Reset old count to new
			liOldCount[iCounterID] = liNewCount;
		}
		else
		{
			// Requested counter ID out of range
			DeltaT = -9999.0;
		}

		// Returns time difference in seconds sunce the last call
		return DeltaT;
	}
	else
	{
		// No high resolution performance counter
		return -9999.0;
	}
}

void CCarbonMed3DRecon::ImageRender()
{
	shrDeltaT(0);

	render_obj->GLDataRendering(translate, clrotate, m_TextureOrderFlag);

	totalTime += shrDeltaT(0);

	computeFPS();
}

void CCarbonMed3DRecon::ImageRenderWgl()
{
	shrDeltaT(0);
	wglMakeCurrent(CarbonhDC, CarbonhRC);
	render_obj->GLDataRendering(translate, clrotate, m_TextureOrderFlag);
	glFlush();
	//取消当前线程选中的RC
	wglMakeCurrent(NULL, NULL);
	SwapBuffers(CarbonhDC);           // 交换缓存 (双缓存)       
	totalTime += shrDeltaT(0);
}

////////////////////////////////////////////////////////////////////////////////
//! Keyboard events handler
////////////////////////////////////////////////////////////////////////////////
void CCarbonMed3DRecon::keyboard(unsigned char key, int x, int y)
{
	render_obj->KeyBoard(key);
}

////////////////////////////////////////////////////////////////////////////////
//! Mouse event handlers
////////////////////////////////////////////////////////////////////////////////
void CCarbonMed3DRecon::mouse(int button, int state, int x, int y)
{
	if (state == GLUT_DOWN) {
		mouse_buttons |= 1 << button;
	}
	else if (state == GLUT_UP) {
		mouse_buttons = 0;
	}

	refxpos = x;
	refypos = y;
}

void CCarbonMed3DRecon::motion(int x, int y)
{
	float dx, dy;
	dx = (float)(x - refxpos);
	dy = (float)(y - refypos);

	if (mouse_buttons == 4)
	{
		// right = zoom
		translate[2] += dy / 100.0f;
	}
	else if (mouse_buttons == 2)
	{
		// middle = translate
		translate[0] += dx / 100.0f;
		translate[1] -= dy / 100.0f;
	}
	else if (mouse_buttons == 1)
	{
		// left = rotate
		clrotate[0] += dy / 5.0f;
		clrotate[1] += dx / 5.0f;
	}

	refxpos = x;
	refypos = y;
	//glutPostRedisplay();
}

void CCarbonMed3DRecon::idle()
{
	animation();
	glutPostRedisplay();
}

void CCarbonMed3DRecon::reshape(int w, int h)
{
	render_obj->ReshapeFunc(w, h, 0);
}

void CCarbonMed3DRecon::reshapeWgl(int w, int h)
{
	render_obj->ReshapeFunc(w, h, 1);
}

///////////////////////////////////////////////////////////////////////////////
// configure projection and viewport
///////////////////////////////////////////////////////////////////////////////
void CCarbonMed3DRecon::setViewport(int x, int y, int w, int h)
{
	// set viewport to be the entire window
	glViewport((GLsizei)x, (GLsizei)y, (GLsizei)w, (GLsizei)h);

	GLfloat fAspect = 0.0f;
	glMatrixMode(GL_PROJECTION);//先投影
	glLoadIdentity();
	if (w <= h)
	{
		fAspect = (GLfloat)h / (GLfloat)w;
		glOrtho(-glhalfworldsize*m_ZoomAspect, glhalfworldsize*m_ZoomAspect, -glhalfworldsize*fAspect*m_ZoomAspect,
			glhalfworldsize*fAspect*m_ZoomAspect, -5 * glhalfworldsize*m_ZoomAspect, 5 * glhalfworldsize*m_ZoomAspect);
	}
	else
	{
		fAspect = (GLfloat)w / (GLfloat)h;
		glOrtho((-glhalfworldsize)*fAspect*m_ZoomAspect, glhalfworldsize*fAspect*m_ZoomAspect, -glhalfworldsize*m_ZoomAspect,
			glhalfworldsize*m_ZoomAspect, -5 * glhalfworldsize*m_ZoomAspect, 5 * glhalfworldsize*m_ZoomAspect);
	}
	/*gluLookAt(m_EyeCenterPt.x, m_EyeCenterPt.y, m_EyeCenterPt.z, m_EyeFoucusCenterPt.x, m_EyeFoucusCenterPt.y, m_EyeFoucusCenterPt.z,
		m_HeadDirection.x, m_HeadDirection.y, m_HeadDirection.z);*/

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

///////////////////////////////////////////////////////////////////////////////
// draw 2D/3D scene
///////////////////////////////////////////////////////////////////////////////
void CCarbonMed3DRecon::draw(int winwidth, int winheight)
{
	shrDeltaT(0);
	RenderWinInfo(winwidth, winheight);
	draw3DReconstruction(winwidth, winheight);
	totalTime += shrDeltaT(0);
	drawAxisInterface();
}

void CCarbonMed3DRecon::drawSlice(int slicewinwidth, int slicewinheight)
{
	shrDeltaT(0);
	RenderWinInfo(slicewinwidth, slicewinheight);

	drawSliceImage();
	totalTime += shrDeltaT(0);
	//drawAxisInterface();
}

void CCarbonMed3DRecon::RenderWinInfo(int width, int height)
{
	// set bottom viewport
	//setViewport(100, 100, winwidth, winheight);
	setViewport(0, 0, width, height);

	// clear buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	char charFramRate[256] = { 0 };
	frameCount++;
	if (frameCount == fpsLimit)
	{
		m_IsGetFrameRate = true;
		char fps[256];
		float ifps = (float)frameCount / (float)(totalTime);
		sprintf(charFramRate, "Frame Rate: %3.1f fps", ifps);
		strFrameRate = charFramRate;
		frameCount = 0;
		totalTime = 0.0;
	}

	string strErrorRate = "Error Rate:";
	string winName = "3D Win";
	GLfloat wnamecolor[4] = { 0.5f, 0.5f, 0.0f, 1.0f };
	GLfloat frameratecolor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat errorRatecolor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glDepthFunc(GL_ALWAYS);     // to avoid visual artifacts with grid lines
	glDisable(GL_LIGHTING);
	drawString(winName.c_str(), winName.length(), wnamecolor, -glhalfworldsize*m_ZoomAspect + 30.0f, glhalfworldsize*m_ZoomAspect - 50.0f, 0.0f);
	drawString(strFrameRate.c_str(), strFrameRate.length(), frameratecolor, -glhalfworldsize*m_ZoomAspect + 30.0f, glhalfworldsize*m_ZoomAspect - 70.0f, 0.0f);
	drawString(strErrorRate.c_str(), strErrorRate.length(), errorRatecolor, -glhalfworldsize*m_ZoomAspect + 30.0f, glhalfworldsize*m_ZoomAspect - 90.0f, 0.0f);
	glEnable(GL_LIGHTING);
	glDepthFunc(GL_LEQUAL);
}

#define MAX_CHAR        128
///////////////////////////////////////////////////////////////////////////////
// draw string
///////////////////////////////////////////////////////////////////////////////
void CCarbonMed3DRecon::drawString(const char* str, int len, GLfloat strcolor[4], float drawstartx, float drawstarty, float drawstartz)
{
	static int isFirstCall = 1;
	static GLuint lists;

	glColor4f(strcolor[0], strcolor[1], strcolor[2], strcolor[3]);
	glRasterPos3f(drawstartx, drawstarty, drawstartz);
	if (isFirstCall) 
	{
		isFirstCall = 0;

		lists = glGenLists(MAX_CHAR);

		wglUseFontBitmaps(wglGetCurrentDC(), 0, MAX_CHAR, lists);
	}
	for (int i = 0; (i < len)&&(str[i] != '\n'); i ++, ++str)
	{
		glCallList(lists + *str);
	}
}

///////////////////////////////////////////////////////////////////////////////
// draw upper window (view from the camera)
///////////////////////////////////////////////////////////////////////////////
void CCarbonMed3DRecon::draw3DReconstruction(int winwidth, int winheight)
{
	GLfloat modelView[16];
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glPushMatrix();

	// First, transform the camera (viewing matrix) from world space to eye space
	glTranslatef(0, 0, -cameraDistance);
	glRotatef(cameraAngleX, 1, 0, 0); // pitch
	glRotatef(cameraAngleY, 0, 1, 0); // heading
	glGetFloatv(GL_MODELVIEW_MATRIX, modelView);
	render_obj->WGLDataRendering(m_TextureOrderFlag);
	render_obj->RegistrationProcessRendering(m_TextureOrderFlag);
	glPopMatrix();
	render_obj->SetModelView(modelView);
}

///////////////////////////////////////////////////////////////////////////////
// draw upper window (view from the camera)
///////////////////////////////////////////////////////////////////////////////
void CCarbonMed3DRecon::drawSliceImage()
{
	GLfloat modelView[16];
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glPushMatrix();
	glTranslatef(0, 0, -cameraDistance);
	//glRotatef(cameraAngleX, 1, 0, 0); // pitch
	//glRotatef(cameraAngleY, 0, 1, 0); // heading
	glGetFloatv(GL_MODELVIEW_MATRIX, modelView);
	render_obj->WGLSliceRendering(m_TextureOrderFlag);
	glPopMatrix();
	render_obj->SetModelView(modelView);
	drawAxisInterface();
}

///////////////////////////////////////////////////////////////////////////////
// draw RefAxisInterface
///////////////////////////////////////////////////////////////////////////////
void CCarbonMed3DRecon::drawAxisInterface()
{
	//-glhalfworldsize*m_ZoomAspect + 30.0f, glhalfworldsize*m_ZoomAspect - 70.0f
	GLfloat transxaxis = -glhalfworldsize*m_ZoomAspect + 60.0f;
	GLfloat transyaxis = -glhalfworldsize*m_ZoomAspect + 60.0f;
	glPushMatrix();
	glTranslatef(transxaxis, transyaxis, 0);
	glRotatef(cameraAngleX, 1, 0, 0); // pitch
	glRotatef(cameraAngleY, 0, 1, 0); // heading
	drawAxis(45.0f);
	glPopMatrix();
}

///////////////////////////////////////////////////////////////////////////////
// draw a grid on the xz plane
///////////////////////////////////////////////////////////////////////////////
void CCarbonMed3DRecon::drawGrid(float size, float step)
{
	// disable lighting
	glDisable(GL_LIGHTING);

	glBegin(GL_LINES);

	glColor3f(0.3f, 0.3f, 0.3f);
	for (float i = step; i <= size; i += step)
	{
		glVertex3f(-size, 0, i);   // lines parallel to X-axis
		glVertex3f(size, 0, i);
		glVertex3f(-size, 0, -i);   // lines parallel to X-axis
		glVertex3f(size, 0, -i);

		glVertex3f(i, 0, -size);   // lines parallel to Z-axis
		glVertex3f(i, 0, size);
		glVertex3f(-i, 0, -size);   // lines parallel to Z-axis
		glVertex3f(-i, 0, size);
	}

	// x-axis
	glColor3f(0.5f, 0, 0);
	glVertex3f(-size, 0, 0);
	glVertex3f(size, 0, 0);

	// z-axis
	glColor3f(0, 0, 0.5f);
	glVertex3f(0, 0, -size);
	glVertex3f(0, 0, size);

	glEnd();

	// enable lighting back
	glEnable(GL_LIGHTING);
}

void CCarbonMed3DRecon::drawAxisVBO(float size)
{

}


///////////////////////////////////////////////////////////////////////////////
// draw the local axis of an object
///////////////////////////////////////////////////////////////////////////////
void CCarbonMed3DRecon::drawAxis(float size)
{
	float arrowlen = size / 4.0f;
	float arrowangle = PI / 9.0f;
	string strxaxis = "x";
	string stryaxis = "y";
	string strzaxis = "z";
	GLfloat straxiscolor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat xaxiscolor[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
	GLfloat yaxiscolor[4] = { 0.0f, 1.0f, 0.0f, 1.0f };
	GLfloat zaxiscolor[4] = { 0.0f, 0.7f, 0.8f, 1.0f };
	glDepthFunc(GL_ALWAYS);     // to avoid visual artifacts with grid lines
	glDisable(GL_LIGHTING);
	//      the light position when you draw GL_LINES
	//      and GL_POINTS. remember the matrix.

	// draw axis
	glLineWidth(1.5);
	glBegin(GL_LINES);
	glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(size, 0, 0);

	glColor4f(0.0f, 1.0f, 0.0f, 1.0f);
	glVertex3f(0, 0, 0);
	glVertex3f(0, size, 0);
	
	glColor4f(0.0f, 0.7f, 0.8f, 1.0f);
	glVertex3f(0, 0, 0);
	glVertex3f(0, 0, size);
	
	glEnd();

	glLineWidth(1);

	// draw arrows on the end of the axis using cone
	GLfloat xvertspt[3];
	xvertspt[0] = size;
	xvertspt[1] = 0.0;
	xvertspt[2] = 0.0f;
	GLfloat yvertspt[3];
	yvertspt[0] = 0.0f;
	yvertspt[1] = size;
	yvertspt[2] = 0.0f;
	GLfloat zvertspt[3];
	zvertspt[0] = 0.0f;
	zvertspt[1] = 0.0f;
	zvertspt[2] = size;
	DrawingCone(xvertspt, xaxiscolor, arrowangle, arrowlen, 0);
	DrawingCone(yvertspt, yaxiscolor, arrowangle, arrowlen, 1);
	DrawingCone(zvertspt, zaxiscolor, arrowangle, arrowlen, 2);

	// restore default settings
	glDepthFunc(GL_LEQUAL);
	drawString(strxaxis.c_str(), strxaxis.length(), straxiscolor, size*1.2f, 0.0f, 0.0f);
	drawString(stryaxis.c_str(), stryaxis.length(), straxiscolor, 0.0f, size*1.2f, 0.0f);
	drawString(strzaxis.c_str(), strzaxis.length(), straxiscolor, 0.0f, 0.0f, size*1.2f);
}

///////////////////////////////////////////////////////////////////////////////
// set a perspective frustum with 6 params similar to glFrustum()
// (left, right, bottom, top, near, far)
// Note: this is for row-major notation. OpenGL needs transpose it
///////////////////////////////////////////////////////////////////////////////
void CCarbonMed3DRecon::setFrustum(float l, float r, float b, float t, float n, float f)
{
	matrixProjection.identity();
	matrixProjection[0] = 2 * n / (r - l);
	matrixProjection[2] = (r + l) / (r - l);
	matrixProjection[5] = 2 * n / (t - b);
	matrixProjection[6] = (t + b) / (t - b);
	matrixProjection[10] = -(f + n) / (f - n);
	matrixProjection[11] = -(2 * f * n) / (f - n);
	matrixProjection[14] = -1;
	matrixProjection[15] = 0;
}

///////////////////////////////////////////////////////////////////////////////
// set a symmetric perspective frustum with 4 params similar to gluPerspective
// (vertical field of view, aspect ratio, near, far)
///////////////////////////////////////////////////////////////////////////////
void CCarbonMed3DRecon::setFrustum(float fovY, float aspectRatio, float front, float back)
{
	float tangent = tanf(fovY / 2 * DEG2RAD);   // tangent of half fovY
	float height = front * tangent;           // half height of near plane
	float width = height * aspectRatio;       // half width of near plane

	// params: left, right, bottom, top, near, far
	setFrustum(-width, width, -height, height, front, back);
}

///////////////////////////////////////////////////////////////////////////////
// set a orthographic frustum with 6 params similar to glOrtho()
// (left, right, bottom, top, near, far)
// Note: this is for row-major notation. OpenGL needs transpose it
///////////////////////////////////////////////////////////////////////////////
void CCarbonMed3DRecon::setOrthoFrustum(float l, float r, float b, float t, float n, float f)
{
	matrixProjection.identity();
	matrixProjection[0] = 2 / (r - l);
	matrixProjection[3] = -(r + l) / (r - l);
	matrixProjection[5] = 2 / (t - b);
	matrixProjection[7] = -(t + b) / (t - b);
	matrixProjection[10] = -2 / (f - n);
	matrixProjection[11] = -(f + n) / (f - n);
}

int CCarbonMed3DRecon::command(int id, int cmd, LPARAM msg)
{
	return 0;
}

void CCarbonMed3DRecon::UILeftButtonDown(float x, float y)
{
	// update mouse position
	setMousePosition((int)x, (int)y);

	// set focus to receive wm_mousewheel event
	::SetFocus(CarbonhWnd);
}

void CCarbonMed3DRecon::UIMouseMoveInterface(float x, float y)
{
	rotateCamera((int)x, (int)y);
}
//UI KeyBoard Event
void CCarbonMed3DRecon::UIKeyEvent(int key)
{
	render_obj->KeyBoard(key);
}
//UI Get slice
void CCarbonMed3DRecon::UIGetSliceData(float *buf, int len)
{
	render_obj->GetNomalizedSliceData(buf);
}
//UI Set Ultrasound
void CCarbonMed3DRecon::UISetUltraSoundBuf(float *buf, int len)
{
	render_obj->SetUltraSoundBuf(buf, len);
}
//UI Get NDI Result
void CCarbonMed3DRecon::UIGetNDIResult(float &x, float &y, float &z, float &azimuth, float &elevation, float &roll)
{
	PositionAngleUnit *precord = 0;
	render_obj->GetNDIRecord(precord);
	x = precord->x;
	y = precord->y;
	z = precord->z;
	azimuth = precord->a;
	elevation = precord->e;
	roll = precord->r;
}
//UI Draw Result
void CCarbonMed3DRecon::UIDrawOpenGLResult()
{
	draw(m_WinWidth, m_WinHeight);
}

///////////////////////////////////////////////////////////////////////////////
// handle Left mouse down
///////////////////////////////////////////////////////////////////////////////
int CCarbonMed3DRecon::lButtonDown(WPARAM state, int x, int y)
{
	// update mouse position
	setMousePosition(x, y);

	// set focus to receive wm_mousewheel event
	::SetFocus(CarbonhWnd);

	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// handle Left mouse up
///////////////////////////////////////////////////////////////////////////////
int CCarbonMed3DRecon::lButtonUp(WPARAM state, int x, int y)
{
	// update mouse position
	setMousePosition(x, y);
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// handle reft mouse down
///////////////////////////////////////////////////////////////////////////////
int CCarbonMed3DRecon::rButtonDown(WPARAM state, int x, int y)
{
	// update mouse position
	setMousePosition(x, y);

	// set focus to receive wm_mousewheel event
	::SetFocus(CarbonhWnd);

	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// handle reft mouse up
///////////////////////////////////////////////////////////////////////////////
int CCarbonMed3DRecon::rButtonUp(WPARAM state, int x, int y)
{
	// update mouse position
	setMousePosition(x, y);
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// handle WM_MOUSEMOVE
///////////////////////////////////////////////////////////////////////////////
int CCarbonMed3DRecon::mouseMove(WPARAM state, int x, int y)
{
	if (state == MK_LBUTTON)
	{
		rotateCamera(x, y);
	}
	if (state == MK_RBUTTON)
	{
		zoomCamera(y);
	}
	//motion(x, y);

	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// handle WM_MOUSEWHEEL
///////////////////////////////////////////////////////////////////////////////
int CCarbonMed3DRecon::mouseWheel(int state, int delta, int x, int y)
{
	//zoomCameraDelta(delta);
	ZoomScale(delta);
	return 0;
}

int CCarbonMed3DRecon::keyDown(int key, LPARAM lParam) 
{ 
	render_obj->KeyBoard(key);
	return 0; 
}

int CCarbonMed3DRecon::keyUp(int key, LPARAM lParam) 
{ 
	return 0; 
}

int CCarbonMed3DRecon::mButtonDown(WPARAM wParam, int x, int y)
{ 
	return 0; 
}

int CCarbonMed3DRecon::mButtonUp(WPARAM wParam, int x, int y)
{ 
	return 0; 
}

///////////////////////////////////////////////////////////////////////////////
// handle WM_SIZE
///////////////////////////////////////////////////////////////////////////////
int CCarbonMed3DRecon::size(int w, int h, WPARAM wParam)
{
	//model->setWindowSize(w, h);
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// handle WM_DESTROY
///////////////////////////////////////////////////////////////////////////////
int CCarbonMed3DRecon::destroy()
{
	// close OpenGL rendering context (RC)
	closeContext(CarbonhWnd, CarbonhDC, CarbonhRC);
	closeContext(CarbonSlicehWnd, CarbonSlicehDC, CarbonSlicehRC);

	return 0;
}
///////////////////////////////////////////////////////////////////////////////
// rotate the camera for subWin2 (3rd person view)
///////////////////////////////////////////////////////////////////////////////
void CCarbonMed3DRecon::rotateCamera(int x, int y)
{
	cameraAngleY += (x - refxpos);
	cameraAngleX += (y - refypos);
	
	//clrotate[1] += (x - refxpos);
	//clrotate[0] += (y - refypos);
	//clrotate[0] += dy / 5.0f;
	//clrotate[1] += dx / 5.0f;
	refxpos = x;
	refypos = y;
}
///////////////////////////////////////////////////////////////////////////////
// zoom the camera for subWin2 (3rd person view)
///////////////////////////////////////////////////////////////////////////////
void CCarbonMed3DRecon::zoomCamera(int y)
{
	cameraDistance -= (y - refypos) * 0.1f;
	refypos = y;
}

void CCarbonMed3DRecon::zoomCameraDelta(int delta)
{
	cameraDistance -= delta;
}

void CCarbonMed3DRecon::ZoomScale(int flag)
{
	if (flag > 0)
	{
		render_obj->ZoomUp();
	}
	else
	{
		render_obj->ZoomDown();
	}
}

///////////////////////////////////////////////////////////////////////////////
// update matrix
///////////////////////////////////////////////////////////////////////////////
void CCarbonMed3DRecon::updateViewMatrix()
{
	// transform the camera (viewing matrix) from world space to eye space
	// Notice all values are negated, because we move the whole scene with the
	// inverse of camera transform    matrixView.identity();
	matrixView.identity();
	matrixView.translate(-cameraPosition[0], -cameraPosition[1], -cameraPosition[2]);
	matrixView.rotateX(-cameraAngle[0]);    // pitch
	matrixView.rotateY(-cameraAngle[1]);    // heading
	matrixView.rotateZ(-cameraAngle[2]);    // roll

	matrixModelView = matrixView * matrixModel;
}

void CCarbonMed3DRecon::updateModelMatrix()
{
	// transform objects from object space to world space
	matrixModel.identity();
	matrixModel.rotateZ(modelAngle[2]);
	matrixModel.rotateY(modelAngle[1]);
	matrixModel.rotateX(modelAngle[0]);
	matrixModel.translate(modelPosition[0], modelPosition[1], modelPosition[2]);

	matrixModelView = matrixView * matrixModel;
}


void CCarbonMed3DRecon::setMousePosition(int x, int y)
{ 
	refxpos = x; 
	refypos = y; 
}

void CCarbonMed3DRecon::setCameraX(float x)
{ 
	cameraPosition[0] = x; 
	updateViewMatrix(); 
}

void CCarbonMed3DRecon::setCameraY(float y)
{ 
	cameraPosition[1] = y; 
	updateViewMatrix(); 
}

void CCarbonMed3DRecon::setCameraZ(float z)
{ 
	cameraPosition[2] = z; 
	updateViewMatrix(); 
}

void CCarbonMed3DRecon::setCameraAngleX(float p)
{ 
	cameraAngle[0] = p; 
	updateViewMatrix(); 
}

void CCarbonMed3DRecon::setCameraAngleY(float h)
{ 
	cameraAngle[1] = h; 
	updateViewMatrix(); 
}

void CCarbonMed3DRecon::setCameraAngleZ(float r)
{ 
	cameraAngle[2] = r; 
	updateViewMatrix(); 
}

float CCarbonMed3DRecon::getCameraX()
{ 
	return cameraPosition[0]; 
}

float CCarbonMed3DRecon::getCameraY()
{ 
	return cameraPosition[1]; 
}

float CCarbonMed3DRecon::getCameraZ()              
{ 
	return cameraPosition[2]; 
}

float CCarbonMed3DRecon::getCameraAngleX()         
{ 
	return cameraAngle[0]; 
}

float CCarbonMed3DRecon::getCameraAngleY()         
{ 
	return cameraAngle[1]; 
}

float CCarbonMed3DRecon::getCameraAngleZ()
{ 
	return cameraAngle[2]; 
}

void CCarbonMed3DRecon::setModelX(float x)         
{
	modelPosition[0] = x; 
	updateModelMatrix(); 
}

void CCarbonMed3DRecon::setModelY(float y)         
{
	modelPosition[1] = y; 
	updateModelMatrix(); 
}

void CCarbonMed3DRecon::setModelZ(float z)         
{ 
	modelPosition[2] = z; 
	updateModelMatrix(); 
}

void CCarbonMed3DRecon::setModelAngleX(float a)    
{
	modelAngle[0] = a; 
	updateModelMatrix(); 
}

void CCarbonMed3DRecon::setModelAngleY(float a)    
{ 
	modelAngle[1] = a; 
	updateModelMatrix(); 
}

void CCarbonMed3DRecon::setModelAngleZ(float a)    
{ 
	modelAngle[2] = a; 
	updateModelMatrix(); 
}

float CCarbonMed3DRecon::getModelX()               
{ 
	return modelPosition[0]; 
}

float CCarbonMed3DRecon::getModelY()               
{ 
	return modelPosition[1]; 
}

float CCarbonMed3DRecon::getModelZ()               
{ 
	return modelPosition[2]; 
}

float CCarbonMed3DRecon::getModelAngleX()          
{ 
	return modelAngle[0]; 
}

float CCarbonMed3DRecon::getModelAngleY()          
{ 
	return modelAngle[1]; 
}

float CCarbonMed3DRecon::getModelAngleZ()          
{ 
	return modelAngle[2]; 
}

// return 16 elements of  target matrix
const float* CCarbonMed3DRecon::getViewMatrixElements()        
{ 
	return matrixView.get(); 
}

const float* CCarbonMed3DRecon::getModelMatrixElements()       
{ 
	return matrixModel.get(); 
}

const float* CCarbonMed3DRecon::getModelViewMatrixElements()   
{ 
	return matrixModelView.get(); 
}

const float* CCarbonMed3DRecon::getProjectionMatrixElements()  
{ 
	return matrixProjection.get(); 
}

//animation
void CCarbonMed3DRecon::animation()
{
	RenderParam param;
	render_obj->animation();
	render_obj->GetRenderingFeature(param);
	render_obj->SetRenderingFeature(param);
}