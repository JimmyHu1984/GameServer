#ifndef DISTRIBUTE_WORKER_H
#define DISTRIBUTE_WORKER_H
#pragma once

#include "Stdafx.h"
#include "DistributeHandler.h"
#include "ThreadWorker.h"

class CDistributeWorker: public CThreadWorker{
public:
	virtual CWinThread* start(CDistributeHandler* tempClass);
	static UINT startThread(LPVOID lparam);			//开始工作

	void distributeUser(IServerUserItem *waitUser, CListManager<CDistributeTable*> *pDistributeTableList, int minUser, int maxUser);
};

#endif