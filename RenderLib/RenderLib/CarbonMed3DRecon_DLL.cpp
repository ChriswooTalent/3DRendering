#include "CarbonMed3DRecon_DLL.h"
#include "CarbonMed3DRecon.h"

CARBONMED3DRECON_API int  Initial3DReconResource(float *dicominfo, float *dicomdata, int framecount, HWND cwnd[3], int *_3dwindowinfo, int *slicewindowinfow, int *uswindowinfo);
CARBONMED3DRECON_API void Release3DReconResource();
CARBONMED3DRECON_API void Call3DRecon(float *maskdata, float *maskinfo);
CARBONMED3DRECON_API void RenderingInit(float *maskdata, float *maskinfo);
CARBONMED3DRECON_API void Showing3D();
CARBONMED3DRECON_API bool Get3DWindowActiveFlag();
CARBONMED3DRECON_API void ShowingSlice();
CARBONMED3DRECON_API bool GetSliceWindowActiveFlag();
CARBONMED3DRECON_API void ShowingUltrasound();
CARBONMED3DRECON_API bool GetUSWindowActiveFlag();
CARBONMED3DRECON_API void UIMouseLeftButtonDown(float x, float y);
CARBONMED3DRECON_API void UIMouseMoveInterface(float x, float y);
CARBONMED3DRECON_API void UIKeyEvent(int key);
CARBONMED3DRECON_API void UIGetSliceData(float *buf, int len);
CARBONMED3DRECON_API void UISetUltraSoundBuf(float *buf, int len);

// Initial Functor resource
int Initial3DReconResource(float *dicominfo, float *dicomdata, int framecount, HWND cwnd[3], int *_3dwindowinfo, int *slicewindowinfow, int *uswindowinfo)
{
	return CCarbonMed3DRecon::InitialFunctorTest(dicominfo, dicomdata, framecount, cwnd, _3dwindowinfo, slicewindowinfow, uswindowinfo);
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

void RenderingInit(float *maskdata, float *maskinfo)
{
	CCarbonMed3DRecon::RenderingInit(maskdata, maskinfo);
}

void Showing3D()
{
	CCarbonMed3DRecon::Showing3D();
}

bool Get3DWindowActiveFlag()
{
	return CCarbonMed3DRecon::Get3DWindowActiveFlag();
}

void ShowingSlice()
{
	CCarbonMed3DRecon::ShowingSlice();
}

bool GetSliceWindowActiveFlag()
{
	return CCarbonMed3DRecon::GetSliceWindowActiveFlag();
}

void ShowingUltrasound()
{
	CCarbonMed3DRecon::ShowingUltrasound();
}

bool GetUSWindowActiveFlag()
{
	return CCarbonMed3DRecon::GetUSWindowActiveFlag();
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
//UI Set Ultrasound Buf
void UISetUltraSoundBuf(float *buf, int len)
{
	CCarbonMed3DRecon::UISetUltraSoundBuf(buf, len);
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