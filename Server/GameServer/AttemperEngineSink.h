#ifndef ATTEMPER_ENGINE_SINK_HEAD_FILE
#define ATTEMPER_ENGINE_SINK_HEAD_FILE

#pragma once

#include "Stdafx.h"
#include "TableFrame.h"
#include "InitParameter.h"
#include "ServerListManager.h"
#include "DataBasePacket.h"
//#include "SensitiveWordsFilter.h"
//#include "BalanceScoreCurve.h"
#include "ListManager.h"
#include "DistributeHandler.h"

using namespace std;

//////////////////////////////////////////////////////////////////////////////////

//��������
#define CLIENT_KIND_FALSH			1									//��ҳ����
#define CLIENT_KIND_MOBILE			2									//�ֻ�����
#define CLIENT_KIND_COMPUTER		3									//��������

#define IDI_CLOSE_SERVER IDI_MAIN_MODULE_START+15

//�󶨲���
struct tagBindParameter
{
	//��������
	DWORD							dwSocketID;							//�����ʶ
	DWORD							dwClientAddr;						//���ӵ�ַ
	DWORD							dwActiveTime;						//����ʱ��

	//�汾��Ϣ
	DWORD							dwPlazaVersion;						//�㳡�汾
	DWORD							dwFrameVersion;						//��ܰ汾
	DWORD							dwProcessVersion;					//���̰汾

	//�û�����
	BYTE							cbClientKind;						//��������
	IServerUserItem *				pIServerUserItem;					//�û��ӿ�
};

//ϵͳ��Ϣ
struct tagSystemMessage
{
	DWORD							dwLastTime;						   //����ʱ��
	DBR_GR_SystemMessage            SystemMessage;                     //ϵͳ��Ϣ
};

//����˵��
typedef CWHArray<CTableFrame *>		CTableFrameArray;					//��������
typedef CMap<DWORD,DWORD,DWORD,DWORD &>  CKickUserItemMap;              //����ӳ�� 
typedef CList<tagSystemMessage *>   CSystemMessageList;                 //ϵͳ��Ϣ

//////////////////////////////////////////////////////////////////////////////////

//���ȹ���
class CAttemperEngineSink : public IAttemperEngineSink, public IMainServiceFrame,
	public IServerUserItemSink
{
	//��Ԫ����
	friend class CServiceUnits;

public:
	UINT distributeCounter;

	CListManager<IServerUserItem*>	m_AndroidWaitList;					//�����˵ȴ�
	CListManager<IServerUserItem*>	m_WaitDistributeList;               //�ȴ�����
	CListManager<IServerUserItem*>  m_AccompanyWaitList;                //���ȴ�
	CListManager<CDistributeTable*>  m_StartTableList;					//�ȴ���������
	CListManager<CDistributeTable*>  m_DistributeTableList;		        //�ȴ���������

	//״̬����
public:
	CCriticalSection				m_CriticalSection; 					//ͬ����
	CCriticalSection				m_csGetTableNum; 					//ͬ����
	bool							m_bCollectUser;						//���ܱ�־
	bool							m_bNeekCorrespond;					//Э����־
	bool                            m_closeServer;                      //�ط���־
	
	//����Ϣ
protected:
	tagBindParameter *				m_pNormalParameter;					//����Ϣ
	tagBindParameter *				m_pAndroidParameter;				//����Ϣ

	//������Ϣ
protected:
	CInitParameter *				m_pInitParameter;					//���ò���
	tagGameParameter *				m_pGameParameter;					//���ò���
	tagGameServiceAttrib *			m_pGameServiceAttrib;				//��������
	tagGameServiceOption *			m_pGameServiceOption;				//��������

	std::map<std::wstring,std::wstring> m_gameParas;                    //��Ϸ����

	//��������
protected:
	CMD_GR_ConfigColumn				m_DataConfigColumn;					//�б�����
	//CMD_GR_ConfigProperty			m_DataConfigProperty;				//��������

	//�������
public:
	CTableFrameArray				m_TableFrameArray;					//��������
	CServerListManager				m_ServerListManager;				//�б�����
	CServerUserManager				m_ServerUserManager;				//�û�����
	CAndroidUserManager				m_AndroidUserManager;				//��������
	//CGamePropertyManager			m_GamePropertyManager;				//���߹���
	CListManager<tagSystemMessage*> m_SystemMessageList;                //ϵͳ��Ϣ
	//CSensitiveWordsFilter			m_WordsFilter;						//���ֹ���
	//add by aisu 20150311
	//CBalanceScoreCurve				m_BalanceScoreCurve;				//ƽ�������

	CDistributeHandler				m_DistributeHandler;				//�����߼�
	//����ӿ�
public:
	ITimerEngine *					m_pITimerEngine;					//ʱ������
	IAttemperEngine *				m_pIAttemperEngine;					//��������
	ITCPSocketService *				m_pITCPSocketService;				//�������
	ITCPNetworkEngine *				m_pITCPNetworkEngine;				//��������
	IGameServiceManager *			m_pIGameServiceManager;				//�������

//	//��������
//public:
//	IGameMatchServiceManager		* m_pIGameMatchServiceManager;		//���������ӿ�

	//��������
public:
	IDataBaseEngine *				m_pIRecordDataBaseEngine;			//��������
	IDataBaseEngine *				m_pIKernelDataBaseEngine;			//��������
	IDBCorrespondManager *          m_pIDBCorrespondManager;            //����Э��
//	std::map<WORD,std::string>      m_wTableNum;                        //�������
//	std::map<WORD,LONG>             m_iGameCount;                       //��Ϸ����
	LONGLONG				        m_nCurrentGameCount;				//��ǰ��Ϸ����

public:
	std::string						GetTableNum();						//����µ��ƾֺ�
	
	void SwitchCSStatus(bool status)
	{
		CWHDataLocker datalock(m_cs_closeServer);
		if (m_closeServer != status)
		{
			m_closeServer = status;

			//close server
			if (status)
			{
				LOGI(L"Start Closing Server");
				SendSystemMessage(L"Server Will Close Soon, in-game Player Will be Logged Off After Current Game",0);

				m_pITimerEngine->SetTimer(IDI_CLOSE_SERVER,10*1000L,TIMES_INFINITY,0);
			}
		}
		
	}

	bool GetCSStatus()
	{
		CWHDataLocker datalock(m_cs_closeServer);
		return m_closeServer;
	}

private:
	CCriticalSection                m_cs_closeServer;

public:
	INT								m_nAndroidUserMessageCount;
	INT								m_nAndroidUserSendedCount;

	//��������
public:
	//���캯��
	CAttemperEngineSink();
	//��������
	virtual ~CAttemperEngineSink();

	void OnLobbyConnect();

	//�����ӿ�
public:
	//�ͷŶ���
	virtual VOID Release() { return; }
	//�ӿڲ�ѯ
	virtual VOID * QueryInterface(REFGUID Guid, DWORD dwQueryVer);

	//�첽�ӿ�
public:
	//�����¼�
	virtual bool OnAttemperEngineStart(IUnknownEx * pIUnknownEx);
	//ֹͣ�¼�
	virtual bool OnAttemperEngineConclude(IUnknownEx * pIUnknownEx);

	//�¼��ӿ�
public:
	//�����¼�
	virtual bool OnEventControl(WORD wIdentifier, VOID * pData, WORD wDataSize);
	//�Զ��¼�
	virtual bool OnEventAttemperData(WORD wRequestID, VOID * pData, WORD wDataSize);

	//�ں��¼�
public:
	//ʱ���¼�
	virtual bool OnEventTimer(DWORD dwTimerID, WPARAM wBindParam);
	//���ݿ��¼�
	virtual bool OnEventDataBase(WORD wRequestID, DWORD dwContextID, VOID * pData, WORD wDataSize);

	//�����¼�
public:
	//�����¼�
	virtual bool OnEventTCPSocketLink(WORD wServiceID, INT nErrorCode);
	//�ر��¼�
	virtual bool OnEventTCPSocketShut(WORD wServiceID, BYTE cbShutReason);
	//��ȡ�¼�
	virtual bool OnEventTCPSocketRead(WORD wServiceID, TCP_Command Command, VOID * pData, WORD wDataSize);

	//�����¼�
public:
	//Ӧ���¼�
	virtual bool OnEventTCPNetworkBind(DWORD dwClientAddr, DWORD dwSocketID);
	//�ر��¼�
	virtual bool OnEventTCPNetworkShut(DWORD dwClientAddr, DWORD dwActiveTime, DWORD dwSocketID);
	//��ȡ�¼�
	virtual bool OnEventTCPNetworkRead(TCP_Command Command, VOID * pData, WORD wDataSize, DWORD dwSocketID);
	//android simulate
	virtual bool OnAndroidUserShouldOff(DWORD dwUserID);
	//��Ϣ�ӿ�
public:
	//������Ϣ
	virtual bool SendRoomMessage(LPCTSTR lpszMessage, WORD wType);
	//��Ϸ��Ϣ
	virtual bool SendGameMessage(LPCTSTR lpszMessage, WORD wType);
	//������Ϣ
	virtual bool SendRoomMessage(IServerUserItem * pIServerUserItem, LPCTSTR lpszMessage, WORD wType);
	//��Ϸ��Ϣ
	virtual bool SendGameMessage(IServerUserItem * pIServerUserItem, LPCTSTR lpszMessage, WORD wType);
	//������Ϣ
	virtual bool SendRoomMessage(DWORD dwSocketID, LPCTSTR lpszMessage, WORD wType, bool bAndroid);
	//ϵͳ��Ϣ
	virtual bool SendSystemMessage(LPCTSTR lpszMessage, WORD wType);

	//����ӿ�
public:
	//��������
	virtual bool SendData(BYTE cbSendMask, WORD wMainCmdID, WORD wSubCmdID, VOID * pData, WORD wDataSize);
	//��������
	virtual bool SendData(DWORD dwSocketID, WORD wMainCmdID, WORD wSubCmdID, VOID * pData, WORD wDataSize);
	//��������
	virtual bool SendData(IServerUserItem * pIServerUserItem, WORD wMainCmdID, WORD wSubCmdID, VOID * pData, WORD wDataSize);

	//���ܽӿ�
public:
	//�����û�
	//virtual bool InsertDistribute(IServerUserItem * pIServerUserItem);
	//�����û�
	virtual bool InsertWaitDistribute(IServerUserItem * pIServerUserItem);
	//ɾ���û�
	virtual bool DeleteWaitDistribute(IServerUserItem * pIServerUserItem);
	//�����û�
	virtual bool DistributeUserGame();
	//�·����û� add by aisu

	//���дʹ���
	//virtual void SensitiveWordFilter(LPCTSTR pMsg, LPTSTR pszFiltered, int nMaxLen);
	// ����ƽ��ֵ���(added by anjay )
	//virtual LONG CalculateBalanceScore(SCORE lOldScore, SCORE lNewScore);

	//�û��ӿ�
public:
	//�û�����
	virtual bool OnEventUserItemScore(IServerUserItem * pIServerUserItem, BYTE cbReason, int* OnEventUserItemScore);
	//�û�״̬
	virtual bool OnEventUserItemStatus(IServerUserItem * pIServerUserItem, BYTE preStatus, WORD wOldTableID=INVALID_TABLE, WORD wOldChairID=INVALID_CHAIR);
	//�û�Ȩ��
	//virtual bool OnEventUserItemRight(IServerUserItem *pIServerUserItem, DWORD dwAddRight, DWORD dwRemoveRight,bool bGameRight=true);

	//�����¼�
protected:
	//��¼�ɹ�
	bool OnDBLogonSuccess(DWORD dwContextID, VOID * pData, WORD wDataSize);
	//��¼ʧ��
	bool OnDBLogonFailure(DWORD dwContextID, VOID * pData, WORD wDataSize);
	//������Ϣ
	bool OnDBGameParameter(DWORD dwContextID, VOID * pData, WORD wDataSize);
	//�б���Ϣ
	bool OnDBGameColumnInfo(DWORD dwContextID, VOID * pData, WORD wDataSize);
	//������Ϣ
	bool OnDBGameAndroidInfo(DWORD dwContextID, VOID * pData, WORD wDataSize);
	//������Ϣ������ɾ����������SERVER�ж�����������...
	bool OnDBGamePropertyInfo(DWORD dwContextID, VOID * pData, WORD wDataSize);
	//������Ϣ
	//bool OnDBUserInsureInfo(DWORD dwContextID, VOID * pData, WORD wDataSize);
   // bool OnDBUserTransRecord(DWORD dwContextID, VOID * pData, WORD wDataSize);
	//���гɹ�
	//bool OnDBUserInsureSuccess(DWORD dwContextID, VOID * pData, WORD wDataSize);
	//����ʧ��
	//bool OnDBUserInsureFailure(DWORD dwContextID, VOID * pData, WORD wDataSize);
	//�û���Ϣ
	//bool OnDBUserInsureUserInfo(DWORD dwContextID, VOID * pData, WORD wDataSize);
	//���߳ɹ�
	//bool OnDBPropertySuccess(DWORD dwContextID, VOID * pData, WORD wDataSize);
	//����ʧ��
	//bool OnDBPropertyFailure(DWORD dwContextID, VOID * pData, WORD wDataSize);
	//�������д�
	//bool OnDBSensitiveWords(DWORD dwContextID, VOID * pData, WORD wDataSize);
	// ת�˻�ִ
	//bool OnDBUserTransferReceipt(DWORD dwContextID, VOID * pData, WORD wDataSize);
	// �����ɹ�
	//bool OnDBUserOperateSuccess(DWORD dwContextID, VOID * pData, WORD wDataSize);
	// ����ʧ��
	//bool OnDBUserOperateFailure(DWORD dwContextID, VOID * pData, WORD wDataSize); 
	// ����ƽ�����������
	//bool OnDBBalanceScoreCurve(DWORD dwContextID, VOID * pData, WORD wDataSize); 
	// ���ƽ�������
	//bool OnDBUserBalanceScore(DWORD dwContextID, VOID * pData, WORD wDataSize);
	// ���ؾ���
	bool OnDBLoadGameCount(DWORD dwContextID, VOID * pData, WORD wDataSize);
	
	//���Ӵ���
protected:
	//ע���¼�
	bool OnTCPSocketMainRegister(WORD wSubCmdID, VOID * pData, WORD wDataSize);
	//�б��¼�
	bool OnTCPSocketMainServiceInfo(WORD wSubCmdID, VOID * pData, WORD wDataSize);
	//�����¼�
	bool OnTCPSocketMainUserCollect(WORD wSubCmdID, VOID * pData, WORD wDataSize);
	//��������
	bool OnTCPSocketMainManagerService(WORD wSubCmdID, VOID * pData, WORD wDataSize);

	//�����¼�
protected:
	//�û�����
	bool OnTCPNetworkMainUser(WORD wSubCmdID, VOID * pData, WORD wDataSize, DWORD dwSocketID);
	//��¼����
	bool OnTCPNetworkMainLogon(WORD wSubCmdID, VOID * pData, WORD wDataSize, DWORD dwSocketID);
	//��Ϸ����
	bool OnTCPNetworkMainGame(WORD wSubCmdID, VOID * pData, WORD wDataSize, DWORD dwSocketID);
	//��ܴ���
	bool OnTCPNetworkMainFrame(WORD wSubCmdID, VOID * pData, WORD wDataSize, DWORD dwSocketID);
	//���д���
	//bool OnTCPNetworkMainInsure(WORD wSubCmdID, VOID * pData, WORD wDataSize, DWORD dwSocketID);
	//��������
	//bool OnTCPNetworkMainManage(WORD wSubCmdID, VOID * pData, WORD wDataSize, DWORD dwSocketID);
	//��������
	//bool OnTCPNetworkMainMatch(WORD wSubCmdID, VOID * pData, WORD wDataSize, DWORD dwSocketID);
	//ECHO����
	bool OnTCPNetworkMainEcho(WORD wSubCmdID, VOID * pData, WORD wDataSize, DWORD dwSocketID);

	//�����¼�
protected:
	//I D ��¼
	bool OnTCPNetworkSubLogonUserID(VOID * pData, WORD wDataSize, DWORD dwSocketID);
	//�ֻ���¼
	bool OnTCPNetworkSubLogonMobile(VOID * pData, WORD wDataSize, DWORD dwSocketID);
	//�ʺŵ�¼
	bool OnTCPNetworkSubLogonAccounts(VOID * pData, WORD wDataSize, DWORD dwSocketID);

	//�û�����
protected:
	//�û�����
	bool OnTCPNetworkSubUserRule(VOID * pData, WORD wDataSize, DWORD dwSocketID);
	//�û��Թ�
	//bool OnTCPNetworkSubUserLookon(VOID * pData, WORD wDataSize, DWORD dwSocketID);
	//�û�����
	bool OnTCPNetworkSubUserSitDown(VOID * pData, WORD wDataSize, DWORD dwSocketID);
	//�û�����
	bool OnTCPNetworkSubUserStandUp(VOID * pData, WORD wDataSize, DWORD dwSocketID);
	//�û�����
	//bool OnTCPNetworkSubUserChat(VOID * pData, WORD wDataSize, DWORD dwSocketID);
	//�û�˽��
	//bool OnTCPNetworkSubWisperChat(VOID * pData, WORD wDataSize, DWORD dwSocketID);
	//�û�����
	//bool OnTCPNetworkSubUserExpression(VOID * pData, WORD wDataSize, DWORD dwSocketID);
	//�û�����
	//bool OnTCPNetworkSubWisperExpression(VOID * pData, WORD wDataSize, DWORD dwSocketID);
	//�������
	//bool OnTCPNetworkSubPropertyBuy(VOID * pData, WORD wDataSize, DWORD dwSocketID);
	//ʹ�õ���
	//bool OnTCPNetwordSubSendTrumpet(VOID * pData, WORD wDataSize, DWORD dwSocketID);
	//�����û�
	//bool OnTCPNetworkSubUserInviteReq(VOID * pData, WORD wDataSize, DWORD dwSocketID);
	//�ܾ�����
	//bool OnTCPNetworkSubUserRepulseSit(VOID * pData, WORD wDataSize, DWORD dwSocketID);
	//�߳�����
	//bool OnTCPNetworkSubMemberKickUser(VOID * pData, WORD wDataSize, DWORD dwSocketID);
	//�����û���Ϣ
	bool OnTCPNetworkSubUserInfoReq(VOID * pData, WORD wDataSize, DWORD dwSocketID);
	//�������λ��
	//bool OnTCPNetworkSubUserChairReq(VOID * pData, WORD wDataSize, DWORD dwSocketID);
	//���������û���Ϣ
	bool OnTCPNetworkSubChairUserInfoReq(VOID * pData, WORD wDataSize, DWORD dwSocketID);
	//bool OnTCPAndroidSendTrumpet();

	//��������
protected:
	//��ѯ����
	//bool OnTCPNetworkSubQueryInsureInfo(VOID * pData, WORD wDataSize, DWORD dwSocketID);
    //ת�˼�¼
   // bool OnTCPNetworkSubQueryTransRecord(VOID * pData, WORD wDataSize, DWORD dwSocketID);
	//�������
	//bool OnTCPNetworkSubSaveScoreRequest(VOID * pData, WORD wDataSize, DWORD dwSocketID);
	//ȡ������
	//bool OnTCPNetworkSubTakeScoreRequest(VOID * pData, WORD wDataSize, DWORD dwSocketID);
	//ת������
	//bool OnTCPNetworkSubTransferScoreRequest(VOID * pData, WORD wDataSize, DWORD dwSocketID);
	//��ѯ�û�����
	//bool OnTCPNetworkSubQueryUserInfoRequest(VOID * pData, WORD wDataSize, DWORD dwSocketID);
	// ����GameID��ѯ�ǳ�
	//bool OnTCPNetworkSubQueryNickNameByGameID(VOID * pData, WORD wDataSize, DWORD dwSocketID);
	// ��֤��������
	//bool OnTCPNetworkSubVerifyInsurePassword(VOID * pData, WORD wDataSize, DWORD dwSocketID);
	// ������������
	//bool OnTCPNetworkSubModifyInsurePassword(VOID * pData, WORD wDataSize, DWORD dwSocketID);

	//��������
protected:
	//��ѯ����
	//bool OnTCPNetworkSubQueryOption(VOID * pData, WORD wDataSize, DWORD dwSocketID);
	//��������
	//bool OnTCPNetworkSubOptionServer(VOID * pData, WORD wDataSize, DWORD dwSocketID);
	//�߳��û�
	//bool OnTCPNetworkSubManagerKickUser(VOID * pData, WORD wDataSize, DWORD dwSocketID);
	//��������
	//bool OnTCPNetworkSubLimitUserChat(VOID * pData, WORD wDataSize, DWORD dwSocketID);
	//�߳������û�
	//bool OnTCPNetworkSubKickAllUser(VOID * pData, WORD wDataSize, DWORD dwSocketID);
	//��ɢ��Ϸ
	//bool OnTCPNetworkSubDismissGame(VOID * pData, WORD wDataSize, DWORD dwSocketID);

	//�ڲ��¼�
protected:
	//�û���¼
	VOID OnEventUserLogon(IServerUserItem * pIServerUserItem, bool bAlreadyOnLine);
	//�û��ǳ�
	VOID OnEventUserLogout(IServerUserItem * pIServerUserItem, DWORD dwLeaveReason);

	//ִ�й���
protected:
	//������Ϸ��
	//bool PerformUnlockScore(DWORD dwUserID, DWORD dwInoutIndex, DWORD dwLeaveReason);
	//�汾���
	bool PerformCheckVersion(DWORD dwPlazaVersion, DWORD dwFrameVersion, DWORD dwClientVersion, DWORD dwSocketID);
	//�л�����
	bool SwitchUserItemConnect(IServerUserItem * pIServerUserItem, TCHAR szMachineID[LEN_MACHINE_ID], WORD wTargetIndex,BYTE cbDeviceType=DEVICE_TYPE_PC,WORD wBehaviorFlags=0,WORD wPageTableCount=0);

	//���ͺ���
protected:
	//�û���Ϣ
	bool SendUserInfoPacket(IServerUserItem * pIServerUserItem, DWORD dwSocketID, bool isMeFlag = true);

	//��������
protected:
	//��ǰ�¼�
	//bool OnEventPropertyBuyPrep(WORD cbRequestArea,WORD wPropertyIndex,IServerUserItem *pISourceUserItem,IServerUserItem *pTargetUserItem);
	//������Ϣ
	//bool SendPropertyMessage(DWORD dwSourceID,DWORD dwTargerID,WORD wPropertyIndex,WORD wPropertyCount);
	//����ЧӦ
	//bool SendPropertyEffect(IServerUserItem * pIServerUserItem);

	//��������
protected:
	//��¼ʧ��
	bool SendLogonFailure(LPCTSTR pszString, LONG lErrorCode, DWORD dwSocketID);
	//����ʧ��
	//bool SendInsureFailure(IServerUserItem * pIServerUserItem, LPCTSTR pszDescribe, LONG lErrorCode,BYTE cbActivityGame);
	//����ʧ��
	bool SendRequestFailure(IServerUserItem * pIServerUserItem, LPCTSTR pszDescribe, LONG lErrorCode);
	//����ʧ��
	//bool SendPropertyFailure(IServerUserItem * pIServerUserItem, LPCTSTR pszDescribe, LONG lErrorCode,WORD wRequestArea);

	//��������
public:
	//���û�
	IServerUserItem * GetBindUserItem(WORD wBindIndex);
	//�󶨲���
	tagBindParameter * GetBindParameter(WORD wBindIndex);
	//��������
	//WORD GetPropertyType(WORD wPropertyIndex);

	//��������
protected:
	//���û���
	bool InitAndroidUser();
	//��������
	bool InitTableFrameArray();
	//��������
	bool SendUIControlPacket(WORD wRequestID, VOID * pData, WORD wDataSize);
	//���ò���
	void SetMobileUserParameter(IServerUserItem * pIServerUserItem,BYTE cbDeviceType,WORD wBehaviorFlags,WORD wPageTableCount);
	//Ⱥ������
	virtual bool SendDataBatchToMobileUser(WORD wCmdTable, WORD wMainCmdID, WORD wSubCmdID, VOID * pData, WORD wDataSize);
	//Ⱥ���û���Ϣ
	bool SendUserInfoPacketBatchToMobileUser(IServerUserItem * pIServerUserItem);
	//�������û���Ϣ
	bool SendViewTableUserInfoPacketToMobileUser(IServerUserItem * pIServerUserItem,DWORD dwUserIDReq);
	//�ֻ�������¼
	bool MobileUserImmediately(IServerUserItem * pIServerUserItem);
	//�������
	//int OnPropertyBuy(VOID * pData, WORD wDataSize, DWORD dwSocketID);
	//����ϵͳ��Ϣ
	bool SendSystemMessage(CMD_GR_SendMessage * pSendMessage, WORD wDataSize);
	//�����Ϣ����
	void ClearSystemMessageData();

	//�����������ĸ�������
protected:
	//������������������
	int GetAndriodTable(IAndroidUserItem * pIAndroidUserItem);

	//��ȡ�ƾ�
	//void GetGameCount();
public:
	//ȡ��
	bool QuickGetScore(DWORD account, double & score);
	//д��
	bool QuickWriteScore(DWORD accountId, DWORD customerId, double score, std::wstring orderNo, std::wstring remark, int typeId = 0);

	//ȡ��ACS��Credit TYPE ID
	int	GetAcsTypeIDCredit();
	//ȡ��ACS��Debit TYPE ID
	int	GetAcsTypeIDDebit();
public:
	//��ʼ����Ϸ����
	void InitGameParas(web::json::value vb)
	{
		if (!vb.size())
		{
			return;
		}

		m_gameParas.clear();
		web::json::value::iterator iter = vb.begin();
		for(; iter != vb.end(); iter++)
		{
			std::wstring keyStr = iter->first.as_string();
			std::wstring valueStr = iter->second.as_string();
			m_gameParas.insert(std::make_pair<std::wstring,std::wstring>(keyStr, valueStr));
		}

	}
	void QueryGameParas(std::wstring key, std::wstring &value)
	{
		if (!key.length())
		{
			value = L"";
			return;
		}

		std::map<std::wstring, std::wstring>::iterator iter =  m_gameParas.find(key);
		if (iter != m_gameParas.end())
		{
			value = iter->second;
			return;
		}

		value = L"";
	}

	void StartLoadAndroid();

};

//////////////////////////////////////////////////////////////////////////////////

#endif