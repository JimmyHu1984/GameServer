#pragma once
#pragma warning(disable:4996)
#pragma warning(disable:4800)

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN		// �� Windows ͷ���ų�����ʹ�õ�����
#endif

// ���������ʹ��������ָ����ƽ̨֮ǰ��ƽ̨�����޸�����Ķ��塣
// �йز�ͬƽ̨����Ӧֵ��������Ϣ����ο� MSDN��
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

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// ĳЩ CString ���캯����Ϊ��ʽ��

#include <afxwin.h>         // MFC ��������ͱ�׼���
#include <afxext.h>         // MFC ��չ

#ifndef _AFX_NO_OLE_SUPPORT
#include <afxole.h>         // MFC OLE ��
#include <afxodlgs.h>       // MFC OLE �Ի�����
#include <afxdisp.h>        // MFC �Զ�����
#endif // _AFX_NO_OLE_SUPPORT

// #ifndef _AFX_NO_DB_SUPPORT
// #include <afxdb.h>			// MFC ODBC ���ݿ���
// #endif // _AFX_NO_DB_SUPPORT

// #ifndef _AFX_NO_DAO_SUPPORT
// #include <afxdao.h>			// MFC DAO ���ݿ���
// #endif // _AFX_NO_DAO_SUPPORT

#include <afxdtctl.h>		// MFC �� Internet Explorer 4 �����ؼ���֧��
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC �� Windows �����ؼ���֧��
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
