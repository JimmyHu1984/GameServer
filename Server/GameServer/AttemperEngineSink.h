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

//连接类型
#define CLIENT_KIND_FALSH			1									//网页类型
#define CLIENT_KIND_MOBILE			2									//手机类型
#define CLIENT_KIND_COMPUTER		3									//电脑类型

#define IDI_CLOSE_SERVER IDI_MAIN_MODULE_START+15

//绑定参数
struct tagBindParameter
{
	//连接属性
	DWORD							dwSocketID;							//网络标识
	DWORD							dwClientAddr;						//连接地址
	DWORD							dwActiveTime;						//激活时间

	//版本信息
	DWORD							dwPlazaVersion;						//广场版本
	DWORD							dwFrameVersion;						//框架版本
	DWORD							dwProcessVersion;					//进程版本

	//用户属性
	BYTE							cbClientKind;						//连接类型
	IServerUserItem *				pIServerUserItem;					//用户接口
};

//系统消息
struct tagSystemMessage
{
	DWORD							dwLastTime;						   //发送时间
	DBR_GR_SystemMessage            SystemMessage;                     //系统消息
};

//数组说明
typedef CWHArray<CTableFrame *>		CTableFrameArray;					//桌子数组
typedef CMap<DWORD,DWORD,DWORD,DWORD &>  CKickUserItemMap;              //踢人映射 
typedef CList<tagSystemMessage *>   CSystemMessageList;                 //系统消息

//////////////////////////////////////////////////////////////////////////////////

//调度钩子
class CAttemperEngineSink : public IAttemperEngineSink, public IMainServiceFrame,
	public IServerUserItemSink
{
	//友元定义
	friend class CServiceUnits;

public:
	UINT distributeCounter;

	CListManager<IServerUserItem*>	m_AndroidWaitList;					//机器人等待
	CListManager<IServerUserItem*>	m_WaitDistributeList;               //等待分配
	CListManager<IServerUserItem*>  m_AccompanyWaitList;                //陪打等待
	CListManager<CDistributeTable*>  m_StartTableList;					//等待分配桌子
	CListManager<CDistributeTable*>  m_DistributeTableList;		        //等待分配桌子

	//状态变量
public:
	CCriticalSection				m_CriticalSection; 					//同步锁
	CCriticalSection				m_csGetTableNum; 					//同步锁
	bool							m_bCollectUser;						//汇总标志
	bool							m_bNeekCorrespond;					//协调标志
	bool                            m_closeServer;                      //关服标志
	
	//绑定信息
protected:
	tagBindParameter *				m_pNormalParameter;					//绑定信息
	tagBindParameter *				m_pAndroidParameter;				//绑定信息

	//配置信息
protected:
	CInitParameter *				m_pInitParameter;					//配置参数
	tagGameParameter *				m_pGameParameter;					//配置参数
	tagGameServiceAttrib *			m_pGameServiceAttrib;				//服务属性
	tagGameServiceOption *			m_pGameServiceOption;				//服务配置

	std::map<std::wstring,std::wstring> m_gameParas;                    //游戏参数

	//配置数据
protected:
	CMD_GR_ConfigColumn				m_DataConfigColumn;					//列表配置
	//CMD_GR_ConfigProperty			m_DataConfigProperty;				//道具配置

	//组件变量
public:
	CTableFrameArray				m_TableFrameArray;					//桌子数组
	CServerListManager				m_ServerListManager;				//列表管理
	CServerUserManager				m_ServerUserManager;				//用户管理
	CAndroidUserManager				m_AndroidUserManager;				//机器管理
	//CGamePropertyManager			m_GamePropertyManager;				//道具管理
	CListManager<tagSystemMessage*> m_SystemMessageList;                //系统消息
	//CSensitiveWordsFilter			m_WordsFilter;						//脏字过滤
	//add by aisu 20150311
	//CBalanceScoreCurve				m_BalanceScoreCurve;				//平衡分曲线

	CDistributeHandler				m_DistributeHandler;				//配桌逻辑
	//组件接口
public:
	ITimerEngine *					m_pITimerEngine;					//时间引擎
	IAttemperEngine *				m_pIAttemperEngine;					//调度引擎
	ITCPSocketService *				m_pITCPSocketService;				//网络服务
	ITCPNetworkEngine *				m_pITCPNetworkEngine;				//网络引擎
	IGameServiceManager *			m_pIGameServiceManager;				//服务管理

//	//比赛服务
//public:
//	IGameMatchServiceManager		* m_pIGameMatchServiceManager;		//比赛管理接口

	//数据引擎
public:
	IDataBaseEngine *				m_pIRecordDataBaseEngine;			//数据引擎
	IDataBaseEngine *				m_pIKernelDataBaseEngine;			//数据引擎
	IDBCorrespondManager *          m_pIDBCorrespondManager;            //数据协调
//	std::map<WORD,std::string>      m_wTableNum;                        //牌桌编号
//	std::map<WORD,LONG>             m_iGameCount;                       //游戏局数
	LONGLONG				        m_nCurrentGameCount;				//当前游戏局数

public:
	std::string						GetTableNum();						//获得新的牌局号
	
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

	//函数定义
public:
	//构造函数
	CAttemperEngineSink();
	//析构函数
	virtual ~CAttemperEngineSink();

	void OnLobbyConnect();

	//基础接口
public:
	//释放对象
	virtual VOID Release() { return; }
	//接口查询
	virtual VOID * QueryInterface(REFGUID Guid, DWORD dwQueryVer);

	//异步接口
public:
	//启动事件
	virtual bool OnAttemperEngineStart(IUnknownEx * pIUnknownEx);
	//停止事件
	virtual bool OnAttemperEngineConclude(IUnknownEx * pIUnknownEx);

	//事件接口
public:
	//控制事件
	virtual bool OnEventControl(WORD wIdentifier, VOID * pData, WORD wDataSize);
	//自定事件
	virtual bool OnEventAttemperData(WORD wRequestID, VOID * pData, WORD wDataSize);

	//内核事件
public:
	//时间事件
	virtual bool OnEventTimer(DWORD dwTimerID, WPARAM wBindParam);
	//数据库事件
	virtual bool OnEventDataBase(WORD wRequestID, DWORD dwContextID, VOID * pData, WORD wDataSize);

	//连接事件
public:
	//连接事件
	virtual bool OnEventTCPSocketLink(WORD wServiceID, INT nErrorCode);
	//关闭事件
	virtual bool OnEventTCPSocketShut(WORD wServiceID, BYTE cbShutReason);
	//读取事件
	virtual bool OnEventTCPSocketRead(WORD wServiceID, TCP_Command Command, VOID * pData, WORD wDataSize);

	//网络事件
public:
	//应答事件
	virtual bool OnEventTCPNetworkBind(DWORD dwClientAddr, DWORD dwSocketID);
	//关闭事件
	virtual bool OnEventTCPNetworkShut(DWORD dwClientAddr, DWORD dwActiveTime, DWORD dwSocketID);
	//读取事件
	virtual bool OnEventTCPNetworkRead(TCP_Command Command, VOID * pData, WORD wDataSize, DWORD dwSocketID);
	//android simulate
	virtual bool OnAndroidUserShouldOff(DWORD dwUserID);
	//消息接口
public:
	//房间消息
	virtual bool SendRoomMessage(LPCTSTR lpszMessage, WORD wType);
	//游戏消息
	virtual bool SendGameMessage(LPCTSTR lpszMessage, WORD wType);
	//房间消息
	virtual bool SendRoomMessage(IServerUserItem * pIServerUserItem, LPCTSTR lpszMessage, WORD wType);
	//游戏消息
	virtual bool SendGameMessage(IServerUserItem * pIServerUserItem, LPCTSTR lpszMessage, WORD wType);
	//房间消息
	virtual bool SendRoomMessage(DWORD dwSocketID, LPCTSTR lpszMessage, WORD wType, bool bAndroid);
	//系统消息
	virtual bool SendSystemMessage(LPCTSTR lpszMessage, WORD wType);

	//网络接口
public:
	//发送数据
	virtual bool SendData(BYTE cbSendMask, WORD wMainCmdID, WORD wSubCmdID, VOID * pData, WORD wDataSize);
	//发送数据
	virtual bool SendData(DWORD dwSocketID, WORD wMainCmdID, WORD wSubCmdID, VOID * pData, WORD wDataSize);
	//发送数据
	virtual bool SendData(IServerUserItem * pIServerUserItem, WORD wMainCmdID, WORD wSubCmdID, VOID * pData, WORD wDataSize);

	//功能接口
public:
	//插入用户
	//virtual bool InsertDistribute(IServerUserItem * pIServerUserItem);
	//插入用户
	virtual bool InsertWaitDistribute(IServerUserItem * pIServerUserItem);
	//删除用户
	virtual bool DeleteWaitDistribute(IServerUserItem * pIServerUserItem);
	//分配用户
	virtual bool DistributeUserGame();
	//新分配用户 add by aisu

	//敏感词过滤
	//virtual void SensitiveWordFilter(LPCTSTR pMsg, LPTSTR pszFiltered, int nMaxLen);
	// 计算平衡分调整(added by anjay )
	//virtual LONG CalculateBalanceScore(SCORE lOldScore, SCORE lNewScore);

	//用户接口
public:
	//用户积分
	virtual bool OnEventUserItemScore(IServerUserItem * pIServerUserItem, BYTE cbReason, int* OnEventUserItemScore);
	//用户状态
	virtual bool OnEventUserItemStatus(IServerUserItem * pIServerUserItem, BYTE preStatus, WORD wOldTableID=INVALID_TABLE, WORD wOldChairID=INVALID_CHAIR);
	//用户权限
	//virtual bool OnEventUserItemRight(IServerUserItem *pIServerUserItem, DWORD dwAddRight, DWORD dwRemoveRight,bool bGameRight=true);

	//数据事件
protected:
	//登录成功
	bool OnDBLogonSuccess(DWORD dwContextID, VOID * pData, WORD wDataSize);
	//登录失败
	bool OnDBLogonFailure(DWORD dwContextID, VOID * pData, WORD wDataSize);
	//配置信息
	bool OnDBGameParameter(DWORD dwContextID, VOID * pData, WORD wDataSize);
	//列表信息
	bool OnDBGameColumnInfo(DWORD dwContextID, VOID * pData, WORD wDataSize);
	//机器信息
	bool OnDBGameAndroidInfo(DWORD dwContextID, VOID * pData, WORD wDataSize);
	//道具信息，不可删除，因启动SERVER有东西埋在这里...
	bool OnDBGamePropertyInfo(DWORD dwContextID, VOID * pData, WORD wDataSize);
	//银行信息
	//bool OnDBUserInsureInfo(DWORD dwContextID, VOID * pData, WORD wDataSize);
   // bool OnDBUserTransRecord(DWORD dwContextID, VOID * pData, WORD wDataSize);
	//银行成功
	//bool OnDBUserInsureSuccess(DWORD dwContextID, VOID * pData, WORD wDataSize);
	//银行失败
	//bool OnDBUserInsureFailure(DWORD dwContextID, VOID * pData, WORD wDataSize);
	//用户信息
	//bool OnDBUserInsureUserInfo(DWORD dwContextID, VOID * pData, WORD wDataSize);
	//道具成功
	//bool OnDBPropertySuccess(DWORD dwContextID, VOID * pData, WORD wDataSize);
	//道具失败
	//bool OnDBPropertyFailure(DWORD dwContextID, VOID * pData, WORD wDataSize);
	//加载敏感词
	//bool OnDBSensitiveWords(DWORD dwContextID, VOID * pData, WORD wDataSize);
	// 转账回执
	//bool OnDBUserTransferReceipt(DWORD dwContextID, VOID * pData, WORD wDataSize);
	// 操作成功
	//bool OnDBUserOperateSuccess(DWORD dwContextID, VOID * pData, WORD wDataSize);
	// 操作失败
	//bool OnDBUserOperateFailure(DWORD dwContextID, VOID * pData, WORD wDataSize); 
	// 加载平衡分曲线配置
	//bool OnDBBalanceScoreCurve(DWORD dwContextID, VOID * pData, WORD wDataSize); 
	// 玩家平衡分数据
	//bool OnDBUserBalanceScore(DWORD dwContextID, VOID * pData, WORD wDataSize);
	// 加载局数
	bool OnDBLoadGameCount(DWORD dwContextID, VOID * pData, WORD wDataSize);
	
	//连接处理
protected:
	//注册事件
	bool OnTCPSocketMainRegister(WORD wSubCmdID, VOID * pData, WORD wDataSize);
	//列表事件
	bool OnTCPSocketMainServiceInfo(WORD wSubCmdID, VOID * pData, WORD wDataSize);
	//汇总事件
	bool OnTCPSocketMainUserCollect(WORD wSubCmdID, VOID * pData, WORD wDataSize);
	//管理服务
	bool OnTCPSocketMainManagerService(WORD wSubCmdID, VOID * pData, WORD wDataSize);

	//网络事件
protected:
	//用户处理
	bool OnTCPNetworkMainUser(WORD wSubCmdID, VOID * pData, WORD wDataSize, DWORD dwSocketID);
	//登录处理
	bool OnTCPNetworkMainLogon(WORD wSubCmdID, VOID * pData, WORD wDataSize, DWORD dwSocketID);
	//游戏处理
	bool OnTCPNetworkMainGame(WORD wSubCmdID, VOID * pData, WORD wDataSize, DWORD dwSocketID);
	//框架处理
	bool OnTCPNetworkMainFrame(WORD wSubCmdID, VOID * pData, WORD wDataSize, DWORD dwSocketID);
	//银行处理
	//bool OnTCPNetworkMainInsure(WORD wSubCmdID, VOID * pData, WORD wDataSize, DWORD dwSocketID);
	//管理处理
	//bool OnTCPNetworkMainManage(WORD wSubCmdID, VOID * pData, WORD wDataSize, DWORD dwSocketID);
	//比赛命令
	//bool OnTCPNetworkMainMatch(WORD wSubCmdID, VOID * pData, WORD wDataSize, DWORD dwSocketID);
	//ECHO测试
	bool OnTCPNetworkMainEcho(WORD wSubCmdID, VOID * pData, WORD wDataSize, DWORD dwSocketID);

	//网络事件
protected:
	//I D 登录
	bool OnTCPNetworkSubLogonUserID(VOID * pData, WORD wDataSize, DWORD dwSocketID);
	//手机登录
	bool OnTCPNetworkSubLogonMobile(VOID * pData, WORD wDataSize, DWORD dwSocketID);
	//帐号登录
	bool OnTCPNetworkSubLogonAccounts(VOID * pData, WORD wDataSize, DWORD dwSocketID);

	//用户命令
protected:
	//用户规则
	bool OnTCPNetworkSubUserRule(VOID * pData, WORD wDataSize, DWORD dwSocketID);
	//用户旁观
	//bool OnTCPNetworkSubUserLookon(VOID * pData, WORD wDataSize, DWORD dwSocketID);
	//用户坐下
	bool OnTCPNetworkSubUserSitDown(VOID * pData, WORD wDataSize, DWORD dwSocketID);
	//用户起立
	bool OnTCPNetworkSubUserStandUp(VOID * pData, WORD wDataSize, DWORD dwSocketID);
	//用户聊天
	//bool OnTCPNetworkSubUserChat(VOID * pData, WORD wDataSize, DWORD dwSocketID);
	//用户私聊
	//bool OnTCPNetworkSubWisperChat(VOID * pData, WORD wDataSize, DWORD dwSocketID);
	//用户表情
	//bool OnTCPNetworkSubUserExpression(VOID * pData, WORD wDataSize, DWORD dwSocketID);
	//用户表情
	//bool OnTCPNetworkSubWisperExpression(VOID * pData, WORD wDataSize, DWORD dwSocketID);
	//购买道具
	//bool OnTCPNetworkSubPropertyBuy(VOID * pData, WORD wDataSize, DWORD dwSocketID);
	//使用道具
	//bool OnTCPNetwordSubSendTrumpet(VOID * pData, WORD wDataSize, DWORD dwSocketID);
	//邀请用户
	//bool OnTCPNetworkSubUserInviteReq(VOID * pData, WORD wDataSize, DWORD dwSocketID);
	//拒绝厌友
	//bool OnTCPNetworkSubUserRepulseSit(VOID * pData, WORD wDataSize, DWORD dwSocketID);
	//踢出命令
	//bool OnTCPNetworkSubMemberKickUser(VOID * pData, WORD wDataSize, DWORD dwSocketID);
	//请求用户信息
	bool OnTCPNetworkSubUserInfoReq(VOID * pData, WORD wDataSize, DWORD dwSocketID);
	//请求更换位置
	//bool OnTCPNetworkSubUserChairReq(VOID * pData, WORD wDataSize, DWORD dwSocketID);
	//请求椅子用户信息
	bool OnTCPNetworkSubChairUserInfoReq(VOID * pData, WORD wDataSize, DWORD dwSocketID);
	//bool OnTCPAndroidSendTrumpet();

	//银行命令
protected:
	//查询银行
	//bool OnTCPNetworkSubQueryInsureInfo(VOID * pData, WORD wDataSize, DWORD dwSocketID);
    //转账记录
   // bool OnTCPNetworkSubQueryTransRecord(VOID * pData, WORD wDataSize, DWORD dwSocketID);
	//存款请求
	//bool OnTCPNetworkSubSaveScoreRequest(VOID * pData, WORD wDataSize, DWORD dwSocketID);
	//取款请求
	//bool OnTCPNetworkSubTakeScoreRequest(VOID * pData, WORD wDataSize, DWORD dwSocketID);
	//转账请求
	//bool OnTCPNetworkSubTransferScoreRequest(VOID * pData, WORD wDataSize, DWORD dwSocketID);
	//查询用户请求
	//bool OnTCPNetworkSubQueryUserInfoRequest(VOID * pData, WORD wDataSize, DWORD dwSocketID);
	// 根据GameID查询昵称
	//bool OnTCPNetworkSubQueryNickNameByGameID(VOID * pData, WORD wDataSize, DWORD dwSocketID);
	// 验证银行密码
	//bool OnTCPNetworkSubVerifyInsurePassword(VOID * pData, WORD wDataSize, DWORD dwSocketID);
	// 更改银行密码
	//bool OnTCPNetworkSubModifyInsurePassword(VOID * pData, WORD wDataSize, DWORD dwSocketID);

	//管理命令
protected:
	//查询设置
	//bool OnTCPNetworkSubQueryOption(VOID * pData, WORD wDataSize, DWORD dwSocketID);
	//房间设置
	//bool OnTCPNetworkSubOptionServer(VOID * pData, WORD wDataSize, DWORD dwSocketID);
	//踢出用户
	//bool OnTCPNetworkSubManagerKickUser(VOID * pData, WORD wDataSize, DWORD dwSocketID);
	//限制聊天
	//bool OnTCPNetworkSubLimitUserChat(VOID * pData, WORD wDataSize, DWORD dwSocketID);
	//踢出所有用户
	//bool OnTCPNetworkSubKickAllUser(VOID * pData, WORD wDataSize, DWORD dwSocketID);
	//解散游戏
	//bool OnTCPNetworkSubDismissGame(VOID * pData, WORD wDataSize, DWORD dwSocketID);

	//内部事件
protected:
	//用户登录
	VOID OnEventUserLogon(IServerUserItem * pIServerUserItem, bool bAlreadyOnLine);
	//用户登出
	VOID OnEventUserLogout(IServerUserItem * pIServerUserItem, DWORD dwLeaveReason);

	//执行功能
protected:
	//解锁游戏币
	//bool PerformUnlockScore(DWORD dwUserID, DWORD dwInoutIndex, DWORD dwLeaveReason);
	//版本检查
	bool PerformCheckVersion(DWORD dwPlazaVersion, DWORD dwFrameVersion, DWORD dwClientVersion, DWORD dwSocketID);
	//切换连接
	bool SwitchUserItemConnect(IServerUserItem * pIServerUserItem, TCHAR szMachineID[LEN_MACHINE_ID], WORD wTargetIndex,BYTE cbDeviceType=DEVICE_TYPE_PC,WORD wBehaviorFlags=0,WORD wPageTableCount=0);

	//发送函数
protected:
	//用户信息
	bool SendUserInfoPacket(IServerUserItem * pIServerUserItem, DWORD dwSocketID, bool isMeFlag = true);

	//辅助函数
protected:
	//购前事件
	//bool OnEventPropertyBuyPrep(WORD cbRequestArea,WORD wPropertyIndex,IServerUserItem *pISourceUserItem,IServerUserItem *pTargetUserItem);
	//道具消息
	//bool SendPropertyMessage(DWORD dwSourceID,DWORD dwTargerID,WORD wPropertyIndex,WORD wPropertyCount);
	//道具效应
	//bool SendPropertyEffect(IServerUserItem * pIServerUserItem);

	//辅助函数
protected:
	//登录失败
	bool SendLogonFailure(LPCTSTR pszString, LONG lErrorCode, DWORD dwSocketID);
	//银行失败
	//bool SendInsureFailure(IServerUserItem * pIServerUserItem, LPCTSTR pszDescribe, LONG lErrorCode,BYTE cbActivityGame);
	//请求失败
	bool SendRequestFailure(IServerUserItem * pIServerUserItem, LPCTSTR pszDescribe, LONG lErrorCode);
	//道具失败
	//bool SendPropertyFailure(IServerUserItem * pIServerUserItem, LPCTSTR pszDescribe, LONG lErrorCode,WORD wRequestArea);

	//辅助函数
public:
	//绑定用户
	IServerUserItem * GetBindUserItem(WORD wBindIndex);
	//绑定参数
	tagBindParameter * GetBindParameter(WORD wBindIndex);
	//道具类型
	//WORD GetPropertyType(WORD wPropertyIndex);

	//辅助函数
protected:
	//配置机器
	bool InitAndroidUser();
	//配置桌子
	bool InitTableFrameArray();
	//发送请求
	bool SendUIControlPacket(WORD wRequestID, VOID * pData, WORD wDataSize);
	//设置参数
	void SetMobileUserParameter(IServerUserItem * pIServerUserItem,BYTE cbDeviceType,WORD wBehaviorFlags,WORD wPageTableCount);
	//群发数据
	virtual bool SendDataBatchToMobileUser(WORD wCmdTable, WORD wMainCmdID, WORD wSubCmdID, VOID * pData, WORD wDataSize);
	//群发用户信息
	bool SendUserInfoPacketBatchToMobileUser(IServerUserItem * pIServerUserItem);
	//发可视用户信息
	bool SendViewTableUserInfoPacketToMobileUser(IServerUserItem * pIServerUserItem,DWORD dwUserIDReq);
	//手机立即登录
	bool MobileUserImmediately(IServerUserItem * pIServerUserItem);
	//购买道具
	//int OnPropertyBuy(VOID * pData, WORD wDataSize, DWORD dwSocketID);
	//发送系统消息
	bool SendSystemMessage(CMD_GR_SendMessage * pSendMessage, WORD wDataSize);
	//清除消息数据
	void ClearSystemMessageData();

	//机器人坐桌的辅助函数
protected:
	//评估机器人座在哪桌
	int GetAndriodTable(IAndroidUserItem * pIAndroidUserItem);

	//获取牌局
	//void GetGameCount();
public:
	//取分
	bool QuickGetScore(DWORD account, double & score);
	//写分
	bool QuickWriteScore(DWORD accountId, DWORD customerId, double score, std::wstring orderNo, std::wstring remark, int typeId = 0);

	//取得ACS的Credit TYPE ID
	int	GetAcsTypeIDCredit();
	//取得ACS的Debit TYPE ID
	int	GetAcsTypeIDDebit();
public:
	//初始化游戏参数
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
