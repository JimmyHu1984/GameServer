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
	1.������·����ʧ�ܶ���
	2.����ACS������timeout�趨
	3.���ӻش���DLL�ж�_acsResult����
////////////////////////////////////////////////////////////////////////////////*/
//INTERNET CONNECT RESULT
#define INTERNET_REQUEST_FAIL				0
#define INTERNET_REQUEST_SUCCESS			1
#define INTERNET_REQUEST_TIMEOUT			2
#define INTERNET_REQUEST_HOST_NAME_ERROR	3 
#define INTERNET_REQUEST_UNKNOWN			4
int HttpRequest(web::http::method m, wchar_t url[], utility::string_t builder, bool type, utility::string_t & v, std::map<wchar_t*,wchar_t*> hAdd,int flag = 0);

//���ò���
class CInitParameter
{
	//������Ϣ
public:
	WORD							m_wConnectTime;						//����ʱ��
	WORD							m_wCollectTime;						//ͳ��ʱ��

	//Э����Ϣ
public:
	WORD							m_wCorrespondPort;					//Э���˿�
	tagAddressInfo					m_CorrespondAddress;				//Э����ַ

	//������Ϣ
public:
	TCHAR							m_szServerName[LEN_SERVER];			//��������
	TCHAR                           m_LobbyURL[LEN_URL];                //����URL
	TCHAR                           m_ACS_USER_QUERY[LEN_URL];          //ACS��ѯ
	TCHAR                           m_ACS_DEBIT[LEN_URL];               //ACS�۷�
	TCHAR                           m_ACS_CREDIT[LEN_URL];              //ACS�ӷ�
	DWORD                           m_testscore;                        //������

	//������Ϣ
public:
	tagAddressInfo					m_ServiceAddress;					//�����ַ
	tagDataBaseParameter			m_TreasureDBParameter;				//���ӵ�ַ
	tagDataBaseParameter			m_PlatformDBParameter;				//���ӵ�ַ

	//��������
public:
	//���캯��
	CInitParameter();
	//��������
	virtual ~CInitParameter();

	//���ܺ���
public:
	//��ʼ��
	VOID InitParameter();
	//��������
	VOID LoadInitParameter();
};

//////////////////////////////////////////////////////////////////////////////////

#endif
