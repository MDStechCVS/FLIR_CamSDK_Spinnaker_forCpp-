// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

#pragma once

#ifndef _SECURE_ATL
#define _SECURE_ATL 1
#endif

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN        // Exclude rarely-used stuff from Windows headers
#endif

#include "targetver.h"

                  
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS  // 일부 CString 생성자는 명시적으로 선언됩니다.

// turns off MFC's hiding of some common and often safely ignored warning messages
#define _AFX_ALL_WARNINGS

#ifdef PT_VLD
#include <vld.h>
#endif

#include <afxwin.h>         // MFC 핵심 및 표준 구성 요소입니다.
#include <afxext.h>         // MFC 확장입니다.
#include <afxmt.h>
#include <stdint.h>


#include <afxdisp.h>        // MFC 자동화 클래스입니다.


#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>           // Internet Explorer 4 공용 컨트롤에 대한 MFC 지원입니다.
#endif

#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>         // Windows 공용 컨트롤에 대한 MFC 지원입니다.
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxcontrolbars.h> // MFC의 리본 및 컨트롤 막대 지원


//#include <GdiPlus.h> 
//using namespace Gdiplus;
//#pragma comment(lib, "gdiplus.lib") 

#include <stdio.h>
#include <complex>
#include <cmath>
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <thread>
#include <windows.h>
#include <complex>
#include <regex>
#include <winsock2.h>
#include <iphlpapi.h>
#include <WS2tcpip.h>
#include "mmsystem.h"
#include <ctime>
#include <sys/timeb.h>
#include <condition_variable>

#include <Spinnaker.h>
#include <SpinGenApi/SpinnakerGenApi.h>

#include <opencv\cv.h>
#include <opencv\highgui.h>
#include <opencv2/opencv.hpp>
#include <opencv2\core.hpp>
#include <opencv2/ImgProc.hpp>

#include "fff.h"
#include "MDSColorpalette.h"

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")

#define WM_UPDATE_IMAGE (WM_USER + 1)
#define WM_DISPLAY_IMAGE (WM_USER + 2)

using namespace std;
using namespace cv;

#define RGB_BLACK			RGB(0,0,0)
#define RGB_RED				RGB(255,0,0)
#define RGB_BLUE			RGB(0,0,255)
#define RGB_GREEN			RGB(0,255,0)
#define RGB_WHITE			RGB(255,255,255)
#define RGBYELLOW			RGB(255,255,0)
#define RGB_SKYBLUE			RGB(0,128,255)
#define RGB_PURPLE			RGB(255,0,255)

#define NOMINMAX 

#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif


