#include "Stdafx.h"
#include "DistributeHandler.h"
#include "StartTableWorker.h"
#include "DistributeWorker.h"
#include "ThreadWorker.h"

CDistributeHandler::CDistributeHandler(){
	static CDistributeWorker* temp_1 = new CDistributeWorker(); 
	this->workerPool.addTail(temp_1);
	static CStartTableWorker* temp_2 = new CStartTableWorker(); 
	this->workerPool.addTail(temp_2);
	
}
CDistributeHandler::~CDistributeHandler(){
	CWinThread* currentThreadPtr;
	while(this->threadPool.getAndPopFirstElement(currentThreadPtr)){
		TerminateThread(currentThreadPtr->m_hThread,0);
		CloseHandle(currentThreadPtr->m_hThread);
		WaitForSingleObject(currentThreadPtr->m_hThread, INFINITE);
	}

	CThreadWorker* currentObjectPtr;
	while(this->workerPool.getAndPopFirstElement(currentObjectPtr)){
		delete(currentObjectPtr);
	}
}

BOOL CDistributeHandler::bindPoolAndParameter(CListManager<IServerUserItem*> &tempAndroidWaitList, CListManager<IServerUserItem*> &tempUserWaitList, 
		CListManager<IServerUserItem*> &tempAccompanyWaitList, CListManager<CDistributeTable*> &tempStartTableList,
		CListManager<CDistributeTable*> &m_DistributeTableList, int tempMaxUser, int tempMinUser){

	this->initialParamter();

	this->pAndroidWaitList = &tempAndroidWaitList;
	this->pUserWaitList = &tempUserWaitList;
	this->pAccompanyWaitList = &tempAccompanyWaitList;
	this->pStartTableList = &tempStartTableList;
	this->pDistributeTableList = &m_DistributeTableList;

	this->clearTablePool();

	this->maxUser = tempMaxUser;
	this->minUser = tempMinUser;

	if(this->pAndroidWaitList == NULL){
		return FALSE;
	}
	return TRUE;
}

void CDistributeHandler::clearTablePool(){
	this->pStartTableList->removeAll();
	this->pDistributeTableList->removeAll();
}

void CDistributeHandler::initialParamter(){
	this->pAndroidWaitList = NULL;
	this->pUserWaitList = NULL;
	this->pAccompanyWaitList = NULL;
	this->pStartTableList = NULL;
	this->pDistributeTableList = NULL;

	this->maxUser = 0;
	this->minUser = 0;
}

BOOL CDistributeHandler::checkParameter(){
	if(this->pAndroidWaitList == NULL){
		::OutputDebugStringA("this->pAndroidWaitList == NULL");
		return FALSE;
	}
	if(this->pUserWaitList == NULL){
		::OutputDebugStringA("this->pUserWaitList == NULL");
		return FALSE;
	}
	if(this->pAccompanyWaitList == NULL){
		::OutputDebugStringA("this->pAccompanyWaitList == NULL");
		return FALSE;
	}
	if(this->pStartTableList == NULL){
		::OutputDebugStringA("this->pStartTableList == NULL");
		return FALSE;
	}
	if(this->pDistributeTableList == NULL){
		::OutputDebugStringA("this->pDistributeTableList == NULL");
		return FALSE;
	}
	if(this->maxUser == 0){
		::OutputDebugStringA("this->maxUser == 0");
		return FALSE;
	}
	if(this->minUser == 0){
		::OutputDebugStringA("this->minUser == 0");
		return FALSE;
	}
	return TRUE;
}


BOOL CDistributeHandler::startThread(){
	::OutputDebugStringA("CDistributeHandler::startThread()");
	if(!this->checkParameter()){
		::OutputDebugStringA("Check parameter fail.");
		return FALSE;
	}

	CThreadWorker* currentPtr = this->workerPool.getFirstElement();
	while(currentPtr != NULL){
		threadPool.addTail(currentPtr->start(this));
		currentPtr = this->workerPool.getNextElement(currentPtr);
	}
	return TRUE;
}


BOOL CDistributeHandler::stopThread(){
	::OutputDebugStringA("CDistributeHandler::stopThread()");
	CWinThread* currentThreadPtr;
	while(this->threadPool.getAndPopFirstElement(currentThreadPtr)){
		TerminateThread(currentThreadPtr->m_hThread,0);
		CloseHandle(currentThreadPtr->m_hThread);
		WaitForSingleObject(currentThreadPtr->m_hThread, INFINITE);
	}
	return TRUE;
}
