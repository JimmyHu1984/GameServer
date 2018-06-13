#include "Stdafx.h"
#include "Resource.h"
#include "GameServerDlg.h"
#include "ServerParameterFile.h"
#include "ServerParameterSection.h"
#include "ServerParameterItem.h"

//////////////////////////////////////////////////////////////////////////////////

//消息定义
#define WM_PROCESS_CMD_LINE			(WM_USER+100)						//处理命令

//////////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CGameServerDlg, CDialog)

	//系统消息
	ON_WM_QUERYENDSESSION()
	ON_BN_CLICKED(IDC_OPEN_SERVER, OnBnClickedOpenServer)
	ON_BN_CLICKED(IDC_STOP_SERVICE, OnBnClickedStopService)
	ON_BN_CLICKED(IDC_START_SERVICE, OnBnClickedStartService)
	ON_BN_CLICKED(IDC_CREATE_SERVER, OnBnClickedCreateServer)
	ON_BN_CLICKED(IDC_OPTION_SERVER, OnBnClickedOptionServer)
	ON_BN_CLICKED(IDC_OPTION_MATCH, OnBnClickedOptionMatch)

	//自定消息
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


//构造函数
CGameServerDlg::CGameServerDlg() : CDialog(IDD_DLG_GAME_SERVER)
{
	//配置参数
	m_bAutoControl=false;
	m_bOptionSuccess=false;
	ZeroMemory(&m_ModuleInitParameter,sizeof(m_ModuleInitParameter));
	
	return;
}

//析构函数
CGameServerDlg::~CGameServerDlg(){


	
}

//控件绑定
VOID CGameServerDlg::DoDataExchange(CDataExchange * pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TRACE_MESSAGE, m_TraceServiceControl);
}

//初始化函数
BOOL CGameServerDlg::OnInitDialog()
{
	__super::OnInitDialog();

	//设置标题
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

	//设置图标
	HICON hIcon=LoadIcon(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDR_MAINFRAME));
	SetIcon(hIcon,TRUE);
	SetIcon(hIcon,FALSE);

	//设置组件
	m_ServiceUnits.SetServiceUnitsSink(this);

	//命令处理
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
	//取得目前版號的方式(VS_VERSION_INFO)
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

//确定消息
VOID CGameServerDlg::OnOK()
{
	return;
}

//取消函数
VOID CGameServerDlg::OnCancel()
{
	//关闭询问
	if (m_ServiceUnits.GetServiceStatus()!=ServiceStatus_Stop)
	{
		LPCTSTR pszQuestion=TEXT("GameServer is Running, sure to quit?");
		if (AfxMessageBox(pszQuestion,MB_YESNO|MB_DEFBUTTON2|MB_ICONQUESTION)!=IDYES) return;
	}

	//停止服务
	m_ServiceUnits.ConcludeService();

	__super::OnCancel();
}

//消息解释
BOOL CGameServerDlg::PreTranslateMessage(MSG * pMsg)
{
	//按键过滤
	if ((pMsg->message==WM_KEYDOWN)&&(pMsg->wParam==VK_ESCAPE))
	{
		return TRUE;
	}

	return __super::PreTranslateMessage(pMsg);
}

//服务状态
VOID CGameServerDlg::OnServiceUnitsStatus(enServiceStatus ServiceStatus)
{
	//状态设置
	switch (ServiceStatus)
	{
	case ServiceStatus_Stop:	//停止状态
		{
			//更新标题
			UpdateServerTitle(ServiceStatus);

			//服务按钮
			GetDlgItem(IDC_STOP_SERVICE)->EnableWindow(FALSE);
			GetDlgItem(IDC_START_SERVICE)->EnableWindow(TRUE);

			//配置按钮
			GetDlgItem(IDC_OPEN_MATCH)->EnableWindow(TRUE);
			GetDlgItem(IDC_OPEN_SERVER)->EnableWindow(TRUE);
			GetDlgItem(IDC_CREATE_SERVER)->EnableWindow(TRUE);
			GetDlgItem(IDC_OPTION_SERVER)->EnableWindow(TRUE);
			/*if((m_ModuleInitParameter.GameServiceOption.wServerType&GAME_GENRE_MATCH)!=0)
				GetDlgItem(IDC_OPTION_MATCH)->EnableWindow(TRUE);*/

			//运行按钮
			GetDlgItem(IDC_RUN_PARAMETER)->EnableWindow(FALSE);
			GetDlgItem(IDC_SERVICE_CONTROL)->EnableWindow(FALSE);

			//提示信息
			LPCTSTR pszDescribe=TEXT("Service Stop Succeeded");
			CTraceService::TraceString(pszDescribe,TraceLevel_Normal);

			break;
		}
	case ServiceStatus_RegFail:  //注册失败
		{
			//更新标题
			UpdateServerTitle(ServiceStatus);

			//设置按钮
			GetDlgItem(IDC_STOP_SERVICE)->EnableWindow(TRUE);
			GetDlgItem(IDC_START_SERVICE)->EnableWindow(FALSE);

			//配置按钮
			GetDlgItem(IDC_OPEN_MATCH)->EnableWindow(FALSE);
			GetDlgItem(IDC_OPEN_SERVER)->EnableWindow(FALSE);
			GetDlgItem(IDC_CREATE_SERVER)->EnableWindow(FALSE);
			GetDlgItem(IDC_OPTION_SERVER)->EnableWindow(FALSE);
			GetDlgItem(IDC_OPTION_MATCH)->EnableWindow(FALSE);

			//运行按钮
			GetDlgItem(IDC_RUN_PARAMETER)->EnableWindow(FALSE);
			GetDlgItem(IDC_SERVICE_CONTROL)->EnableWindow(FALSE);

			//提示信息
			LPCTSTR pszDescribe=TEXT("Game Server Register Failed");
			CTraceService::TraceString(pszDescribe,TraceLevel_Normal);

			break;
		}
	case ServiceStatus_Config:	//配置状态
		{
			//更新标题
			UpdateServerTitle(ServiceStatus);

			//设置按钮
			GetDlgItem(IDC_STOP_SERVICE)->EnableWindow(TRUE);
			GetDlgItem(IDC_START_SERVICE)->EnableWindow(FALSE);

			//配置按钮
			GetDlgItem(IDC_OPEN_MATCH)->EnableWindow(FALSE);
			GetDlgItem(IDC_OPEN_SERVER)->EnableWindow(FALSE);
			GetDlgItem(IDC_CREATE_SERVER)->EnableWindow(FALSE);
			GetDlgItem(IDC_OPTION_SERVER)->EnableWindow(FALSE);
			GetDlgItem(IDC_OPTION_MATCH)->EnableWindow(FALSE);

			//运行按钮
			GetDlgItem(IDC_RUN_PARAMETER)->EnableWindow(FALSE);
			GetDlgItem(IDC_SERVICE_CONTROL)->EnableWindow(FALSE);

			//提示信息
			LPCTSTR pszDescribe=TEXT("Initializing...");
			CTraceService::TraceString(pszDescribe,TraceLevel_Normal);

			break;
		}
	case ServiceStatus_Service:	//服务状态
		{
			//更新标题
			UpdateServerTitle(ServiceStatus);

			//服务按钮
			GetDlgItem(IDC_STOP_SERVICE)->EnableWindow(TRUE);
			GetDlgItem(IDC_START_SERVICE)->EnableWindow(FALSE);

			//配置按钮
			GetDlgItem(IDC_OPEN_MATCH)->EnableWindow(FALSE);
			GetDlgItem(IDC_OPEN_SERVER)->EnableWindow(FALSE);
			GetDlgItem(IDC_CREATE_SERVER)->EnableWindow(FALSE);
			GetDlgItem(IDC_OPTION_SERVER)->EnableWindow(FALSE);

			//运行按钮
			GetDlgItem(IDC_RUN_PARAMETER)->EnableWindow(TRUE);
			GetDlgItem(IDC_SERVICE_CONTROL)->EnableWindow(TRUE);



			if(m_ServiceUnits.m_AttemperEngineSink.GetAcsTypeIDCredit()== -1){
			//输出信息
				CTraceService::TraceString(L"Read ACS Game Type Fail",TraceLevel_Exception);
				ASSERT(FALSE);
				return;
			}

			//提示信息
			LPCTSTR pszDescribe=TEXT("Service Start Succeeded");
			CTraceService::TraceString(pszDescribe,TraceLevel_Normal);

			LOGI(TEXT("Service Start Succeeded"));

			break;
		}
	}

	return;
}

//更新图标
VOID CGameServerDlg::UpdateServerLogo(LPCTSTR pszServerDLL)
{
	//加载资源
	HINSTANCE hInstance=AfxLoadLibrary(pszServerDLL);

	//加载图形
	if (hInstance!=NULL)
	{
		//设置资源
		AfxSetResourceHandle(hInstance);

		//设置资源
		CStatic * pServerLogo=(CStatic *)GetDlgItem(IDC_SERVER_LOGO);
		pServerLogo->SetIcon(::LoadIcon(hInstance,TEXT("SERVER_ICON")));

		//释放资源
		AfxFreeLibrary(hInstance);
		AfxSetResourceHandle(GetModuleHandle(NULL));
	}

	return;
}

//更新标题
VOID CGameServerDlg::UpdateServerTitle(enServiceStatus ServiceStatus)
{
	//变量定义
	LPCTSTR pszStatusName=NULL;
	LPCTSTR pszServerName=NULL;

	//状态字符
	switch (ServiceStatus)
	{
	case ServiceStatus_Stop:	//停止状态
		{
			pszStatusName=TEXT("Stop");
			break;
		}
	case ServiceStatus_Config:	//配置状态
		{
			pszStatusName=TEXT("Initialize");
			break;
		}
	case ServiceStatus_Service:	//运行状态
		{
			pszStatusName=TEXT("Running");
			break;
		}
	case ServiceStatus_RegFail: //注册失败
		{
			pszStatusName = TEXT("Register Failed");
			break;
		}
	}

	//房间名字
	if (m_bOptionSuccess==false) pszServerName=TEXT("GameServer");
	else pszServerName=m_ModuleInitParameter.GameServiceOption.szServerName;

	//构造标题
	TCHAR szTitle[128]=TEXT("");
	_sntprintf(szTitle,CountArray(szTitle),TEXT("[ %s ] -- [ %s ]"),pszServerName,pszStatusName);

	//设置标题
	SetWindowText(szTitle);

	return;
}

//更新状态
VOID CGameServerDlg::UpdateParameterStatus(tagModuleInitParameter & ModuleInitParameter)
{
	//设置变量
	m_bOptionSuccess=true;
	m_ModuleInitParameter=ModuleInitParameter;

	//更新标题
	UpdateServerTitle(ServiceStatus_Stop);
	UpdateServerLogo(ModuleInitParameter.GameServiceAttrib.szServerDLLName);

	//设置按钮
	GetDlgItem(IDC_START_SERVICE)->EnableWindow(TRUE);
	GetDlgItem(IDC_OPTION_SERVER)->EnableWindow(TRUE);

	/*if((ModuleInitParameter.GameServiceOption.wServerType&GAME_GENRE_MATCH)!=0)
	{
		GetDlgItem(IDC_OPTION_MATCH)->EnableWindow(TRUE);
	}*/

	//设置控件
	SetDlgItemText(IDC_GAME_NAME,m_ModuleInitParameter.GameServiceAttrib.szGameName);
	SetDlgItemText(IDC_SERVER_NAME,m_ModuleInitParameter.GameServiceOption.szServerName);

	//监听端口
	if (m_ModuleInitParameter.GameServiceOption.wServerPort==0)
	{
		SetDlgItemText(IDC_SERVER_PORT,TEXT("AutoConfig"));
	}
	else
	{
		SetDlgItemInt(IDC_SERVER_PORT,m_ModuleInitParameter.GameServiceOption.wServerPort);
	}

	//设置模块
	LPCTSTR pszServerDLLName=m_ModuleInitParameter.GameServiceAttrib.szServerDLLName;

	m_ServiceUnits.CollocateService(pszServerDLLName,m_ModuleInitParameter.GameServiceOption);

	//构造提示
	TCHAR szString[256]=TEXT("");
	LPCTSTR pszServerName=m_ModuleInitParameter.GameServiceOption.szServerName;
	_sntprintf(szString,CountArray(szString),TEXT("[ %s ] Room Paras Load Succeeded"),pszServerName);

	//输出信息
	CTraceService::TraceString(szString,TraceLevel_Normal);

	return;
}

//启动房间
bool CGameServerDlg::StartServerService(WORD wServerID)
{
	//变量定义
	tagDataBaseParameter DataBaseParameter;
	ZeroMemory(&DataBaseParameter,sizeof(DataBaseParameter));

	//设置参数
	InitDataBaseParameter(DataBaseParameter);
	m_ModuleDBParameter.SetPlatformDBParameter(DataBaseParameter);

	//读取配置
	CDlgServerItem DlgServerItem;
	if (DlgServerItem.OpenGameServer(wServerID)==false)
	{
		CTraceService::TraceString(TEXT("Room Paras NotFind or Load Error"),TraceLevel_Exception);
		return false;
	}

	//更新状态
	UpdateParameterStatus(DlgServerItem.m_ModuleInitParameter);

	//启动服务
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

//获取连接
bool CGameServerDlg::InitDataBaseParameter(tagDataBaseParameter & DataBaseParameter)
{
	//获取路径
	TCHAR szWorkDir[MAX_PATH]=TEXT("");
	CWHService::GetWorkDirectory(szWorkDir,CountArray(szWorkDir));

	//构造路径
	TCHAR szIniFile[MAX_PATH]=TEXT("");
	_sntprintf(szIniFile,CountArray(szIniFile),TEXT("%s\\ServerParameter.ini"),szWorkDir);

	//读取配置
	CWHIniData IniData;
	IniData.SetIniFilePath(szIniFile);

	//连接信息
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

//启动服务
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

	//启动服务
	try
	{
		m_ServiceUnits.StartService();
		//如果游戏种类回传失败
	}
	catch (...)
	{
		ASSERT(FALSE);
	}

	return;
}

//停止服务
VOID CGameServerDlg::OnBnClickedStopService()
{
	//停止服务
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

//打开房间
VOID CGameServerDlg::OnBnClickedOpenServer()
{
	//变量定义
	tagDataBaseParameter DataBaseParameter;
	ZeroMemory(&DataBaseParameter,sizeof(DataBaseParameter));

	//设置参数
	InitDataBaseParameter(DataBaseParameter);
	m_ModuleDBParameter.SetPlatformDBParameter(DataBaseParameter);

	//配置房间
	CDlgServerItem DlgServerItem;
	if (DlgServerItem.OpenGameServer()==false) return;

	//更新状态
	UpdateParameterStatus(DlgServerItem.m_ModuleInitParameter);

	ReadParametersFromFile(serverParameterFile);

	return;
}

//创建房间
VOID CGameServerDlg::OnBnClickedCreateServer()
{
	//变量定义
	tagDataBaseParameter DataBaseParameter;
	ZeroMemory(&DataBaseParameter,sizeof(DataBaseParameter));

	//设置参数
	InitDataBaseParameter(DataBaseParameter);
	m_ModuleDBParameter.SetPlatformDBParameter(DataBaseParameter);

	//创建房间
	CDlgServerWizard DlgServerWizard;
	if (DlgServerWizard.CreateGameServer()==false) return;

	//更新状态
	UpdateParameterStatus(DlgServerWizard.m_ModuleInitParameter);

	return;
}

//配置房间
VOID CGameServerDlg::OnBnClickedOptionServer()
{
	//游戏模块
	CGameServiceManagerHelper GameServiceManager;

	GameServiceManager.SetModuleCreateInfo(m_ModuleInitParameter.GameServiceAttrib.szServerDLLName,GAME_SERVICE_CREATE_NAME);

	//加载模块
	if (GameServiceManager.CreateInstance()==false)
	{
		AfxMessageBox(TEXT("Service Moudle NotFind or Load Error, Room Config Fail"),MB_ICONERROR);
		return;
	}
	//配置房间
	CDlgServerWizard DlgServerWizard;

	DlgServerWizard.SetWizardParameter(GameServiceManager.GetInterface(),&m_ModuleInitParameter.GameServiceOption);

	//创建房间
	if (DlgServerWizard.CreateGameServer()==false) return;

	//更新状态
	UpdateParameterStatus(DlgServerWizard.m_ModuleInitParameter);

	WriteParametersToFile(serverParameterFile);

	return;
}

//配置比赛
VOID CGameServerDlg::OnBnClickedOptionMatch()
{
	//不是比赛就过滤
	//if((m_ModuleInitParameter.GameServiceOption.wServerType&GAME_GENRE_MATCH)==0) return;
	return;

	//变量定义
	tagDataBaseParameter DataBaseParameter;
	ZeroMemory(&DataBaseParameter,sizeof(DataBaseParameter));

	//设置参数
	InitDataBaseParameter(DataBaseParameter);
	m_ModuleDBParameter.SetPlatformDBParameter(DataBaseParameter);

	//配置房间
	CDlgServerMatch DlgServerMatch;
	if (DlgServerMatch.OpenGameMatch()==false) return;

	//更新状态
	UpdateParameterStatus(DlgServerMatch.m_ModuleInitParameter);


}

//关闭询问
BOOL CGameServerDlg::OnQueryEndSession()
{
	//提示消息
	if (m_ServiceUnits.GetServiceStatus()!=ServiceStatus_Stop)
	{
		CTraceService::TraceString(TEXT("Service is Running, System Logoff Request Failed"),TraceLevel_Warning);
		return FALSE;
	}

	return TRUE;
}

//命令处理
LRESULT CGameServerDlg::OnMessageProcessCmdLine(WPARAM wParam, LPARAM lParam)
{
	//变量定义
	CWHCommandLine CommandLine;
	LPCTSTR pszCommandLine=(LPCTSTR)(lParam);

	//房间标识
	TCHAR szSrverID[32]=TEXT("");
	if (CommandLine.SearchCommandItem(pszCommandLine,TEXT("/ServerID:"),szSrverID,CountArray(szSrverID))==true)
	{
		//获取房间
		WORD wServerID=(WORD)(_tstol(szSrverID));

		//启动房间
		if (wServerID!=0)
		{
			//设置变量
			m_bAutoControl=true;

			//启动房间
			StartServerService(wServerID);
		}
	}

	return 0L;
}

//////////////////////////////////////////////////////////////////////////////////


void CGameServerDlg::OnBnClickedOpenMatch()
{
	//变量定义
	tagDataBaseParameter DataBaseParameter;
	ZeroMemory(&DataBaseParameter,sizeof(DataBaseParameter));

	//设置参数
	InitDataBaseParameter(DataBaseParameter);
	m_ModuleDBParameter.SetPlatformDBParameter(DataBaseParameter);

	//配置房间
	CDlgServerMatch DlServerMatch;
	if (DlServerMatch.OpenGameMatch()==false) return;

	//更新状态
	UpdateParameterStatus(DlServerMatch.m_ModuleInitParameter);
	return;	
}
