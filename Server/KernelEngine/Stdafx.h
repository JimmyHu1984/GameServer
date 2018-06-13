#pragma once
#pragma warning(disable:4996)
#pragma warning(disable:4800)

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN		// 从 Windows 头中排除极少使用的资料
#endif

// 如果您必须使用下列所指定的平台之前的平台，则修改下面的定义。
// 有关不同平台的相应值的最新信息，请参考 MSDN。
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

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// 某些 CString 构造函数将为显式的

#include <afxwin.h>         // MFC 核心组件和标准组件
#include <afxext.h>         // MFC 扩展

#ifndef _AFX_NO_OLE_SUPPORT
#include <afxole.h>         // MFC OLE 类
#include <afxodlgs.h>       // MFC OLE 对话框类
#include <afxdisp.h>        // MFC 自动化类
#endif // _AFX_NO_OLE_SUPPORT

// #ifndef _AFX_NO_DB_SUPPORT
// #include <afxdb.h>			// MFC ODBC 数据库类
// #endif // _AFX_NO_DB_SUPPORT

// #ifndef _AFX_NO_DAO_SUPPORT
// #include <afxdao.h>			// MFC DAO 数据库类
// #endif // _AFX_NO_DAO_SUPPORT

#include <afxdtctl.h>		// MFC 对 Internet Explorer 4 公共控件的支持
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC 对 Windows 公共控件的支持
#endif // _AFX_NO_AFXCMN_SUPPORT


#ifndef _DEBUG
#ifndef _UNICODE
	#pragma comment (lib,"../../../Lib/Ansi/ServiceCore.lib")
#else
	#pragma comment (lib,"../../../Lib/Unicode/ServiceCore.lib")
#endif
#else
#ifndef _UNICODE
	#pragma comment (lib,"../../../Lib/Ansi/ServiceCoreD.lib")
#else
	#pragma comment (lib,"../../../Lib/Unicode/ServiceCoreD.lib")
#endif
#endif
