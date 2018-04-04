#include "CarbonMed3DRecon_DLL.h"
#include "CarbonMed3DRecon.h"

CARBONMED3DRECON_API int  Initial3DReconResource(float *dicominfo, float *dicomdata, int framecount, HWND cur3Dwin, int startx, int starty, int winwidth, int winheight);
CARBONMED3DRECON_API void Release3DReconResource();
CARBONMED3DRECON_API void Call3DRecon(float *maskdata, float *maskinfo);
CARBONMED3DRECON_API void UIMouseLeftButtonDown(float x, float y);
CARBONMED3DRECON_API void UIMouseMoveInterface(float x, float y);
CARBONMED3DRECON_API void UIKeyEvent(int key);
CARBONMED3DRECON_API void UIGetSliceData(float *buf, int len);

// Initial Functor resource
int Initial3DReconResource(float *dicominfo, float *dicomdata, int framecount, HWND cur3Dwin, int startx, int starty, int winwidth, int winheight)
{
	return CCarbonMed3DRecon::InitialFunctorTest(dicominfo, dicomdata, framecount, cur3Dwin, startx, starty, winwidth, winheight);
}
// Release Functor resource
void Release3DReconResource()
{
	CCarbonMed3DRecon::ReleaseFunctor();
}
// functor
void Call3DRecon(float *maskdata, float *maskinfo)
{
	CCarbonMed3DRecon::Functor(maskdata, maskinfo);
}

//UI Mouse Event
void UIMouseLeftButtonDown(float x, float y)
{
	CCarbonMed3DRecon::UILeftButtonDown(x, y);
}

//UI Mouse Move Event
void UIMouseMoveInterface(float x, float y)
{
	CCarbonMed3DRecon::UIMouseMoveInterface(x, y);
}

//UI KeyBoard Event
void UIKeyEvent(int key)
{
	CCarbonMed3DRecon::UIKeyEvent(key);
}
//UI Get slice
void UIGetSliceData(float *buf, int len)
{
	CCarbonMed3DRecon::UIGetSliceData(buf, len);
}
//UI Get NDI Result
void UIGetNDIResult(float &x, float &y, float &z, float &azimuth, float &elevation, float &roll)
{
	CCarbonMed3DRecon::UIGetNDIResult(x, y, z, azimuth, elevation, roll);
}
//UI Draw Result
void UIDrawOpenGLResult()
{
	CCarbonMed3DRecon::UIDrawOpenGLResult();
}