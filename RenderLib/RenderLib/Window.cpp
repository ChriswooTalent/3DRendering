///////////////////////////////////////////////////////////////////////////////
// Window.cpp
// ==========
// A class of Window for MS Windows
// It registers window class(WNDCLASSEX) with RegisterClassEx() and creates a 
// window with CreateWindowEx() API call.
//
//  AUTHOR: Song Ho Ahn
// CREATED: 2005-03-16
// UPDATED: 2013-01-10
///////////////////////////////////////////////////////////////////////////////

#pragma warning(disable : 4996)
#include <sstream>
#include <iostream>
#include <cstring>
#include "Window.h"
using std::wstringstream;
using std::wcout;
using std::endl;
using namespace Win;
#define WINWIDTH 512
#define WINHEIGHT 512




///////////////////////////////////////////////////////////////////////////////
// constructor with params
// NOTE: Windows does not clip a child window from the parent client's area.
// To prevent the parent window from drawing over its child window area, the
// parent window must have WS_CLIPCHILDREN flag.
///////////////////////////////////////////////////////////////////////////////
Window::Window(WNDPROC _procfunc) : handle(0), winStyle(WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN),
                                                 winStyleEx(WS_EX_CLIENTEDGE), x(CW_USEDEFAULT), y(CW_USEDEFAULT),
												 width(WINWIDTH), height(WINHEIGHT),
                                                 menuHandle(0)
{
	instance = GetModuleHandle(NULL);
	parentHandle = NULL;
    // copy string
	wcsncpy(this->title, L"WindowGL", MAX_STRING - 1);
	wcsncpy(this->className, L"WindowGL", MAX_STRING - 1);

    // populate window class struct
    winClass.cbSize        = sizeof(WNDCLASSEX);
    winClass.style         = 0;                                     // class styles: CS_OWNDC, CS_PARENTDC, CS_CLASSDC, CS_GLOBALCLASS, ...
	winClass.lpfnWndProc   = _procfunc;                  // pointer to window procedure
    winClass.cbClsExtra    = 0;
    winClass.cbWndExtra    = 0;
	winClass.hInstance     = instance;                              // owner of this class
    winClass.hIcon         = LoadIcon(NULL, IDI_APPLICATION);   // default icon
    winClass.hIconSm       = 0;
    winClass.hCursor       = LoadCursor(0, IDC_ARROW);              // default arrow cursor
    winClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);   // default white brush
    winClass.lpszMenuName  = 0;
    winClass.lpszClassName = className;
    winClass.hIconSm       = LoadIcon(instance, IDI_APPLICATION);   // default small icon
}



///////////////////////////////////////////////////////////////////////////////
// destructor
///////////////////////////////////////////////////////////////////////////////
Window::~Window()
{
    ::UnregisterClass(className, instance);
}



///////////////////////////////////////////////////////////////////////////////
// create a window
///////////////////////////////////////////////////////////////////////////////
HWND Window::create()
{
    // register a window class
    if(!::RegisterClassEx(&winClass)) 
		return 0;

    handle = ::CreateWindowEx(winStyleEx,           // window border with a sunken edge
                              className,            // name of a registered window class
                              title,                // caption of window
                              winStyle,             // window style
                              x,                    // x position
                              y,                    // y position
                              width,                // witdh
                              height,               // height
                              parentHandle,         // handle to parent window
                              menuHandle,           // handle to menu
                              instance,             // application instance
                              NULL);  // window creation data

    //this->show(SW_SHOWDEFAULT);                     // make it visible

    return handle;
}

HWND Window::InitWithExistedWindow(HWND winhandle)
{
	if (!::RegisterClassEx(&winClass))
		return 0;

	handle = ::CreateWindowEx(winStyleEx,           // window border with a sunken edge
		className,            // name of a registered window class
		title,                // caption of window
		winStyle,             // window style
		x,                    // x position
		y,                    // y position
		width,                // witdh
		height,               // height
		parentHandle,         // handle to parent window
		menuHandle,           // handle to menu
		instance,             // application instance
		NULL);  // window creation data
	handle = winhandle;
	return handle;
}



///////////////////////////////////////////////////////////////////////////////
// show the window on the screen
///////////////////////////////////////////////////////////////////////////////
void Window::show(int cmdShow)
{
    ::ShowWindow(handle, cmdShow);
    ::UpdateWindow(handle);
}

void Window::showExistedWindow(HWND winhandle, int cmdShow)
{
	::ShowWindow(winhandle, cmdShow);
	::UpdateWindow(winhandle);
}



///////////////////////////////////////////////////////////////////////////////
// print itself
///////////////////////////////////////////////////////////////////////////////
void Window::printSelf() const
{
    wstringstream wss;                          // wide char output string stream buffer

    // build output string
    wss << L"=== Win::Window object ===\n"
        << L"Name: " << title << L"\n"
        << L"Position: (" << x << L", " << y << L")\n"
        << L"Width: " << width << L"\n"
        << L"Height: " << height << L"\n"
        << L"Handle: " << handle << L"\n"
        << L"Parent Handle: " << parentHandle << L"\n"
        << L"Menu Handle: " << menuHandle << L"\n"
        << L"Instance: " << instance << L"\n"
        << endl;

    wcout << wss.str();                         // print the string to the console
}



///////////////////////////////////////////////////////////////////////////////
// load an icon using resource ID and convert it to HICON
///////////////////////////////////////////////////////////////////////////////
HICON Window::loadIcon(int id)
{
    return (HICON)::LoadImage(instance, MAKEINTRESOURCE(id), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE);
}



///////////////////////////////////////////////////////////////////////////////
// load an icon using resource ID and convert it to HICON
///////////////////////////////////////////////////////////////////////////////
HICON Window::loadCursor(int id)
{
    return (HCURSOR)::LoadImage(instance, MAKEINTRESOURCE(id), IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE);
}
