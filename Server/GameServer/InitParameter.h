#ifndef INIR_PARAMETER_HEAD_FILE
#define INIR_PARAMETER_HEAD_FILE

#pragma once

#include "Stdafx.h"
#include <cpprest/http_client.h>
#include <cpprest/filestream.h>
#include <cpprest/json.h>
#include <boost/property_tree/json_parser.hpp>
#include <iostream>
#ifdef DEBUG
#pragma comment (lib,"../../../Lib/CppRest/lib/x86/Debug/cpprest100_1_1.lib")
#else
#pragma comment (lib,"../../../Lib/CppRest/lib/x86/Release/cpprest100_1_1.lib")
#endif

/*////////////////////////////////////////////////////////////////////////////////
TCG-12714
	1.增加网路连线失败定义
	2.增加ACS帐务处理timeout设定
	3.增加回传给DLL判断_acsResult参数
////////////////////////////////////////////////////////////////////////////////*/
//INTERNET CONNECT RESULT
#define INTERNET_REQUEST_FAIL				0
#define INTERNET_REQUEST_SUCCESS			1
#define INTERNET_REQUEST_TIMEOUT			2
#define INTERNET_REQUEST_HOST_NAME_ERROR	3 
#define INTERNET_REQUEST_UNKNOWN			4
int HttpRequest(web::http::method m, wchar_t url[], utility::string_t builder, bool type, utility::string_t & v, std::map<wchar_t*,wchar_t*> hAdd,int flag = 0);

//配置参数
class CInitParameter
{
	//配置信息
public:
	WORD							m_wConnectTime;						//重连时间
	WORD							m_wCollectTime;						//统计时间

	//协调信息
public:
	WORD							m_wCorrespondPort;					//协调端口
	tagAddressInfo					m_CorrespondAddress;				//协调地址

	//配置信息
public:
	TCHAR							m_szServerName[LEN_SERVER];			//服务器名
	TCHAR                           m_LobbyURL[LEN_URL];                //大厅URL
	TCHAR                           m_ACS_USER_QUERY[LEN_URL];          //ACS查询
	TCHAR                           m_ACS_DEBIT[LEN_URL];               //ACS扣分
	TCHAR                           m_ACS_CREDIT[LEN_URL];              //ACS加分
	DWORD                           m_testscore;                        //试玩金币

	//连接信息
public:
	tagAddressInfo					m_ServiceAddress;					//服务地址
	tagDataBaseParameter			m_TreasureDBParameter;				//连接地址
	tagDataBaseParameter			m_PlatformDBParameter;				//连接地址

	//函数定义
public:
	//构造函数
	CInitParameter();
	//析构函数
	virtual ~CInitParameter();

	//功能函数
public:
	//初始化
	VOID InitParameter();
	//加载配置
	VOID LoadInitParameter();
};

//////////////////////////////////////////////////////////////////////////////////

#endif
