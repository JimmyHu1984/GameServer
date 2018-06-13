#include "Stdafx.h"
#include "DistributeWorker.h"
#include "DistributeHandler.h"


CWinThread* CDistributeWorker::start(CDistributeHandler* tempClass){
	return AfxBeginThread(this->startThread,(LPVOID)tempClass);
}

UINT CDistributeWorker::startThread(LPVOID lparam){
	CDistributeHandler *tempClass = (CDistributeHandler*) lparam;
	
	//CListManager<IServerUserItem*>  *pAndroidWaitList = tempClass->pAndroidWaitList;
	CListManager<IServerUserItem*>	*pUserWaitList = tempClass->pUserWaitList;
	//CListManager<IServerUserItem*>  *pAccompanyWaitList = tempClass->pAccompanyWaitList;
	//CListManager<CDistributeTable>  *pStartTableList = tempClass->pStartTableList;
	CListManager<CDistributeTable*>  *pDistributeTableList = tempClass->pDistributeTableList;
	int maxUser = tempClass->maxUser;
	int minUser = tempClass->minUser;
	CDistributeWorker distributeWoker;
	CStringA outStr;
	while(TRUE){
		pDistributeTableList->processLock.Lock();
		IServerUserItem *waitUser;
		if(pUserWaitList->getAndPopFirstElement(waitUser)){	//有等待中的用户
			distributeWoker.distributeUser(waitUser, pDistributeTableList, minUser, maxUser);
		}
		pDistributeTableList->processLock.UnLock();
		Sleep(1000);
	}
	
	return 0;
}

/**
	配桌
*/
void CDistributeWorker::distributeUser(IServerUserItem *waitUser, CListManager<CDistributeTable*> *pDistributeTableList, int minUser, int maxUser){
//	DebugString("User inside LINE:" << __LINE__);
	CDistributeTable *table;
	table = pDistributeTableList->getFirstElement();
	while(table != NULL){
		if(table->isUserCanEnterTable(waitUser)){	//安排入桌
			return;
		}
		table = pDistributeTableList->getNextElement(table);
	}
	table = new CDistributeTable(waitUser, minUser, maxUser);	//开新桌
	pDistributeTableList->addTail(table);	//放入LIST
	
	return;
}