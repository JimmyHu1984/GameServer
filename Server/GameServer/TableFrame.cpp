#include "StdAfx.h"
#include "TableFrame.h"
#include "ServiceUnits.h"
#include "InitParameter.h"
#include "AttemperEngineSink.h"
#include "StockManager.h"
#include "DataBasePacket.h"
#include <iostream>
//////////////////////////////////////////////////////////////////////////////////

//���߶���
#define IDI_OFF_LINE				(TIME_TABLE_SINK_RANGE+1)			//���߱�ʶ
#define IDI_DISTRIBUTE_START        (TIME_TABLE_SINK_RANGE+2)           //������ʱ
#define MAX_OFF_LINE				3									//���ߴ���
#define TIME_OFF_LINE				60000L								//����ʱ��

//////////////////////////////////////////////////////////////////////////////////

//�������
CStockManager						g_StockManager;						//������

//��Ϸ��¼
CGameScoreRecordArray				CTableFrame::m_GameScoreRecordBuffer;

//////////////////////////////////////////////////////////////////////////////////

//���캯��
CTableFrame::CTableFrame()
{
	//��������
	m_wTableID=0;
	m_wChairCount=0;
	m_cbStartMode=START_MODE_ALL_READY;
	m_wUserCount=0;

	//��־����
	m_bGameStarted=false;
	m_bDrawStarted=false;
	m_bTableStarted=false;
	//ZeroMemory(m_bAllowLookon,sizeof(m_bAllowLookon));
	ZeroMemory(m_lFrozenedScore,sizeof(m_lFrozenedScore));

	//��Ϸ����
	m_lCellScore=0L;
	m_cbGameStatus=GAME_STATUS_FREE;
	m_wDrawCount=0;

	//ʱ�����
	m_dwDrawStartTime=0L;
	ZeroMemory(&m_SystemTimeStart,sizeof(m_SystemTimeStart));

	//��̬����
	m_dwTableOwnerID=0L;
	ZeroMemory(m_szEnterPassword,sizeof(m_szEnterPassword));

	//���߱���
	ZeroMemory(m_wOffLineCount,sizeof(m_wOffLineCount));
	ZeroMemory(m_dwOffLineTime,sizeof(m_dwOffLineTime));

	//������Ϣ
	m_pGameParameter=NULL;
	m_pGameServiceAttrib=NULL;
	m_pGameServiceOption=NULL;

	//����ӿ�
	m_pITimerEngine=NULL;
	m_pITableFrameSink=NULL;
	m_pIMainServiceFrame=NULL;
	m_pIAndroidUserManager=NULL;

	//���Žӿ�
	m_pITableUserAction=NULL;
	m_pITableUserRequest=NULL;
	//m_pIMatchTableAction=NULL;

	//���ݽӿ�
	m_pIKernelDataBaseEngine=NULL;
	m_pIRecordDataBaseEngine=NULL;

	//�����ӿ�
	//m_pIGameMatchSink=NULL;

	//�û�����
	ZeroMemory(m_TableUserItemArray,sizeof(m_TableUserItemArray));

	m_strCurrentTableNum = "";
	return;
}

//��������
CTableFrame::~CTableFrame()
{
	//�ͷŶ���
	SafeRelease(m_pITableFrameSink);
	//SafeRelease(m_pIMatchTableAction);

	//if (m_pIGameMatchSink!=NULL)
	//{
	//	SafeDelete(m_pIGameMatchSink);
	//}

	return;
}

//�ӿڲ�ѯ
VOID * CTableFrame::QueryInterface(REFGUID Guid, DWORD dwQueryVer)
{
	QUERYINTERFACE(ITableFrame,Guid,dwQueryVer);
	QUERYINTERFACE_IUNKNOWNEX(ITableFrame,Guid,dwQueryVer);
	return NULL;
}

//��ʼ��Ϸ
bool CTableFrame::StartGame()
{
	//LOGI(L"CTableFrame::StartGame");
	CAttemperEngineSink * pAttemperEngineSink=(CAttemperEngineSink *)m_pIMainServiceFrame;
	m_strCurrentTableNum = pAttemperEngineSink->GetTableNum();
	LOGI(L"CTableFrame::StartGame TableNum = " << m_strCurrentTableNum.c_str());
	//��Ϸ״̬
	ASSERT(m_bDrawStarted==false);
	if(m_bDrawStarted==true){
		LOGI(L"Start Game Fail cause m_bDrawStarted = true");
		return false;
	}
	if(pAttemperEngineSink->GetCSStatus()){
		LOGW(L"CAttemperEngineSink::OnTCPNetworkSubLogonAccounts, Lobby tell close now, can not logon");
		return false;
	}
	//�������
	bool bGameStarted=m_bGameStarted;
	bool bTableStarted=m_bTableStarted;

	//����״̬
	m_bGameStarted=true;
	m_bDrawStarted=true;
	m_bTableStarted=true;

	//��ʼʱ��
	GetLocalTime(&m_SystemTimeStart);
	m_dwDrawStartTime=(DWORD)time(NULL);

	if(bGameStarted==false){	//��ʼ����
		ZeroMemory(m_wOffLineCount,sizeof(m_wOffLineCount));
		ZeroMemory(m_dwOffLineTime,sizeof(m_dwOffLineTime));
		for(WORD i=0;i<m_wChairCount;i++){	//�����û�
			IServerUserItem *pIServerUserItem=GetTableUserItem(i);
			if(pIServerUserItem!=NULL){	//�����û�
				//������Ϸ��
				if(m_pGameServiceOption->lServiceScore>0L){
					m_lFrozenedScore[i]=m_pGameServiceOption->lServiceScore;
					pIServerUserItem->FrozenedUserScore(m_pGameServiceOption->lServiceScore);
				}

				//����״̬
				BYTE cbUserStatus=pIServerUserItem->GetUserStatus();
				if ((cbUserStatus!=US_OFFLINE)&&(cbUserStatus!=US_PLAYING)) pIServerUserItem->SetUserStatus(US_PLAYING,m_wTableID,i);
				
			}
		}

		//����״̬
		if (bTableStarted!=m_bTableStarted) SendTableStatus();
	}

	//����֪ͨ
	bool bStart=true;
	//if(m_pIGameMatchSink!=NULL) bStart=m_pIGameMatchSink->OnEventGameStart(this, m_wChairCount);

	//֪ͨ�¼�
	ASSERT(m_pITableFrameSink!=NULL);
	if (m_pITableFrameSink!=NULL&&bStart) m_pITableFrameSink->OnEventGameStart();

	return true;
}

//��ɢ��Ϸ
bool CTableFrame::DismissGame()
{

	//LOGI(L"CTableFrame::DismissGame");

	//״̬�ж�
	ASSERT(m_bTableStarted==true);
	if (m_bTableStarted==false) return false;

	//������Ϸ
	if ((m_bGameStarted==true)&&(m_pITableFrameSink->OnEventGameConclude(INVALID_CHAIR,NULL,GER_DISMISS)==false))
	{
		ASSERT(FALSE);
		return false;
	}

	//����״̬
	if ((m_bGameStarted==false)&&(m_bTableStarted==true))
	{
		//���ñ���
		m_bTableStarted=false;

		//����״̬
		SendTableStatus();
	}

	return false;
}

//������Ϸ
bool CTableFrame::ConcludeGame(BYTE cbGameStatus)
{

	//LOGI(L"CTableFrame::ConcludeGame");

	//Ч��״̬
	ASSERT(m_bGameStarted==true);
	if (m_bGameStarted==false) return false;

	//�������
	bool bDrawStarted=m_bDrawStarted;

	//����״̬
	m_bDrawStarted=false;
	m_cbGameStatus=cbGameStatus;
	m_bGameStarted=(cbGameStatus>=GAME_STATUS_PLAY)?true:false;
	m_wDrawCount++;

	//��Ϸ��¼
	//RecordGameScore(bDrawStarted);
	
	//��������
	if (m_bGameStarted==false)
	{
		//��������
		bool bOffLineWait=false;

		//�����û�
		for (WORD i=0;i<m_wChairCount;i++)
		{
			//��ȡ�û�
			IServerUserItem * pIServerUserItem=GetTableUserItem(i);

			//�û�����
			if (pIServerUserItem!=NULL)
			{
				CAttemperEngineSink * pAttemperEngineSink=(CAttemperEngineSink *)m_pIMainServiceFrame;
				if (!pAttemperEngineSink)
				{
					LOGE(L"CTableFrame::ConcludeGame pAttemperEngineSink = NULL");
					return false;
				}

				tagUserInfo* tui = pIServerUserItem->GetUserInfo();
// 				std::string lastTableNum;
// 				lastTableNum.append((char*)GetTableNum());
				tui->lastTableNum = GetTableNum();
				tagTimeInfo* TimeInfo=pIServerUserItem->GetTimeInfo();
				//��Ϸʱ��
				DWORD dwCurrentTime=(DWORD)time(NULL);
				TimeInfo->dwEndGameTimer=dwCurrentTime;
				//������Ϸ��
				if (m_lFrozenedScore[i]!=0L) 
				{
					pIServerUserItem->UnFrozenedUserScore(m_lFrozenedScore[i]);
					m_lFrozenedScore[i]=0L;
				}

				//����״̬
				if(pIServerUserItem->GetUserStatus()==US_OFFLINE)
				{
					//���ߴ���
					bOffLineWait=true;
					PerformStandUpAction(pIServerUserItem);
				}
				else if (CServerRule::IsAllowAvertCheatMode(m_pGameServiceOption->dwServerRule))
				{
					PerformStandUpAction(pIServerUserItem,1);
				}
				else
				{
					//����״̬
					pIServerUserItem->SetUserStatus(US_SIT,m_wTableID,i);

					//��������ͺ��Ե�
					//if(m_pGameServiceOption->wServerType&GAME_GENRE_MATCH)continue;
					//��������
					if (pIServerUserItem->IsAndroidUser()==true)
					{
						//���һ���
						CAttemperEngineSink * pAttemperEngineSink=(CAttemperEngineSink *)m_pIMainServiceFrame;
						tagBindParameter * pBindParameter=pAttemperEngineSink->GetBindParameter(pIServerUserItem->GetBindIndex());
						IAndroidUserItem * pIAndroidUserItem=m_pIAndroidUserManager->SearchAndroidUserItem(pIServerUserItem->GetUserID(),
							pBindParameter->dwSocketID);

						//��������
						if (pIAndroidUserItem!=NULL)
						{	
							//��ȡʱ��
							SYSTEMTIME SystemTime;
							GetLocalTime(&SystemTime);
							DWORD dwTimeMask=(1L<<SystemTime.wHour);

							//��ȡ����
							tagAndroidService * pAndroidService=pIAndroidUserItem->GetAndroidService();
							tagAndroidParameter * pAndroidParameter=pIAndroidUserItem->GetAndroidParameter();

							//������Ϣ
							if(pAndroidService->dwResidualPlayDraw>0)
								--pAndroidService->dwResidualPlayDraw;

							//����ʱ��
							if (((pIServerUserItem->GetTableID()==GetTableID())&&(pAndroidParameter->dwServiceTime&dwTimeMask)==0L))
							{
								LOGI("CTableFrame::ConcludeGame PerformStandUpAction, (pAndroidParameter->dwServiceTime&dwTimeMask == 0L), NickName="<<pIServerUserItem->GetNickName());
								PerformStandUpAction(pIServerUserItem);
								continue;
							}

	
// 							DWORD dwLogonTime=pIServerUserItem->GetLogonTime();
// 							DWORD dwReposeTime=pAndroidService->dwReposeTime;
// 							if ((dwLogonTime+dwReposeTime)<dwCurrentTime)
// 							{
// 								LOGI("CTableFrame::ConcludeGame PerformStandUpAction, ((dwLogonTime+dwReposeTime)<dwCurrentTime), NickName="<<pIServerUserItem->GetNickName());
// 								PerformStandUpAction(pIServerUserItem);
// 								continue;
// 							}


							//�����ж�
							if ((pIServerUserItem->GetTableID()==GetTableID())&&(pAndroidService->dwResidualPlayDraw==0))
							{
								LOGI("CTableFrame::ConcludeGame PerformStandUpAction, (pAndroidService->dwResidualPlayDraw==0), NickName="<<pIServerUserItem->GetNickName());
								PerformStandUpAction(pIServerUserItem);
								continue;
							}
						}
					}
				}
			}
		}

		//ɾ��ʱ��
		if (bOffLineWait==true)
		{
			KillGameTimer(IDI_OFF_LINE);
		}
	}

	//֪ͨ����
	//if(m_pIGameMatchSink!=NULL) m_pIGameMatchSink->OnEventGameEnd(this,0, NULL, cbGameStatus);

	//��������
	ASSERT(m_pITableFrameSink!=NULL);
	if (m_pITableFrameSink!=NULL) m_pITableFrameSink->RepositionSink();

	//�߳����
	if (m_bGameStarted==false)
	{
		for (WORD i=0;i<m_wChairCount;i++)
		{
			//��ȡ�û�
			if (m_TableUserItemArray[i]==NULL) continue;
			IServerUserItem * pIServerUserItem=m_TableUserItemArray[i];

			//��������
			if ((m_pGameServiceOption->lMinEnterScore!=0L)&&(pIServerUserItem->GetUserScore()<m_pGameServiceOption->lMinEnterScore))
			{
				//������ʾ
				TCHAR szDescribe[128]=TEXT("");
				if (m_pGameServiceOption->wServerType&GAME_GENRE_GOLD)
				{
					_sntprintf(szDescribe,CountArray(szDescribe),TEXT("������Ϸ������ ") SCORE_STRING TEXT("�����ܼ�����Ϸ��"),m_pGameServiceOption->lMinEnterScore/100);
				}
				else
				{
					_sntprintf(szDescribe,CountArray(szDescribe),TEXT("������Ϸ�������� ") SCORE_STRING TEXT("�����ܼ�����Ϸ��"),m_pGameServiceOption->lMinEnterScore/100);
				}

				//������Ϣ(�����˷���С�����Ҫ���Ҳ���˳����䣬��Ϊ����ȡǮ������Ϸ)
				if (pIServerUserItem->IsAndroidUser()==true)
					SendGameMessage(pIServerUserItem,szDescribe,SMT_CHAT|SMT_CLOSE_GAME|SMT_EJECT);
				else
					SendGameMessage(pIServerUserItem,szDescribe,SMT_NOMONEY);

				//�û�����
				LOGI("CTableFrame::ConcludeGame PerformStandUpAction, "<<szDescribe<<", NickName="<<pIServerUserItem->GetNickName());
				PerformStandUpAction(pIServerUserItem);

				continue;
			}

			//�ر��ж�
			if ((CServerRule::IsForfendGameEnter(m_pGameServiceOption->dwServerRule)==true)&&(pIServerUserItem->GetMasterOrder()==0))
			{
				//������Ϣ
				LPCTSTR pszMessage=TEXT("����ϵͳά������ǰ��Ϸ���ӽ�ֹ�û�������Ϸ��");
				SendGameMessage(pIServerUserItem,pszMessage,SMT_EJECT|SMT_CHAT|SMT_CLOSE_GAME);

				//�û�����
				LOGI("CTableFrame::ConcludeGame PerformStandUpAction "<<pszMessage<<", NickName="<<pIServerUserItem->GetNickName());
				PerformStandUpAction(pIServerUserItem);

				continue;
			}		
			//ɾ��������
			/*for(int i=0;i<m_pGameServiceOption->kickoutplayer[0];i++)
			{
				if(m_pGameServiceOption->kickoutplayer[i+1] == pIServerUserItem->GetUserID())
				{					
					//�û�����				
					PerformStandUpAction(pIServerUserItem,0);		
					if(pIServerUserItem->IsAndroidUser())
					{
						IAndroidUserItem* iaui = m_pIAndroidUserManager->SearchAndroidUserItem(pIServerUserItem->GetUserID());
						if(iaui)
						{					
							CAttemperEngineSink * pAttemperEngineSink=(CAttemperEngineSink *)m_pIMainServiceFrame;
							tagBindParameter * pBindParameter=pAttemperEngineSink->GetBindParameter(pIServerUserItem->GetBindIndex());
							m_pIAndroidUserManager->DeleteAndroidUserItem(pBindParameter->dwSocketID);					
						}
					}
					m_pGameServiceOption->kickoutplayer[0]=m_pGameServiceOption->kickoutplayer[0]-1;
					m_pGameServiceOption->kickoutplayer[i+1]=0;
					return false;
				}	
			}*/
		}
	}
	//��������
	ConcludeTable();
	//����״̬
	SendTableStatus();

	return true;
}

std::string CTableFrame::GetNewSerailNumber(){
	CAttemperEngineSink * pAttemperEngineSink=(CAttemperEngineSink *)m_pIMainServiceFrame;
	m_strCurrentTableNum = pAttemperEngineSink->GetTableNum();
	return m_strCurrentTableNum;
}

//��������
bool CTableFrame::ConcludeTable()
{
	//LOGI(L"CTableFrame::ConcludeTable");

	//��������
	if ((m_bGameStarted==false)&&(m_bTableStarted==true))
	{
		//�����ж�
		WORD wTableUserCount=GetSitUserCount();
		if (wTableUserCount==0) m_bTableStarted=false;
		if (m_pGameServiceAttrib->wChairCount==MAX_CHAIR) m_bTableStarted=false;

		//ģʽ�ж�
		if (m_cbStartMode==START_MODE_FULL_READY) m_bTableStarted=false;
		if (m_cbStartMode==START_MODE_PAIR_READY) m_bTableStarted=false;
		if (m_cbStartMode==START_MODE_ALL_READY) m_bTableStarted=false;
	}

	return true;
}

//д�����
bool CTableFrame::WriteUserScore(WORD wChairID, tagScoreInfo & ScoreInfo, DWORD dwGameMemal, DWORD dwPlayGameTime, INT* _acsResult)
{

	//LOGI(L"CTableFrame::WriteUserScore");

	//Ч�����
	ASSERT((wChairID<m_wChairCount)&&(ScoreInfo.cbType!=SCORE_TYPE_NULL));
	if ((wChairID>=m_wChairCount)&&(ScoreInfo.cbType==SCORE_TYPE_NULL)) return false;

	//��ȡ�û�
	ASSERT(GetTableUserItem(wChairID)!=NULL);
	IServerUserItem * pIServerUserItem=GetTableUserItem(wChairID);
	TCHAR szMessage[128]=TEXT("");

	//д�����
	if (pIServerUserItem!=NULL)
	{
		//��������
		//DWORD dwUserMemal=0L;
		SCORE lRevenueScore=__min(m_lFrozenedScore[wChairID],m_pGameServiceOption->lServiceScore);
		//SCORE lRevenueScore=m_lFrozenedScore[wChairID];

		//�۷����
		if (m_pGameServiceOption->lServiceScore>0L 
			&& m_pGameServiceOption->wServerType == GAME_GENRE_GOLD
			&& m_pITableFrameSink->QueryBuckleServiceCharge(wChairID))
		{
			//�۷����
			ScoreInfo.lScore-=lRevenueScore;
			ScoreInfo.lRevenue+=lRevenueScore;

			//������Ϸ��
			pIServerUserItem->UnFrozenedUserScore(m_lFrozenedScore[wChairID]);
			m_lFrozenedScore[wChairID]=0L;
		}

		//���Ƽ���
		/*if(dwGameMemal != INVALID_DWORD)
		{
			dwUserMemal = dwGameMemal;
		}
		else if (ScoreInfo.lRevenue>0L)
		{
			WORD wMedalRate=m_pGameParameter->wMedalRate;
			dwUserMemal=(DWORD)(ScoreInfo.lRevenue*wMedalRate/1000L);
		}*/

		//��Ϸʱ��
		DWORD dwCurrentTime=(DWORD)time(NULL);
		DWORD dwPlayTimeCount=((m_bDrawStarted==true)?(dwCurrentTime-m_dwDrawStartTime):0L);
		if(dwPlayGameTime!=INVALID_DWORD) dwPlayTimeCount=dwPlayGameTime;

		//��������
		//tagUserProperty * pUserProperty=pIServerUserItem->GetUserProperty();

		//�����ж�
		//if(m_pGameServiceOption->wServerType == GAME_GENRE_SCORE)
		//{
		//	if (ScoreInfo.lScore>0L)
		//	{
		//		//�ı�����
		//		if ((pUserProperty->wPropertyUseMark&PT_USE_MARK_FOURE_SCORE)!=0)
		//		{
		//			//��������
		//			DWORD dwValidTime=pUserProperty->PropertyInfo[1].wPropertyCount*pUserProperty->PropertyInfo[1].dwValidNum;
		//			if(pUserProperty->PropertyInfo[1].dwEffectTime+dwValidTime>dwCurrentTime)
		//			{
		//				//���ַ���
		//				ScoreInfo.lScore *= 4;
		//				_sntprintf(szMessage,CountArray(szMessage),TEXT("[ %s ] ʹ����[ �ı����ֿ� ]���÷ַ��ı���)"),pIServerUserItem->GetNickName());
		//			}
		//			else
		//			{
		//				pUserProperty->wPropertyUseMark&=~PT_USE_MARK_FOURE_SCORE;
		//			}
		//		} //˫������
		//		else if ((pUserProperty->wPropertyUseMark&PT_USE_MARK_DOUBLE_SCORE)!=0)
		//		{
		//			//��������
		//			DWORD dwValidTime=pUserProperty->PropertyInfo[0].wPropertyCount*pUserProperty->PropertyInfo[0].dwValidNum;
		//			if (pUserProperty->PropertyInfo[0].dwEffectTime+dwValidTime>dwCurrentTime)
		//			{
		//				//���ַ���
		//				ScoreInfo.lScore*=2L;
		//				_sntprintf(szMessage,CountArray(szMessage),TEXT("[ %s ] ʹ����[ ˫�����ֿ� ]���÷ַ�����"), pIServerUserItem->GetNickName());
		//			}
		//			else
		//			{
		//				pUserProperty->wPropertyUseMark&=~PT_USE_MARK_DOUBLE_SCORE;
		//			}
		//		}
		//	}
		//	else
		//	{
		//		//������
		//		if ((pUserProperty->wPropertyUseMark&PT_USE_MARK_POSSESS)!=0)
		//		{
		//			//��������
		//			DWORD dwValidTime=pUserProperty->PropertyInfo[3].wPropertyCount*pUserProperty->PropertyInfo[3].dwValidNum;
		//			if(pUserProperty->PropertyInfo[3].dwEffectTime+dwValidTime>dwCurrentTime)
		//			{
		//				//���ַ���
		//				ScoreInfo.lScore = 0;
		//				_sntprintf(szMessage,CountArray(szMessage),TEXT("[ %s ] ʹ����[ �������� ]�����ֲ��䣡"),pIServerUserItem->GetNickName());
		//			}
		//			else
		//			{
		//				pUserProperty->wPropertyUseMark &= ~PT_USE_MARK_POSSESS;
		//			}
		//		}
		//	}
		//}

		LOGI("----- wirte score table number: " << m_strCurrentTableNum );
		/*pIServerUserItem->WriteUserScore(ScoreInfo.lScore, ScoreInfo.lGrade, ScoreInfo.lRevenue, 
			dwUserMemal, ScoreInfo.cbType, dwPlayTimeCount, m_strCurrentTableNum, _acsResult);*/
		pIServerUserItem->WriteUserScore(ScoreInfo.lScore, ScoreInfo.lGrade, ScoreInfo.lRevenue, 
			0, ScoreInfo.cbType, dwPlayTimeCount, m_strCurrentTableNum, _acsResult);

		//��Ϸ��¼
//		LOGI("CTableFrame::WriteUserScore ������Ϸ��¼");
		//if (pIServerUserItem->IsAndroidUser()==false && CServerRule::IsRecordGameScore(m_pGameServiceOption->dwServerRule)==true)
		//{
		//	//��������
		//	tagGameScoreRecord * pGameScoreRecord=NULL;

		//	//��ѯ���
		//	if (m_GameScoreRecordBuffer.GetCount()>0L)
		//	{
		//		//��ȡ����
		//		INT_PTR nCount=m_GameScoreRecordBuffer.GetCount();
		//		pGameScoreRecord=m_GameScoreRecordBuffer[nCount-1];

		//		//ɾ������
		//		m_GameScoreRecordBuffer.RemoveAt(nCount-1);
		//	}

		//	//��������
		//	if (pGameScoreRecord==NULL)
		//	{
		//		try
		//		{
		//			//��������
		//			pGameScoreRecord=new tagGameScoreRecord;
		//			if (pGameScoreRecord==NULL) throw TEXT("��Ϸ��¼���󴴽�ʧ��");
		//		}
		//		catch (...)
		//		{
		//			ASSERT(FALSE);
		//		}
		//	}

		//	//��¼����
		//	if (pGameScoreRecord!=NULL)
		//	{
		//		//�û���Ϣ
		//		pGameScoreRecord->wChairID=wChairID;
		//		pGameScoreRecord->dwUserID=pIServerUserItem->GetUserID();
		//		pGameScoreRecord->cbAndroid=(pIServerUserItem->IsAndroidUser()?TRUE:FALSE);

		//		//�û���Ϣ
		//		pGameScoreRecord->dwDBQuestID=pIServerUserItem->GetDBQuestID();
		//		pGameScoreRecord->dwInoutIndex=pIServerUserItem->GetInoutIndex();

		//		//��Ϸ����Ϣ
		//		pGameScoreRecord->lScore=ScoreInfo.lScore;
		//		pGameScoreRecord->lGrade=ScoreInfo.lGrade;
		//		pGameScoreRecord->lRevenue=ScoreInfo.lRevenue;

		//		//������Ϣ
		//		//pGameScoreRecord->dwUserMemal=dwUserMemal;
		//		pGameScoreRecord->dwPlayTimeCount=dwPlayTimeCount;

		//		//��������˰
		//		if(pIServerUserItem->IsAndroidUser())
		//		{
		//			pGameScoreRecord->lScore += pGameScoreRecord->lRevenue;
		//			pGameScoreRecord->lRevenue = 0L;
		//		}

		//		//��������
		//		m_GameScoreRecordActive.Add(pGameScoreRecord);
		//	}
		//}

		//��Ϸ��¼
//		LOGI("CTableFrame::WriteUserScore ��¼��Ϸ��¼");
		/*if(dwGameMemal != INVALID_DWORD || dwPlayGameTime!=INVALID_DWORD)
		{
			RecordGameScore(true);
		}*/
	}
	else
	{
		//�뿪�û�
		CTraceService::TraceString(TEXT("Genre Score Not Support!"),TraceLevel_Exception);

		return false;
	}

	//�㲥��Ϣ
//	LOGI("CTableFrame::WriteUserScore ���͹㲥��Ϣ");
	if (szMessage[0]!=0)
	{
		//��������
		IServerUserItem * pISendUserItem = NULL;
		WORD wEnumIndex=0;

		//��Ϸ���
		for (WORD i=0;i<m_wChairCount;i++)
		{
			//��ȡ�û�
			pISendUserItem=GetTableUserItem(i);
			if(pISendUserItem==NULL) continue;

			//������Ϣ
			SendGameMessage(pISendUserItem, szMessage, SMT_CHAT);
		}

		//�Թ��û�
		//do
		//{
		//	pISendUserItem=EnumLookonUserItem(wEnumIndex++);
		//	if(pISendUserItem!=NULL) 
		//	{
		//		//������Ϣ
		//		SendGameMessage(pISendUserItem, szMessage, SMT_CHAT);
		//	}
		//} while (pISendUserItem!=NULL);
	}
//	LOGI("CTableFrame::WriteUserScore ���͹㲥��Ϣ����");

	return true;
}

//д�����
bool CTableFrame::WriteTableScore(tagScoreInfo ScoreInfoArray[], WORD wScoreCount)
{
	//LOGI(L"CTableFrame::WriteTableScore");

	//Ч�����
	ASSERT(wScoreCount==m_wChairCount);
	if (wScoreCount!=m_wChairCount) return false;

	//д�����
	for (WORD i=0;i<m_wChairCount;i++)
	{
		if (ScoreInfoArray[i].cbType!=SCORE_TYPE_NULL)
		{
			WriteUserScore(i,ScoreInfoArray[i]);
		}
	}

	return true;
}

//����˰��
SCORE CTableFrame::CalculateRevenue(WORD wChairID, SCORE lScore)
{
	//LOGI(L"CTableFrame::CalculateRevenue");

	//Ч�����
	ASSERT(wChairID<m_wChairCount);
	if (wChairID>=m_wChairCount) return 0L;

	//����˰��
	if ((m_pGameServiceOption->wRevenueRatio>0)&&(lScore>=REVENUE_BENCHMARK))
	{
		//��ȡ�û�
		ASSERT(GetTableUserItem(wChairID)!=NULL);
		IServerUserItem * pIServerUserItem=GetTableUserItem(wChairID);

		//����˰��
		SCORE lRevenue=lScore*m_pGameServiceOption->wRevenueRatio/REVENUE_DENOMINATOR;

		return lRevenue;
	}

	return 0L;
}

//�����޶�
SCORE CTableFrame::QueryConsumeQuota(IServerUserItem * pIServerUserItem)
{
	//LOGI(L"CTableFrame::QueryConsumeQuota");

	//�û�Ч��
	ASSERT(pIServerUserItem->GetTableID()==m_wTableID);
	if (pIServerUserItem->GetTableID()!=m_wTableID) return 0L;

	//��ѯ���
	SCORE lTrusteeScore=pIServerUserItem->GetTrusteeScore();
	SCORE lMinEnterScore=m_pGameServiceOption->lMinTableScore;
	SCORE lUserConsumeQuota=m_pITableFrameSink->QueryConsumeQuota(pIServerUserItem);

	//Ч����
	ASSERT((lUserConsumeQuota>=0L)&&(lUserConsumeQuota<=pIServerUserItem->GetUserScore()-lMinEnterScore));
	if ((lUserConsumeQuota<0L)||(lUserConsumeQuota>pIServerUserItem->GetUserScore()-lMinEnterScore)) return 0L;

	return lUserConsumeQuota+lTrusteeScore;
}

//Ѱ���û�
IServerUserItem * CTableFrame::SearchUserItem(DWORD dwUserID)
{
	//LOGI(L"CTableFrame::SearchUserItem");

	//��������
	WORD wEnumIndex=0;
	IServerUserItem * pIServerUserItem=NULL;

	//�����û�
	for (WORD i=0;i<m_wChairCount;i++)
	{
		pIServerUserItem=GetTableUserItem(i);
		if ((pIServerUserItem!=NULL)&&(pIServerUserItem->GetUserID()==dwUserID)) return pIServerUserItem;
	}

	//�Թ��û�
	/*do
	{
		pIServerUserItem=EnumLookonUserItem(wEnumIndex++);
		if ((pIServerUserItem!=NULL)&&(pIServerUserItem->GetUserID()==dwUserID)) return pIServerUserItem;
	} while (pIServerUserItem!=NULL);*/

	return NULL;
}

//��Ϸ�û�
IServerUserItem * CTableFrame::GetTableUserItem(WORD wChairID)
{
	//LOGI(L"CTableFrame::GetTableUserItem");

	//Ч�����
	ASSERT(wChairID<m_wChairCount);
	if (wChairID>=m_wChairCount) return NULL;

	//��ȡ�û�
	return m_TableUserItemArray[wChairID];
}

//�Թ��û�
//IServerUserItem * CTableFrame::EnumLookonUserItem(WORD wEnumIndex)
//{
//	//LOGI(L"CTableFrame::EnumLookonUserItem");
//
//	if (wEnumIndex>=m_LookonUserItemArray.GetCount()) return NULL;
//	return m_LookonUserItemArray[wEnumIndex];
//}

//����ʱ��
bool CTableFrame::SetGameTimer(DWORD dwTimerID, DWORD dwElapse, DWORD dwRepeat, WPARAM dwBindParameter)
{
	//LOGI(L"CTableFrame::SetGameTimer");

	//Ч�����
	ASSERT((dwTimerID>0)&&(dwTimerID<TIME_TABLE_MODULE_RANGE));
	if ((dwTimerID<=0)||(dwTimerID>=TIME_TABLE_MODULE_RANGE)) return false;

	//����ʱ��
	DWORD dwEngineTimerID=IDI_TABLE_MODULE_START+m_wTableID*TIME_TABLE_MODULE_RANGE;
	if (m_pITimerEngine!=NULL) m_pITimerEngine->SetTimer(dwEngineTimerID+dwTimerID,dwElapse,dwRepeat,dwBindParameter);

	return true;
}

//ɾ��ʱ��
bool CTableFrame::KillGameTimer(DWORD dwTimerID)
{
	//LOGI(L"CTableFrame::KillGameTimer");

	//Ч�����
	ASSERT((dwTimerID>0)&&(dwTimerID<=TIME_TABLE_MODULE_RANGE));
	if ((dwTimerID<=0)||(dwTimerID>TIME_TABLE_MODULE_RANGE)) return false;

	//ɾ��ʱ��
	DWORD dwEngineTimerID=IDI_TABLE_MODULE_START+m_wTableID*TIME_TABLE_MODULE_RANGE;
	if (m_pITimerEngine!=NULL)
	{
		m_pITimerEngine->KillTimer(dwEngineTimerID+dwTimerID);
	}
	return true;
}

//��������
bool CTableFrame::SendUserItemData(IServerUserItem * pIServerUserItem, WORD wSubCmdID)
{
	//LOGI(L"CTableFrame::SendUserItemData");

	//״̬Ч��
	ASSERT((pIServerUserItem!=NULL)&&(pIServerUserItem->IsClientReady()==true));
	if ((pIServerUserItem==NULL)&&(pIServerUserItem->IsClientReady()==false)) return false;

	//��������
	m_pIMainServiceFrame->SendData(pIServerUserItem,MDM_GF_GAME,wSubCmdID,NULL,0);

	return true;
}

//��������
bool CTableFrame::SendUserItemData(IServerUserItem * pIServerUserItem, WORD wSubCmdID, VOID * pData, WORD wDataSize)
{
	//LOGI(L"CTableFrame::SendUserItemData");

	//״̬Ч��
	ASSERT((pIServerUserItem!=NULL)&&(pIServerUserItem->IsClientReady()==true));
	if ((pIServerUserItem==NULL)&&(pIServerUserItem->IsClientReady()==false)) return false;

	//��������
	m_pIMainServiceFrame->SendData(pIServerUserItem,MDM_GF_GAME,wSubCmdID,pData,wDataSize);

	return true;
}

//��������
bool CTableFrame::SendTableData(WORD wChairID, WORD wSubCmdID)
{
	//LOGI(L"CTableFrame::SendTableData");

	//�û�Ⱥ��
	if (wChairID==INVALID_CHAIR)
	{
		for (WORD i=0;i<m_wChairCount;i++)
		{
			//��ȡ�û�
			IServerUserItem * pIServerUserItem=GetTableUserItem(i);
			if (pIServerUserItem==NULL) continue;

			//Ч��״̬
			ASSERT(pIServerUserItem->IsClientReady()==true);
			if (pIServerUserItem->IsClientReady()==false) continue;

			//��������
			m_pIMainServiceFrame->SendData(pIServerUserItem,MDM_GF_GAME,wSubCmdID,NULL,0);
		}

		return true;
	}
	else
	{
		//��ȡ�û�
		IServerUserItem * pIServerUserItem=GetTableUserItem(wChairID);
		if (pIServerUserItem==NULL) return false;

		//Ч��״̬
// 		ASSERT(pIServerUserItem->IsClientReady()==true);
// 		if (pIServerUserItem->IsClientReady()==false) return false;

		//��������
		m_pIMainServiceFrame->SendData(pIServerUserItem,MDM_GF_GAME,wSubCmdID,NULL,0);

		return true;
	}

	return false;
}

//��������
bool CTableFrame::SendTableData(WORD wChairID, WORD wSubCmdID, VOID * pData, WORD wDataSize,WORD wMainCmdID)
{

	//LOGI(L"CTableFrame::SendTableData");
	
	//�û�Ⱥ��
	if (wChairID==INVALID_CHAIR)
	{

		for (WORD i=0;i<m_wChairCount;i++)
		{
			//��ȡ�û�
			IServerUserItem * pIServerUserItem=GetTableUserItem(i);

			if ((pIServerUserItem==NULL)||(pIServerUserItem->IsClientReady()==false)) continue;
			if (pIServerUserItem == NULL) continue;

			//��������
			m_pIMainServiceFrame->SendData(pIServerUserItem,wMainCmdID,wSubCmdID,pData,wDataSize);
		}
		return true;
	}else{
		//��ȡ�û�
		IServerUserItem * pIServerUserItem=GetTableUserItem(wChairID);
		//if ((pIServerUserItem==NULL)||(pIServerUserItem->IsClientReady()==false)) return false;

		//��������
		m_pIMainServiceFrame->SendData(pIServerUserItem,wMainCmdID,wSubCmdID,pData,wDataSize);
		return true;
	}
	return false;
}

//��������
//bool CTableFrame::SendLookonData(WORD wChairID, WORD wSubCmdID)
//{
//
//	//LOGI(L"CTableFrame::SendLookonData");
//
//	//��������
//	WORD wEnumIndex=0;
//	IServerUserItem * pIServerUserItem=NULL;
//
//	//ö���û�
//	do
//	{
//		//��ȡ�û�
//		pIServerUserItem=EnumLookonUserItem(wEnumIndex++);
//		if (pIServerUserItem==NULL) break;
//
//		//Ч��״̬
//		ASSERT(pIServerUserItem->IsClientReady()==true);
//		if (pIServerUserItem->IsClientReady()==false) return false;
//
//		//��������
//		if ((wChairID==INVALID_CHAIR)||(pIServerUserItem->GetChairID()==wChairID))
//		{
//			m_pIMainServiceFrame->SendData(pIServerUserItem,MDM_GF_GAME,wSubCmdID,NULL,0);
//		}
//
//	} while (true);
//
//	return true;
//}
//
////��������
//bool CTableFrame::SendLookonData(WORD wChairID, WORD wSubCmdID, VOID * pData, WORD wDataSize)
//{
//
//	//LOGI(L"CTableFrame::SendLookonData");
//
//	//��������
//	WORD wEnumIndex=0;
//	IServerUserItem * pIServerUserItem=NULL;
//
//	//ö���û�
//	do
//	{
//		//��ȡ�û�
//		pIServerUserItem=EnumLookonUserItem(wEnumIndex++);
//		if (pIServerUserItem==NULL) break;
//
//		//Ч��״̬
//		ASSERT(pIServerUserItem->IsClientReady()==true);
//		if (pIServerUserItem->IsClientReady()==false) return false;
//
//		//��������
//		if ((wChairID==INVALID_CHAIR)||(pIServerUserItem->GetChairID()==wChairID))
//		{
//			m_pIMainServiceFrame->SendData(pIServerUserItem,MDM_GF_GAME,wSubCmdID,pData,wDataSize);
//		}
//
//	} while (true);
//
//	return true;
//}

//������Ϣ
bool CTableFrame::SendGameMessage(LPCTSTR lpszMessage, WORD wType)
{

	//LOGI(L"CTableFrame::SendGameMessage");

	//��������
	WORD wEnumIndex=0;

	//������Ϣ
	for (WORD i=0;i<m_wChairCount;i++)
	{
		//��ȡ�û�
		IServerUserItem * pIServerUserItem=GetTableUserItem(i);
		if ((pIServerUserItem==NULL)||(pIServerUserItem->IsClientReady()==false)) continue;

		//������Ϣ
		m_pIMainServiceFrame->SendGameMessage(pIServerUserItem,lpszMessage,wType);
	}

	//ö���û�
	//do
	//{
	//	//��ȡ�û�
	//	IServerUserItem * pIServerUserItem=EnumLookonUserItem(wEnumIndex++);
	//	if (pIServerUserItem==NULL) break;

	//	//Ч��״̬
	//	ASSERT(pIServerUserItem->IsClientReady()==true);
	//	if (pIServerUserItem->IsClientReady()==false) return false;

	//	//������Ϣ
	//	m_pIMainServiceFrame->SendGameMessage(pIServerUserItem,lpszMessage,wType);

	//} while (true);

	return true;
}

//������Ϣ
bool CTableFrame::SendRoomMessage(IServerUserItem * pIServerUserItem, LPCTSTR lpszMessage, WORD wType)
{
	//LOGI(L"CTableFrame::SendRoomMessage");

	//�û�Ч��
	ASSERT(pIServerUserItem!=NULL);
	if (pIServerUserItem==NULL) return false;

	//������Ϣ
	m_pIMainServiceFrame->SendRoomMessage(pIServerUserItem,lpszMessage,wType);

	return true;
}

//��Ϸ��Ϣ
bool CTableFrame::SendGameMessage(IServerUserItem * pIServerUserItem, LPCTSTR lpszMessage, WORD wType)
{
	//LOGI(L"CTableFrame::SendGameMessage");

	//�û�Ч��
	ASSERT(pIServerUserItem!=NULL);
	if (pIServerUserItem==NULL) return false;

	//������Ϣ
	return m_pIMainServiceFrame->SendGameMessage(pIServerUserItem,lpszMessage,wType);
}

//����ϵͳ��Ϣ
bool CTableFrame::SendSystemMessage(LPCTSTR lpszMessage, WORD wType)
{
	//LOGI(L"CTableFrame::SendSystemMessage");
	return m_pIMainServiceFrame->SendSystemMessage(lpszMessage, wType);
}

//���ͳ���
bool CTableFrame::SendGameScene(IServerUserItem * pIServerUserItem, VOID * pData, WORD wDataSize)
{
	//LOGI(L"CTableFrame::SendGameScene");

	//�û�Ч��
	ASSERT((pIServerUserItem!=NULL)&&(pIServerUserItem->IsClientReady()==true));
	if ((pIServerUserItem==NULL)||(pIServerUserItem->IsClientReady()==false)) return false;

	//���ͳ���
	ASSERT(m_pIMainServiceFrame!=NULL);
	m_pIMainServiceFrame->SendData(pIServerUserItem,MDM_GF_FRAME,SUB_GF_GAME_SCENE,pData,wDataSize);

	return true;
}

//�����¼�
bool CTableFrame::OnEventUserOffLine(IServerUserItem * pIServerUserItem)
{
	//LOGI(L"CTableFrame::OnEventUserOffLine");

	//����Ч��
	ASSERT(pIServerUserItem!=NULL);
	if (pIServerUserItem==NULL) return false;

	//�û�����
//	tagUserInfo * pUserInfo=pIServerUserItem->GetUserInfo();
//	IServerUserItem * pITableUserItem=m_TableUserItemArray[pUserInfo->wChairID];

	//�û�����
	WORD wChairID=pIServerUserItem->GetChairID();
	BYTE cbUserStatus=pIServerUserItem->GetUserStatus();

	//��Ϸ�û�
	if (cbUserStatus!=US_LOOKON)
	{
		//Ч���û�
		ASSERT(pIServerUserItem==GetTableUserItem(wChairID));
		if (pIServerUserItem!=GetTableUserItem(wChairID)) return false;
		//if(m_pIGameMatchSink)m_pIGameMatchSink->SetUserOffline(wChairID,true);

		//���ߴ���
		if (cbUserStatus==US_PLAYING)//&&(m_wOffLineCount[wChairID]<MAX_OFF_LINE))
		{
			//�û�����
			pIServerUserItem->SetClientReady(false);
			pIServerUserItem->SetUserStatus(US_OFFLINE,m_wTableID,wChairID);

			if(m_pITableFrameSink)
				m_pITableFrameSink->OnUserStatusChange(wChairID, US_OFFLINE);

			//���ߴ���
			if (m_dwOffLineTime[wChairID]==0L)
			{
				//���ñ���
				m_wOffLineCount[wChairID]++;
				m_dwOffLineTime[wChairID]=(DWORD)time(NULL);

				//ʱ������
				WORD wOffLineCount=GetOffLineUserCount();
// 				if (wOffLineCount==1)
// 				{
// 					SetGameTimer(IDI_OFF_LINE,TIME_OFF_LINE,1,wChairID);
// 				}
			}

			return true;
		}
	}

	//�û�����
	PerformStandUpAction(pIServerUserItem);

	//ɾ���û�
	ASSERT(pIServerUserItem->GetUserStatus()==US_FREE);
	pIServerUserItem->SetUserStatus(US_NULL,INVALID_TABLE,INVALID_CHAIR);

	return true;
}

//�����¼�
bool CTableFrame::OnUserScroeNotify(WORD wChairID, IServerUserItem * pIServerUserItem, BYTE cbReason)
{
	//LOGI(L"CTableFrame::OnUserScroeNotify");

	//֪ͨ��Ϸ
	return m_pITableFrameSink->OnUserScroeNotify(wChairID,pIServerUserItem,cbReason);
}

//ʱ���¼�
bool CTableFrame::OnEventTimer(DWORD dwTimerID, WPARAM dwBindParameter)
{
	//LOGI(L"CTableFrame::OnEventTimer");

	//�ص��¼�
	if ((dwTimerID>=0)&&(dwTimerID<TIME_TABLE_SINK_RANGE))
	{
		ASSERT(m_pITableFrameSink!=NULL);
		return m_pITableFrameSink->OnTimerMessage(dwTimerID,dwBindParameter);
	}

	//�¼�����
	switch (dwTimerID)
	{
	case IDI_OFF_LINE:	//�����¼�
		{

			//Ч��״̬
			ASSERT(m_bGameStarted==true);
			if (m_bGameStarted==false) return false;

			//��������
			DWORD dwOffLineTime=0L;
			WORD wOffLineChair=INVALID_CHAIR;

			//Ѱ���û�
			for (WORD i=0;i<m_wChairCount;i++)
			{
				if ((m_dwOffLineTime[i]!=0L)&&((m_dwOffLineTime[i]<dwOffLineTime)||(wOffLineChair==INVALID_CHAIR)))
				{
					wOffLineChair=i;
					dwOffLineTime=m_dwOffLineTime[i];
				}
			}

			LOGI("=====%d======" << wOffLineChair);
			//λ���ж�
			ASSERT(wOffLineChair!=INVALID_CHAIR);
			if (wOffLineChair==INVALID_CHAIR) return false;

			//�û��ж�
			ASSERT(dwBindParameter<m_wChairCount);
			if (wOffLineChair!=(WORD)dwBindParameter)
			{ 
				//ʱ�����
				DWORD dwCurrentTime=(DWORD)time(NULL);
				DWORD dwLapseTime=dwCurrentTime-m_dwOffLineTime[wOffLineChair];

				//����ʱ��
				dwLapseTime=__min(dwLapseTime,TIME_OFF_LINE-2000L);
				SetGameTimer(IDI_OFF_LINE,TIME_OFF_LINE-dwLapseTime,1,wOffLineChair);

				return true;
			}

			//��ȡ�û�
			ASSERT(GetTableUserItem(wOffLineChair)!=NULL);
			IServerUserItem * pIServerUserItem=GetTableUserItem(wOffLineChair);

			//������Ϸ
			if (pIServerUserItem!=NULL)
			{
				//���ñ���
				m_dwOffLineTime[wOffLineChair]=0L;

				//�û�����
				LOGI("CTableFrame::OnEventTimer PerformStandUpAction IDI_OFF_LINE, NickName="<<pIServerUserItem->GetNickName());
				PerformStandUpAction(pIServerUserItem);
			}

			//����ʱ��
			if (m_bGameStarted==true)
			{
				//��������
				DWORD dwOffLineTime=0L;
				WORD wOffLineChair=INVALID_CHAIR;

				//Ѱ���û�
				for (WORD i=0;i<m_wChairCount;i++)
				{
					if ((m_dwOffLineTime[i]!=0L)&&((m_dwOffLineTime[i]<dwOffLineTime)||(wOffLineChair==INVALID_CHAIR)))
					{
						wOffLineChair=i;
						dwOffLineTime=m_dwOffLineTime[i];
					}
				}

				//����ʱ��
				if (wOffLineChair!=INVALID_CHAIR)
				{
					//ʱ�����
					DWORD dwCurrentTime=(DWORD)time(NULL);
					DWORD dwLapseTime=dwCurrentTime-m_dwOffLineTime[wOffLineChair];

					//����ʱ��
					dwLapseTime=__min(dwLapseTime,TIME_OFF_LINE-2000L);
					SetGameTimer(IDI_OFF_LINE,TIME_OFF_LINE-dwLapseTime,1,wOffLineChair);
				}
			}

			return true;
		}
	case IDI_DISTRIBUTE_START:
		{
			KillGameTimer(IDI_DISTRIBUTE_START);
			StartGame();

			return true;
		}break;


	}

	//�������
	ASSERT(FALSE);

	return false;
}

//��Ϸ�¼�
bool CTableFrame::OnEventSocketGame(WORD wSubCmdID, VOID * pData, WORD wDataSize, IServerUserItem * pIServerUserItem)
{
	//LOGI(L"CTableFrame::OnEventSocketGame");

	//Ч�����
	ASSERT(pIServerUserItem!=NULL);
	ASSERT(m_pITableFrameSink!=NULL);

	//��Ϣ����
	return m_pITableFrameSink->OnGameMessage(wSubCmdID,pData,wDataSize,pIServerUserItem);
}

//����¼�
bool CTableFrame::OnEventSocketFrame(WORD wSubCmdID, VOID * pData, WORD wDataSize, IServerUserItem * pIServerUserItem)
{
	//LOGI(L"CTableFrame::OnEventSocketFrame");

	//��Ϸ����
	if (m_pITableFrameSink->OnFrameMessage(wSubCmdID,pData,wDataSize,pIServerUserItem)==true) return true;

	//��������
	//if(m_pIGameMatchSink!=NULL && m_pIGameMatchSink->OnFrameMessage(wSubCmdID,pData,wDataSize,pIServerUserItem)==true) return true;

	//Ĭ�ϴ���
	switch (wSubCmdID)
	{
	case SUB_GF_GAME_OPTION:	//��Ϸ����
		{
			//Ч�����
			ASSERT(wDataSize==sizeof(CMD_GF_GameOption));
			if (wDataSize!=sizeof(CMD_GF_GameOption)) return false;

			//��������
			CMD_GF_GameOption * pGameOption=(CMD_GF_GameOption *)pData;

			//��ȡ����
			WORD wChairID=pIServerUserItem->GetChairID();
			BYTE cbUserStatus=pIServerUserItem->GetUserStatus();

			//��������
			if ((cbUserStatus!=US_LOOKON)&&((m_dwOffLineTime[wChairID]!=0L)))
			{
				//���ñ���
				m_dwOffLineTime[wChairID]=0L;

				//ɾ��ʱ��
				WORD wOffLineCount=GetOffLineUserCount();
				if (wOffLineCount==0)
				{
					KillGameTimer(IDI_OFF_LINE);
				}
			}

			//����״̬
			pIServerUserItem->SetClientReady(true);
			//if (cbUserStatus!=US_LOOKON) m_bAllowLookon[wChairID]=pGameOption->cbAllowLookon?true:false;

			//����״̬
			CMD_GF_GameStatus GameStatus;
			GameStatus.cbGameStatus=m_cbGameStatus;
			//GameStatus.cbAllowLookon=m_bAllowLookon[wChairID]?TRUE:FALSE;
			GameStatus.cbAllowLookon=FALSE;
			m_pIMainServiceFrame->SendData(pIServerUserItem,MDM_GF_FRAME,SUB_GF_GAME_STATUS,&GameStatus,sizeof(GameStatus));

			TCHAR welcomMsg[128] = TEXT("");
			_sntprintf(welcomMsg,sizeof(welcomMsg),TEXT("��ӭ�����롰%s-%s����Ϸ��ף����Ϸ��죡"),m_pGameServiceAttrib->szGameName,m_pGameServiceOption->szRoomName.c_str());
			LOGI("Welcome you to this game " << m_pGameServiceOption->szRoomType);
			m_pIMainServiceFrame->SendGameMessage(pIServerUserItem,welcomMsg,SMT_CHAT);

			//���ͳ���
			bool bSendSecret=(cbUserStatus!=US_LOOKON);
			m_pITableFrameSink->OnEventSendGameScene(wChairID,pIServerUserItem,m_cbGameStatus,bSendSecret);

			//��ʼ�ж�
			if ((cbUserStatus==US_READY)&&(EfficacyStartGame(wChairID)==true))
			{
                StartGame();
			}

			return true;
		}
	case SUB_GF_USER_READY:		//�û�׼��
		{
			//��ȡ����
			WORD wChairID=pIServerUserItem->GetChairID();
			BYTE cbUserStatus=pIServerUserItem->GetUserStatus();

			//Ч��״̬
			ASSERT(GetTableUserItem(wChairID)==pIServerUserItem);
			if (GetTableUserItem(wChairID)!=pIServerUserItem) return false;

			//Ч��״̬
			//ASSERT(cbUserStatus==US_SIT);
			if (cbUserStatus!=US_SIT) return true;

			//�����׷����ж�
// 			if((CServerRule::IsAllowAvertCheatMode(m_pGameServiceOption->dwServerRule)&&(m_pGameServiceAttrib->wChairCount < MAX_CHAIR))
// 				&& (m_wDrawCount >= m_pGameServiceOption->wDistributeDrawCount || CheckDistribute()))
// 			{
// 				//������Ϣ
// 				LPCTSTR pszMessage=TEXT("ϵͳ���·������ӣ����Ժ�");
// 				SendGameMessage(pIServerUserItem,pszMessage,SMT_CHAT);
// 
// 				//������Ϣ
// 				m_pIMainServiceFrame->InsertWaitDistribute(pIServerUserItem);
// 
// 				//�û�����
// 				LOGI("CTableFrame::OnEventTimer PerformStandUpAction SUB_GF_USER_READY, NickName="<<pIServerUserItem->GetNickName());
// 				PerformStandUpAction(pIServerUserItem);
// 
// 				return true;
// 			}

			//�¼�֪ͨ
			if (m_pITableUserAction!=NULL)
			{
				m_pITableUserAction->OnActionUserOnReady(wChairID,pIServerUserItem,pData,wDataSize);
			}

			//�¼�֪ͨ
			/*if(m_pIMatchTableAction!=NULL && !m_pIMatchTableAction->OnActionUserOnReady(wChairID,pIServerUserItem, pData,wDataSize))
				return true;*/

			//��ʼ�ж�
			if (EfficacyStartGame(wChairID)==false)
			{
				pIServerUserItem->SetUserStatus(US_READY,m_wTableID,wChairID);
			}
			else
			{
				LOGI("user ready to start");
				StartGame(); 
			}

			return true;
		}
	//case SUB_GF_USER_CHAT:		//�û�����
	//	{
	//		//��������
	//		CMD_GF_C_UserChat * pUserChat=(CMD_GF_C_UserChat *)pData;

	//		//Ч�����
	//		ASSERT(wDataSize<=sizeof(CMD_GF_C_UserChat));
	//		ASSERT(wDataSize>=(sizeof(CMD_GF_C_UserChat)-sizeof(pUserChat->szChatString)));
	//		ASSERT(wDataSize==(sizeof(CMD_GF_C_UserChat)-sizeof(pUserChat->szChatString)+pUserChat->wChatLength*sizeof(pUserChat->szChatString[0])));

	//		//Ч�����
	//		if (wDataSize>sizeof(CMD_GF_C_UserChat)) return false;
	//		if (wDataSize<(sizeof(CMD_GF_C_UserChat)-sizeof(pUserChat->szChatString))) return false;
	//		if (wDataSize!=(sizeof(CMD_GF_C_UserChat)-sizeof(pUserChat->szChatString)+pUserChat->wChatLength*sizeof(pUserChat->szChatString[0]))) return false;

	//		//Ŀ���û�
	//		if ((pUserChat->dwTargetUserID!=0)&&(SearchUserItem(pUserChat->dwTargetUserID)==NULL))
	//		{
	//			ASSERT(FALSE);
	//			return true;
	//		}

	//		//״̬�ж�
	//		if ((CServerRule::IsForfendGameChat(m_pGameServiceOption->dwServerRule)==true)&&(pIServerUserItem->GetMasterOrder()==0L))
	//		{
	//			SendGameMessage(pIServerUserItem,TEXT("��Ǹ����ǰ��Ϸ�����ֹ��Ϸ���죡"),SMT_CHAT);
	//			return true;
	//		}

	//		//Ȩ���ж�
	//		if (CUserRight::CanRoomChat(pIServerUserItem->GetUserRight())==false)
	//		{
	//			SendGameMessage(pIServerUserItem,TEXT("��Ǹ����û����Ϸ�����Ȩ�ޣ�����Ҫ����������ϵ��Ϸ�ͷ���ѯ��"),SMT_EJECT|SMT_CHAT);
	//			return true;
	//		}

	//		//������Ϣ
	//		CMD_GF_S_UserChat UserChat;
	//		ZeroMemory(&UserChat,sizeof(UserChat));

	//		//�ַ�����
	//		//m_pIMainServiceFrame->SensitiveWordFilter(pUserChat->szChatString,UserChat.szChatString,CountArray(UserChat.szChatString));

	//		//��������
	//		UserChat.dwChatColor=pUserChat->dwChatColor;
	//		UserChat.wChatLength=pUserChat->wChatLength;
	//		UserChat.dwTargetUserID=pUserChat->dwTargetUserID;
	//		UserChat.dwSendUserID=pIServerUserItem->GetUserID();
	//		UserChat.wChatLength=CountStringBuffer(UserChat.szChatString);

	//		//��������
	//		WORD wHeadSize=sizeof(UserChat)-sizeof(UserChat.szChatString);
	//		WORD wSendSize=wHeadSize+UserChat.wChatLength*sizeof(UserChat.szChatString[0]);

	//		//��Ϸ�û�
	//		for (WORD i=0;i<m_wChairCount;i++)
	//		{
	//			//��ȡ�û�
	//			IServerUserItem * pIServerUserItem=GetTableUserItem(i);
	//			if ((pIServerUserItem==NULL)||(pIServerUserItem->IsClientReady()==false)) continue;

	//			m_pIMainServiceFrame->SendData(pIServerUserItem,MDM_GF_FRAME,SUB_GF_USER_CHAT,&UserChat,wSendSize);
	//		}

	//		//�Թ��û�
	//		WORD wEnumIndex=0;
	//		IServerUserItem * pIServerUserItem=NULL;

	//		//ö���û�
	//		//do
	//		//{
	//		//	//��ȡ�û�
	//		//	pIServerUserItem=EnumLookonUserItem(wEnumIndex++);
	//		//	if (pIServerUserItem==NULL) break;

	//		//	//��������
	//		//	if (pIServerUserItem->IsClientReady()==true)
	//		//	{
	//		//		m_pIMainServiceFrame->SendData(pIServerUserItem,MDM_GF_FRAME,SUB_GF_USER_CHAT,&UserChat,wSendSize);
	//		//	}
	//		//} while (true);

	//		return true;
	//	}
	//case SUB_GF_USER_EXPRESSION:	//�û�����
	//	{
	//		//Ч�����
	//		ASSERT(wDataSize==sizeof(CMD_GF_C_UserExpression));
	//		if (wDataSize!=sizeof(CMD_GF_C_UserExpression)) return false;

	//		//��������
	//		CMD_GF_C_UserExpression * pUserExpression=(CMD_GF_C_UserExpression *)pData;

	//		//Ŀ���û�
	//		if ((pUserExpression->dwTargetUserID!=0)&&(SearchUserItem(pUserExpression->dwTargetUserID)==NULL))
	//		{
	//			ASSERT(FALSE);
	//			return true;
	//		}

	//		//״̬�ж�
	//		if ((CServerRule::IsForfendGameChat(m_pGameServiceOption->dwServerRule)==true)&&(pIServerUserItem->GetMasterOrder()==0L))
	//		{
	//			SendGameMessage(pIServerUserItem,TEXT("��Ǹ����ǰ��Ϸ�����ֹ��Ϸ���죡"),SMT_CHAT);
	//			return true;
	//		}

	//		//Ȩ���ж�
	//		if (CUserRight::CanRoomChat(pIServerUserItem->GetUserRight())==false)
	//		{
	//			SendGameMessage(pIServerUserItem,TEXT("��Ǹ����û����Ϸ�����Ȩ�ޣ�����Ҫ����������ϵ��Ϸ�ͷ���ѯ��"),SMT_EJECT|SMT_CHAT);
	//			return true;
	//		}

	//		//������Ϣ
	//		CMD_GR_S_UserExpression UserExpression;
	//		ZeroMemory(&UserExpression,sizeof(UserExpression));

	//		//��������
	//		UserExpression.wItemIndex=pUserExpression->wItemIndex;
	//		UserExpression.dwSendUserID=pIServerUserItem->GetUserID();
	//		UserExpression.dwTargetUserID=pUserExpression->dwTargetUserID;

	//		//��Ϸ�û�
	//		for (WORD i=0;i<m_wChairCount;i++)
	//		{
	//			//��ȡ�û�
	//			IServerUserItem * pIServerUserItem=GetTableUserItem(i);
	//			if ((pIServerUserItem==NULL)||(pIServerUserItem->IsClientReady()==false)) continue;

	//			//��������
	//			m_pIMainServiceFrame->SendData(pIServerUserItem,MDM_GF_FRAME,SUB_GF_USER_EXPRESSION,&UserExpression,sizeof(UserExpression));
	//		}

	//		//�Թ��û�
	//		WORD wEnumIndex=0;
	//		IServerUserItem * pIServerUserItem=NULL;

	//		//ö���û�
	//		//do
	//		//{
	//		//	//��ȡ�û�
	//		//	pIServerUserItem=EnumLookonUserItem(wEnumIndex++);
	//		//	if (pIServerUserItem==NULL) break;

	//		//	//��������
	//		//	if (pIServerUserItem->IsClientReady()==true)
	//		//	{
	//		//		m_pIMainServiceFrame->SendData(pIServerUserItem,MDM_GF_FRAME,SUB_GF_USER_EXPRESSION,&UserExpression,sizeof(UserExpression));
	//		//	}
	//		//} while (true);

	//		return true;
	//	}
	//case SUB_GF_LOOKON_CONFIG:		//�Թ�����
	//	{
	//		//Ч�����
	//		ASSERT(wDataSize==sizeof(CMD_GF_LookonConfig));
	//		if (wDataSize<sizeof(CMD_GF_LookonConfig)) return false;

	//		//��������
	//		CMD_GF_LookonConfig * pLookonConfig=(CMD_GF_LookonConfig *)pData;

	//		//Ŀ���û�
	//		if ((pLookonConfig->dwUserID!=0)&&(SearchUserItem(pLookonConfig->dwUserID)==NULL))
	//		{
	//			ASSERT(FALSE);
	//			return true;
	//		}

	//		//�û�Ч��
	//		ASSERT(pIServerUserItem->GetUserStatus()!=US_LOOKON);
	//		if (pIServerUserItem->GetUserStatus()==US_LOOKON) return false;

	//		//�Թ۴���
	//		if (pLookonConfig->dwUserID!=0L)
	//		{
	//			for (INT_PTR i=0;i<m_LookonUserItemArray.GetCount();i++)
	//			{
	//				//��ȡ�û�
	//				IServerUserItem * pILookonUserItem=m_LookonUserItemArray[i];
	//				if (pILookonUserItem->GetUserID()!=pLookonConfig->dwUserID) continue;
	//				if (pILookonUserItem->GetChairID()!=pIServerUserItem->GetChairID()) continue;

	//				//������Ϣ
	//				CMD_GF_LookonStatus LookonStatus;
	//				LookonStatus.cbAllowLookon=pLookonConfig->cbAllowLookon;

	//				//������Ϣ
	//				ASSERT(m_pIMainServiceFrame!=NULL);
	//				m_pIMainServiceFrame->SendData(pILookonUserItem,MDM_GF_FRAME,SUB_GF_LOOKON_STATUS,&LookonStatus,sizeof(LookonStatus));

	//				break;
	//			}
	//		}
	//		else
	//		{
	//			//�����ж�
	//			bool bAllowLookon=(pLookonConfig->cbAllowLookon==TRUE)?true:false;
	//			if (bAllowLookon==m_bAllowLookon[pIServerUserItem->GetChairID()]) return true;

	//			//���ñ���
	//			m_bAllowLookon[pIServerUserItem->GetChairID()]=bAllowLookon;

	//			//������Ϣ
	//			CMD_GF_LookonStatus LookonStatus;
	//			LookonStatus.cbAllowLookon=pLookonConfig->cbAllowLookon;

	//			//������Ϣ
	//			for (INT_PTR i=0;i<m_LookonUserItemArray.GetCount();i++)
	//			{
	//				//��ȡ�û�
	//				IServerUserItem * pILookonUserItem=m_LookonUserItemArray[i];
	//				if (pILookonUserItem->GetChairID()!=pIServerUserItem->GetChairID()) continue;

	//				//������Ϣ
	//				ASSERT(m_pIMainServiceFrame!=NULL);
	//				m_pIMainServiceFrame->SendData(pILookonUserItem,MDM_GF_FRAME,SUB_GF_LOOKON_STATUS,&LookonStatus,sizeof(LookonStatus));
	//			}
	//		}

	//		return true;
	//	}
	}

	return false;
}

//��ȡ��λ
WORD CTableFrame::GetNullChairID()
{

	//LOGI(L"CTableFrame::GetNullChairID");

	//��������
	for (WORD i=0;i<m_wChairCount;i++)
	{
		if (m_TableUserItemArray[i]==NULL)
		{
			return i;
		}
	}

	return INVALID_CHAIR;
}

//�����λ
WORD CTableFrame::GetRandNullChairID()
{
	//LOGI(L"CTableFrame::GetRandNullChairID");

	//��������
	WORD wIndex = rand()%m_wChairCount;
	for (WORD i=wIndex; i<m_wChairCount+wIndex; i++)
	{
		if (m_TableUserItemArray[i%m_wChairCount]==NULL)
		{
			return i%m_wChairCount;
		}
	}

	return INVALID_CHAIR;
}

//�û���Ŀ
WORD CTableFrame::GetSitUserCount()
{
	//LOGI(L"CTableFrame::GetSitUserCount");

	//��������
	WORD wUserCount=0;

	//��Ŀͳ��
	for (WORD i=0;i<m_wChairCount;i++)
	{
		if (GetTableUserItem(i)!=NULL)
		{
			wUserCount++;
		}
	}

	return wUserCount;
}

//�Թ���Ŀ
//WORD CTableFrame::GetLookonUserCount()
//{
//	//LOGI(L"CTableFrame::GetLookonUserCount");
//
//	//��ȡ��Ŀ
//	INT_PTR nLookonCount=m_LookonUserItemArray.GetCount();
//
//	return (WORD)(nLookonCount);
//}

//������Ŀ
WORD CTableFrame::GetOffLineUserCount()
{
	//LOGI(L"CTableFrame::GetOffLineUserCount");

	//��������
	WORD wOffLineCount=0;

	//��������
	for (WORD i=0;i<m_wChairCount;i++)
	{
		if (m_dwOffLineTime[i]!=0L)
		{
			wOffLineCount++;
		}
	}

	return wOffLineCount;
}

//����״��
WORD CTableFrame::GetTableUserInfo(tagTableUserInfo & TableUserInfo)
{
	ZeroMemory(&TableUserInfo,sizeof(TableUserInfo));
	for(WORD i = 0; i < m_pGameServiceAttrib->wChairCount; i++){	//�û�����
		IServerUserItem *pIServerUserItem = GetTableUserItem(i);
		if(pIServerUserItem == NULL){
			continue;
		}
		if(!pIServerUserItem->IsAndroidUser()){	//һ���û�
			++TableUserInfo.wTableUserCount;
		}else{
			++TableUserInfo.wTableAndroidCount;	//������
		}
		if(pIServerUserItem->GetUserStatus() == US_READY){	//׼���ж�
			++TableUserInfo.wTableReadyCount;
		}
	}
	switch (m_cbStartMode){	//��������
	case START_MODE_ALL_READY:		//����׼��
		TableUserInfo.wMinUserCount = 2;
		break;
	case START_MODE_PAIR_READY:		//��Կ�ʼ
		TableUserInfo.wMinUserCount = 2;
		break;
	case START_MODE_TIME_CONTROL:	//ʱ�����
		TableUserInfo.wMinUserCount = 1;
		break;
	default:						//Ĭ��ģʽ
		TableUserInfo.wMinUserCount = m_pGameServiceAttrib->wChairCount;
		break;
	}
	return TableUserInfo.wTableAndroidCount+TableUserInfo.wTableUserCount;
}

//��������
bool CTableFrame::InitializationFrame(WORD wTableID, tagTableFrameParameter & TableFrameParameter)
{

	//LOGI(L"CTableFrame::InitializationFrame");

	//���ñ���
	m_wTableID=wTableID;
	m_wChairCount=TableFrameParameter.pGameServiceAttrib->wChairCount;

	//���ò���
	m_pGameParameter=TableFrameParameter.pGameParameter;
	m_pGameServiceAttrib=TableFrameParameter.pGameServiceAttrib;
	m_pGameServiceOption=TableFrameParameter.pGameServiceOption;

	//����ӿ�
	m_pITimerEngine=TableFrameParameter.pITimerEngine;
	m_pIMainServiceFrame=TableFrameParameter.pIMainServiceFrame;
	m_pIAndroidUserManager=TableFrameParameter.pIAndroidUserManager;
	m_pIKernelDataBaseEngine=TableFrameParameter.pIKernelDataBaseEngine;
	m_pIRecordDataBaseEngine=TableFrameParameter.pIRecordDataBaseEngine;

	//��������
	IGameServiceManager * pIGameServiceManager=TableFrameParameter.pIGameServiceManager;
	m_pITableFrameSink=(ITableFrameSink *)pIGameServiceManager->CreateTableFrameSink(IID_ITableFrameSink,VER_ITableFrameSink);

	//�����ж�
	if (m_pITableFrameSink==NULL)
	{
		ASSERT(FALSE);
		return false;
	}

	//��������
	IUnknownEx * pITableFrame=QUERY_ME_INTERFACE(IUnknownEx);
	if (m_pITableFrameSink->Initialization(pITableFrame)==false) return false;

	//���ñ���
	m_lCellScore=m_pGameServiceOption->lCellScore;

	//��չ�ӿ�
	m_pITableUserAction=QUERY_OBJECT_PTR_INTERFACE(m_pITableFrameSink,ITableUserAction);
	m_pITableUserRequest=QUERY_OBJECT_PTR_INTERFACE(m_pITableFrameSink,ITableUserRequest);

	//��������ģʽ
	//if((TableFrameParameter.pGameServiceOption->wServerType&GAME_GENRE_MATCH)!=0 && TableFrameParameter.pIGameMatchServiceManager!=NULL)
	//{
	//	IUnknownEx * pIUnknownEx=QUERY_ME_INTERFACE(IUnknownEx);
	//	IGameMatchServiceManager * pIGameMatchServiceManager=QUERY_OBJECT_PTR_INTERFACE(TableFrameParameter.pIGameMatchServiceManager,IGameMatchServiceManager);
	//	if (pIGameMatchServiceManager==NULL)
	//	{
	//		ASSERT(FALSE);
	//		return false;
	//	}
	//	m_pIGameMatchSink=(IGameMatchSink *)pIGameMatchServiceManager->CreateGameMatchSink(IID_IGameMatchSink,VER_IGameMatchSink);

	//	//�����ж�
	//	if (m_pIGameMatchSink==NULL)
	//	{
	//		ASSERT(FALSE);
	//		return false;
	//	}

	//	//��չ�ӿ�
	//	m_pIMatchTableAction=QUERY_OBJECT_PTR_INTERFACE(m_pIGameMatchSink,ITableUserAction);
	//	if (m_pIGameMatchSink->InitTableFrameSink(pIUnknownEx)==false) 
	//	{
	//		return false;
	//	}
	//}

	return true;
}

//��������
bool CTableFrame::PerformStandUpAction(IServerUserItem * pIServerUserItem, int flag)
{

	//LOGI(L"CTableFrame::PerformStandUpAction");

	//Ч�����
	ASSERT(pIServerUserItem!=NULL);
	ASSERT(pIServerUserItem->GetTableID()==m_wTableID);
	ASSERT(pIServerUserItem->GetChairID()<=m_wChairCount);

	// ��ӡ��־
	//�û�����
	WORD wChairID=pIServerUserItem->GetChairID();
	BYTE cbUserStatus=pIServerUserItem->GetUserStatus();
	IServerUserItem * pITableUserItem=GetTableUserItem(wChairID);

	//��Ϸ�û�
	if ((m_bGameStarted==true)&&((cbUserStatus==US_PLAYING)||(cbUserStatus==US_OFFLINE)))
	{
		//������Ϸ
		BYTE cbConcludeReason=(cbUserStatus==US_OFFLINE)?GER_NETWORK_ERROR:GER_USER_LEAVE;
		m_pITableFrameSink->OnEventGameConclude(wChairID,pIServerUserItem,cbConcludeReason);

		//�뿪�ж�
		if (m_TableUserItemArray[wChairID]!=pIServerUserItem) return true;
	}
	//���ñ���
	if (pIServerUserItem==pITableUserItem)
	{
		//���ñ���
		m_TableUserItemArray[wChairID]=NULL;

		//������Ϸ��
		if (m_lFrozenedScore[wChairID]!=0L)
		{
			pIServerUserItem->UnFrozenedUserScore(m_lFrozenedScore[wChairID]);
			m_lFrozenedScore[wChairID]=0L;
		}
		//�¼�֪ͨ
		if (m_pITableUserAction!=NULL)
		{
			m_pITableUserAction->OnActionUserStandUp(wChairID,pIServerUserItem,false);
		}
		//�¼�֪ͨ
		//if(m_pIMatchTableAction!=NULL) m_pIMatchTableAction->OnActionUserStandUp(wChairID,pIServerUserItem,false);
	
		//�û�״̬
		if (flag == 1)
		{
			pIServerUserItem->SetClientReady(false);
			pIServerUserItem->SetUserStatus((cbUserStatus==US_OFFLINE)?US_NULL:US_FREE,INVALID_TABLE,INVALID_CHAIR);		
		}

		//��������
		bool bTableLocked=IsTableLocked();
		bool bTableStarted=IsTableStarted();
		WORD wTableUserCount=GetSitUserCount();

		//���ñ���
		m_wUserCount=wTableUserCount;

		//������Ϣ
		if (wTableUserCount==0)
		{
			m_dwTableOwnerID=0L; 
			m_szEnterPassword[0]=0;
		}

		//�����Թ�
		//if (wTableUserCount==0)
		//{
		//	for (INT_PTR i=0;i<m_LookonUserItemArray.GetCount();i++)
		//	{
		//		SendGameMessage(m_LookonUserItemArray[i],TEXT("����Ϸ������������Ѿ��뿪�ˣ�"),SMT_CLOSE_GAME|SMT_EJECT);
		//	}
		//}
		//��������
		ConcludeTable();
		if(flag == 0)
		{
			//clear �û�״̬
			tagUserInfo * UserInfo = pIServerUserItem->GetUserInfo();
			web::http::uri_builder builder;
			builder.append_query(U("action"),U("userLogout"));

			TCHAR parameterStr[50];
			_sntprintf(parameterStr,sizeof(parameterStr),TEXT("%d,%s,%d"),m_pGameServiceOption->wKindID,
				m_pGameServiceOption->szRoomType, UserInfo->dwUserID);

			builder.append_query(U("parameter"),parameterStr);
			utility::string_t v;
			std::map<wchar_t*,wchar_t*> hAdd;
			LOGI(L"userLogout = " << parameterStr);
			if(HttpRequest(web::http::methods::POST,CServiceUnits::g_pServiceUnits->m_InitParameter.m_LobbyURL,builder.to_string(),true,v,hAdd))
			{
				web::json::value root = web::json::value::parse(v);
				utility::string_t actin = root[U("action")].as_string();
				if (root[U("errorCode")] == 0)
				{
					//�û�״̬
					pIServerUserItem->SetClientReady(false);
					pIServerUserItem->SetUserStatus((cbUserStatus==US_OFFLINE)?US_NULL:US_FREE,INVALID_TABLE,INVALID_CHAIR);
				}
				else
				{
					LOGW(L"clear user status fail");
				}
			}
			else
			{
				LOGW(L"clear user status fail");
			}

		}
		//��ʼ�ж�
		if (EfficacyStartGame(INVALID_CHAIR)==true)
		{			
			StartGame();
		}
		//����״̬
		if ((bTableLocked!=IsTableLocked())||(bTableStarted!=IsTableStarted()))
		{
			SendTableStatus();
		}
		return true;
	}
	else
	{
		//��������
		//for (INT_PTR i=0;i<m_LookonUserItemArray.GetCount();i++)
		//{
		//	if (pIServerUserItem==m_LookonUserItemArray[i])
		//	{
		//		//ɾ������
		//		m_LookonUserItemArray.RemoveAt(i);

		//		//�¼�֪ͨ
		//		if (m_pITableUserAction!=NULL)
		//		{
		//			m_pITableUserAction->OnActionUserStandUp(wChairID,pIServerUserItem,true);
		//		}

		//		//�¼�֪ͨ
		//		//if(m_pIMatchTableAction!=NULL) m_pIMatchTableAction->OnActionUserStandUp(wChairID,pIServerUserItem,true);

		//		//�û�״̬
		//		pIServerUserItem->SetClientReady(false);
		//		pIServerUserItem->SetUserStatus(US_FREE,INVALID_TABLE,INVALID_CHAIR);

		//		return true;
		//	}
		//}

		//������� 
		//ASSERT(FALSE);
	}

	return true;
}

//�Թ۶���
//bool CTableFrame::PerformLookonAction(WORD wChairID, IServerUserItem * pIServerUserItem)
//{
//
//	LOGI(L"CTableFrame::PerformLookonAction");
//
//	//Ч�����
//	ASSERT((pIServerUserItem!=NULL)&&(wChairID<m_wChairCount));
//	ASSERT((pIServerUserItem->GetTableID()==INVALID_TABLE)&&(pIServerUserItem->GetChairID()==INVALID_CHAIR));
//
//	//��������
//	tagUserInfo * pUserInfo=pIServerUserItem->GetUserInfo();
//	tagUserRule * pUserRule=pIServerUserItem->GetUserRule();
//	IServerUserItem * pITableUserItem=GetTableUserItem(wChairID);
//
//	//��Ϸ״̬
//	if ((m_bGameStarted==false)&&(pIServerUserItem->GetMasterOrder()==0L))
//	{
//		SendRequestFailure(pIServerUserItem,TEXT("��Ϸ��û�п�ʼ�������Թ۴���Ϸ����"),REQUEST_FAILURE_NORMAL);
//		return false;
//	}
//
//	//ģ�⴦��
//	if (m_pGameServiceAttrib->wChairCount < MAX_CHAIR && pIServerUserItem->IsAndroidUser()==false)
//	{
//		//�������
//		CAttemperEngineSink * pAttemperEngineSink=(CAttemperEngineSink *)m_pIMainServiceFrame;
//
//		//���һ���
//		for (WORD i=0; i<m_pGameServiceAttrib->wChairCount; i++)
//		{
//			//��ȡ�û�
//			IServerUserItem *pIUserItem=m_TableUserItemArray[i];
//			if(pIUserItem==NULL) continue;
//			if(pIUserItem->IsAndroidUser()==false)break;
//
//			//��ȡ����
//			tagBindParameter * pBindParameter=pAttemperEngineSink->GetBindParameter(pIUserItem->GetBindIndex());
//			IAndroidUserItem * pIAndroidUserItem=m_pIAndroidUserManager->SearchAndroidUserItem(pIUserItem->GetUserID(),pBindParameter->dwSocketID);
//			tagAndroidParameter * pAndroidParameter=pIAndroidUserItem->GetAndroidParameter();
//
//			//ģ���ж�
//			if((pAndroidParameter->dwServiceGender&ANDROID_SIMULATE)!=0
//				&& (pAndroidParameter->dwServiceGender&ANDROID_PASSIVITY)==0
//				&& (pAndroidParameter->dwServiceGender&ANDROID_INITIATIVE)==0)
//			{
//				SendRequestFailure(pIServerUserItem,TEXT("��Ǹ����ǰ��Ϸ���ӽ�ֹ�û��Թۣ�"),REQUEST_FAILURE_NORMAL);
//				return false;
//			}
//
//			break;
//		}
//	}
//
//
//	//�Թ��ж�
//	if (CServerRule::IsAllowAndroidSimulate(m_pGameServiceOption->dwServerRule)==true
//		&& (CServerRule::IsAllowAndroidAttend(m_pGameServiceOption->dwServerRule)==false))
//	{
//		if ((pITableUserItem!=NULL)&&(pITableUserItem->IsAndroidUser()==true))
//		{
//			SendRequestFailure(pIServerUserItem,TEXT("��Ǹ����ǰ��Ϸ�����ֹ�û��Թۣ�"),REQUEST_FAILURE_NORMAL);
//			return false;
//		}
//	}
//
//	//״̬�ж�
//	if ((CServerRule::IsForfendGameLookon(m_pGameServiceOption->dwServerRule)==true)&&(pIServerUserItem->GetMasterOrder()==0))
//	{
//		SendRequestFailure(pIServerUserItem,TEXT("��Ǹ����ǰ��Ϸ�����ֹ�û��Թۣ�"),REQUEST_FAILURE_NORMAL);
//		return false;
//	}
//
//	//�����ж�
//	if ((pITableUserItem==NULL)&&(pIServerUserItem->GetMasterOrder()==0L))
//	{
//		SendRequestFailure(pIServerUserItem,TEXT("���������λ��û����Ϸ��ң��޷��Թ۴���Ϸ��"),REQUEST_FAILURE_NORMAL);
//		return false;
//	}
//
//	//����Ч��
//	if ((IsTableLocked()==true)&&(pIServerUserItem->GetMasterOrder()==0L)&&(lstrcmp(pUserRule->szPassword,m_szEnterPassword)!=0))
//	{
//		SendRequestFailure(pIServerUserItem,TEXT("��Ϸ���������벻��ȷ�������Թ���Ϸ��"),REQUEST_FAILURE_PASSWORD);
//		return false;
//	}
//
//	//��չЧ��
//	if (m_pITableUserRequest!=NULL)
//	{
//		//��������
//		tagRequestResult RequestResult;
//		ZeroMemory(&RequestResult,sizeof(RequestResult));
//
//		//����Ч��
//		if (m_pITableUserRequest->OnUserRequestLookon(wChairID,pIServerUserItem,RequestResult)==false)
//		{
//			//������Ϣ
//			SendRequestFailure(pIServerUserItem,RequestResult.szFailureReason,RequestResult.cbFailureCode);
//
//			return false;
//		}
//	}
//
//	//�����û�
//	m_LookonUserItemArray.Add(pIServerUserItem);
//
//	//�û�״̬
//	pIServerUserItem->SetClientReady(false);
//	pIServerUserItem->SetUserStatus(US_LOOKON,m_wTableID,wChairID);
//
//	//�¼�֪ͨ
//	if (m_pITableUserAction!=NULL)
//	{
//		m_pITableUserAction->OnActionUserSitDown(wChairID,pIServerUserItem,true);
//	}
//
//	//�¼�֪ͨ
//	//if(m_pIMatchTableAction!=NULL) m_pIMatchTableAction->OnActionUserSitDown(wChairID,pIServerUserItem,true);
//	return true;
//}

//���¶���
bool CTableFrame::PerformSitDownAction(WORD wChairID, IServerUserItem * pIServerUserItem, LPCTSTR lpszPassword)
{

	LOGI(L"CTableFrame::PerformSitDownAction " << pIServerUserItem->GetUserID() << "---- " << pIServerUserItem->GetUserStatus());
	int a = 0;
	//Ч�����
	ASSERT((pIServerUserItem!=NULL)&&(wChairID<m_wChairCount));
	ASSERT((pIServerUserItem->GetTableID()==INVALID_TABLE)&&(pIServerUserItem->GetChairID()==INVALID_CHAIR));

	//��������
	tagUserInfo * pUserInfo=pIServerUserItem->GetUserInfo();
	tagUserRule * pUserRule=pIServerUserItem->GetUserRule();
	IServerUserItem * pITableUserItem=GetTableUserItem(wChairID);

	//���ֱ���
	SCORE lUserScore=pIServerUserItem->GetUserScore();
	SCORE lMinTableScore=m_pGameServiceOption->lMinTableScore;
	SCORE lLessEnterScore=m_pITableFrameSink->QueryLessEnterScore(wChairID,pIServerUserItem);

	//////////////////////////////////////////////////////////////////////////////////	
	//for(int i=0;i<m_pGameServiceOption->kickoutplayer[0];i++){
	/*	if(/*1943m_pGameServiceOption->kickoutplayer[i+1]!pIServerUserItem->GetUserID())
		{
			if(!(pIServerUserItem->IsAndroidUser()))
				SendRequestFailure(pIServerUserItem,TEXT("��Ǹ,���ѱ�����Ա�߳���"),REQUEST_FAILURE_NORMAL);
			//�û�����				
			PerformStandUpAction(pIServerUserItem,0);		
			if(pIServerUserItem->IsAndroidUser())
			{
				IAndroidUserItem* iaui = m_pIAndroidUserManager->SearchAndroidUserItem(pIServerUserItem->GetUserID());
				if(iaui)
				{					
					CAttemperEngineSink * pAttemperEngineSink=(CAttemperEngineSink *)m_pIMainServiceFrame;
					tagBindParameter * pBindParameter=pAttemperEngineSink->GetBindParameter(pIServerUserItem->GetBindIndex());
					m_pIAndroidUserManager->DeleteAndroidUserItem(pBindParameter->dwSocketID);					
				}
			}
		//	m_pGameServiceOption->kickoutplayer[0]=m_pGameServiceOption->kickoutplayer[0]-1;
		//	m_pGameServiceOption->kickoutplayer[i+1]=0;
			return false;
		}
	//}*/
	/*//�ı�����ֵ��
	//	for(int i=1;i<m_pGameServiceOption->ChangeLuckvalue[0][0];i++)
		//{
			if(1943/*m_pGameServiceOption->ChangeLuckvalue[i][0]/ == pIServerUserItem->GetUserID())
			{
				pIServerUserItem->SetFrotue(200/m_pGameServiceOption->ChangeLuckvalue[i][1]/);
			//	break;
			}
		//}*/
			//LOGI("--------- player lucky value " << pIServerUserItem->GetFortune());
	////////////////////////////////////////
	//״̬�ж�
	if ((CServerRule::IsForfendGameEnter(m_pGameServiceOption->dwServerRule)==true)&&(pIServerUserItem->GetMasterOrder()==0))
	{
		DebugString("CTableFrame::PerformSitDownAction, dwServerRule="<<m_pGameServiceOption->dwServerRule<<", GetMasterOrder="<<pIServerUserItem->GetMasterOrder())
		SendRequestFailure(pIServerUserItem,TEXT("��Ǹ����ǰ��Ϸ���ӽ�ֹ�û����룡"),REQUEST_FAILURE_NORMAL);
		return false;
	}

	//ģ�⴦��
	if (m_pGameServiceAttrib->wChairCount < MAX_CHAIR && pIServerUserItem->IsAndroidUser()==false)
	{
		//�������
		CAttemperEngineSink * pAttemperEngineSink=(CAttemperEngineSink *)m_pIMainServiceFrame;

		//���һ���
		for (WORD i=0; i<m_pGameServiceAttrib->wChairCount; i++)
		{
			//��ȡ�û�
			IServerUserItem *pIUserItem=m_TableUserItemArray[i];
			if(pIUserItem==NULL) continue;
			if(pIUserItem->IsAndroidUser()==false)break;

			//��ȡ����
			tagBindParameter * pBindParameter=pAttemperEngineSink->GetBindParameter(pIUserItem->GetBindIndex());
			IAndroidUserItem * pIAndroidUserItem=m_pIAndroidUserManager->SearchAndroidUserItem(pIUserItem->GetUserID(),pBindParameter->dwSocketID);
			tagAndroidParameter * pAndroidParameter=pIAndroidUserItem->GetAndroidParameter();			
			//ģ���ж�
			if((pAndroidParameter->dwServiceGender&ANDROID_SIMULATE)!=0
				&& (pAndroidParameter->dwServiceGender&ANDROID_PASSIVITY)==0
				&& (pAndroidParameter->dwServiceGender&ANDROID_INITIATIVE)==0)
			{
				DebugString("CTableFrame::PerformSitDownAction, dwServiceGender="<<pAndroidParameter->dwServiceGender);
				SendRequestFailure(pIServerUserItem,TEXT("��Ǹ����ǰ��Ϸ���ӽ�ֹ�û����룡"),REQUEST_FAILURE_NORMAL);
				return false;
			}

			break;
		}
	}

	//��̬����
	bool bDynamicJoin=true;
	if (m_pGameServiceAttrib->cbDynamicJoin==FALSE) bDynamicJoin=false;
	if (CServerRule::IsAllowDynamicJoin(m_pGameServiceOption->dwServerRule)==false) bDynamicJoin=false;

	//��Ϸ״̬
	if ((m_bGameStarted==true)&&(bDynamicJoin==false))
	{
		DebugString("��Ϸ�Ѿ���ʼ�ˣ����ڲ��ܽ�����Ϸ����");
		SendRequestFailure(pIServerUserItem,TEXT("��Ϸ�Ѿ���ʼ�ˣ����ڲ��ܽ�����Ϸ����"),REQUEST_FAILURE_NORMAL);
		return false;
	}

	//�����ж�
	if (pITableUserItem!=NULL)
	{
		//������
		if(CServerRule::IsAllowAvertCheatMode(m_pGameServiceOption->dwServerRule)) return false;

		//������Ϣ
		TCHAR szDescribe[128]=TEXT("");
		_sntprintf(szDescribe,CountArray(szDescribe),TEXT("�����Ѿ��� [ %s ] �����ȵ��ˣ��´ζ���Ҫ����ˣ�"),pITableUserItem->GetNickName());

		//������Ϣ
		SendRequestFailure(pIServerUserItem,szDescribe,REQUEST_FAILURE_NORMAL);
		DebugString("�����Ѿ��� [ %s ] �����ȵ��ˣ��´ζ���Ҫ����ˣ�");
		return false;
	}

	//����Ч��
	if((IsTableLocked()==true&&(pIServerUserItem->GetMasterOrder()==0L))
		&&((lpszPassword==NULL)||(lstrcmp(lpszPassword,m_szEnterPassword)!=0)))
	{
		SendRequestFailure(pIServerUserItem,TEXT("��Ϸ���������벻��ȷ�����ܼ�����Ϸ��"),REQUEST_FAILURE_PASSWORD);
		DebugString("��Ϸ���������벻��ȷ�����ܼ�����Ϸ��");
		return false;
	}

	//����Ч��
	if (EfficacyEnterTableScoreRule(wChairID,pIServerUserItem)==false) return false;
	if (EfficacyIPAddress(pIServerUserItem)==false) return false;
	if (EfficacyScoreRule(pIServerUserItem)==false) return false;

	//��չЧ��
	if (m_pITableUserRequest!=NULL)
	{
		//��������
		tagRequestResult RequestResult;
		ZeroMemory(&RequestResult,sizeof(RequestResult));

		//����Ч��
		if (m_pITableUserRequest->OnUserRequestSitDown(wChairID,pIServerUserItem,RequestResult)==false)
		{
			//������Ϣ
			SendRequestFailure(pIServerUserItem,RequestResult.szFailureReason,RequestResult.cbFailureCode);
			DebugString("SendRequestFailure");
			return false;
		}
	}

	//���ñ���
	m_wUserCount=GetSitUserCount();

	//������Ϣ
	if (GetSitUserCount()==0)
	{
		//״̬����
		bool bTableLocked=IsTableLocked();

		//���ñ���
		m_dwTableOwnerID=pIServerUserItem->GetUserID();
		lstrcpyn(m_szEnterPassword,lpszPassword,CountArray(m_szEnterPassword));

		//����״̬
		if (bTableLocked!=IsTableLocked()) SendTableStatus();
	}
	else if(GetSitUserCount() > 0)
	{
		if (!IsTableLocked())
		{

			if (lpszPassword)
			{
				if (wcslen(lpszPassword))
				{
					//������Ϣ
					TCHAR szDescribe[128]=TEXT("");
					_sntprintf(szDescribe,CountArray(szDescribe),TEXT("�÷����Ѿ����˽����ˣ���������ʧ��"));

					//������Ϣ
					SendRequestFailure(pIServerUserItem,szDescribe,REQUEST_FAILURE_NORMAL);
					DebugString("�÷����Ѿ����˽����ˣ���������ʧ��");
					return false;
				}
			}

		}
	}

	//���ñ���
	m_TableUserItemArray[wChairID]=pIServerUserItem;
	m_wDrawCount=0;

	//�û�״̬
	if ((IsGameStarted()==false)||(m_cbStartMode!=START_MODE_TIME_CONTROL))
	{
		if (CServerRule::IsAllowAvertCheatMode(m_pGameServiceOption->dwServerRule)==false)
		{
			pIServerUserItem->SetClientReady(false);
			pIServerUserItem->SetUserStatus(US_SIT,m_wTableID,wChairID);
		}
		else
		{
			pIServerUserItem->SetClientReady(false);
			//pIServerUserItem->SetUserStatus(US_READY,m_wTableID,wChairID);
			pIServerUserItem->SetUserStatus(US_SIT,m_wTableID,wChairID);
		}
	}
	else
	{
		//���ñ���
		m_wOffLineCount[wChairID]=0L;
		m_dwOffLineTime[wChairID]=0L;

		//������Ϸ��
		if (m_pGameServiceOption->lServiceScore>0L)
		{
			m_lFrozenedScore[wChairID]=m_pGameServiceOption->lServiceScore;
			pIServerUserItem->FrozenedUserScore(m_pGameServiceOption->lServiceScore);
		}

		//����״̬
		pIServerUserItem->SetClientReady(false);
		pIServerUserItem->SetUserStatus(US_PLAYING,m_wTableID,wChairID);
	}

	//�¼�֪ͨ
	if (m_pITableUserAction!=NULL)
	{
		m_pITableUserAction->OnActionUserSitDown(wChairID,pIServerUserItem,false);
	}

	//�¼�֪ͨ
	//if(m_pIMatchTableAction!=NULL) m_pIMatchTableAction->OnActionUserSitDown(wChairID,pIServerUserItem,false);

	return true;
}

void CTableFrame::RemovePreSitInfo()
{
	ZeroMemory(m_TableUserItemArray,sizeof(m_TableUserItemArray));
}

bool CTableFrame::CanSitDown(WORD wChairID, IServerUserItem * pIServerUserItem, LPCTSTR lpszPassword, bool bInSeat)
{
	if(!bInSeat)
	{
		int a = 0;
		//Ч�����
		ASSERT((pIServerUserItem!=NULL)&&(wChairID<m_wChairCount));
		ASSERT((pIServerUserItem->GetTableID()==INVALID_TABLE)&&(pIServerUserItem->GetChairID()==INVALID_CHAIR));

		//��������
		tagUserInfo * pUserInfo=pIServerUserItem->GetUserInfo();
		tagUserRule * pUserRule=pIServerUserItem->GetUserRule();
		IServerUserItem * pITableUserItem=GetTableUserItem(wChairID);

		//���ֱ���
		SCORE lUserScore=pIServerUserItem->GetUserScore();
		SCORE lMinTableScore=m_pGameServiceOption->lMinTableScore;
		SCORE lLessEnterScore=m_pITableFrameSink->QueryLessEnterScore(wChairID,pIServerUserItem);

		//״̬�ж�
		if ((CServerRule::IsForfendGameEnter(m_pGameServiceOption->dwServerRule)==true)&&(pIServerUserItem->GetMasterOrder()==0))
		{
			LOGI("CTableFrame::PerformSitDownAction, dwServerRule="<<m_pGameServiceOption->dwServerRule<<", GetMasterOrder="<<pIServerUserItem->GetMasterOrder())
			SendRequestFailure(pIServerUserItem,TEXT("��Ǹ����ǰ��Ϸ���ӽ�ֹ�û����룡"),REQUEST_FAILURE_NORMAL);
			return false;
		}

		//ģ�⴦��
		if (m_pGameServiceAttrib->wChairCount < MAX_CHAIR && pIServerUserItem->IsAndroidUser()==false)
		{
			//�������
			CAttemperEngineSink * pAttemperEngineSink=(CAttemperEngineSink *)m_pIMainServiceFrame;

			//���һ���
			for (WORD i=0; i<m_pGameServiceAttrib->wChairCount; i++)
			{
				//��ȡ�û�
				IServerUserItem *pIUserItem=m_TableUserItemArray[i];
				if(pIUserItem==NULL) continue;
				if(pIUserItem->IsAndroidUser()==false)break;

				//��ȡ����
				tagBindParameter * pBindParameter=pAttemperEngineSink->GetBindParameter(pIUserItem->GetBindIndex());
				IAndroidUserItem * pIAndroidUserItem=m_pIAndroidUserManager->SearchAndroidUserItem(pIUserItem->GetUserID(),pBindParameter->dwSocketID);
				tagAndroidParameter * pAndroidParameter=pIAndroidUserItem->GetAndroidParameter();

				//ģ���ж�
				if((pAndroidParameter->dwServiceGender&ANDROID_SIMULATE)!=0
					&& (pAndroidParameter->dwServiceGender&ANDROID_PASSIVITY)==0
					&& (pAndroidParameter->dwServiceGender&ANDROID_INITIATIVE)==0)
				{
					LOGI("CTableFrame::PerformSitDownAction, dwServiceGender="<<pAndroidParameter->dwServiceGender);
					SendRequestFailure(pIServerUserItem,TEXT("��Ǹ����ǰ��Ϸ���ӽ�ֹ�û����룡"),REQUEST_FAILURE_NORMAL);
					return false;
				}

				break;
			}
		}

		//��̬����
		bool bDynamicJoin=true;
		if (m_pGameServiceAttrib->cbDynamicJoin==FALSE) bDynamicJoin=false;
		if (CServerRule::IsAllowDynamicJoin(m_pGameServiceOption->dwServerRule)==false) bDynamicJoin=false;

		//��Ϸ״̬
		if ((m_bGameStarted==true)&&(bDynamicJoin==false))
		{
			SendRequestFailure(pIServerUserItem,TEXT("��Ϸ�Ѿ���ʼ�ˣ����ڲ��ܽ�����Ϸ����"),REQUEST_FAILURE_NORMAL);
			return false;
		}

		//�����ж�
		if (pITableUserItem!=NULL)
		{
			//������
			if(CServerRule::IsAllowAvertCheatMode(m_pGameServiceOption->dwServerRule)) return false;

			//������Ϣ
			TCHAR szDescribe[128]=TEXT("");
			_sntprintf(szDescribe,CountArray(szDescribe),TEXT("�����Ѿ��� [ %s ] �����ȵ��ˣ��´ζ���Ҫ����ˣ�"),pITableUserItem->GetNickName());

			//������Ϣ
			SendRequestFailure(pIServerUserItem,szDescribe,REQUEST_FAILURE_NORMAL);

			return false;
		}

		//����Ч��
		if((IsTableLocked()==true&&(pIServerUserItem->GetMasterOrder()==0L))
			&&((lpszPassword==NULL)||(lstrcmp(lpszPassword,m_szEnterPassword)!=0)))
		{
			SendRequestFailure(pIServerUserItem,TEXT("��Ϸ���������벻��ȷ�����ܼ�����Ϸ��"),REQUEST_FAILURE_PASSWORD);
			return false;
		}

		//����Ч��
		if (EfficacyEnterTableScoreRule(wChairID,pIServerUserItem)==false) return false;
		if (EfficacyIPAddress(pIServerUserItem)==false) return false;
		if (EfficacyScoreRule(pIServerUserItem)==false) return false;

		//��չЧ��
		if (m_pITableUserRequest!=NULL)
		{
			//��������
			tagRequestResult RequestResult;
			ZeroMemory(&RequestResult,sizeof(RequestResult));

			//����Ч��
			if (m_pITableUserRequest->OnUserRequestSitDown(wChairID,pIServerUserItem,RequestResult)==false)
			{
				//������Ϣ
				SendRequestFailure(pIServerUserItem,RequestResult.szFailureReason,RequestResult.cbFailureCode);

				return false;
			}
		}

		//���ñ���
		m_wUserCount=GetSitUserCount();

		//������Ϣ
		if (GetSitUserCount()==0)
		{
			//״̬����
			bool bTableLocked=IsTableLocked();

			//���ñ���
			m_dwTableOwnerID=pIServerUserItem->GetUserID();
			lstrcpyn(m_szEnterPassword,lpszPassword,CountArray(m_szEnterPassword));

			//����״̬
			if (bTableLocked!=IsTableLocked()) SendTableStatus();
		}
		else if(GetSitUserCount() > 0)
		{
			if (!IsTableLocked())
			{

				if (lpszPassword)
				{
					if (wcslen(lpszPassword))
					{
						//������Ϣ
						TCHAR szDescribe[128]=TEXT("");
						_sntprintf(szDescribe,CountArray(szDescribe),TEXT("�÷����Ѿ����˽����ˣ���������ʧ��"));

						//������Ϣ
						SendRequestFailure(pIServerUserItem,szDescribe,REQUEST_FAILURE_NORMAL);

						return false;
					}
				}

			}
		}
		//��������ʱ�� �����Ҹ���ȥ ������λ�ӵ�ʱ�� �� �������ҵ�λ����
		m_TableUserItemArray[wChairID]=pIServerUserItem;
		m_wDrawCount=0;

		return true;
	}
	else
	{
		//�û�״̬
		if ((IsGameStarted()==false)||(m_cbStartMode!=START_MODE_TIME_CONTROL))
		{
			if (CServerRule::IsAllowAvertCheatMode(m_pGameServiceOption->dwServerRule)==false)
			{
				pIServerUserItem->SetClientReady(false);
				pIServerUserItem->SetUserStatus(US_SIT,m_wTableID,wChairID);
			}
			else
			{
				pIServerUserItem->SetClientReady(false);
				//pIServerUserItem->SetUserStatus(US_READY,m_wTableID,wChairID);
				pIServerUserItem->SetUserStatus(US_SIT,m_wTableID,wChairID);
			}
		}
		else
		{
			//���ñ���
			m_wOffLineCount[wChairID]=0L;
			m_dwOffLineTime[wChairID]=0L;

			//������Ϸ��
			if (m_pGameServiceOption->lServiceScore>0L)
			{
				m_lFrozenedScore[wChairID]=m_pGameServiceOption->lServiceScore;
				pIServerUserItem->FrozenedUserScore(m_pGameServiceOption->lServiceScore);
			}

			//����״̬
			pIServerUserItem->SetClientReady(false);
			pIServerUserItem->SetUserStatus(US_PLAYING,m_wTableID,wChairID);
		}

		//�¼�֪ͨ
		if (m_pITableUserAction!=NULL)
		{
			m_pITableUserAction->OnActionUserSitDown(wChairID,pIServerUserItem,false);
		}

		//�¼�֪ͨ
		//if(m_pIMatchTableAction!=NULL) m_pIMatchTableAction->OnActionUserSitDown(wChairID,pIServerUserItem,false);
	}
	return true;
}


//����״̬
bool CTableFrame::SendTableStatus()
{
	//LOGI(L"CTableFrame::SendTableStatus");

	//��������
	CMD_GR_TableStatus TableStatus;
	ZeroMemory(&TableStatus,sizeof(TableStatus));

	//��������
	TableStatus.wTableID=m_wTableID;
	TableStatus.TableStatus.cbTableLock=IsTableLocked()?TRUE:FALSE;
	TableStatus.TableStatus.cbPlayStatus=IsTableStarted()?TRUE:FALSE;

	//��������
	m_pIMainServiceFrame->SendData(BG_COMPUTER,MDM_GR_STATUS,SUB_GR_TABLE_STATUS,&TableStatus,sizeof(TableStatus));

	//�ֻ�����

	return true;
}

//����ʧ��
bool CTableFrame::SendRequestFailure(IServerUserItem * pIServerUserItem, LPCTSTR pszDescribe, LONG lErrorCode)
{
	LOGI(L"CTableFrame::SendRequestFailure");

	//��������
	CMD_GR_RequestFailure RequestFailure;
	ZeroMemory(&RequestFailure,sizeof(RequestFailure));

	//��������
	RequestFailure.lErrorCode=lErrorCode;
	lstrcpyn(RequestFailure.szDescribeString,pszDescribe,CountArray(RequestFailure.szDescribeString));

	//��������
	WORD wDataSize=CountStringBuffer(RequestFailure.szDescribeString);
	WORD wHeadSize=sizeof(RequestFailure)-sizeof(RequestFailure.szDescribeString);
	m_pIMainServiceFrame->SendData(pIServerUserItem,MDM_GR_USER,SUB_GR_REQUEST_FAILURE,&RequestFailure,wHeadSize+wDataSize);

	return true;
}
//��ʼЧ��
bool CTableFrame::EfficacyStartGame(WORD wReadyChairID)
{
	//LOGI(L"CTableFrame::EfficacyStartGame");

	//״̬�ж�
	if (m_bGameStarted==true) return false;

	//ģʽ����
	if (m_cbStartMode==START_MODE_TIME_CONTROL) return false;
	if (m_cbStartMode==START_MODE_MASTER_CONTROL) return false;

	//׼������
	WORD wReadyUserCount=0;
	bool bRealuser = false;
	for (WORD i = 0; i<m_wChairCount;i++){
		//��ȡ�û�
		IServerUserItem * pITableUserItem=GetTableUserItem(i);
		if (pITableUserItem==NULL){ 
			continue;
		}

		//�û�ͳ��
		if (pITableUserItem!=NULL){
			#if ANDROID_AUTO_PALY
				bRealuser = true;
			#endif
			//״̬�ж�
			if(!pITableUserItem->IsAndroidUser() && (pITableUserItem->GetUserStatus()==US_READY)){ 
				//����Ҫ��һ�����ǻ����˲��ܿ�ʼ��Ϸ
				bRealuser = true;
			}
			if (pITableUserItem->IsClientReady() == false){ 
				return false;
			}
			if ((wReadyChairID != i) && (pITableUserItem->GetUserStatus() != US_READY)){
				return false;
			}
			//�û�����
			wReadyUserCount++;
		}
	}
	if(!bRealuser) return false;
	//��ʼ����
	switch (m_cbStartMode){
	case START_MODE_ALL_READY:			//����׼��
		if(wReadyUserCount >= 2L){		//��Ŀ�ж�
			return true;
		}else{
			return false;
		}
		break;
	case START_MODE_FULL_READY:			//���˿�ʼ
		if(wReadyUserCount == m_wChairCount){	//�����ж�
			return true;
		}else{
			return false;
		}
		break;
	case START_MODE_PAIR_READY:			//��Կ�ʼ	��ȷ���Ƿ��и���ģʽ?
		if(wReadyUserCount == m_wChairCount){	//��Ŀ�ж�
			return true;
		}
		if((wReadyUserCount < 2L) || (wReadyUserCount % 2) != 0){
			return false;
		}
		
		for(WORD i = 0; i < m_wChairCount / 2; i++){	//λ���ж�
			IServerUserItem *pICurrentUserItem = GetTableUserItem(i);
			IServerUserItem *pITowardsUserItem = GetTableUserItem(i + m_wChairCount / 2);
			//λ�ù���
			/*
			if((pICurrentUserItem == NULL) && (pITowardsUserItem!=NULL)) return false;
			if((pICurrentUserItem!=NULL)&&(pITowardsUserItem==NULL)) return false;
			*/
			if(pICurrentUserItem == NULL || pITowardsUserItem == NULL){
				return false;
			}
		}
		return true;
	default:
		ASSERT(FALSE);
		return false;
	}

	return false;
}

//��ַЧ��
bool CTableFrame::EfficacyIPAddress(IServerUserItem * pIServerUserItem)
{
	//LOGI(L"CTableFrame::EfficacyIPAddress");

	//����Ա��������
	if(pIServerUserItem->GetMasterOrder()!=0) return true;

	//�����ж�
	if (CServerRule::IsForfendGameRule(m_pGameServiceOption->dwServerRule)==true) return true;

	//������
	if(CServerRule::IsAllowAvertCheatMode(m_pGameServiceOption->dwServerRule)) return true;

	//��ַЧ��
	const tagUserRule * pUserRule=pIServerUserItem->GetUserRule(),*pTableUserRule=NULL;
	bool bCheckSameIP=pUserRule->bLimitSameIP;
	for (WORD i=0;i<m_wChairCount;i++)
	{
		//��ȡ�û�
		IServerUserItem * pITableUserItem=GetTableUserItem(i);
		if (pITableUserItem!=NULL && (!pITableUserItem->IsAndroidUser()) && (pITableUserItem->GetMasterOrder()==0))
		{
			pTableUserRule=pITableUserItem->GetUserRule();
			if (pTableUserRule->bLimitSameIP==true) 
			{
				bCheckSameIP=true;
				break;
			}
		}
	}

	//��ַЧ��
	if (bCheckSameIP==true)
	{
		DWORD dwUserIP=pIServerUserItem->GetClientAddr();
		for (WORD i=0;i<m_wChairCount;i++)
		{
			//��ȡ�û�
			IServerUserItem * pITableUserItem=GetTableUserItem(i);
			if ((pITableUserItem!=NULL)&&(pITableUserItem != pIServerUserItem)&&(!pITableUserItem->IsAndroidUser())&&(pITableUserItem->GetMasterOrder()==0)&&(pITableUserItem->GetClientAddr()==dwUserIP))
			{
				if (!pUserRule->bLimitSameIP)
				{
					//������Ϣ
					LPCTSTR pszDescribe=TEXT("����Ϸ����������˲�����ͬ IP ��ַ�������Ϸ���� IP ��ַ�����ҵ� IP ��ַ��ͬ�����ܼ�����Ϸ��");
					SendRequestFailure(pIServerUserItem,pszDescribe,REQUEST_FAILURE_NORMAL);
					return false;
				}
				else
				{
					//������Ϣ
					LPCTSTR pszDescribe=TEXT("�������˲�����ͬ IP ��ַ�������Ϸ������Ϸ���������� IP ��ַ��ͬ����ң����ܼ�����Ϸ��");
					SendRequestFailure(pIServerUserItem,pszDescribe,REQUEST_FAILURE_NORMAL);
					return false;
				}
			}
		}
		for (WORD i=0;i<m_wChairCount-1;i++)
		{
			//��ȡ�û�
			IServerUserItem * pITableUserItem=GetTableUserItem(i);
			if (pITableUserItem!=NULL && (!pITableUserItem->IsAndroidUser()) && (pITableUserItem->GetMasterOrder()==0))
			{
				for (WORD j=i+1;j<m_wChairCount;j++)
				{
					//��ȡ�û�
					IServerUserItem * pITableNextUserItem=GetTableUserItem(j);
					if ((pITableNextUserItem!=NULL) && (!pITableNextUserItem->IsAndroidUser()) && (pITableNextUserItem->GetMasterOrder()==0)&&(pITableUserItem->GetClientAddr()==pITableNextUserItem->GetClientAddr()))
					{
						LPCTSTR pszDescribe=TEXT("�������˲�����ͬ IP ��ַ�������Ϸ������Ϸ������ IP ��ַ��ͬ����ң����ܼ�����Ϸ��");
						SendRequestFailure(pIServerUserItem,pszDescribe,REQUEST_FAILURE_NORMAL);
						return false;
					}
				}
			}
		}
	}
	return true;
}

//����Ч��
bool CTableFrame::EfficacyScoreRule(IServerUserItem * pIServerUserItem)
{

	//LOGI(L"CTableFrame::EfficacyScoreRule");

	//����Ա��������
	if(pIServerUserItem->GetMasterOrder()!=0) return true;

	//�����ж�
	if (CServerRule::IsForfendGameRule(m_pGameServiceOption->dwServerRule)==true) return true;

	//������
	if(CServerRule::IsAllowAvertCheatMode(m_pGameServiceOption->dwServerRule)) return true;

	//��������
	//WORD wWinRate=pIServerUserItem->GetUserWinRate();
	//WORD wFleeRate=pIServerUserItem->GetUserFleeRate();

	//���ַ�Χ
	//for (WORD i=0;i<m_wChairCount;i++)
	//{
	//	//��ȡ�û�
	//	IServerUserItem * pITableUserItem=GetTableUserItem(i);

	//	//����Ч��
	//	if (pITableUserItem!=NULL)
	//	{
	//		//��ȡ����
	//		tagUserRule * pTableUserRule=pITableUserItem->GetUserRule();

	//		//����Ч��
	//		if ((pTableUserRule->bLimitFleeRate)&&(wFleeRate>pTableUserRule->wMaxFleeRate))
	//		{
	//			//������Ϣ
	//			TCHAR szDescribe[128]=TEXT("");
	//			_sntprintf(szDescribe,CountArray(szDescribe),TEXT("����������̫�ߣ��� %s ���õ����ò��������ܼ�����Ϸ��"),pITableUserItem->GetNickName());

	//			//������Ϣ
	//			SendRequestFailure(pIServerUserItem,szDescribe,REQUEST_FAILURE_NORMAL);

	//			return false;
	//		}

	//		//ʤ��Ч��
	//		if ((pTableUserRule->bLimitWinRate)&&(wWinRate<pTableUserRule->wMinWinRate))
	//		{
	//			//������Ϣ
	//			TCHAR szDescribe[128]=TEXT("");
	//			_sntprintf(szDescribe,CountArray(szDescribe),TEXT("����ʤ��̫�ͣ��� %s ���õ����ò��������ܼ�����Ϸ��"),pITableUserItem->GetNickName());

	//			//������Ϣ
	//			SendRequestFailure(pIServerUserItem,szDescribe,REQUEST_FAILURE_NORMAL);

	//			return false;
	//		}

	//		//����Ч��
	//		if (pTableUserRule->bLimitGameScore==true)
	//		{
	//			//��߻���
	//			if (pIServerUserItem->GetUserScore()>pTableUserRule->lMaxGameScore)
	//			{
	//				//������Ϣ
	//				TCHAR szDescribe[128]=TEXT("");
	//				_sntprintf(szDescribe,CountArray(szDescribe),TEXT("���Ļ���̫�ߣ��� %s ���õ����ò��������ܼ�����Ϸ��"),pITableUserItem->GetNickName());

	//				//������Ϣ
	//				SendRequestFailure(pIServerUserItem,szDescribe,REQUEST_FAILURE_NORMAL);

	//				return false;
	//			}

	//			//��ͻ���
	//			if (pIServerUserItem->GetUserScore()<pTableUserRule->lMinGameScore)
	//			{
	//				//������Ϣ
	//				TCHAR szDescribe[128]=TEXT("");
	//				_sntprintf(szDescribe,CountArray(szDescribe),TEXT("���Ļ���̫�ͣ��� %s ���õ����ò��������ܼ�����Ϸ��"),pITableUserItem->GetNickName());

	//				//������Ϣ
	//				SendRequestFailure(pIServerUserItem,szDescribe,REQUEST_FAILURE_NORMAL);

	//				return false;
	//			}
	//		}
	//	}
	//}

	return true;
}

//����Ч��
bool CTableFrame::EfficacyEnterTableScoreRule(WORD wChairID, IServerUserItem * pIServerUserItem)
{

	//LOGI(L"CTableFrame::EfficacyEnterTableScoreRule");

	//���ֱ���
	SCORE lUserScore=pIServerUserItem->GetUserScore();
	SCORE lMinTableScore=m_pGameServiceOption->lMinEnterScore;
	SCORE lLessEnterScore=m_pITableFrameSink->QueryLessEnterScore(wChairID,pIServerUserItem);
	if (((lMinTableScore!=0L)&&(lUserScore<lMinTableScore))||((lLessEnterScore!=0L)&&(lUserScore<lLessEnterScore)))
	{
		//������Ϣ
		TCHAR szDescribe[128]=TEXT("");
		if(m_pGameServiceOption->wServerType==GAME_GENRE_GOLD)
			_sntprintf(szDescribe,CountArray(szDescribe),TEXT("��Ҳ���") SCORE_STRING TEXT(",�޷�������Ϸ"),__max(lMinTableScore/100,lLessEnterScore/100));
		/*else if(m_pGameServiceOption->wServerType==GAME_GENRE_MATCH)
			_sntprintf(szDescribe,CountArray(szDescribe),TEXT("��Ҳ���") SCORE_STRING TEXT(",�޷�������Ϸ"),__max(lMinTableScore/100,lLessEnterScore/100));*/
		else
			_sntprintf(szDescribe,CountArray(szDescribe),TEXT("��Ҳ���") SCORE_STRING TEXT(",�޷�������Ϸ"),__max(lMinTableScore/100,lLessEnterScore/100));

		//������Ϣ
		SendRequestFailure(pIServerUserItem,szDescribe,REQUEST_FAILURE_NOSCORE);

		return false;
	}

	return true;
}

//bool CTableFrame::SetMatchInterface(IUnknownEx * pIUnknownEx)
//{
//	//LOGI(L"CTableFrame::SetMatchInterface");
//
//	return NULL;
//}

//������
//bool CTableFrame::CheckDistribute()
//{
//	//LOGI(L"CTableFrame::CheckDistribute");
//
//	//������
//	if(CServerRule::IsAllowAvertCheatMode(m_pGameServiceOption->dwServerRule))
//	{
//		//����״��
//		tagTableUserInfo TableUserInfo;
//		WORD wUserSitCount=GetTableUserInfo(TableUserInfo);
//
//		//�û�����
//		if(wUserSitCount < TableUserInfo.wMinUserCount)
//		{
//			return true;
//		}
//	}
//
//	return false;
//}

//��Ϸ��¼
//void CTableFrame::RecordGameScore(bool bDrawStarted)
//{
//	//LOGI(L"CTableFrame::RecordGameScore");
//
//	if (bDrawStarted==true)
//	{
//		//д���¼
//		if (CServerRule::IsRecordGameScore(m_pGameServiceOption->dwServerRule)==true)
//		{
//			//��������
//			DBR_GR_GameScoreRecord GameScoreRecord;
//			ZeroMemory(&GameScoreRecord,sizeof(GameScoreRecord));
//
//			//���ñ���
//			GameScoreRecord.wTableID=m_wTableID;
//			GameScoreRecord.dwPlayTimeCount=(bDrawStarted==true)?(DWORD)time(NULL)-m_dwDrawStartTime:0;
//
//			//��Ϸʱ��
//			GameScoreRecord.SystemTimeStart=m_SystemTimeStart;
//			GetLocalTime(&GameScoreRecord.SystemTimeConclude);
//
//			//�û�����
//			for (INT_PTR i=0;i<m_GameScoreRecordActive.GetCount();i++)
//			{
//				//��ȡ����
//				ASSERT(m_GameScoreRecordActive[i]!=NULL);
//				tagGameScoreRecord * pGameScoreRecord=m_GameScoreRecordActive[i];
//
//				//�û���Ŀ
//				if (pGameScoreRecord->cbAndroid==FALSE)
//				{
//					GameScoreRecord.wUserCount++;
//				}
//				else
//				{
//					GameScoreRecord.wAndroidCount++;
//				}
//
//				//����ͳ��
//				//GameScoreRecord.dwUserMemal+=pGameScoreRecord->dwUserMemal;
//				GameScoreRecord.dwUserMemal = 0;
//
//				//ͳ����Ϣ
//				if (pGameScoreRecord->cbAndroid==FALSE)
//				{
//					GameScoreRecord.lWasteCount-=(pGameScoreRecord->lScore+pGameScoreRecord->lRevenue);
//					GameScoreRecord.lRevenueCount+=pGameScoreRecord->lRevenue;
//				}
//
//				//��Ϸ����Ϣ
//				if (GameScoreRecord.wRecordCount<CountArray(GameScoreRecord.GameScoreRecord))
//				{
//					WORD wIndex=GameScoreRecord.wRecordCount++;
//					CopyMemory(&GameScoreRecord.GameScoreRecord[wIndex],pGameScoreRecord,sizeof(tagGameScoreRecord));
//				}
//			}
//
//			//Ͷ������
//			if(GameScoreRecord.wUserCount > 0)
//			{
//				WORD wHeadSize=sizeof(GameScoreRecord)-sizeof(GameScoreRecord.GameScoreRecord);
//				WORD wDataSize=sizeof(GameScoreRecord.GameScoreRecord[0])*GameScoreRecord.wRecordCount;
//				m_pIRecordDataBaseEngine->PostDataBaseRequest(DBR_GR_GAME_SCORE_RECORD,0,&GameScoreRecord,wHeadSize+wDataSize);
//			}
//		}
//
//		//������¼
//		if (m_GameScoreRecordActive.GetCount()>0L)
//		{
//			m_GameScoreRecordBuffer.Append(m_GameScoreRecordActive);
//			m_GameScoreRecordActive.RemoveAll();
//		}
//	}
//
//}

void CTableFrame::QueryGameParas(std::wstring key, std::wstring &value)
{
	if (!m_pIMainServiceFrame)
	{
		LOGE(L"QueryGameParas:m_pIMainServiceFrame = NULL");
		return;
	}

	CAttemperEngineSink * pAttemperEngineSink=(CAttemperEngineSink *)m_pIMainServiceFrame;
	if(!pAttemperEngineSink)
	{
		LOGE(L"QueryGameParas: pAttemperEngineSink = NULL");
		return;
	}

	pAttemperEngineSink->QueryGameParas(key,value);

}

void CTableFrame::QuickClearTable(IServerUserItem* pIServerUserItem)
{

	m_TableUserItemArray[pIServerUserItem->GetChairID()] = NULL;

	ConcludeGame(m_cbGameStatus);
	ConcludeTable();

	//clear �û�״̬
	tagUserInfo * UserInfo = pIServerUserItem->GetUserInfo();
	web::http::uri_builder builder;
	builder.append_query(U("action"),U("userLogout"));

	TCHAR parameterStr[50];
	_sntprintf(parameterStr,sizeof(parameterStr),TEXT("%d,%s,%d"),m_pGameServiceOption->wKindID,
		m_pGameServiceOption->szRoomType, UserInfo->dwUserID);

	builder.append_query(U("parameter"),parameterStr);
	utility::string_t v;
	std::map<wchar_t*,wchar_t*> hAdd;
	LOGI(L"userLogout" << parameterStr);
	if(HttpRequest(web::http::methods::POST,CServiceUnits::g_pServiceUnits->m_InitParameter.m_LobbyURL,builder.to_string(),true,v,hAdd))
	{
		web::json::value root = web::json::value::parse(v);
		utility::string_t actin = root[U("action")].as_string();
		if (root[U("errorCode")] == 0)
		{
			//�û�״̬
			pIServerUserItem->SetClientReady(false);
			pIServerUserItem->SetUserStatus(US_NULL,pIServerUserItem->GetTableID(),pIServerUserItem->GetChairID());
		}
		else
		{
			LOGW(L"clear user status fail");
		}
	}
	else
	{
		LOGW(L"clear user status fail");
	}

}

//ȡ��
bool CTableFrame::QuickGetScore(DWORD account, double & score)
{
	CAttemperEngineSink * pAttemperEngineSink=(CAttemperEngineSink *)m_pIMainServiceFrame;
	if(!pAttemperEngineSink)
	{
		LOGE(L"QuickGetScore: pAttemperEngineSink = NULL");
		return false;
	}
	return pAttemperEngineSink->QuickGetScore(account, score);
}

//д��
bool CTableFrame::QuickWriteScore(DWORD accountId, DWORD customerId, double score, std::wstring orderNo, std::wstring remark, int typeId)
{
	CAttemperEngineSink * pAttemperEngineSink=(CAttemperEngineSink *)m_pIMainServiceFrame;
	if(!pAttemperEngineSink)
	{
		LOGE(L"QuickWriteScore: pAttemperEngineSink = NULL");
		return false;
	}
	return pAttemperEngineSink->QuickWriteScore(accountId,customerId,score,orderNo,remark,typeId);
}

bool CTableFrame::kickPlayer(IServerUserItem * pIServerUserItem){
	
	try{
		CAttemperEngineSink * pAttemperEngineSink=(CAttemperEngineSink *)m_pIMainServiceFrame;
		//ͬ����
		CWHDataLocker dataLocker(pAttemperEngineSink->m_CriticalSection); 					

		WORD wBindIndex = pIServerUserItem->GetBindIndex();
		tagBindParameter * pBindParameter = pAttemperEngineSink->GetBindParameter(wBindIndex);
		pAttemperEngineSink->m_pITCPNetworkEngine->CloseSocket(pBindParameter->dwSocketID);
		//ɾ�������Ѷ
		pAttemperEngineSink->DeleteWaitDistribute(pIServerUserItem);
		pAttemperEngineSink->m_ServerUserManager.DeleteUserItem(pIServerUserItem);
		return true;
	}catch(...){
		CString outStr;
		outStr.Format(L"szAccount: %s Fail", pIServerUserItem->GetUserInfo()->szAccount);
		::OutputDebugString(outStr);

		return false;
	}
}