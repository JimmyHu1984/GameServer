#include "StdAfx.h"
#include "InitParameter.h"


using namespace utility;
using namespace web;
using namespace web::http;
using namespace web::http::client;
using namespace concurrency::streams;


//////////////////////////////////////////////////////////////////////////////////

//时间定义
#define TIME_CONNECT				30									//重连时间
#define TIME_COLLECT				30									//统计时间
#define ACS_TIMEOUT					15									//统计时间

//客户时间
#define TIME_INTERMIT				0									//中断时间
#define TIME_ONLINE_COUNT			600									//人数时间

//////////////////////////////////////////////////////////////////////////////////

//构造函数
CInitParameter::CInitParameter()
{ 
	m_testscore = 0;
	InitParameter();
}

int HttpRequest(method m, wchar_t url[], utility::string_t builder, bool type,utility::string_t & v, std::map<wchar_t*,wchar_t*> hAdd,int flag)
{
	http_client_config webConfig;
	utility::seconds timeOutSec(ACS_TIMEOUT);
	webConfig.set_timeout(timeOutSec);
	http_client client(url, webConfig);	

	http_request request(std::move(m));
	std::map<wchar_t*,wchar_t*>::iterator iter;
	for(iter = hAdd.begin(); iter != hAdd.end(); iter++)
	{
		request.headers().add(iter->first,iter->second);
	}

	if (!flag)
		request.set_request_uri(builder);
	else
	{
		request.set_body(builder,L"application/json");
	    //request.headers().add(U("Content-Length"),request.body().buf_size);
	}

	auto reqFunc = [url](http_response response){

		std::wostringstream stream;
		stream << url <<  L" returned returned status code " << L'.' << response.status_code();

		if(response.status_code() == status_codes::OK)
		{
			stream << L"Content type: " << response.headers().content_type();
			stream << L" Content length: " << response.headers().content_length() << L"bytes";
			LOGI(stream.str());

			auto bodyStream = response.body();
			concurrency::streams::stringstreambuf sbuffer;
			auto& target = sbuffer.collection();

			bodyStream.read_to_end(sbuffer).get();

			stream.str(std::wstring());
			stream << target.c_str();
			LOGI(stream.str());

			stream.clear();
			sbuffer.close();

			throw(stream.str());

		}
		else
		{
			throw(0);
		}

	};

	try
	{
		//syn or asyn
		if(type)client.request(request).then(reqFunc).wait();
		else client.request(request).then(reqFunc);
	}

	//返回正确
	catch(utility::string_t &jsonStr)
	{
		v = jsonStr;
		return INTERNET_REQUEST_SUCCESS;
	}

	//连接错误
	catch(json::json_exception excep)
	{
		std::wcout << excep.what() << std::endl;
		return INTERNET_REQUEST_FAIL;
	}

	//http连接异常
	catch(web::http::http_exception excep)
	{
		std::error_code serverError = excep.error_code();
		if(ERROR_INTERNET_TIMEOUT == serverError.value()){
			return INTERNET_REQUEST_TIMEOUT; 
		}
		if(ERROR_INTERNET_NAME_NOT_RESOLVED == serverError.value()){
			return INTERNET_REQUEST_HOST_NAME_ERROR; 
		}

		std::wcout << excep.what() << std::endl;
		return INTERNET_REQUEST_FAIL;
	}

	//其他异常
	catch(...)
	{
		std::wcout << L"异常错误" << std::endl;
		return INTERNET_REQUEST_UNKNOWN;
	}
    
	// Handle error cases, for now return empty json value...
    /* Output:
    Content-Type must be application/json to extract (is: text/html)
    */
}

//析构函数
CInitParameter::~CInitParameter()
{
}

//初始化
VOID CInitParameter::InitParameter()
{
	//时间定义
	m_wConnectTime=TIME_CONNECT;
	m_wCollectTime=TIME_COLLECT;

	//协调信息
	m_wCorrespondPort=PORT_CENTER;
	ZeroMemory(&m_CorrespondAddress,sizeof(m_CorrespondAddress));

	//配置信息
	ZeroMemory(m_szServerName,sizeof(m_szServerName));
	ZeroMemory(&m_ServiceAddress,sizeof(m_ServiceAddress));
	ZeroMemory(&m_TreasureDBParameter,sizeof(m_TreasureDBParameter));
	ZeroMemory(&m_PlatformDBParameter,sizeof(m_PlatformDBParameter));


	//Load Lobby URL
	TCHAR szWorkDir[MAX_PATH]=TEXT("");
	CWHService::GetWorkDirectory(szWorkDir,CountArray(szWorkDir));

	//构造路径
	TCHAR szIniFile[MAX_PATH]=TEXT("");
	_sntprintf(szIniFile,CountArray(szIniFile),TEXT("%s\\%s"),szWorkDir,U("ServerParameter.ini"));

	//读取配置
	CWHIniData IniData;
	IniData.SetIniFilePath(szIniFile);
	IniData.ReadEncryptString(U("URL"),U("Lobby"),NULL,m_LobbyURL,LEN_URL);
	IniData.ReadEncryptString(U("URL"),U("ACS_USER_QUERY"),NULL,m_ACS_USER_QUERY,LEN_URL);
	IniData.ReadEncryptString(U("URL"),U("ACS_CREDIT"),NULL,m_ACS_CREDIT,LEN_URL);
	
	std::wstring strtestacsurl = m_ACS_CREDIT;
	int idotindex =	strtestacsurl.find(L"test:");
	if(idotindex != -1)
	{
		std::wstring strtestscore =	strtestacsurl.substr(idotindex + 5);
		m_testscore = _wtoi(strtestscore.c_str());
	}

	IniData.ReadEncryptString(U("URL"),U("ACS_DEBIT"),NULL,m_ACS_DEBIT,LEN_URL);

	return;
}

//加载配置
VOID CInitParameter::LoadInitParameter()
{
	//重置参数
	InitParameter();

	//获取路径
	TCHAR szWorkDir[MAX_PATH]=TEXT("");
	CWHService::GetWorkDirectory(szWorkDir,CountArray(szWorkDir));

	//构造路径
	TCHAR szIniFile[MAX_PATH]=TEXT("");
	_sntprintf(szIniFile,CountArray(szIniFile),TEXT("%s\\ServerParameter.ini"),szWorkDir);

	//读取配置
	CWHIniData IniData;
	IniData.SetIniFilePath(szIniFile);

	//读取配置
	IniData.ReadEncryptString(TEXT("ServerInfo"),TEXT("ServiceName"),NULL,m_szServerName,CountArray(m_szServerName));
	IniData.ReadEncryptString(TEXT("ServerInfo"),TEXT("ServiceAddr"),NULL,m_ServiceAddress.szAddress,CountArray(m_ServiceAddress.szAddress));

	TCHAR portStr[20] = L"";
	//协调信息
	m_wCorrespondPort=PORT_CENTER;
	IniData.ReadEncryptString(TEXT("ServerInfo"),TEXT("CorrespondAddr"),NULL,m_CorrespondAddress.szAddress,CountArray(m_CorrespondAddress.szAddress));

	//连接信息
	IniData.ReadEncryptString(TEXT("TreasureDB"),TEXT("DBPort"),NULL,portStr,sizeof(portStr));
	m_TreasureDBParameter.wDataBasePort=_wtoi(portStr);
	ZeroMemory(portStr,sizeof(portStr));

	IniData.ReadEncryptString(TEXT("TreasureDB"),TEXT("DBAddr"),NULL,m_TreasureDBParameter.szDataBaseAddr,CountArray(m_TreasureDBParameter.szDataBaseAddr));
	IniData.ReadEncryptString(TEXT("TreasureDB"),TEXT("DBUser"),NULL,m_TreasureDBParameter.szDataBaseUser,CountArray(m_TreasureDBParameter.szDataBaseUser));
	IniData.ReadEncryptString(TEXT("TreasureDB"),TEXT("DBPass"),NULL,m_TreasureDBParameter.szDataBasePass,CountArray(m_TreasureDBParameter.szDataBasePass));
	IniData.ReadEncryptString(TEXT("TreasureDB"),TEXT("DBName"),szTreasureDB,m_TreasureDBParameter.szDataBaseName,CountArray(m_TreasureDBParameter.szDataBaseName));

	//连接信息
	IniData.ReadEncryptString(TEXT("PlatformDB"),TEXT("DBPort"),NULL,portStr,sizeof(portStr));
	m_PlatformDBParameter.wDataBasePort=_wtoi(portStr);
	ZeroMemory(portStr,sizeof(portStr));
	
	IniData.ReadEncryptString(TEXT("PlatformDB"),TEXT("DBAddr"),NULL,m_PlatformDBParameter.szDataBaseAddr,CountArray(m_PlatformDBParameter.szDataBaseAddr));
	IniData.ReadEncryptString(TEXT("PlatformDB"),TEXT("DBUser"),NULL,m_PlatformDBParameter.szDataBaseUser,CountArray(m_PlatformDBParameter.szDataBaseUser));
	IniData.ReadEncryptString(TEXT("PlatformDB"),TEXT("DBPass"),NULL,m_PlatformDBParameter.szDataBasePass,CountArray(m_PlatformDBParameter.szDataBasePass));
	IniData.ReadEncryptString(TEXT("PlatformDB"),TEXT("DBName"),szPlatformDB,m_PlatformDBParameter.szDataBaseName,CountArray(m_PlatformDBParameter.szDataBaseName));
	
	return;
}

//////////////////////////////////////////////////////////////////////////////////
