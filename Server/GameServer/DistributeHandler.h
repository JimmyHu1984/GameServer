#ifndef DISTRIBUTE_HANDLER_H
#define DISTRIBUTE_HANDLER_H
#include "Stdafx.h"
#include "ListManager.h"
#include "DistributeTable.h"
#include "GameServiceHead.h"


class CThreadWorker;

class CDistributeHandler{
public:
	CDistributeHandler();
	~CDistributeHandler();
	BOOL bindPoolAndParameter(CListManager<IServerUserItem*> &tempAndroidWaitList, CListManager<IServerUserItem*> &tempUserWaitList, 
		CListManager<IServerUserItem*> &tempAccompanyWaitList, CListManager<CDistributeTable*> &tempStartTableList,
		CListManager<CDistributeTable*> &m_DistributeTableList, int tempMaxUser, int tempMinUser);
	BOOL startThread();
	BOOL stopThread();

	CWinThread* pThreadDistributeUsher;
	CWinThread* pThreadStartGameUsher;

	int maxUser;
	int minUser;

	CListManager<IServerUserItem*>  *pAndroidWaitList;					//�����˵ȴ�POOL
	CListManager<IServerUserItem*>	*pUserWaitList;						//�ȴ�����POOL
	CListManager<IServerUserItem*>  *pAccompanyWaitList;                //���ȴ�POOL
	CListManager<CDistributeTable*>  *pStartTableList;					//�ȴ���������POOL
	CListManager<CDistributeTable*>  *pDistributeTableList;		        //�ȴ���������POOL
	
private:
	void initialParamter();
	void clearTablePool();
	BOOL checkParameter();
	CListManager<CWinThread*>		threadPool;
	CListManager<CThreadWorker*>	workerPool;
};


#endif