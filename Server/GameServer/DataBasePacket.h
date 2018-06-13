#ifndef DATA_BASE_PACKET_HEAD_FILE
#define DATA_BASE_PACKET_HEAD_FILE

//////////////////////////////////////////////////////////////////////////////////
//�������ݰ�

//�û��¼�
#define	DBR_GR_LOGON_USERID			100									//I D ��¼
#define	DBR_GR_LOGON_MOBILE			101									//�ֻ���¼
#define	DBR_GR_LOGON_ACCOUNTS		102									//�ʺŵ�¼
//#define DBR_GR_LOGON_MATCHTIMER		103									//��ʱِ��¼

//ϵͳ�¼�
#define DBR_GR_WRITE_GAME_SCORE			200									//��Ϸ����
//#define DBR_GR_LEAVE_GAME_SERVER		201									//�뿪����
#define DBR_GR_GAME_SCORE_RECORD		202									//���ּ�¼
#define DBR_GR_MANAGE_USER_RIGHT		203									//Ȩ�޹���
//#define DBR_GR_LOAD_SYSTEM_MESSAGE		204									//ϵͳ��Ϣ
//#define DBR_GR_LOAD_SENSITIVE_WORDS		205									//���д�
//#define DBR_GR_LOAD_BALANCE_SCORE_CURVE 206									// ƽ�������
#define DBR_GR_RESET_GAME_SCORE_LOCKER	207									// ������Ϸ��
#define DBR_GR_LOAD_GAME_COUNT          208                                 //���ؾ���

//�����¼�
#define DBR_GR_LOAD_PARAMETER		300									//���ز���
#define DBR_GR_LOAD_GAME_COLUMN		301									//�����б�
#define DBR_GR_LOAD_ANDROID_USER	302									//���ػ���
#define DBR_GR_LOAD_GAME_PROPERTY	303									//���ص���

//�����¼�
//#define DBR_GR_USER_SAVE_SCORE		400									//������Ϸ��
//#define DBR_GR_USER_TAKE_SCORE		401									//��ȡ��Ϸ��
//#define DBR_GR_USER_TRANSFER_SCORE	402									//ת����Ϸ��
//#define DBR_GR_QUERY_INSURE_INFO	403									//��ѯ����
//#define DBR_GR_QUERY_TRANSFER_USER_INFO	    404							//��ѯ�û�
//#define DBR_GR_QUERY_TRANSRECORD    405                                 //ת�˼�¼
//#define DBR_GR_QUERY_NICKNAME_BY_GAMEID		406							// ͨ��GameID��ѯ�ǳ�
//#define DBR_GR_VERIFY_INSURE_PASSWORD		407							// ��֤��������
//#define DBR_GR_MODIFY_INSURE_PASSWORD		408							// �޸���������	

//��Ϸ�¼�
//#define DBR_GR_PROPERTY_REQUEST		500									//��������
//#define DBR_GR_GAME_FRAME_REQUEST	502									//��Ϸ����

//�����¼�
//#define DBR_GR_MATCH_FEE			600									//��������
//#define DBR_GR_MATCH_START			601									//������ʼ
//#define DBR_GR_MATCH_OVER			602									//��������
//#define DBR_GR_MATCH_REWARD			603									//��������
//#define DBR_GR_MATCH_QUIT			604									//�˳�����
//#define DBR_GR_MATCH_ENTER_FOR		605									//��������

//////////////////////////////////////////////////////////////////////////////////
//�����Ϣ

//�߼��¼�
#define DBO_GR_LOGON_SUCCESS		100									//��¼�ɹ�
#define DBO_GR_LOGON_FAILURE		101									//��¼ʧ��

//�����¼�
#define DBO_GR_GAME_PARAMETER		200									//������Ϣ
#define DBO_GR_GAME_COLUMN_INFO		201									//�б���Ϣ
#define DBR_GR_GAME_ANDROID_INFO	202									//������Ϣ
#define DBO_GR_GAME_PROPERTY_INFO	203									//������Ϣ

//��������
//#define DBO_GR_USER_INSURE_INFO		300									//��������
//#define DBO_GR_USER_INSURE_SUCCESS	301									//���гɹ�
//#define DBO_GR_USER_INSURE_FAILURE	302									//����ʧ��
//#define DBO_GR_USER_INSURE_USER_INFO   303								//�û�����
//#define DBR_GR_TRANS_RECORD         304                                 //ת�˼�¼
//#define DBO_GR_INSURE_TRANS_RECEIPT	305									// ת�˻�ִ

//��Ϸ�¼�
//#define DBO_GR_PROPERTY_SUCCESS		400									//���߳ɹ�
//#define DBO_GR_PROPERTY_FAILURE		401									//����ʧ��
//#define DBO_GR_GAME_FRAME_RESULT	402									//��Ϸ���
#define DBO_GP_LOAD_GAME_COUNT      403                                 //��ѯ����

//�����¼�
//#define DBO_GR_MATCH_EVENT_START	500									//�¼���Χ
//#define DBO_GR_MATCH_FEE_RESULT		500									//��������
//#define DBO_GR_MATCH_RANK			501									//��������
//#define DBO_GR_MATCH_QUIT_RESULT	502									//�������
//#define DBO_GR_MATCH_EVENT_END		599									//�¼���Χ

//ϵͳ�¼�
//#define DBO_GR_SYSTEM_MESSAGE_RESULT	600									//��Ϣ���
//#define DBO_GR_SENSITIVE_WORDS		    601									//���д�
//#define DBO_GR_BALANCE_SCORE_CURVE		602									// ƽ���

// ����ʱ��
//#define DBO_GR_OPERATE_SUCCESS		700									// �����ɹ�
//#define DBO_GR_OPERATE_FAILURE		701									// ����ʧ��

//�û��¼�
//#define DBO_GR_USER_BALANCE_SCORE	800									// �û�ƽ���

//GLS USER LOGON
#define GLS_USER_LOGON_INPUT_PARAMETER_ERROR	251
#define GLS_USER_LOGON_GET_GAME_SERVER_ERROR	253
#define GLS_USER_LOGON_USER_NOT_EXIST			254
#define GLS_USER_LOGON_INVALID_PARAMETER		259
#define GLS_USER_LOGON_SYSTEM_ERROR				261
#define GLS_USER_LOGON_ALREADY_IN_OTHER_GAME	262
#define GLS_USER_LOGON_GAME_SERVER_SHUT_DOWN	267

//////////////////////////////////////////////////////////////////////////////////

//ID ��¼
struct DBR_GR_LogonUserID
{		
	//��¼��Ϣ
	DWORD							dwUserID;							//�û� I D
	TCHAR							szPassword[LEN_MD5];				//��¼����

	//������Ϣ
	DWORD							dwClientAddr;						//���ӵ�ַ
	TCHAR							szMachineID[LEN_MACHINE_ID];		//������ʶ

	DWORD acsNormalAccount;//acs��ͨ�˺�id��������Ϸ��ʹ��Ǯ
	DWORD acsSafeAccount;//acs�ʽ��˻�id���������˺�
};

//ID ��¼
struct DBR_GR_LogonMobile
{		
	//��¼��Ϣ
	DWORD							dwUserID;							//�û� I D
	TCHAR							szPassword[LEN_MD5];				//��¼����
	BYTE                            cbDeviceType;                       //�豸����
	WORD                            wBehaviorFlags;                     //��Ϊ��ʶ
	WORD                            wPageTableCount;                    //��ҳ����

	//������Ϣ
	DWORD							dwClientAddr;						//���ӵ�ַ
	TCHAR							szMachineID[LEN_MACHINE_ID];		//������ʶ
};

//�ʺŵ�¼
struct DBR_GR_LogonAccounts
{
	//��¼��Ϣ
	TCHAR							szPassword[LEN_MD5];				//��¼����
	TCHAR							szAccounts[LEN_ACCOUNTS];			//��¼�ʺ�
	DWORD							dwUserID;							//�û� I D

	//������Ϣ
	DWORD							dwClientAddr;						//���ӵ�ַ
	TCHAR							szMachineID[LEN_MACHINE_ID];		//������ʶ
	TCHAR                           szUserArea[32];                     //�������
};

//��Ϸ����
struct DBR_GR_WriteGameScore
{
	//�û���Ϣ
	DWORD							dwUserID;							//�û���ʶ
	TCHAR							szAccounts[LEN_ACCOUNTS];			//��¼�ʺ�
	DWORD acsNormalAccount;                                             //acs��ͨ�˺�id��������Ϸ��ʹ��Ǯ
	DWORD acsSafeAccount;                                               //acs�ʽ��˻�id���������˺�
	DWORD							dwClientAddr;						//���ӵ�ַ

	//�û���Ϣ
	DWORD							dwDBQuestID;						//�����ʶ
	DWORD							dwInoutIndex;						//��������
	std::string                     bzTableNum;                         //���ӱ��                
	// ������ţ���λ�ŵ���Ϣ
//	WORD							wTableID;							// ����
//	WORD							wChairID;							// ���Ӻ�

	//��Ϸ�ұ���
	tagVariationInfo				VariationInfo;						//��ȡ��Ϣ
};

//�뿪����
//struct DBR_GR_LeaveGameServer
//{
//	//�û���Ϣ
//	DWORD							dwUserID;							//�û���ʶ
//	DWORD							dwInoutIndex;						//��¼����
//	DWORD							dwLeaveReason;						//�뿪ԭ��
//	DWORD							dwOnLineTimeCount;					//����ʱ��
//
//	//��Ϸ�ұ���
//	tagVariationInfo				RecordInfo;							//��¼��Ϣ
//	tagVariationInfo				VariationInfo;						//��ȡ��Ϣ
//
//	//ϵͳ��Ϣ
//	DWORD							dwClientAddr;						//���ӵ�ַ
//	TCHAR							szMachineID[LEN_MACHINE_ID];		//������ʶ
//};

//��Ϸ��¼
//struct DBR_GR_GameScoreRecord
//{
//	//������Ϣ
//	WORD							wTableID;							//���Ӻ���
//	WORD							wUserCount;							//�û���Ŀ
//	WORD							wAndroidCount;						//������Ŀ
//
//	//���˰��
//	SCORE							lWasteCount;						//�����Ŀ
//	SCORE							lRevenueCount;						//˰����Ŀ
//
//	//ͳ����Ϣ
//	DWORD							dwUserMemal;						//������Ŀ
//	DWORD							dwPlayTimeCount;					//��Ϸʱ��
//
//	//ʱ����Ϣ
//	SYSTEMTIME						SystemTimeStart;					//��ʼʱ��
//	SYSTEMTIME						SystemTimeConclude;					//����ʱ��
//
//	//���ּ�¼
//	WORD							wRecordCount;						//��¼��Ŀ
//	tagGameScoreRecord				GameScoreRecord[MAX_CHAIR];			//��Ϸ��¼
//};

//������Ϸ��
//struct DBR_GR_UserSaveScore
//{
//	BYTE                            cbActivityGame;                     //��Ϸ����
//	DWORD							dwUserID;							//�û� I D
//	SCORE							lSaveScore;							//������Ϸ��
//	DWORD							dwClientAddr;						//���ӵ�ַ
//	TCHAR							szMachineID[LEN_MACHINE_ID];		//��������
//};

//ȡ����Ϸ��
//struct DBR_GR_UserTakeScore
//{
//	BYTE                            cbActivityGame;                     //��Ϸ����
//	DWORD							dwUserID;							//�û� I D
//	SCORE							lTakeScore;							//��ȡ��Ϸ��
//	DWORD							dwClientAddr;						//���ӵ�ַ
//	TCHAR							szPassword[LEN_PASSWORD];			//��������
//	TCHAR							szMachineID[LEN_MACHINE_ID];		//��������
//};

//ת����Ϸ��
//struct DBR_GR_UserTransferScore
//{
//	BYTE                            cbActivityGame;                     //��Ϸ����
//	DWORD							dwUserID;							//�û� I D
//	DWORD							dwClientAddr;						//���ӵ�ַ
//	BYTE                            cbByNickName;                       //�ǳ�����
//	SCORE							lTransferScore;						//ת����Ϸ��
//	TCHAR							szNickName[LEN_NICKNAME];			//Ŀ���û�
//	TCHAR							szPassword[LEN_PASSWORD];			//��������
//	TCHAR							szMachineID[LEN_MACHINE_ID];		//��������
//};

//��ѯ����
//struct DBR_GR_QueryInsureInfo
//{
//	BYTE                            cbActivityGame;                     //��Ϸ����
//	DWORD							dwUserID;							//�û� I D
//	DWORD							dwClientAddr;						//���ӵ�ַ
//};
//ת�˼�¼
//struct DBR_GR_UserTransRecord
//{
//    DWORD                               dwGameID;
//    TCHAR                               szSourceAccount[LEN_ACCOUNTS];
//    TCHAR                               szTargetAccounts[LEN_ACCOUNTS];
//    LONGLONG                            lSwapScore;
//    LONGLONG                            lRevenue;
//    TCHAR                               szTime[32];
//	DWORD								dwTargetID;
//    bool                                bOver;
//};
//��ѯ�û�
//struct DBR_GR_QueryTransferUserInfo
//{
//	BYTE                            cbActivityGame;                     //��Ϸ����
//	BYTE                            cbByNickName;                       //�ǳ�����
//	TCHAR							szNickName[LEN_NICKNAME];			//Ŀ���û�
//	DWORD							dwUserID;							//�û� I D
//};

// ��ѯ�ǳ�
//struct DBR_GR_QueryNickNameByGameID
//{
//	DWORD							dwUserID;							// �����ѯ����ҵ�UserID
//	DWORD							dwGameID;							// �û�GameID
//};

// ��֤�û���������
//struct DBR_GR_VerifyInsurePassword
//{
//	DWORD							dwUserID;						//�û� I D
//	TCHAR							szInsurePassword[LEN_MD5];		//�ʺ��ǳ�
//};

//�޸�����
//struct DBR_GR_ModifyInsurePass
//{
//	DWORD							dwUserID;							//�û� I D
//	DWORD							dwClientAddr;						//���ӵ�ַ
//	TCHAR							szDesPassword[LEN_PASSWORD];		//�û�����
//	TCHAR							szScrPassword[LEN_PASSWORD];		//�û�����
//};

//��������
//struct DBR_GR_PropertyRequest
//{
//	//������Ϣ
//	WORD							wItemCount;							//������Ŀ
//	WORD							wPropertyIndex;						//��������
//	DWORD							dwSourceUserID;						//�������
//	DWORD							dwTargetUserID;						//ʹ�ö���
//
//	//����ģʽ
//	BYTE                            cbRequestArea;			            //ʹ�÷�Χ 
//	BYTE							cbConsumeScore;						//��������
//	SCORE							lFrozenedScore;						//�������
//
//	//�û�Ȩ��
//	//DWORD                           dwUserRight;						//��ԱȨ��
//
//	//ϵͳ��Ϣ
//	WORD							wTableID;							//���Ӻ���
//	DWORD							dwInoutIndex;						//�����ʶ
//	DWORD							dwClientAddr;						//���ӵ�ַ
//	TCHAR							szMachineID[LEN_MACHINE_ID];		//��������
//};

//�����ɹ�
//struct DBO_GR_OperateSuccess
//{
//	LONG							lResultCode;						//��������
//	TCHAR							szDescribeString[128];				//�ɹ���Ϣ
//};

//////////////////////////////////////////////////////////////////////////////////

//��¼�ɹ�
struct DBO_GR_LogonSuccess
{
	//��������
	WORD							wFaceID;							//ͷ���ʶ
	DWORD							dwUserID;							//�û� I D
	DWORD							dwGameID;							//��Ϸ I D
	DWORD							dwGroupID;							//���� I D
	DWORD							dwCustomID;							//�Զ�ͷ��
	TCHAR							szNickName[LEN_NICKNAME];			//�ʺ��ǳ�
	TCHAR							szGroupName[LEN_GROUP_NAME];		//��������
	TCHAR                           szAccount[LEN_ACCOUNTS];            //�û��ʺ�

	//�û�����
	BYTE							cbGender;							//�û��Ա�
	BYTE							cbMemberOrder;						//��Ա�ȼ�
	BYTE							cbMasterOrder;						//����ȼ�
	TCHAR							szUnderWrite[LEN_UNDER_WRITE];		//����ǩ��

	//������Ϣ
	SCORE							lScore;								//�û�����
	SCORE							lGrade;								//�û���Ϸ��
	SCORE							lInsure;							//�û�����

	//��Ϸ��Ϣ
	DWORD							dwWinCount;							//ʤ������
	DWORD							dwLostCount;						//ʧ������
	DWORD							dwDrawCount;						//�;�����
	DWORD							dwFleeCount;						//��������
	DWORD							dwUserMedal;						//�û�����
	DWORD							dwExperience;						//�û�����
	LONG							lLoveLiness;						//�û�����

	//�û�Ȩ��
	//DWORD							dwUserRight;						//�û�Ȩ��
	//DWORD							dwMasterRight;						//����Ȩ��

	//��������
	DWORD							dwInoutIndex;						//��¼����
	TCHAR							szMachineID[LEN_MACHINE_ID];		//������ʶ

	//�ֻ�����
	BYTE                            cbDeviceType;                       //�豸����
	WORD                            wBehaviorFlags;                     //��Ϊ��ʶ
	WORD                            wPageTableCount;                    //��ҳ����

	// �����Ӯ(added by anjay)
	SCORE							lTotalWin;							// �����Ϸ��Ӯ
	SCORE							lTotalLose;							// �����Ϸ����

	//������Ϣ
	TCHAR							szPassword[LEN_MD5];				//��¼����
	TCHAR							szDescribeString[128];				//������Ϣ

	DWORD acsNormalAccount;//acs��ͨ�˺�id��������Ϸ��ʹ��Ǯ
	DWORD acsSafeAccount;//acs�ʽ��˻�id���������˺�

	BOOL                            bAccompanyFlag;                     //����־
	short                           fortuneProb;						//����ֵ
	std::wstring					szClientIP;                         //�û�IP
	BYTE                            robotLevel;                         //�����˵ȼ�
	TCHAR							channelName[128];					//�û�Ʒ��
	BOOL							isEnableKill;						//�Ƿ�����ɱ������
	UINT							killNum;							//ɱ��ֵ
	std::wstring                    szCountry;                          //����
	std::wstring                    szUserArea;                         //�������
	//std::map<std::wstring, DWORD>	userMixMerchant;					//�ɻ�����Ʒ��
	TCHAR							userMixMerchant[128];				//�ɻ�����Ʒ��
	DBO_GR_LogonSuccess(){
		this->wFaceID = 0;
		this->dwUserID = 0;
		this->dwGameID = 0;
		this->dwGroupID = 0;
		this->dwCustomID = 0;
		memset(this->szNickName, 0, sizeof(szNickName));
		memset(this->szGroupName, 0, sizeof(szGroupName));
		memset(this->szAccount, 0, sizeof(szAccount));
		this->cbGender = 0;
		this->cbMemberOrder = 0;
		this->cbMasterOrder = 0;
		memset(this->szUnderWrite, 0, sizeof(this->szUnderWrite));
		this->lScore = 0;
		this->lGrade = 0;
		this->lInsure = 0;
		this->dwWinCount = 0;
		this->dwLostCount = 0;
		this->dwDrawCount = 0;
		this->dwFleeCount = 0;
		this->dwUserMedal = 0;
		this->dwExperience = 0;
		this->lLoveLiness = 0;
		//this->dwUserRight = 0;
		//this->dwMasterRight = 0;
		this->dwInoutIndex = 0;
		memset(this->szMachineID, 0, sizeof(this->szMachineID));
		this->cbDeviceType = 0;
		this->wBehaviorFlags = 0;
		this->wPageTableCount = 0;
		this->lTotalWin = 0;
		this->lTotalLose = 0;
		memset(this->szPassword, 0, sizeof(this->szPassword));
		memset(this->szDescribeString, 0, sizeof(this->szDescribeString));
		this->acsNormalAccount = 0;
		this->acsSafeAccount = 0;
		this->bAccompanyFlag = 0;
		this->fortuneProb = 0;
		this->szClientIP = L"";
		this->robotLevel = 0;
		memset(this->channelName, 0, sizeof(this->channelName));
		this->isEnableKill = 0;
		this->killNum = 100;
		this->szCountry = L"";
		this->szUserArea = L"";
		//this->userMixMerchant.clear();
		memset(this->userMixMerchant, 0, sizeof(this->userMixMerchant));
	}
};

//��¼ʧ��
struct DBO_GR_LogonFailure
{
	LONG							lResultCode;						//�������
	TCHAR							szDescribeString[128];				//������Ϣ
};

//������Ϣ
struct DBO_GR_GameParameter
{
	//������Ϣ
	WORD							wMedalRate;							//���ƻ���
	WORD							wRevenueRate;						//����˰��

	//�汾��Ϣ
	DWORD							dwClientVersion;					//�ͻ��汾
	DWORD							dwServerVersion;					//����汾
};

//�б���Ϣ
struct DBO_GR_GameColumnInfo
{
	LONG							lResultCode;						//�������
	BYTE							cbColumnCount;						//�б���Ŀ
	tagColumnItem					ColumnItemInfo[MAX_COLUMN];			//�б���Ϣ
};

//������Ϣ
struct DBO_GR_GameAndroidInfo
{
	LONG							lResultCode;						//�������
	WORD							wAndroidCount;						//�û���Ŀ
	tagAndroidParameter				AndroidParameter[MAX_ANDROID];		//������Ϣ
};

//������Ϣ
struct DBO_GR_GamePropertyInfo
{
	LONG							lResultCode;						//�������
	BYTE							cbPropertyCount;					//������Ŀ
	tagPropertyInfo					PropertyInfo[MAX_PROPERTY];			//������Ϣ
};

//��������
struct DBO_GR_UserInsureInfo
{
	BYTE                            cbActivityGame;                     //��Ϸ����
	WORD							wRevenueTake;						//˰�ձ���
	WORD							wRevenueTransfer;					//˰�ձ���
	WORD							wServerID;							//�����ʶ
	SCORE							lUserScore;							//�û���Ϸ��
	SCORE							lUserInsure;						//������Ϸ��
	SCORE							lTransferPrerequisite;				//ת������
};

//���гɹ�
struct DBO_GR_UserInsureSuccess
{
	BYTE                            cbActivityGame;                     //��Ϸ����
	DWORD							dwUserID;							//�û� I D
	SCORE							lSourceScore;						//ԭ����Ϸ��
	SCORE							lSourceInsure;						//ԭ����Ϸ��
	SCORE							lInsureRevenue;						//����˰��
	SCORE							lFrozenedScore;						//�������
	SCORE							lVariationScore;					//��Ϸ�ұ仯
	SCORE							lVariationInsure;					//���б仯
	TCHAR							szDescribeString[128];				//������Ϣ
};

//����ʧ��
struct DBO_GR_UserInsureFailure
{
	BYTE                            cbActivityGame;                     //��Ϸ����
	LONG							lResultCode;						//��������
	SCORE							lFrozenedScore;						//�������
	TCHAR							szDescribeString[128];				//������Ϣ
};

//�û�����
struct DBO_GR_UserTransferUserInfo
{
	BYTE                            cbActivityGame;                     //��Ϸ����
	DWORD							dwGameID;							//�û� I D
	TCHAR							szNickName[LEN_NICKNAME];			//�ʺ��ǳ�
};

//���߳ɹ�
struct DBO_GR_S_PropertySuccess
{
	//������Ϣ
	WORD							wItemCount;							//������Ŀ
	WORD							wPropertyIndex;						//��������
	DWORD							dwSourceUserID;						//�������
	DWORD							dwTargetUserID;						//ʹ�ö���

	//����ģʽ
	BYTE                            cbRequestArea;						//����Χ
	BYTE							cbConsumeScore;						//��������
	SCORE							lFrozenedScore;						//�������

	//��ԱȨ��
	//DWORD                           dwUserRight;						//��ԱȨ��

	//�����Ϣ
	SCORE							lConsumeGold;						//������Ϸ��
	LONG							lSendLoveLiness;					//��������
	LONG							lRecvLoveLiness;					//��������

	//��Ա��Ϣ
	BYTE							cbMemberOrder;						//��Ա�ȼ�
};

//����ʧ��
struct DBO_GR_PropertyFailure
{
	BYTE                            cbRequestArea;						//����Χ
	LONG							lResultCode;						//��������
	SCORE							lFrozenedScore;						//�������
	TCHAR							szDescribeString[128];				//������Ϣ
};

//��������
//struct DBO_GR_MatchRank
//{
//	BYTE							cbRank;								//��������
//	TCHAR							szNickName[LEN_NICKNAME];			//�û��ǳ�
//	LONG							lMatchScore;						//�û��÷�
//};

struct DBO_GR_UserBalanceScore
{
	DWORD							dwUserID;							// ��ҵ�����
	LONG							lBalanceScore;						// ��ҵ�ƽ���
	LONG							lBuffTime;							// ƽ���Buff����ʱ��
	LONG							lBuffGames;							// ƽ���Buff������Ϸ����
	LONG							lBuffAmount;						// ƽ���Buff����(����������)
	bool							bLogon;								// �Ƿ��ǵ�¼ʱ�õ���ƽ���
};

// ƽ�����������
struct DBO_GR_BalanceScoreCurve
{
	SCORE							lScore;								// ��Ӯ�ܺ͵ķ���
	LONG							lBalanceScore;						// ��Ӧ��ƽ��ֵ����ķ���
};

//Ȩ�޹���
struct DBR_GR_ManageUserRight
{
	DWORD							dwUserID;							//Ŀ���û�
	DWORD							dwAddRight;							//���Ȩ��
	DWORD							dwRemoveRight;						//ɾ��Ȩ��
	bool							bGameRight;							//��ϷȨ��
};

//��������
//struct DBR_GR_MatchFee
//{
//	//��������
//	DWORD							dwUserID;							//�û�ID
//	DWORD							dwMatchFee;							//��������
//
//	//ϵͳ��Ϣ
//	DWORD							dwInoutIndex;						//�����ʶ
//	DWORD							dwClientAddr;						//���ӵ�ַ
//	DWORD							dwMatchID;							//����ID
//	DWORD							dwMatchNO;							//��������
//	TCHAR							szMachineID[LEN_MACHINE_ID];		//��������
//};

//������ʼ
//struct DBR_GR_MatchStart
//{
//	DWORD							dwMatchID;						//������ʶ
//	DWORD							dwMatchNo;						//��������
//	WORD							wMatchCount;					//�ܳ���
//	LONG							lInitScore;						//��ʼ����
//};

//��������
//struct DBR_GR_MatchOver
//{
//	DWORD							dwMatchID;						//������ʶ
//	DWORD							dwMatchNo;						//��������
//	WORD							wMatchCount;					//�ܳ���
//};

//��������
//struct DBR_GR_MatchReward
//{
//	DWORD								dwUserID;							//�û� I D
//	DWORD								dwMatchID;							//���� I D 
//	DWORD								dwMatchNO;							//��������
//	WORD								wRank;								//��������
//	SCORE								lMatchScore;						//�����÷�
//	DWORD								dwExperience;						//�û�����
//	DWORD								dwGold;								//��Ϸ��
//	DWORD								dwMedal;							//�û�����
//	DWORD								dwClientAddr;						//���ӵ�ַ
//};

//��ʱģʽ������¼
//struct DBR_GR_MatchEnterFor
//{
//	DBR_GR_LogonUserID			LogonUserID;
//};


//ϵͳ��Ϣ
struct DBR_GR_SystemMessage
{
	BYTE                            cbMessageID;                        //��ϢID
	BYTE							cbMessageType;						//��Ϣ����
	BYTE                            cbAllRoom;                          //ȫ�巿��
	DWORD							dwTimeRate;						    //ʱ��Ƶ��
	__time64_t                      tConcludeTime;                      //����ʱ��
	TCHAR							szSystemMessage[LEN_USER_CHAT];		//ϵͳ��Ϣ
};
//////////////////////////////////////////////////////////////////////////////////

#endif
