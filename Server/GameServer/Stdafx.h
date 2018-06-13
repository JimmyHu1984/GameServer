#pragma once
#pragma warning(disable:4996)
#pragma warning(disable:4800)

//////////////////////////////////////////////////////////////////////////////////
//MFC 文件

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN
#endif

#ifndef WINVER
#define WINVER 0x0600
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif						

#ifndef _WIN32_WINDOWS
#define _WIN32_WINDOWS 0x0610
#endif

#ifndef _WIN32_IE
#define _WIN32_IE 0x0600
#endif

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS

#define _AFX_ALL_WARNINGS

#include <AfxWin.h>
#include <AfxExt.h>
#include <AfxDisp.h>
#include <AfxDtctl.h>

#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <AfxCmn.h>
#endif

#ifndef TEST_KERNELENGINE
//#define TEST_KERNELENGINE
#endif


//////////////////////////////////////////////////////////////////////////////////
//包含文件

//平台定义
#include "..\..\Global\Platform.h"
#include "..\..\Protocol\CMD_Correspond.h"
#include "..\..\Protocol\CMD_GameServer.h"
#include "..\..\Protocol\CMD_LogonServer.h"
#include "..\..\Module\DebugViewHandler\DebugViewHandler.h"

//组件定义
#include "..\..\Public\ServiceCore\ServiceCoreHead.h"
#include "GameServiceHead.h"
//#include "..\..\Server\MatchServer\MatchServiceHead.h"
#ifndef TEST_KERNELENGINE
#include "KernelEngineHead.h"
#else
#include "..\..\Server\6603内核引擎源码\KernelEngineHead.h"
#endif
#include "..\..\Server\ModuleManager\ModuleManagerHead.h"

//////////////////////////////////////////////////////////////////////////////////
//链接代码

#ifndef _DEBUG
#ifndef _UNICODE
	#pragma comment (lib,"../../../Lib/Ansi/ServiceCore.lib")
	#pragma comment (lib,"../../../Lib/Ansi/GameService.lib")
	#pragma comment (lib,"../../../Lib/Ansi/KernelEngine.lib")
	#pragma comment (lib,"../../../Lib/Ansi/ModuleManager.lib")
#else
	#pragma comment (lib,"../../../Lib/Unicode/ServiceCore.lib")
	#pragma comment (lib,"../../../Lib/Unicode/GameService.lib")
	#pragma comment (lib,"../../../Lib/Unicode/KernelEngine.lib")
	#pragma comment (lib,"../../../Lib/Unicode/ModuleManager.lib")
#endif
#else


#ifndef _UNICODE
	#pragma comment (lib,"../../../Lib/Ansi/ServiceCoreD.lib")
	#pragma comment (lib,"../../../Lib/Ansi/GameServiceD.lib")
	#pragma comment (lib,"../../../Lib/Ansi/KernelEngineD.lib")
	#pragma comment (lib,"../../../Lib/Ansi/ModuleManagerD.lib")
#else
	#pragma comment (lib,"../../../Lib/Unicode/ServiceCoreD.lib")
	#pragma comment (lib,"../../../Lib/Unicode/GameServiceD.lib")
	#ifdef TEST_KERNELENGINE
	#pragma comment (lib,"../../../Lib/KernelEngineD.lib")
	#else
	#pragma comment (lib,"../../../Lib/Unicode/KernelEngineD.lib")
	#endif
	#pragma comment (lib,"../../../Lib/Unicode/ModuleManagerD.lib")
#endif
#endif

//////////////////////////////////////////////////////////////////////////////////
