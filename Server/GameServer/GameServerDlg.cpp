#include "Stdafx.h"
#include "Resource.h"
#include "GameServerDlg.h"
#include "ServerParameterFile.h"
#include "ServerParameterSection.h"
#include "ServerParameterItem.h"

//////////////////////////////////////////////////////////////////////////////////

//��Ϣ����
#define WM_PROCESS_CMD_LINE			(WM_USER+100)						//��������

//////////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CGameServerDlg, CDialog)

	//ϵͳ��Ϣ
	ON_WM_QUERYENDSESSION()
	ON_BN_CLICKED(IDC_OPEN_SERVER, OnBnClickedOpenServer)
	ON_BN_CLICKED(IDC_STOP_SERVICE, OnBnClickedStopService)
	ON_BN_CLICKED(IDC_START_SERVICE, OnBnClickedStartService)
	ON_BN_CLICKED(IDC_CREATE_SERVER, OnBnClickedCreateServer)
	ON_BN_CLICKED(IDC_OPTION_SERVER, OnBnClickedOptionServer)
	ON_BN_CLICKED(IDC_OPTION_MATCH, OnBnClickedOptionMatch)

	//�Զ���Ϣ
	ON_MESSAGE(WM_PROCESS_CMD_LINE,OnMessageProcessCmdLine)

	ON_BN_CLICKED(IDC_OPEN_MATCH, &CGameServerDlg::OnBnClickedOpenMatch)
END_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////////////////

enum EServerParamterItemsNames{
	ERobotEnterLimit,
	ERobotLimit,
	ERealAILimit,
	ERoomType,
	ERoomRobotType
};
const char *ServerParamterItemsNames[] = {
	"RobotEnterLimit", 
	"RobotLimit",
	"RealAILimit",
	"RoomType",
	"RoomRobbortType"
};

const char *serverParameterFile = "ServerParameterSaveFile.ini";


//���캯��
CGameServerDlg::CGameServerDlg() : CDialog(IDD_DLG_GAME_SERVER)
{
	//���ò���
	m_bAutoControl=false;
	m_bOptionSuccess=false;
	ZeroMemory(&m_ModuleInitParameter,sizeof(m_ModuleInitParameter));
	
	return;
}

//��������
CGameServerDlg::~CGameServerDlg(){


	
}

//�ؼ���
VOID CGameServerDlg::DoDataExchange(CDataExchange * pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TRACE_MESSAGE, m_TraceServiceControl);
}

//��ʼ������
BOOL CGameServerDlg::OnInitDialog()
{
	__super::OnInitDialog();

	//���ñ���
	CString tmpTitle;
	tmpTitle.Format(L"GameServer v%s --[ Stop ]", GetProjectVersion());
	SetWindowText(tmpTitle);

	DWORD pId = GetCurrentProcessId();
	CString strPid;
	strPid.Format(L" - %d", pId);

	GetDlgItem(IDC_STATIC_VERTION)->SetWindowTextW(GetProjectVersion() + strPid);

	if(!this->isSaveProcessIDToFileSuccess(pId)){
		AfxMessageBox(L"Write Process Id Fail");
	}

	//����ͼ��
	HICON hIcon=LoadIcon(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDR_MAINFRAME));
	SetIcon(hIcon,TRUE);
	SetIcon(hIcon,FALSE);

	//�������
	m_ServiceUnits.SetServiceUnitsSink(this);

	//�����
	LPCTSTR pszCmdLine=AfxGetApp()->m_lpCmdLine;
	if(pszCmdLine[0] != 0){ 
		PostMessage(WM_PROCESS_CMD_LINE,0,(LPARAM)pszCmdLine);
	}
	return TRUE;
}

BOOL CGameServerDlg::isSaveProcessIDToFileSuccess(DWORD _pId){
	try{
		CFileFind fileFinder;
		CFile file;
		CStringA strPId;

		if(fileFinder.FindFile(L"ProcessID.txt")){
			file.Remove(L"ProcessID.txt");
		}

		strPId.Format("%d", _pId);
		file.Open(L"ProcessID.txt", CFile::modeCreate | CFile::modeWrite);
		file.Write(strPId, strPId.GetLength());
		file.Flush();
		file.Close();

		return TRUE;
	}catch(...){
		return FALSE;
	}

	
}

CString CGameServerDlg::GetProjectVersion()  
{ 
	//ȡ��Ŀǰ��̖�ķ�ʽ(VS_VERSION_INFO)
    CString strVersion = L"";  
	TCHAR szFullPath[MAX_PATH]; 
	DWORD dwVerInfoSize = 0; 
	DWORD dwVerHnd; 
	WORD m_nProdVersion[4]; 
	VS_FIXEDFILEINFO * pFileInfo; 


	GetModuleFileName(NULL, szFullPath, sizeof(szFullPath)); 
	dwVerInfoSize = GetFileVersionInfoSize(szFullPath, &dwVerHnd); 
	if (dwVerInfoSize) 
	{ 
	// If we were able to get the information, process it: 
	HANDLE hMem; 
	LPVOID lpvMem; 
	unsigned int uInfoSize = 0; 

	hMem = GlobalAlloc(GMEM_MOVEABLE, dwVerInfoSize); 
	lpvMem = GlobalLock(hMem); 
	GetFileVersionInfo(szFullPath, dwVerHnd, dwVerInfoSize, lpvMem); 

	::VerQueryValue(lpvMem, (LPTSTR)_T( "\\"), (void**)&pFileInfo, &uInfoSize); 

	// Product version from the FILEVERSION of the version info resource 
	m_nProdVersion[0] = HIWORD(pFileInfo-> dwProductVersionMS); 
	m_nProdVersion[1] = LOWORD(pFileInfo-> dwProductVersionMS); 
	m_nProdVersion[2] = HIWORD(pFileInfo-> dwProductVersionLS); 
	m_nProdVersion[3] = LOWORD(pFileInfo-> dwProductVersionLS); 
	strVersion.Format(_T( "%d.%d.%d.%d "),m_nProdVersion[0], 
	m_nProdVersion[1],m_nProdVersion[2],m_nProdVersion[3]); 

	GlobalUnlock(hMem); 
	GlobalFree(hMem); 

	} 
    return strVersion;  
}  

//ȷ����Ϣ
VOID CGameServerDlg::OnOK()
{
	return;
}

//ȡ������
VOID CGameServerDlg::OnCancel()
{
	//�ر�ѯ��
	if (m_ServiceUnits.GetServiceStatus()!=ServiceStatus_Stop)
	{
		LPCTSTR pszQuestion=TEXT("GameServer is Running, sure to quit?");
		if (AfxMessageBox(pszQuestion,MB_YESNO|MB_DEFBUTTON2|MB_ICONQUESTION)!=IDYES) return;
	}

	//ֹͣ����
	m_ServiceUnits.ConcludeService();

	__super::OnCancel();
}

//��Ϣ����
BOOL CGameServerDlg::PreTranslateMessage(MSG * pMsg)
{
	//��������
	if ((pMsg->message==WM_KEYDOWN)&&(pMsg->wParam==VK_ESCAPE))
	{
		return TRUE;
	}

	return __super::PreTranslateMessage(pMsg);
}

//����״̬
VOID CGameServerDlg::OnServiceUnitsStatus(enServiceStatus ServiceStatus)
{
	//״̬����
	switch (ServiceStatus)
	{
	case ServiceStatus_Stop:	//ֹͣ״̬
		{
			//���±���
			UpdateServerTitle(ServiceStatus);

			//����ť
			GetDlgItem(IDC_STOP_SERVICE)->EnableWindow(FALSE);
			GetDlgItem(IDC_START_SERVICE)->EnableWindow(TRUE);

			//���ð�ť
			GetDlgItem(IDC_OPEN_MATCH)->EnableWindow(TRUE);
			GetDlgItem(IDC_OPEN_SERVER)->EnableWindow(TRUE);
			GetDlgItem(IDC_CREATE_SERVER)->EnableWindow(TRUE);
			GetDlgItem(IDC_OPTION_SERVER)->EnableWindow(TRUE);
			/*if((m_ModuleInitParameter.GameServiceOption.wServerType&GAME_GENRE_MATCH)!=0)
				GetDlgItem(IDC_OPTION_MATCH)->EnableWindow(TRUE);*/

			//���а�ť
			GetDlgItem(IDC_RUN_PARAMETER)->EnableWindow(FALSE);
			GetDlgItem(IDC_SERVICE_CONTROL)->EnableWindow(FALSE);

			//��ʾ��Ϣ
			LPCTSTR pszDescribe=TEXT("Service Stop Succeeded");
			CTraceService::TraceString(pszDescribe,TraceLevel_Normal);

			break;
		}
	case ServiceStatus_RegFail:  //ע��ʧ��
		{
			//���±���
			UpdateServerTitle(ServiceStatus);

			//���ð�ť
			GetDlgItem(IDC_STOP_SERVICE)->EnableWindow(TRUE);
			GetDlgItem(IDC_START_SERVICE)->EnableWindow(FALSE);

			//���ð�ť
			GetDlgItem(IDC_OPEN_MATCH)->EnableWindow(FALSE);
			GetDlgItem(IDC_OPEN_SERVER)->EnableWindow(FALSE);
			GetDlgItem(IDC_CREATE_SERVER)->EnableWindow(FALSE);
			GetDlgItem(IDC_OPTION_SERVER)->EnableWindow(FALSE);
			GetDlgItem(IDC_OPTION_MATCH)->EnableWindow(FALSE);

			//���а�ť
			GetDlgItem(IDC_RUN_PARAMETER)->EnableWindow(FALSE);
			GetDlgItem(IDC_SERVICE_CONTROL)->EnableWindow(FALSE);

			//��ʾ��Ϣ
			LPCTSTR pszDescribe=TEXT("Game Server Register Failed");
			CTraceService::TraceString(pszDescribe,TraceLevel_Normal);

			break;
		}
	case ServiceStatus_Config:	//����״̬
		{
			//���±���
			UpdateServerTitle(ServiceStatus);

			//���ð�ť
			GetDlgItem(IDC_STOP_SERVICE)->EnableWindow(TRUE);
			GetDlgItem(IDC_START_SERVICE)->EnableWindow(FALSE);

			//���ð�ť
			GetDlgItem(IDC_OPEN_MATCH)->EnableWindow(FALSE);
			GetDlgItem(IDC_OPEN_SERVER)->EnableWindow(FALSE);
			GetDlgItem(IDC_CREATE_SERVER)->EnableWindow(FALSE);
			GetDlgItem(IDC_OPTION_SERVER)->EnableWindow(FALSE);
			GetDlgItem(IDC_OPTION_MATCH)->EnableWindow(FALSE);

			//���а�ť
			GetDlgItem(IDC_RUN_PARAMETER)->EnableWindow(FALSE);
			GetDlgItem(IDC_SERVICE_CONTROL)->EnableWindow(FALSE);

			//��ʾ��Ϣ
			LPCTSTR pszDescribe=TEXT("Initializing...");
			CTraceService::TraceString(pszDescribe,TraceLevel_Normal);

			break;
		}
	case ServiceStatus_Service:	//����״̬
		{
			//���±���
			UpdateServerTitle(ServiceStatus);

			//����ť
			GetDlgItem(IDC_STOP_SERVICE)->EnableWindow(TRUE);
			GetDlgItem(IDC_START_SERVICE)->EnableWindow(FALSE);

			//���ð�ť
			GetDlgItem(IDC_OPEN_MATCH)->EnableWindow(FALSE);
			GetDlgItem(IDC_OPEN_SERVER)->EnableWindow(FALSE);
			GetDlgItem(IDC_CREATE_SERVER)->EnableWindow(FALSE);
			GetDlgItem(IDC_OPTION_SERVER)->EnableWindow(FALSE);

			//���а�ť
			GetDlgItem(IDC_RUN_PARAMETER)->EnableWindow(TRUE);
			GetDlgItem(IDC_SERVICE_CONTROL)->EnableWindow(TRUE);



			if(m_ServiceUnits.m_AttemperEngineSink.GetAcsTypeIDCredit()== -1){
			//�����Ϣ
				CTraceService::TraceString(L"Read ACS Game Type Fail",TraceLevel_Exception);
				ASSERT(FALSE);
				return;
			}

			//��ʾ��Ϣ
			LPCTSTR pszDescribe=TEXT("Service Start Succeeded");
			CTraceService::TraceString(pszDescribe,TraceLevel_Normal);

			LOGI(TEXT("Service Start Succeeded"));

			break;
		}
	}

	return;
}

//����ͼ��
VOID CGameServerDlg::UpdateServerLogo(LPCTSTR pszServerDLL)
{
	//������Դ
	HINSTANCE hInstance=AfxLoadLibrary(pszServerDLL);

	//����ͼ��
	if (hInstance!=NULL)
	{
		//������Դ
		AfxSetResourceHandle(hInstance);

		//������Դ
		CStatic * pServerLogo=(CStatic *)GetDlgItem(IDC_SERVER_LOGO);
		pServerLogo->SetIcon(::LoadIcon(hInstance,TEXT("SERVER_ICON")));

		//�ͷ���Դ
		AfxFreeLibrary(hInstance);
		AfxSetResourceHandle(GetModuleHandle(NULL));
	}

	return;
}

//���±���
VOID CGameServerDlg::UpdateServerTitle(enServiceStatus ServiceStatus)
{
	//��������
	LPCTSTR pszStatusName=NULL;
	LPCTSTR pszServerName=NULL;

	//״̬�ַ�
	switch (ServiceStatus)
	{
	case ServiceStatus_Stop:	//ֹͣ״̬
		{
			pszStatusName=TEXT("Stop");
			break;
		}
	case ServiceStatus_Config:	//����״̬
		{
			pszStatusName=TEXT("Initialize");
			break;
		}
	case ServiceStatus_Service:	//����״̬
		{
			pszStatusName=TEXT("Running");
			break;
		}
	case ServiceStatus_RegFail: //ע��ʧ��
		{
			pszStatusName = TEXT("Register Failed");
			break;
		}
	}

	//��������
	if (m_bOptionSuccess==false) pszServerName=TEXT("GameServer");
	else pszServerName=m_ModuleInitParameter.GameServiceOption.szServerName;

	//�������
	TCHAR szTitle[128]=TEXT("");
	_sntprintf(szTitle,CountArray(szTitle),TEXT("[ %s ] -- [ %s ]"),pszServerName,pszStatusName);

	//���ñ���
	SetWindowText(szTitle);

	return;
}

//����״̬
VOID CGameServerDlg::UpdateParameterStatus(tagModuleInitParameter & ModuleInitParameter)
{
	//���ñ���
	m_bOptionSuccess=true;
	m_ModuleInitParameter=ModuleInitParameter;

	//���±���
	UpdateServerTitle(ServiceStatus_Stop);
	UpdateServerLogo(ModuleInitParameter.GameServiceAttrib.szServerDLLName);

	//���ð�ť
	GetDlgItem(IDC_START_SERVICE)->EnableWindow(TRUE);
	GetDlgItem(IDC_OPTION_SERVER)->EnableWindow(TRUE);

	/*if((ModuleInitParameter.GameServiceOption.wServerType&GAME_GENRE_MATCH)!=0)
	{
		GetDlgItem(IDC_OPTION_MATCH)->EnableWindow(TRUE);
	}*/

	//���ÿؼ�
	SetDlgItemText(IDC_GAME_NAME,m_ModuleInitParameter.GameServiceAttrib.szGameName);
	SetDlgItemText(IDC_SERVER_NAME,m_ModuleInitParameter.GameServiceOption.szServerName);

	//�����˿�
	if (m_ModuleInitParameter.GameServiceOption.wServerPort==0)
	{
		SetDlgItemText(IDC_SERVER_PORT,TEXT("AutoConfig"));
	}
	else
	{
		SetDlgItemInt(IDC_SERVER_PORT,m_ModuleInitParameter.GameServiceOption.wServerPort);
	}

	//����ģ��
	LPCTSTR pszServerDLLName=m_ModuleInitParameter.GameServiceAttrib.szServerDLLName;

	m_ServiceUnits.CollocateService(pszServerDLLName,m_ModuleInitParameter.GameServiceOption);

	//������ʾ
	TCHAR szString[256]=TEXT("");
	LPCTSTR pszServerName=m_ModuleInitParameter.GameServiceOption.szServerName;
	_sntprintf(szString,CountArray(szString),TEXT("[ %s ] Room Paras Load Succeeded"),pszServerName);

	//�����Ϣ
	CTraceService::TraceString(szString,TraceLevel_Normal);

	return;
}

//��������
bool CGameServerDlg::StartServerService(WORD wServerID)
{
	//��������
	tagDataBaseParameter DataBaseParameter;
	ZeroMemory(&DataBaseParameter,sizeof(DataBaseParameter));

	//���ò���
	InitDataBaseParameter(DataBaseParameter);
	m_ModuleDBParameter.SetPlatformDBParameter(DataBaseParameter);

	//��ȡ����
	CDlgServerItem DlgServerItem;
	if (DlgServerItem.OpenGameServer(wServerID)==false)
	{
		CTraceService::TraceString(TEXT("Room Paras NotFind or Load Error"),TraceLevel_Exception);
		return false;
	}

	//����״̬
	UpdateParameterStatus(DlgServerItem.m_ModuleInitParameter);

	//��������
	try
	{
		m_ServiceUnits.StartService();
	}
	catch (...)
	{
		ASSERT(FALSE);
	}

	return true;
}

//��ȡ����
bool CGameServerDlg::InitDataBaseParameter(tagDataBaseParameter & DataBaseParameter)
{
	//��ȡ·��
	TCHAR szWorkDir[MAX_PATH]=TEXT("");
	CWHService::GetWorkDirectory(szWorkDir,CountArray(szWorkDir));

	//����·��
	TCHAR szIniFile[MAX_PATH]=TEXT("");
	_sntprintf(szIniFile,CountArray(szIniFile),TEXT("%s\\ServerParameter.ini"),szWorkDir);

	//��ȡ����
	CWHIniData IniData;
	IniData.SetIniFilePath(szIniFile);

	//������Ϣ
	TCHAR portStr[20] = L"";
	IniData.ReadEncryptString(TEXT("PlatformDB"),TEXT("DBPort"),NULL,portStr,sizeof(portStr));
	DataBaseParameter.wDataBasePort=_wtoi(portStr);
	IniData.ReadEncryptString(TEXT("PlatformDB"),TEXT("DBAddr"),NULL,DataBaseParameter.szDataBaseAddr,CountArray(DataBaseParameter.szDataBaseAddr));
	IniData.ReadEncryptString(TEXT("PlatformDB"),TEXT("DBUser"),NULL,DataBaseParameter.szDataBaseUser,CountArray(DataBaseParameter.szDataBaseUser));
	IniData.ReadEncryptString(TEXT("PlatformDB"),TEXT("DBPass"),NULL,DataBaseParameter.szDataBasePass,CountArray(DataBaseParameter.szDataBasePass));
	IniData.ReadEncryptString(TEXT("PlatformDB"),TEXT("DBName"),szPlatformDB,DataBaseParameter.szDataBaseName,CountArray(DataBaseParameter.szDataBaseName));

	return true;
}

VOID CGameServerDlg::ReadParametersFromFile(const char *filename)
{
	ServerParameterSection section;
	char namebuffer[256];
	wcstombs(namebuffer, m_ModuleInitParameter.GameServiceOption.szServerName, 256);
	section.read(filename, namebuffer);

	ServerParameterItems *serverParameterItems = section.getParameterItems();

	for(ServerParameterItems::iterator it = serverParameterItems->begin(); it != serverParameterItems->end();++it) {
		
		if (!strcmp(it->getName(), ServerParamterItemsNames[ERobotEnterLimit])) {
			m_ModuleInitParameter.GameServiceOption.dwRobotEnterLimit = stoi(it->getValue());
		} else if (!strcmp(it->getName(), ServerParamterItemsNames[ERobotLimit])) {
			m_ModuleInitParameter.GameServiceOption.wRobotLimit = stoi(it->getValue());
		} else if (!strcmp(it->getName(), ServerParamterItemsNames[ERealAILimit])) {
			m_ModuleInitParameter.GameServiceOption.wAccompanyLimit = stoi(it->getValue());
		} else if (!strcmp(it->getName(), ServerParamterItemsNames[ERoomType])) {
			wcscpy(m_ModuleInitParameter.GameServiceOption.szRoomType, CA2W(it->getValue()));
		} else if (!strcmp(it->getName(), ServerParamterItemsNames[ERoomRobotType])) {
			m_ModuleInitParameter.GameServiceOption.bOnlyOnePlayer = ((stoi(it->getValue()) == 1)?true:false) ;			 			
		}
	}

	if (serverParameterItems->size() > 0)
	{
		m_ServiceUnits.CollocateService(m_ModuleInitParameter.GameServiceAttrib.szServerDLLName,m_ModuleInitParameter.GameServiceOption);
	}
}

VOID CGameServerDlg::WriteParametersToFile(const char *filename)
{
	ServerParameterFile parameterFile;
	char strbuf[512];

	wcstombs(strbuf, m_ModuleInitParameter.GameServiceOption.szServerName, 512);
	parameterFile.removeSection(filename, strbuf);

	sprintf(strbuf, "%d", m_ModuleInitParameter.GameServiceOption.dwRobotEnterLimit);
	ServerParameterItem maxPlayer(ServerParamterItemsNames[ERobotEnterLimit], strbuf);
	sprintf(strbuf, "%d", m_ModuleInitParameter.GameServiceOption.wRobotLimit);
	ServerParameterItem robotLimit(ServerParamterItemsNames[ERobotLimit], strbuf);
	sprintf(strbuf, "%d", m_ModuleInitParameter.GameServiceOption.wAccompanyLimit);
	ServerParameterItem realAILimit(ServerParamterItemsNames[ERealAILimit], strbuf);
	wcstombs(strbuf, m_ModuleInitParameter.GameServiceOption.szRoomType, 512);
	ServerParameterItem roomType(ServerParamterItemsNames[ERoomType], strbuf);
	sprintf(strbuf, "%d", (m_ModuleInitParameter.GameServiceOption.bOnlyOnePlayer == true)?1:0);
	ServerParameterItem roomRobbortType(ServerParamterItemsNames[ERoomRobotType], strbuf);

	ServerParameterSection serverParameterSection;

	wcstombs(strbuf, m_ModuleInitParameter.GameServiceOption.szServerName, 512);
	serverParameterSection.setName(strbuf);
	serverParameterSection.addParameterItem(roomType);
	serverParameterSection.addParameterItem(maxPlayer);
	serverParameterSection.addParameterItem(robotLimit);
	serverParameterSection.addParameterItem(realAILimit);
	serverParameterSection.addParameterItem(roomRobbortType);

	serverParameterSection.write(filename);
}

//��������
VOID CGameServerDlg::OnBnClickedStartService()
{
	//int tmpLogID;
	//try{
	//	tmpLogID = ILog4zManager::GetInstance()->CreateLogger("ZJH____.log", "GameLog", LOG_LEVEL_PVP, true , false , 2);	
	//}catch(...){
	//	return;
	//}
	//LOGI("test LOGI");
	//LOG_PVP(tmpLogID, "TEST MESSAGE");
	//return;

	//��������
	try
	{
		m_ServiceUnits.StartService();
		//�����Ϸ����ش�ʧ��
	}
	catch (...)
	{
		ASSERT(FALSE);
	}

	return;
}

//ֹͣ����
VOID CGameServerDlg::OnBnClickedStopService()
{
	//ֹͣ����
	try
	{
		m_ServiceUnits.ConcludeService();
	}
	catch (...)
	{
		ASSERT(FALSE);
	}

	return;
}

//�򿪷���
VOID CGameServerDlg::OnBnClickedOpenServer()
{
	//��������
	tagDataBaseParameter DataBaseParameter;
	ZeroMemory(&DataBaseParameter,sizeof(DataBaseParameter));

	//���ò���
	InitDataBaseParameter(DataBaseParameter);
	m_ModuleDBParameter.SetPlatformDBParameter(DataBaseParameter);

	//���÷���
	CDlgServerItem DlgServerItem;
	if (DlgServerItem.OpenGameServer()==false) return;

	//����״̬
	UpdateParameterStatus(DlgServerItem.m_ModuleInitParameter);

	ReadParametersFromFile(serverParameterFile);

	return;
}

//��������
VOID CGameServerDlg::OnBnClickedCreateServer()
{
	//��������
	tagDataBaseParameter DataBaseParameter;
	ZeroMemory(&DataBaseParameter,sizeof(DataBaseParameter));

	//���ò���
	InitDataBaseParameter(DataBaseParameter);
	m_ModuleDBParameter.SetPlatformDBParameter(DataBaseParameter);

	//��������
	CDlgServerWizard DlgServerWizard;
	if (DlgServerWizard.CreateGameServer()==false) return;

	//����״̬
	UpdateParameterStatus(DlgServerWizard.m_ModuleInitParameter);

	return;
}

//���÷���
VOID CGameServerDlg::OnBnClickedOptionServer()
{
	//��Ϸģ��
	CGameServiceManagerHelper GameServiceManager;

	GameServiceManager.SetModuleCreateInfo(m_ModuleInitParameter.GameServiceAttrib.szServerDLLName,GAME_SERVICE_CREATE_NAME);

	//����ģ��
	if (GameServiceManager.CreateInstance()==false)
	{
		AfxMessageBox(TEXT("Service Moudle NotFind or Load Error, Room Config Fail"),MB_ICONERROR);
		return;
	}
	//���÷���
	CDlgServerWizard DlgServerWizard;

	DlgServerWizard.SetWizardParameter(GameServiceManager.GetInterface(),&m_ModuleInitParameter.GameServiceOption);

	//��������
	if (DlgServerWizard.CreateGameServer()==false) return;

	//����״̬
	UpdateParameterStatus(DlgServerWizard.m_ModuleInitParameter);

	WriteParametersToFile(serverParameterFile);

	return;
}

//���ñ���
VOID CGameServerDlg::OnBnClickedOptionMatch()
{
	//���Ǳ����͹���
	//if((m_ModuleInitParameter.GameServiceOption.wServerType&GAME_GENRE_MATCH)==0) return;
	return;

	//��������
	tagDataBaseParameter DataBaseParameter;
	ZeroMemory(&DataBaseParameter,sizeof(DataBaseParameter));

	//���ò���
	InitDataBaseParameter(DataBaseParameter);
	m_ModuleDBParameter.SetPlatformDBParameter(DataBaseParameter);

	//���÷���
	CDlgServerMatch DlgServerMatch;
	if (DlgServerMatch.OpenGameMatch()==false) return;

	//����״̬
	UpdateParameterStatus(DlgServerMatch.m_ModuleInitParameter);


}

//�ر�ѯ��
BOOL CGameServerDlg::OnQueryEndSession()
{
	//��ʾ��Ϣ
	if (m_ServiceUnits.GetServiceStatus()!=ServiceStatus_Stop)
	{
		CTraceService::TraceString(TEXT("Service is Running, System Logoff Request Failed"),TraceLevel_Warning);
		return FALSE;
	}

	return TRUE;
}

//�����
LRESULT CGameServerDlg::OnMessageProcessCmdLine(WPARAM wParam, LPARAM lParam)
{
	//��������
	CWHCommandLine CommandLine;
	LPCTSTR pszCommandLine=(LPCTSTR)(lParam);

	//�����ʶ
	TCHAR szSrverID[32]=TEXT("");
	if (CommandLine.SearchCommandItem(pszCommandLine,TEXT("/ServerID:"),szSrverID,CountArray(szSrverID))==true)
	{
		//��ȡ����
		WORD wServerID=(WORD)(_tstol(szSrverID));

		//��������
		if (wServerID!=0)
		{
			//���ñ���
			m_bAutoControl=true;

			//��������
			StartServerService(wServerID);
		}
	}

	return 0L;
}

//////////////////////////////////////////////////////////////////////////////////


void CGameServerDlg::OnBnClickedOpenMatch()
{
	//��������
	tagDataBaseParameter DataBaseParameter;
	ZeroMemory(&DataBaseParameter,sizeof(DataBaseParameter));

	//���ò���
	InitDataBaseParameter(DataBaseParameter);
	m_ModuleDBParameter.SetPlatformDBParameter(DataBaseParameter);

	//���÷���
	CDlgServerMatch DlServerMatch;
	if (DlServerMatch.OpenGameMatch()==false) return;

	//����״̬
	UpdateParameterStatus(DlServerMatch.m_ModuleInitParameter);
	return;	
}
