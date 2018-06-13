#include "Stdafx.h"
#include "GameServer.h"
#include "GameServerDlg.h"
#include "DumpFile.h"
using namespace web;
using namespace web::http;
using namespace web::http::client;

#include <iostream>
using namespace std;


/*
	TCG-11940:
	增加 GLS USER LOGON 时，登陆失败的信习处理
*/

//////////////////////////////////////////////////////////////////////////////////

//程序对象
CGameServerApp theApp;

//////////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CGameServerApp, CWinApp)
END_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////////////////

//构造函数
CGameServerApp::CGameServerApp()
{
}

struct CMD_S_CallStart
{
	BYTE                                lCallTime;                          //倒计时
	BYTE                                bGameCode[18];                      //牌局编号
	LONGLONG                            testScore[5];
};	

BOOL CGameServerApp::InitInstance()
{
	__super::InitInstance();

	DeclareDumpFile();//write dump 

// 	AllocConsole();
// 	freopen( "CONOUT$", "w+t", stdout );// 申请写
// 	freopen( "CONIN$", "r+t", stdin );  // 申请读

	ILog4zManager::GetInstance()->Start();
	LOGI(TEXT("start to log"))

	//设置组件
	AfxInitRichEdit2();
	InitCommonControls();
	AfxEnableControlContainer();

	//设置注册表
	SetRegistryKey(szProduct);	
	//显示窗口
	CGameServerDlg GameServerDlg;
	m_pMainWnd=&GameServerDlg;
	GameServerDlg.DoModal();

	//ILog4zManager::GetInstance()->Stop();

	return FALSE;
}

//////////////////////////////////////////////////////////////////////////////////
