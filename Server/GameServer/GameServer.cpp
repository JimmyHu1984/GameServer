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
	���� GLS USER LOGON ʱ����½ʧ�ܵ���ϰ����
*/

//////////////////////////////////////////////////////////////////////////////////

//�������
CGameServerApp theApp;

//////////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CGameServerApp, CWinApp)
END_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////////////////

//���캯��
CGameServerApp::CGameServerApp()
{
}

struct CMD_S_CallStart
{
	BYTE                                lCallTime;                          //����ʱ
	BYTE                                bGameCode[18];                      //�ƾֱ��
	LONGLONG                            testScore[5];
};	

BOOL CGameServerApp::InitInstance()
{
	__super::InitInstance();

	DeclareDumpFile();//write dump 

// 	AllocConsole();
// 	freopen( "CONOUT$", "w+t", stdout );// ����д
// 	freopen( "CONIN$", "r+t", stdin );  // �����

	ILog4zManager::GetInstance()->Start();
	LOGI(TEXT("start to log"))

	//�������
	AfxInitRichEdit2();
	InitCommonControls();
	AfxEnableControlContainer();

	//����ע���
	SetRegistryKey(szProduct);	
	//��ʾ����
	CGameServerDlg GameServerDlg;
	m_pMainWnd=&GameServerDlg;
	GameServerDlg.DoModal();

	//ILog4zManager::GetInstance()->Stop();

	return FALSE;
}

//////////////////////////////////////////////////////////////////////////////////
