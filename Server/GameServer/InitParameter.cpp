#include "StdAfx.h"
#include "InitParameter.h"


using namespace utility;
using namespace web;
using namespace web::http;
using namespace web::http::client;
using namespace concurrency::streams;


//////////////////////////////////////////////////////////////////////////////////

//ʱ�䶨��
#define TIME_CONNECT				30									//����ʱ��
#define TIME_COLLECT				30									//ͳ��ʱ��
#define ACS_TIMEOUT					15									//ͳ��ʱ��

//�ͻ�ʱ��
#define TIME_INTERMIT				0									//�ж�ʱ��
#define TIME_ONLINE_COUNT			600									//����ʱ��

//////////////////////////////////////////////////////////////////////////////////

//���캯��
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

	//������ȷ
	catch(utility::string_t &jsonStr)
	{
		v = jsonStr;
		return INTERNET_REQUEST_SUCCESS;
	}

	//���Ӵ���
	catch(json::json_exception excep)
	{
		std::wcout << excep.what() << std::endl;
		return INTERNET_REQUEST_FAIL;
	}

	//http�����쳣
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

	//�����쳣
	catch(...)
	{
		std::wcout << L"�쳣����" << std::endl;
		return INTERNET_REQUEST_UNKNOWN;
	}
    
	// Handle error cases, for now return empty json value...
    /* Output:
    Content-Type must be application/json to extract (is: text/html)
    */
}

//��������
CInitParameter::~CInitParameter()
{
}

//��ʼ��
VOID CInitParameter::InitParameter()
{
	//ʱ�䶨��
	m_wConnectTime=TIME_CONNECT;
	m_wCollectTime=TIME_COLLECT;

	//Э����Ϣ
	m_wCorrespondPort=PORT_CENTER;
	ZeroMemory(&m_CorrespondAddress,sizeof(m_CorrespondAddress));

	//������Ϣ
	ZeroMemory(m_szServerName,sizeof(m_szServerName));
	ZeroMemory(&m_ServiceAddress,sizeof(m_ServiceAddress));
	ZeroMemory(&m_TreasureDBParameter,sizeof(m_TreasureDBParameter));
	ZeroMemory(&m_PlatformDBParameter,sizeof(m_PlatformDBParameter));


	//Load Lobby URL
	TCHAR szWorkDir[MAX_PATH]=TEXT("");
	CWHService::GetWorkDirectory(szWorkDir,CountArray(szWorkDir));

	//����·��
	TCHAR szIniFile[MAX_PATH]=TEXT("");
	_sntprintf(szIniFile,CountArray(szIniFile),TEXT("%s\\%s"),szWorkDir,U("ServerParameter.ini"));

	//��ȡ����
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

//��������
VOID CInitParameter::LoadInitParameter()
{
	//���ò���
	InitParameter();

	//��ȡ·��
	TCHAR szWorkDir[MAX_PATH]=TEXT("");
	CWHService::GetWorkDirectory(szWorkDir,CountArray(szWorkDir));

	//����·��
	TCHAR szIniFile[MAX_PATH]=TEXT("");
	_sntprintf(szIniFile,CountArray(szIniFile),TEXT("%s\\ServerParameter.ini"),szWorkDir);

	//��ȡ����
	CWHIniData IniData;
	IniData.SetIniFilePath(szIniFile);

	//��ȡ����
	IniData.ReadEncryptString(TEXT("ServerInfo"),TEXT("ServiceName"),NULL,m_szServerName,CountArray(m_szServerName));
	IniData.ReadEncryptString(TEXT("ServerInfo"),TEXT("ServiceAddr"),NULL,m_ServiceAddress.szAddress,CountArray(m_ServiceAddress.szAddress));

	TCHAR portStr[20] = L"";
	//Э����Ϣ
	m_wCorrespondPort=PORT_CENTER;
	IniData.ReadEncryptString(TEXT("ServerInfo"),TEXT("CorrespondAddr"),NULL,m_CorrespondAddress.szAddress,CountArray(m_CorrespondAddress.szAddress));

	//������Ϣ
	IniData.ReadEncryptString(TEXT("TreasureDB"),TEXT("DBPort"),NULL,portStr,sizeof(portStr));
	m_TreasureDBParameter.wDataBasePort=_wtoi(portStr);
	ZeroMemory(portStr,sizeof(portStr));

	IniData.ReadEncryptString(TEXT("TreasureDB"),TEXT("DBAddr"),NULL,m_TreasureDBParameter.szDataBaseAddr,CountArray(m_TreasureDBParameter.szDataBaseAddr));
	IniData.ReadEncryptString(TEXT("TreasureDB"),TEXT("DBUser"),NULL,m_TreasureDBParameter.szDataBaseUser,CountArray(m_TreasureDBParameter.szDataBaseUser));
	IniData.ReadEncryptString(TEXT("TreasureDB"),TEXT("DBPass"),NULL,m_TreasureDBParameter.szDataBasePass,CountArray(m_TreasureDBParameter.szDataBasePass));
	IniData.ReadEncryptString(TEXT("TreasureDB"),TEXT("DBName"),szTreasureDB,m_TreasureDBParameter.szDataBaseName,CountArray(m_TreasureDBParameter.szDataBaseName));

	//������Ϣ
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
