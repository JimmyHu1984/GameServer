#include "StdAfx.h"
#include "ServiceUnits.h"
#include "ControlPacket.h"
#include "../../Lib/boost_1_57_0/boost/locale/encoding.hpp"
#include "../../Lib/boost_1_57_0/boost/locale/info.hpp"
#include "../../Lib/boost_1_57_0/boost/locale.hpp"
#include "IniReader.h"

//////////////////////////////////////////////////////////////////////////////////

//静态变量
CServiceUnits *			CServiceUnits::g_pServiceUnits=NULL;			//对象指针

//////////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CServiceUnits, CWnd)
	ON_MESSAGE(WM_UICONTROL_REQUEST,OnUIControlRequest)
END_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////////////////
//构造函数
CServiceUnits::CServiceUnits()
{
	//服务参数
	m_ServiceStatus=ServiceStatus_Stop;

	//设置接口
	m_pIServiceUnitsSink=NULL;

	//设置对象
	ASSERT(g_pServiceUnits==NULL);
	if (g_pServiceUnits==NULL) g_pServiceUnits=this;

	//组件配置
	ZeroMemory(&m_GameParameter,sizeof(m_GameParameter));
	ZeroMemory(&m_DataBaseParameter,sizeof(m_DataBaseParameter));
	ZeroMemory(&m_GameServiceAttrib,sizeof(m_GameServiceAttrib));
	ZeroMemory(&m_GameServiceOption,sizeof(m_GameServiceOption));

	return;
}

//析构函数
CServiceUnits::~CServiceUnits()
{
}

//设置接口
bool CServiceUnits::SetServiceUnitsSink(IServiceUnitsSink * pIServiceUnitsSink)
{
	//设置变量
	m_pIServiceUnitsSink=pIServiceUnitsSink;

	return true;
}

//投递请求
bool CServiceUnits::PostControlRequest(WORD wIdentifier, VOID * pData, WORD wDataSize)
{
	//状态判断
	ASSERT(IsWindow(m_hWnd));
	if (IsWindow(m_hWnd)==FALSE) return false;

	//插入队列
	CWHDataLocker DataLocker(m_CriticalSection);
	if (m_DataQueue.InsertData(wIdentifier,pData,wDataSize)==false) return false;

	//发送消息
	PostMessage(WM_UICONTROL_REQUEST,wIdentifier,wDataSize);

	return true;
}

//启动服务
bool CServiceUnits::StartService()
{
	//效验状态
	ASSERT(m_ServiceStatus==ServiceStatus_Stop || m_ServiceStatus == ServiceStatus_RegFail);
	if (m_ServiceStatus!=ServiceStatus_Stop && m_ServiceStatus!=ServiceStatus_RegFail) return false;

	//clear log
// 	TCHAR szWorkDir[MAX_PATH]=TEXT("");
// 	CWHService::GetWorkDirectory(szWorkDir,CountArray(szWorkDir));
// 
// 	TCHAR szLogFile[MAX_PATH]=TEXT("");
// 	_sntprintf(szLogFile,CountArray(szLogFile),TEXT("%s\\%s\\"),szWorkDir,L"log");

	//DeleteLog(szLogFile, m_GameServiceOption.iLogClearDays*24*3600L);

	m_clct.InitThread(this,m_GameServiceOption.iLogClearDays*24*3600L);
	m_clct.StartThread();
	
	//设置状态
	SetServiceStatus(ServiceStatus_Config);

	//创建窗口
	if (m_hWnd==NULL)
	{
		CRect rcCreate(0,0,0,0);
		Create(NULL,NULL,WS_CHILD,rcCreate,AfxGetMainWnd(),100);
	}

	//创建模块
	if (CreateServiceDLL()==false)
	{
		ConcludeService();
		return false;
	}

	//调整参数
	if (RectifyServiceParameter()==false)
	{
		ConcludeService();
		return false;
	}

	//配置服务
	if (InitializeService()==false)
	{
		ConcludeService();
		return false;
	}

	//启动内核
	if (StartKernelService()==false)
	{
		ConcludeService();
		return false;
	}

	//加载配置
	SendControlPacket(CT_LOAD_SERVICE_CONFIG,NULL,0);


	//启动比赛
	//if (m_GameMatchServiceManager.GetInterface()!=NULL && m_GameMatchServiceManager->StartService()==false) return false;

	return true;
}

//停止服务
bool CServiceUnits::ConcludeService()
{
	//设置变量
	SetServiceStatus(ServiceStatus_Stop);
	m_cht.ConcludeThread(INFINITE);

	//内核组件
	if (m_TimerEngine.GetInterface()!=NULL) m_TimerEngine->ConcludeService();
	if (m_AttemperEngine.GetInterface()!=NULL) m_AttemperEngine->ConcludeService();
	if (m_TCPSocketService.GetInterface()!=NULL) m_TCPSocketService->ConcludeService();
	if (m_TCPNetworkEngine.GetInterface()!=NULL) m_TCPNetworkEngine->ConcludeService();

	//数据引擎
	if (m_RecordDataBaseEngine.GetInterface()!=NULL) m_RecordDataBaseEngine->ConcludeService();
	if (m_KernelDataBaseEngine.GetInterface()!=NULL) m_KernelDataBaseEngine->ConcludeService();
	m_DBCorrespondManager.ConcludeService();

	//注销组件
	if (m_GameServiceManager.GetInterface()!=NULL) m_GameServiceManager.CloseInstance();
	//if(m_GameMatchServiceManager.GetInterface()!=NULL)m_GameMatchServiceManager.CloseInstance();


	::OutputDebugStringA("stop Thread");
	m_AttemperEngineSink.m_DistributeHandler.stopThread();
	return true;
}

//游戏配置
bool CServiceUnits::CollocateService(LPCTSTR pszGameModule, tagGameServiceOption & GameServiceOption)
{
	//效验状态
	ASSERT(m_ServiceStatus==ServiceStatus_Stop);
	if (m_ServiceStatus!=ServiceStatus_Stop) return false;

	//配置模块
	m_GameServiceOption=GameServiceOption;
	m_GameServiceManager.SetModuleCreateInfo(pszGameModule,GAME_SERVICE_CREATE_NAME);
	return true;
}

//创建模块
bool CServiceUnits::CreateServiceDLL()
{
	//时间引擎
	if ((m_TimerEngine.GetInterface()==NULL)&&(m_TimerEngine.CreateInstance()==false))
	{
		CTraceService::TraceString(m_TimerEngine.GetErrorDescribe(),TraceLevel_Exception);
		LOGI("CServiceUnits::CreateServiceDLL, m_TimerEngine.GetErrorDescribe, "<<m_TimerEngine.GetErrorDescribe());
		return false;
	}

	//调度引擎
	if ((m_AttemperEngine.GetInterface()==NULL)&&(m_AttemperEngine.CreateInstance()==false))
	{
		CTraceService::TraceString(m_AttemperEngine.GetErrorDescribe(),TraceLevel_Exception);
		LOGI("CServiceUnits::CreateServiceDLL, m_AttemperEngine.GetErrorDescribe, "<<m_AttemperEngine.GetErrorDescribe());
		return false;
	}

	//网络组件
	if ((m_TCPSocketService.GetInterface()==NULL)&&(m_TCPSocketService.CreateInstance()==false))
	{
		CTraceService::TraceString(m_TCPSocketService.GetErrorDescribe(),TraceLevel_Exception);
		LOGI("CServiceUnits::CreateServiceDLL, m_TCPSocketService.GetErrorDescribe, "<<m_TCPSocketService.GetErrorDescribe());
		return false;
	}

	//网络引擎
	if ((m_TCPNetworkEngine.GetInterface()==NULL)&&(m_TCPNetworkEngine.CreateInstance()==false))
	{
		CTraceService::TraceString(m_TCPNetworkEngine.GetErrorDescribe(),TraceLevel_Exception);
		LOGI("CServiceUnits::CreateServiceDLL, m_TCPNetworkEngine.GetErrorDescribe, "<<m_TCPNetworkEngine.GetErrorDescribe());
		return false;
	}

	//数据组件
	if ((m_KernelDataBaseEngine.GetInterface()==NULL)&&(m_KernelDataBaseEngine.CreateInstance()==false))
	{
		CTraceService::TraceString(m_KernelDataBaseEngine.GetErrorDescribe(),TraceLevel_Exception);
		LOGI("CServiceUnits::CreateServiceDLL, m_KernelDataBaseEngine.GetErrorDescribe, "<<m_KernelDataBaseEngine.GetErrorDescribe());
		return false;
	}

	//数据组件
	if ((m_RecordDataBaseEngine.GetInterface()==NULL)&&(m_RecordDataBaseEngine.CreateInstance()==false))
	{
		CTraceService::TraceString(m_RecordDataBaseEngine.GetErrorDescribe(),TraceLevel_Exception);
		LOGI("CServiceUnits::CreateServiceDLL, m_RecordDataBaseEngine.GetErrorDescribe, "<<m_RecordDataBaseEngine.GetErrorDescribe());
		return false;
	}

	//游戏模块
	if ((m_GameServiceManager.GetInterface()==NULL)&&(m_GameServiceManager.CreateInstance()==false))
	{
		CTraceService::TraceString(m_GameServiceManager.GetErrorDescribe(),TraceLevel_Exception);
		LOGI("CServiceUnits::CreateServiceDLL, m_GameServiceManager.GetErrorDescribe, "<<m_GameServiceManager.GetErrorDescribe());
		return false;
	}

	//比赛模块
	//if ((m_GameServiceOption.wServerType&GAME_GENRE_MATCH)!=0)
	//{
	//	if ((m_GameMatchServiceManager.GetInterface()==NULL)&&(m_GameMatchServiceManager.CreateInstance()==false))
	//	{
	//		CTraceService::TraceString(m_GameMatchServiceManager.GetErrorDescribe(),TraceLevel_Exception);
	//		LOGI("CServiceUnits::CreateServiceDLL, m_GameMatchServiceManager.GetErrorDescribe, "<<m_GameMatchServiceManager.GetErrorDescribe());
	//		return false;
	//	}
	//}

	return true;
}

//配置组件
bool CServiceUnits::InitializeService()
{
	//加载参数
	m_InitParameter.LoadInitParameter();

	//配置参数
	m_GameParameter.wMedalRate=1L;
	m_GameParameter.wRevenueRate=1L;

	//连接信息
	LPCTSTR pszDataBaseAddr=m_GameServiceOption.szDataBaseAddr;
	LPCTSTR pszDataBaseName=m_GameServiceOption.szDataBaseName;
	if (LoadDataBaseParameter(pszDataBaseAddr,pszDataBaseName,m_DataBaseParameter)==false) return false;

	//组件接口
	IUnknownEx * pIAttemperEngine=m_AttemperEngine.GetInterface();
	IUnknownEx * pITCPNetworkEngine=m_TCPNetworkEngine.GetInterface();
	IUnknownEx * pIAttemperEngineSink=QUERY_OBJECT_INTERFACE(m_AttemperEngineSink,IUnknownEx);

	//数据引擎
	IUnknownEx * pIDataBaseEngineRecordSink[CountArray(m_RecordDataBaseSink)];
	IUnknownEx * pIDataBaseEngineKernelSink[CountArray(m_KernelDataBaseSink)];
	for (WORD i=0;i<CountArray(pIDataBaseEngineRecordSink);i++) pIDataBaseEngineRecordSink[i]=QUERY_OBJECT_INTERFACE(m_RecordDataBaseSink[i],IUnknownEx);
	for (WORD i=0;i<CountArray(pIDataBaseEngineKernelSink);i++) pIDataBaseEngineKernelSink[i]=QUERY_OBJECT_INTERFACE(m_KernelDataBaseSink[i],IUnknownEx);

	//绑定接口
	if (m_AttemperEngine->SetAttemperEngineSink(pIAttemperEngineSink)==false) return false;
	if (m_RecordDataBaseEngine->SetDataBaseEngineSink(pIDataBaseEngineRecordSink,CountArray(pIDataBaseEngineRecordSink))==false) return false;
	if (m_KernelDataBaseEngine->SetDataBaseEngineSink(pIDataBaseEngineKernelSink,CountArray(pIDataBaseEngineKernelSink))==false) return false;

	//内核组件
	if (m_TimerEngine->SetTimerEngineEvent(pIAttemperEngine)==false) return false;
	if (m_AttemperEngine->SetNetworkEngine(pITCPNetworkEngine)==false) return false;
	if (m_TCPNetworkEngine->SetTCPNetworkEngineEvent(pIAttemperEngine)==false) return false;

	//协调服务
	if (m_TCPSocketService->SetServiceID(NETWORK_CORRESPOND)==false) return false;
	if (m_TCPSocketService->SetTCPSocketEvent(pIAttemperEngine)==false) return false;

	//数据协调
	m_DBCorrespondManager.InitDBCorrespondManager(m_KernelDataBaseEngine.GetInterface());

	//调度回调
	m_AttemperEngineSink.m_pInitParameter=&m_InitParameter;
	m_AttemperEngineSink.m_pGameParameter=&m_GameParameter;
	m_AttemperEngineSink.m_pGameServiceAttrib=&m_GameServiceAttrib;
	m_AttemperEngineSink.m_pGameServiceOption=&m_GameServiceOption;

	//调度回调
	m_AttemperEngineSink.m_pITimerEngine=m_TimerEngine.GetInterface();
	m_AttemperEngineSink.m_pIAttemperEngine=m_AttemperEngine.GetInterface();
	m_AttemperEngineSink.m_pITCPSocketService=m_TCPSocketService.GetInterface();
	m_AttemperEngineSink.m_pITCPNetworkEngine=m_TCPNetworkEngine.GetInterface();
	m_AttemperEngineSink.m_pIGameServiceManager=m_GameServiceManager.GetInterface();
	m_AttemperEngineSink.m_pIRecordDataBaseEngine=m_RecordDataBaseEngine.GetInterface();
	m_AttemperEngineSink.m_pIKernelDataBaseEngine=m_KernelDataBaseEngine.GetInterface();
	//m_AttemperEngineSink.m_pIGameMatchServiceManager=m_GameMatchServiceManager.GetInterface();
	m_AttemperEngineSink.m_pIDBCorrespondManager=(IDBCorrespondManager*)m_DBCorrespondManager.QueryInterface(IID_IDBCorrespondManager,VER_IDBCorrespondManager);

	//数据回调
	for (INT i=0;i<CountArray(m_RecordDataBaseSink);i++)
	{
		m_RecordDataBaseSink[i].m_pInitParameter=&m_InitParameter;
		m_RecordDataBaseSink[i].m_pGameParameter=&m_GameParameter;
		m_RecordDataBaseSink[i].m_pDataBaseParameter=&m_DataBaseParameter;
		m_RecordDataBaseSink[i].m_pGameServiceAttrib=&m_GameServiceAttrib;
		m_RecordDataBaseSink[i].m_pGameServiceOption=&m_GameServiceOption;
		m_RecordDataBaseSink[i].m_pIGameServiceManager=m_GameServiceManager.GetInterface();
		m_RecordDataBaseSink[i].m_pIDataBaseEngineEvent=QUERY_OBJECT_PTR_INTERFACE(pIAttemperEngine,IDataBaseEngineEvent);
	}

	//数据回调
	for (INT i=0;i<CountArray(m_KernelDataBaseSink);i++)
	{
		m_KernelDataBaseSink[i].m_pInitParameter=&m_InitParameter;
		m_KernelDataBaseSink[i].m_pGameParameter=&m_GameParameter;
		m_KernelDataBaseSink[i].m_pDataBaseParameter=&m_DataBaseParameter;
		m_KernelDataBaseSink[i].m_pGameServiceAttrib=&m_GameServiceAttrib;
		m_KernelDataBaseSink[i].m_pGameServiceOption=&m_GameServiceOption;
		m_KernelDataBaseSink[i].m_pIGameServiceManager=m_GameServiceManager.GetInterface();
		m_KernelDataBaseSink[i].m_pIDataBaseEngineEvent=QUERY_OBJECT_PTR_INTERFACE(pIAttemperEngine,IDataBaseEngineEvent);
		m_KernelDataBaseSink[i].m_pIDBCorrespondManager=(IDBCorrespondManager*)m_DBCorrespondManager.QueryInterface(IID_IDBCorrespondManager,VER_IDBCorrespondManager);
	}

	//配置网络
	m_TCPNetworkEngine->SetServiceParameter(m_GameServiceOption.wServerPort,m_GameServiceOption.wMaxPlayer,szCompilation);

	return true;
}

//启动内核
bool CServiceUnits::StartKernelService()
{
	//时间引擎
	if (m_TimerEngine->StartService()==false)
	{
		ASSERT(FALSE);
		return false;
	}

	//调度引擎
	if (m_AttemperEngine->StartService()==false)
	{
		ASSERT(FALSE);
		return false;
	}

	//协调引擎
	if (m_TCPSocketService->StartService()==false)
	{
		ASSERT(FALSE);
		return false;
	}

	//数据引擎
	if (m_RecordDataBaseEngine->StartService()==false)
	{
		ASSERT(FALSE);
		return false;
	}

	//数据引擎
	if (m_KernelDataBaseEngine->StartService()==false)
	{
		ASSERT(FALSE);
		return false;
	}

	//数据协调
	if (m_DBCorrespondManager.StartService()==false)
	{
		ASSERT(FALSE);
		return false;
	}

	return true;
}

//启动网络
bool CServiceUnits::StartNetworkService()
{
	//网络引擎
	if (m_TCPNetworkEngine->StartService()==false)
	{
		ASSERT(FALSE);
		return false;
	}

	return true;
}

//调整参数
bool CServiceUnits::RectifyServiceParameter()
{
	//读取属性
	m_GameServiceManager->GetServiceAttrib(m_GameServiceAttrib);
	if (lstrcmp(m_GameServiceAttrib.szServerDLLName,m_GameServiceManager.m_szModuleDllName) != 0){
		CTraceService::TraceString(TEXT("Game DLL Infomation Error"), TraceLevel_Exception);
		return false;
	}

	//调整参数
	if (m_GameServiceManager->RectifyParameter(m_GameServiceOption) == false){
		CTraceService::TraceString(TEXT("Game Moudle Parameter Error"), TraceLevel_Exception);
		return false;
	}

	//服务类型
	if ((m_GameServiceOption.wServerType&m_GameServiceAttrib.wSupporType) == 0){
		CTraceService::TraceString(TEXT("Game Moudle Room Info Error"), TraceLevel_Exception);
		return false;
	}

	//占位调整
	if (m_GameServiceAttrib.wChairCount == MAX_CHAIR){
		CServerRule::SetAllowAndroidSimulate(m_GameServiceOption.dwServerRule, false);
	}

	//作弊模式
	if ((m_GameServiceOption.cbDistributeRule&DISTRIBUTE_ALLOW) != 0){
		//设置作弊
		CServerRule::SetAllowAvertCheatMode(m_GameServiceOption.dwServerRule, true);

		//最少人数
		m_GameServiceOption.wMinDistributeUser = __max(m_GameServiceAttrib.wChairCount, m_GameServiceOption.wMinDistributeUser);

		//最大人数
		if (m_GameServiceOption.wMaxDistributeUser != 0){
			m_GameServiceOption.wMaxDistributeUser = __max(m_GameServiceOption.wMaxDistributeUser, m_GameServiceOption.wMinDistributeUser);
		}
	}

	//游戏记录
	//if(m_GameServiceOption.wServerType & (GAME_GENRE_GOLD/*|GAME_GENRE_MATCH*/)){
	//	CServerRule::SetRecordGameScore(m_GameServiceOption.dwServerRule, true);
	//}

	//即时写分
	if(m_GameServiceOption.wServerType & (GAME_GENRE_GOLD/*|GAME_GENRE_MATCH*/)){
		CServerRule::SetImmediateWriteScore(m_GameServiceOption.dwServerRule, true);
	}

	//挂接设置
	if(m_GameServiceOption.wSortID == 0){ 
		m_GameServiceOption.wSortID = 500;
	}
	if(m_GameServiceOption.wKindID == 0){
		m_GameServiceOption.wKindID = m_GameServiceAttrib.wKindID;
	}

	//最大人数
	WORD wMaxPlayer = m_GameServiceOption.wTableCount * m_GameServiceAttrib.wChairCount;
	m_GameServiceOption.wMaxPlayer = __max(m_GameServiceOption.wMaxPlayer, wMaxPlayer + RESERVE_USER_COUNT);

	//最小积分
	if (m_GameServiceOption.wServerType & GAME_GENRE_GOLD){
		m_GameServiceOption.lMinTableScore += m_GameServiceOption.lServiceScore;
		m_GameServiceOption.lMinTableScore = __max(m_GameServiceOption.lMinTableScore, m_GameServiceOption.lServiceScore);
	}

	//限制调整
	if (m_GameServiceOption.lMaxEnterScore != 0L){
		m_GameServiceOption.lMaxEnterScore = __max(m_GameServiceOption.lMaxEnterScore, m_GameServiceOption.lMinTableScore);
	}

	//比赛调整
	//if (m_GameMatchServiceManager.GetInterface() != NULL){
	//	m_GameMatchServiceManager->RectifyServiceOption(&m_GameServiceOption, &m_GameServiceAttrib);
	//}

	return true;
}

wstring UTF8ToUnicode( const string& str ){ 

	int  len = 0; 
	len = str.length(); 
	int  unicodeLen = ::MultiByteToWideChar( CP_UTF8, 0, str.c_str(), -1, NULL, 0 );  
	wchar_t *  pUnicode;  
	pUnicode = new  wchar_t[unicodeLen+1];  
	memset(pUnicode,0,(unicodeLen+1)*sizeof(wchar_t));  
	::MultiByteToWideChar( CP_UTF8, 0, str.c_str(), -1, (LPWSTR)pUnicode, unicodeLen );  
	wstring  rt;  
	rt = ( wchar_t* )pUnicode; 
	delete  pUnicode;    
	return  rt; 
}

//设置状态
bool CServiceUnits::SetServiceStatus(enServiceStatus ServiceStatus) {
	//状态判断
	if (m_ServiceStatus!=ServiceStatus) {
		//错误通知
		if ((m_ServiceStatus != ServiceStatus_Service) && (ServiceStatus == ServiceStatus_Stop)) {
			LPCTSTR pszString=TEXT("服务启动失败");
			CTraceService::TraceString(pszString, TraceLevel_Exception);
		}
		//设置变量
		m_ServiceStatus = ServiceStatus;
		//connect with java
		if (m_ServiceStatus == ServiceStatus_Service){
			web::http::uri_builder builder;
			builder.append_query(U("action"),U("initGameServer"));

			//大厅注册
			TCHAR parameterStr[50] = U("");
			_sntprintf(parameterStr, sizeof(parameterStr), TEXT("%d,%s,%s,%d,%s"), m_GameServiceOption.wKindID,
				m_GameServiceAttrib.szGameName, m_InitParameter.m_ServiceAddress.szAddress, m_GameServiceOption.wServerPort, m_GameServiceOption.szRoomType);
			
			builder.append_query(U("parameter"),parameterStr);
			utility::string_t v;
			std::map<wchar_t*,wchar_t*> hAdd;
			LOGI(L"connect with JAVA:" << parameterStr);
			if (HttpRequest(web::http::methods::POST, m_InitParameter.m_LobbyURL, builder.to_string(), true, v, hAdd)) {
				
				CString out(v.c_str());
				::OutputDebugString(out);

				web::json::value root = web::json::value::parse(v);
				utility::string_t actionStr = root[U("action")].as_string();

				ASSERT(actionStr == U("initGameServer"));

				int errorCode = root[U("errorCode")].as_integer();

				if(!errorCode){ 
					//返回正确
					ZeroMemory(m_GameServiceOption.szRoomType,sizeof(m_GameServiceOption.szRoomType));
					lstrcpyn(m_GameServiceOption.szRoomType,root[U("roomType")].as_string().c_str(),CountArray(m_GameServiceOption.szRoomType));
					
					ZeroMemory(m_GameServiceOption.szGameType,sizeof(m_GameServiceOption.szGameType));
					lstrcpyn(m_GameServiceOption.szGameType,root[U("gameType")].as_string().c_str(),LEN_SERVER);
					
					ZeroMemory(m_GameServiceOption.szRoomName_A,sizeof(m_GameServiceOption.szRoomName));
					lstrcpyn(m_GameServiceOption.szRoomName_A,root[U("roomName")].as_string().c_str(),LEN_SERVER);

					ZeroMemory(m_GameServiceOption.gameRoomOnlyId,sizeof(m_GameServiceOption.gameRoomOnlyId));
					lstrcpyn(m_GameServiceOption.gameRoomOnlyId,root[U("gameRoomOnlyId")].as_string().c_str(),sizeof(m_GameServiceOption.gameRoomOnlyId));

					char tempName[LEN_SERVER*2] = "";
					setlocale(LC_ALL,"C");
					wcstombs(tempName, root[U("roomName")].as_string().c_str(), LEN_SERVER);

					std::string tempStrName;
					tempStrName.append(tempName);
					m_GameServiceOption.szRoomName = UTF8ToUnicode(tempStrName);

					std::string tempStr = boost::locale::conv::between(tempName,"GBK","UTF-8");
					//m_GameServiceOption.szRoomName = tempStr;

					TCHAR szMessage[128]=TEXT("");
					int len = MultiByteToWideChar(CP_ACP,0,tempStr.c_str(),-1,NULL,0);
					TCHAR *RoomName_W = new TCHAR[len];
					ZeroMemory(RoomName_W,sizeof(TCHAR)*len);
					MultiByteToWideChar(CP_ACP,0,tempStr.c_str(),tempStr.size(),RoomName_W,len*sizeof(TCHAR));

					ZeroMemory(m_GameServiceOption.szRoomName_w,sizeof(m_GameServiceOption.szRoomName_w));
					lstrcpyn(m_GameServiceOption.szRoomName_w,RoomName_W,len);

					delete[] RoomName_W; 

					m_GameServiceOption.dwRoomID = root[U("roomId")].as_integer();

					web::json::value gameParas;
					web::json::value returnParams = root[U("params")];
					for(int i = 0; i < returnParams.size(); i++){
						std::wstring keyStr = returnParams[i][U("key")].as_string();
						web::json::value val = returnParams[i][U("value")];
						if(!StrCmpW(keyStr.c_str(),U("MIN_MONEY"))){
							 //最低带入
							m_GameServiceOption.lMinEnterScore = _wtoi(val.as_string().c_str()) * 100;
						}else if(!StrCmpW(keyStr.c_str(),U("MAX_WAGER"))){
							//最大下注
							m_GameServiceOption.dwMaxScore = _wtoi(val.as_string().c_str()) * 100;
						}else if(!StrCmpW(keyStr.c_str(),U("SCORE_VALUES"))) {
							//筹码值
							ZeroMemory(m_GameServiceOption.cbScoreValues,sizeof(m_GameServiceOption.cbScoreValues));
							std::string subValue = "";
							int valueIndex = 0;
							for (int i = 0 ; i < val.as_string().size(); i++){
								char cHAR = val.as_string().c_str()[i];
								if(cHAR == ',' || i == val.as_string().size() - 1){
									ASSERT(valueIndex <= 19);
									
									if (i == val.as_string().size() - 1){
										subValue = subValue + cHAR;
									}

									m_GameServiceOption.cbScoreValues[valueIndex] = atof(subValue.c_str()) * 100;
									subValue = "";
									valueIndex++;
								}else{ 
									subValue = subValue + cHAR;
								}
							}
						}else if(!StrCmpW(keyStr.substr(0,5).c_str(), U("GAMES"))){
							gameParas[keyStr] = val;
						}else if(!StrCmpW(keyStr.c_str(), U("Table_DistributePro"))){
							int lastPos = -1;
							std::vector<WORD> t_dp;
							std::wstring strValue = val.as_string();
							for (int i = 0; i < strValue.length(); i++){
								if (i == strValue.length() - 1){
									t_dp.push_back(_wtoi(strValue.substr(lastPos+1).c_str()));
									break;
								}

								if (strValue[i] == L','){
									t_dp.push_back(_wtoi(strValue.substr(lastPos+1,i-lastPos-1).c_str()));
									lastPos = i;
								}

							}
							if(m_GameServiceOption.wTableDistributePro){
								delete []m_GameServiceOption.wTableDistributePro;
								m_GameServiceOption.wTableDistributePro = NULL;
							}

							m_GameServiceOption.wTableDistributePro = new WORD[t_dp.size()];
							for (int i = 0; i < t_dp.size();i++){
								m_GameServiceOption.wTableDistributePro[i] = t_dp[i];
							}

						}else if(!StrCmpW(keyStr.c_str(),U("gameType"))){
							std::wstring wStr = val.as_string();
							_sntprintf(m_GameServiceOption.szGameGroup,sizeof(m_GameServiceOption.szGameGroup),TEXT("%s"),wStr.c_str());		
						}else if(!StrCmpW(keyStr.c_str(), U("vassalage"))){
							std::wstring wStr = val.as_string();
							_sntprintf(m_GameServiceOption.szVassalage,sizeof(m_GameServiceOption.szVassalage),TEXT("%s"),wStr.c_str());
						}
					}
					//Read Ini File
					int readIniStatus = CIniReader::getInstance()->readIniFile(this->m_GameServiceAttrib.szServerDLLName);
					if(readIniStatus == INI_STATUS_READ_SUCCESS){		
						auto iter = CIniReader::getInstance()->localParaMeter.begin();
						while(iter != CIniReader::getInstance()->localParaMeter.end()){
							utility::string_t tmpKey(iter->first.begin(), iter->first.end());
							utility::string_t tmpVal(iter->second.begin(), iter->second.end());
							gameParas[tmpKey] = web::json::value::string(tmpVal);
							iter++;
						}
					}else{
						CStringA outStr;
						outStr.Format("Read Config Error: %d", readIniStatus);
						::OutputDebugStringA(outStr);
					}

					if (gameParas.size()){
						m_AttemperEngineSink.InitGameParas(gameParas);
					}

					ExchangeLobby();

					m_cht.InitThread(this);
					bool bSuccess = m_cht.StartThread();
					if (bSuccess == false)  throw TEXT("HeartBeat Lobby Service Run Fail");
					
					//heart beat
					//m_AttemperEngineSink.OnLobbyConnect();
					//获取牌局 
					//m_AttemperEngineSink.GetGameCount();

					m_AttemperEngineSink.StartLoadAndroid();

										
					#if ANDROID_AUTO_PALY
						m_AttemperEngineSink.m_pITimerEngine->SetTimer(TIMER_ANDROID_AUTO_PLAY, 2*1000L, TIMES_INFINITY, NULL);
					#endif
				
				}else{
					//返回错误
					m_ServiceStatus = ServiceStatus_RegFail;
					LOGI(U("游戏服务器注册错误: "), + errorCode);
				}
			}else{
				//错误
				m_ServiceStatus = ServiceStatus_RegFail;
				LOGI(U("游戏服务器注册错误: "), + errorCode);
			}
		}

		//状态通知
		ASSERT(m_pIServiceUnitsSink!=NULL);
		if (m_pIServiceUnitsSink!=NULL){
			m_pIServiceUnitsSink->OnServiceUnitsStatus(m_ServiceStatus);
		}
	}
	return true;
}

//发送控制
bool CServiceUnits::SendControlPacket(WORD wControlID, VOID * pData, WORD wDataSize)
{
	//状态效验
	ASSERT(m_AttemperEngine.GetInterface()!=NULL);
	if (m_AttemperEngine.GetInterface()==NULL) return false;

	//发送控制
	m_AttemperEngine->OnEventControl(wControlID,pData,wDataSize);

	return true;
}

//连接信息
bool CServiceUnits::LoadDataBaseParameter(LPCTSTR pszDataBaseAddr, LPCTSTR pszDataBaseName, tagDataBaseParameter & DataBaseParameter)
{
	//变量定义
	CDataBaseAide PlatformDBAide;
	CDataBaseHelper PlatformDBModule;

	//创建对象
	if ((PlatformDBModule.GetInterface()==NULL)&&(PlatformDBModule.CreateInstance()==false))
	{
		ASSERT(FALSE);
		return false;
	}

	//变量定义
	tagDataBaseParameter * pPlatformDBParameter=&m_InitParameter.m_PlatformDBParameter;

	//设置连接
	PlatformDBModule->SetConnectionInfo(pPlatformDBParameter->szDataBaseAddr,pPlatformDBParameter->wDataBasePort,
		pPlatformDBParameter->szDataBaseName,pPlatformDBParameter->szDataBaseUser,pPlatformDBParameter->szDataBasePass);

	//获取信息
	try
	{
		//发起连接
		PlatformDBModule->OpenConnection();
		PlatformDBAide.SetDataBase(PlatformDBModule.GetInterface());

		//连接信息
		PlatformDBAide.ResetParameter();
		PlatformDBAide.AddParameter(TEXT("@strDataBaseAddr"),pszDataBaseAddr);

		//执行查询
		if (PlatformDBAide.ExecuteProcess(TEXT("GSP_GS_LoadDataBaseInfo"),true)!=DB_SUCCESS)
		{
			//构造信息
			TCHAR szErrorDescribe[128]=TEXT("");
			PlatformDBAide.GetValue_String(TEXT("ErrorDescribe"),szErrorDescribe,CountArray(szErrorDescribe));

			//提示消息
			CTraceService::TraceString(szErrorDescribe,TraceLevel_Exception);
			LOGI("CServiceUnits::LoadDataBaseParameter, GSP_GS_LoadDataBaseInfo, "<<szErrorDescribe);

			return false;
		}

		//读取密文
		TCHAR szDBUserRead[512]=TEXT(""),szDBPassRead[512]=TEXT("");
		PlatformDBAide.GetValue_String(TEXT("DBUser"),szDBUserRead,CountArray(szDBUserRead));
		PlatformDBAide.GetValue_String(TEXT("DBPassword"),szDBPassRead,CountArray(szDBPassRead));

		//获取信息
		DataBaseParameter.wDataBasePort=PlatformDBAide.GetValue_WORD(TEXT("DBPort"));
		lstrcpyn(DataBaseParameter.szDataBaseAddr,pszDataBaseAddr,CountArray(DataBaseParameter.szDataBaseAddr));
		lstrcpyn(DataBaseParameter.szDataBaseName,pszDataBaseName,CountArray(DataBaseParameter.szDataBaseName));

		//解密密文
		TCHAR szDataBaseUser[32]=TEXT(""),szDataBasePass[32]=TEXT("");
		CWHEncrypt::XorCrevasse(szDBUserRead,DataBaseParameter.szDataBaseUser,CountArray(DataBaseParameter.szDataBaseUser));
		CWHEncrypt::XorCrevasse(szDBPassRead,DataBaseParameter.szDataBasePass,CountArray(DataBaseParameter.szDataBasePass));
	}
	catch (IDataBaseException * pIException)
	{
		//错误信息
		LPCTSTR pszDescribe=pIException->GetExceptionDescribe();
		CTraceService::TraceString(pszDescribe,TraceLevel_Exception);
		LOGI("CServiceUnits::LoadDataBaseParameter, "<<pszDescribe);

		return false;
	}

	return true;
}

//控制消息
LRESULT CServiceUnits::OnUIControlRequest(WPARAM wParam, LPARAM lParam)
{
	//变量定义
	tagDataHead DataHead;
	BYTE cbBuffer[MAX_ASYNCHRONISM_DATA];

	//提取数据
	CWHDataLocker DataLocker(m_CriticalSection);
	if (m_DataQueue.DistillData(DataHead,cbBuffer,sizeof(cbBuffer))==false)
	{
		ASSERT(FALSE);
		return NULL;
	}

	//数据处理
	switch (DataHead.wIdentifier)
	{
	case UI_CORRESPOND_RESULT:		//协调成功
		{
			//效验消息
			ASSERT(DataHead.wDataSize==sizeof(CP_ControlResult));
			if (DataHead.wDataSize!=sizeof(CP_ControlResult)) return 0;

			//变量定义
			CP_ControlResult * pControlResult=(CP_ControlResult *)cbBuffer;

			//失败处理
			if ((m_ServiceStatus!=ServiceStatus_Service)&&(pControlResult->cbSuccess==ER_FAILURE))
			{
				ConcludeService();
				return 0;
			}

			//成功处理
			if ((m_ServiceStatus!=ServiceStatus_Service)&&(pControlResult->cbSuccess==ER_SUCCESS))
			{
				//设置状态
				SetServiceStatus(ServiceStatus_Service);
			}

			return 0;
		}
	case UI_SERVICE_CONFIG_RESULT:	//配置结果
		{
			//效验消息
			ASSERT(DataHead.wDataSize==sizeof(CP_ControlResult));
			if (DataHead.wDataSize!=sizeof(CP_ControlResult)) return 0;

			//变量定义
			CP_ControlResult * pControlResult=(CP_ControlResult *)cbBuffer;

			//失败处理
			if ((m_ServiceStatus!=ServiceStatus_Service)&&(pControlResult->cbSuccess==ER_FAILURE))
			{
				ConcludeService();
				return 0;
			}

			//成功处理
			if ((m_ServiceStatus!=ServiceStatus_Service)&&(pControlResult->cbSuccess==ER_SUCCESS))
			{
				//启动网络
				if (StartNetworkService()==false)
				{
					ConcludeService();
					return 0;
				}

				//连接协调
				SendControlPacket(CT_CONNECT_CORRESPOND, NULL, 0);
			}

			return 0;
		}
	}

	return 0;
}
//bool CServiceUnits::GetRandHardware()
//{
//	#include<ctime>
//	time_t now_time;	
//	m_GameServiceOption.GetRandTime = time(NULL);
//	m_GameServiceOption.IntRandFromHardware=100;
//	return true;
//}
bool CServiceUnits::ExchangeLobby()
{
	//GetRandHardware();
	web::http::uri_builder builder;
	builder.append_query(U("action"),U("gameServerHeart"));

	TCHAR parameterStr[50];
	_sntprintf(parameterStr,sizeof(parameterStr),TEXT("%d,%s"),m_GameServiceOption.wKindID, m_GameServiceOption.szRoomType);

	builder.append_query(U("parameter"),parameterStr);
	utility::string_t v;
	std::map<wchar_t*,wchar_t*> hAdd;
	if(HttpRequest(web::http::methods::POST,m_InitParameter.m_LobbyURL,builder.to_string(),true,v,hAdd))
	{
		web::json::value root = web::json::value::parse(v);
		DWORD errorCode = root[U("errorCode")].as_integer();
		DWORD gameNewsChg = root[U("gameNewsChg")].as_integer();
		BOOL closeServer = root[L"redayCloseServer"].as_integer();
		////////////////////////////steve add for kick out players ///////////////////////////////////////////////////////////////
		char *tempStr = new char[v.length()*2 + 1];
			setlocale(LC_ALL,"C");
			wcstombs(tempStr, v.c_str(), v.length()*sizeof(wchar_t));
			boost::property_tree::ptree pt;
		try{
			std::istringstream iss;
			iss.str(tempStr);	
			boost::property_tree::json_parser::read_json(iss,pt);
			iss.clear();
			int errorCode = pt.get<int>("errorCode");
		
			
		}catch(boost::property_tree::file_parser_error &err)
		{				
				
		}
		////////////////////////////////////////////////////////
		if (!errorCode)
		{
			m_AttemperEngineSink.SwitchCSStatus(closeServer);

			if (gameNewsChg)
			{
				//获取跑马灯
				if (GetGameNews())
				{
					char tempName[1024*2] = "";
					setlocale(LC_ALL,"C");
					wcstombs(tempName, m_gameNews.to_string().c_str(), 1024);

					std::string tempStrName;
					tempStrName.append(tempName);
					std::wstring gameNewsStr = UTF8ToUnicode(tempStrName);

					m_AttemperEngineSink.SendGameMessage(gameNewsStr.c_str(),SMT_GAMENEWS);
				}
			}
		}
		else LOGE(L"heart with lobby error!");

	}
	else //错误
	{
		LOGE(L"heart with lobby error!");
	}

	return true;

}


//获取滚动消息
bool CServiceUnits::GetGameNews()
{
	web::http::uri_builder builder;
	builder.append_query(U("action"),U("gameNews"));

	TCHAR parameterStr[50];
	_sntprintf(parameterStr,sizeof(parameterStr),TEXT("%d,%s"),m_GameServiceOption.wKindID, m_GameServiceOption.szRoomType);

	builder.append_query(U("parameter"),parameterStr);
	utility::string_t v;
	std::map<wchar_t*,wchar_t*> hAdd;
	if(HttpRequest(web::http::methods::POST,m_InitParameter.m_LobbyURL,builder.to_string(),true,v,hAdd))
	{
		web::json::value root = web::json::value::parse(v);
		DWORD errorCode = root[U("errorCode")].as_integer();

		if (!errorCode)
		{
			m_gameNews = root[U("gameNews")];
		}
		else
		{
			LOGE(L"GameNews Error!");
			return false;
		}

		return true;
	}
	else return false;
		
	return true;

}

#include <io.h>
bool CServiceUnits::DeleteLog(TCHAR* strDirName,int iSeconds)
{

	if (iSeconds <= 0)
	{
		return false;
	}

	time_t t; 
	tm* local; 
	char srcBuf[128]= {0};  

	t = time(NULL);
	t-=iSeconds;

	local = localtime(&t); //转为本地时间  
	strftime(srcBuf, 128, "%Y%m%d%H%M", local);


	CFileFind tempFind;
	TCHAR strTempFileFind[MAX_PATH];
	swprintf(strTempFileFind,sizeof(strTempFileFind),TEXT("%s//*.*"), strDirName);

	BOOL IsFinded = tempFind.FindFile(strTempFileFind);

	while (IsFinded)
	{
		IsFinded = tempFind.FindNextFile();

		if (!tempFind.IsDots()) 
		{
			TCHAR strFoundFileName[MAX_PATH];

			lstrcpyn(strFoundFileName, tempFind.GetFileName().GetBuffer(MAX_PATH),sizeof(strFoundFileName));

			if (tempFind.IsDirectory())
			{
				TCHAR strTempDir[MAX_PATH];

				swprintf(strTempDir, sizeof(strTempDir),TEXT("%s//%s"), strDirName, strFoundFileName);

				DeleteLog(strTempDir,iSeconds);
			}
			else
			{
				std::wstring timeStr_w = TEXT("");
				WORD cnt = 0;
				for (int i = 0; i < MAX_PATH; i ++)
				{
					if(strFoundFileName[i] == '_')
					{
						cnt ++;
						if(cnt == 1)
						{
							while(cnt==1)
							{
								i++;
								if (strFoundFileName[i] == '_')
								{
									cnt++;
								}
								else timeStr_w+=strFoundFileName[i];
							}
							
							break;
						}
					}

				}

				char tarBuf[128] = "";
				wcstombs(tarBuf,timeStr_w.c_str(),64);

				if(strcmp(srcBuf,tarBuf) > 0)
				{
					TCHAR strTempFileName[MAX_PATH];

					swprintf(strTempFileName,sizeof(strTempFileName), TEXT("%s//%s"), strDirName, strFoundFileName);

					DeleteFile(strTempFileName);

				}
			}
		}
	}

	tempFind.Close();

	return TRUE;

}

//////////////////////////////////////////////////////////////////////////////////

CHeartThread::CHeartThread(void)
{
	m_dwTickCount = 0L;
	m_csu = NULL;
}

CHeartThread::~CHeartThread(void)
{

}

bool CHeartThread::InitThread(CServiceUnits * csu)
{
	m_dwTickCount = 0L;
	m_csu = csu;
	return true;
}

bool CHeartThread::OnEventThreadRun()
{

	Sleep(500);
	m_dwTickCount += 500L;

	if (m_dwTickCount >= 20000L)
	{
		m_dwTickCount = 0L;
		m_csu->ExchangeLobby();
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
CLogClearThread::CLogClearThread(void)
{
	m_dwTickCount = 0;
	m_csu = NULL;
}

CLogClearThread::~CLogClearThread(void)
{

}

bool CLogClearThread::InitThread(CServiceUnits * csu,int iSeconds)
{
	m_csu = csu;
	m_iSeconds = iSeconds;
	return true;
}

bool CLogClearThread::OnEventThreadRun()
{
	//clear log
	TCHAR szWorkDir[MAX_PATH]=TEXT("");
	CWHService::GetWorkDirectory(szWorkDir,CountArray(szWorkDir));

	TCHAR szLogFile[MAX_PATH]=TEXT("");
	_sntprintf(szLogFile,CountArray(szLogFile),TEXT("%s\\%s\\"),szWorkDir,L"log");

	m_csu->DeleteLog(szLogFile,m_iSeconds);

	return false;
}