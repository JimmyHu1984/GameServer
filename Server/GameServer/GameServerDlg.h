#ifndef GAME_SERVER_SERVER_DLG_HEAD_FILE
#define GAME_SERVER_SERVER_DLG_HEAD_FILE

#pragma once

#include "Stdafx.h"
#include "ServiceUnits.h"

//////////////////////////////////////////////////////////////////////////////////

//���Ի���
class CGameServerDlg : public CDialog, public IServiceUnitsSink
{
	//�������
public:
	CServiceUnits					m_ServiceUnits;						//����Ԫ
	CTraceServiceControl			m_TraceServiceControl;				//׷�ٴ���

	//�������
protected:
	CModuleDBParameter				m_ModuleDBParameter;				//ģ�����

	//���ò���
protected:
	bool							m_bAutoControl;						//�Զ�����
	bool							m_bOptionSuccess;					//���ñ�־
	tagModuleInitParameter			m_ModuleInitParameter;				//���ò���

	//��������
public:
	//���캯��
	CGameServerDlg();
	//��������
	virtual ~CGameServerDlg();

	//���غ���
protected:
	//�ؼ���
	virtual VOID DoDataExchange(CDataExchange * pDX);
	//��ʼ������
	virtual BOOL OnInitDialog();
	//ȷ����Ϣ
	virtual VOID OnOK();
	//ȡ������
	virtual VOID OnCancel();
	//��Ϣ����
	virtual BOOL PreTranslateMessage(MSG * pMsg);

	//����ӿ�
public:
	//����״̬
	virtual VOID OnServiceUnitsStatus(enServiceStatus ServiceStatus);

	//��������
protected:
	//����ͼ��
	VOID UpdateServerLogo(LPCTSTR pszServerDLL);
	//���±���
	VOID UpdateServerTitle(enServiceStatus ServiceStatus);
	//����״̬
	VOID UpdateParameterStatus(tagModuleInitParameter & ModuleInitParameter);

	VOID ReadParametersFromFile(const char *filename);
	VOID WriteParametersToFile(const char *filename);

	//�������
protected:
	//��������
	bool StartServerService(WORD wServerID);
	//��ȡ����
	bool InitDataBaseParameter(tagDataBaseParameter & DataBaseParameter);

	//��ť��Ϣ
protected:
	//��������
	VOID OnBnClickedStartService();
	//ֹͣ����
	VOID OnBnClickedStopService();
	//�򿪷���
	VOID OnBnClickedOpenServer();
	//��������
	VOID OnBnClickedCreateServer();
	//���÷���
	VOID OnBnClickedOptionServer();
	//���ñ���
	VOID OnBnClickedOptionMatch();

	//��Ϣӳ��
public:
	//�ر�ѯ��
	BOOL OnQueryEndSession();
	//�����
	LRESULT OnMessageProcessCmdLine(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedOpenMatch();
	CString GetProjectVersion();
	BOOL isSaveProcessIDToFileSuccess(DWORD _pId);

};

//////////////////////////////////////////////////////////////////////////////////

#endif