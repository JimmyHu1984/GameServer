#ifndef START_TABLE_WORKER_H
#define START_TABLE_WORKER_H
#pragma once

#include "Stdafx.h"
#include "DistributeHandler.h"
#include "ThreadWorker.h"

class CStartTableWorker: public CThreadWorker{
public:
	virtual CWinThread* start(CDistributeHandler* tempClass);
	static UINT startThread(LPVOID lparam);			//开始工作

	void readyToStartTable(CListManager<CDistributeTable*>  *pDistributeTableList, CListManager<CDistributeTable*> *pStartTableList, CDistributeTable *idleTable);
	void insertAccompanyUser(CDistributeTable *idleTable, CListManager<IServerUserItem*> *pAccompanyWaitList, int maxUser);
	BOOL isUserDissconnect(CDistributeTable *idleTable);
};

#endif