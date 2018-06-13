#include "Stdafx.h"
#include "StartTableWorker.h"



CWinThread* CStartTableWorker::start(CDistributeHandler* tempClass){
	return AfxBeginThread(this->startThread,(LPVOID)tempClass);
}


UINT CStartTableWorker::startThread(LPVOID lparam){
	CDistributeHandler *tempClass = (CDistributeHandler*) lparam;
	
	CListManager<IServerUserItem*>  *pAndroidWaitList = tempClass->pAndroidWaitList;
	CListManager<IServerUserItem*>	*pUserWaitList = tempClass->pUserWaitList;
	CListManager<IServerUserItem*>  *pAccompanyWaitList = tempClass->pAccompanyWaitList;
	CListManager<CDistributeTable*>  *pStartTableList = tempClass->pStartTableList;
	CListManager<CDistributeTable*>  *pDistributeTableList = tempClass->pDistributeTableList;
	int maxUser = tempClass->maxUser;
	int minUser = tempClass->minUser;

	CStartTableWorker tableWoker;
	while(TRUE){
		pDistributeTableList->processLock.Lock();
		//Mutex start
		CDistributeTable *idleTable;
		idleTable = pDistributeTableList->getFirstElement();
		while(idleTable != NULL){
			BOOL isRemoveTable = FALSE;
			BOOL isDismissTable = tableWoker.isUserDissconnect(idleTable);	//检查是否有用户/陪打掉线
			if(isDismissTable){
				for(auto tableUser = idleTable->tableSeat.begin(); tableUser != idleTable->tableSeat.end(); ++tableUser){
					IServerUserItem *user = *tableUser;
					switch(user->GetUserStatus()){
					case US_NULL:
					case US_OFFLINE:
						break;
					default:
						if(user->IsAndroidUser()){
							TCHAR outStr[1024] = {0};
							_sntprintf(outStr, CountArray(outStr), L"将 %s(%s) 打回AI POOL", user->GetUserInfo()->szAccount, user->GetUserInfo()->szNickName);
							::OutputDebugStringW(outStr);

							pAndroidWaitList->addTail(user);
						}else if(user->IsAccompanyUser()){
							TCHAR outStr[1024] = {0};
							_sntprintf(outStr, CountArray(outStr), L"将 %s(%s) 打回陪打 POOL", user->GetUserInfo()->szAccount, user->GetUserInfo()->szNickName);
							::OutputDebugStringW(outStr);

							pAccompanyWaitList->addTail(user);
						}else{
							TCHAR outStr[1024] = {0};
							_sntprintf(outStr, CountArray(outStr), L"将 %s(%s) 打回USER POOL", user->GetUserInfo()->szAccount, user->GetUserInfo()->szNickName);
							::OutputDebugStringW(outStr);

							pUserWaitList->addTail(user);
						}
						break;
					}
				}
				::OutputDebugStringA("解散该桌");
				delete idleTable;
				isRemoveTable = TRUE;	//移除该桌子
			}else if(idleTable->getTableUserCount() == maxUser){	//人数已满，开局
				tableWoker.readyToStartTable(pDistributeTableList, pStartTableList, idleTable);
				isRemoveTable = TRUE;	//移除该桌子
			}else if(idleTable->isReadyToAssign()){	//人数未满，放入AI或陪打
				::OutputDebugStringA("插入陪打用户");
				tableWoker.insertAccompanyUser(idleTable, pAccompanyWaitList, maxUser);	//放入陪打
				if(idleTable->getTableUserCount() < maxUser){	//放入陪打后人数未满，放入AI
					::OutputDebugStringA("插入AI用户");
					tableWoker.insertAccompanyUser(idleTable, pAndroidWaitList, maxUser);	//放入AI
				}
				if(idleTable->getTableUserCount() >= minUser){	//满足最小开局人数，开局
					//::OutputDebugStringA("满足最小人数，开局");
					tableWoker.readyToStartTable(pDistributeTableList, pStartTableList, idleTable);
					isRemoveTable = TRUE;
				}
			}
			if(isRemoveTable){	//移除桌子并取得下一桌
				::OutputDebugStringA("移除桌子并取得下一桌");
				CDistributeTable *tmpTable;
				tmpTable = pDistributeTableList->getNextElement(idleTable);
				pDistributeTableList->removeElement(idleTable);
				idleTable = tmpTable;
			}else{
				//::OutputDebugStringA("Get next idle table");
				idleTable = pDistributeTableList->getNextElement(idleTable);
			}
		}
		////Mutex end
		pDistributeTableList->processLock.UnLock();
		Sleep(1000);
	}
	return 0;
}

void CStartTableWorker::readyToStartTable(CListManager<CDistributeTable*>  *pDistributeTableList, CListManager<CDistributeTable*> *pStartTableList, CDistributeTable *idleTable){
	//检查是否启用杀数
	if(idleTable->isSatisfyKill()){		//启用杀数，设定幸运值
		idleTable->setUserLuckyValue();
	}else{									//不启用杀数，设定为原始幸运值
		idleTable->setUserOriginalLuckyValue();
	}
	pStartTableList->addTail(idleTable);
	pDistributeTableList->removeElement(idleTable);
	return;
}

void CStartTableWorker::insertAccompanyUser(CDistributeTable *idleTable, CListManager<IServerUserItem*> *pAccompanyWaitList, int maxUser){
	if(pAccompanyWaitList->getCount() <= 0){	//没有陪打 or AI可用
		::OutputDebugStringA("No ai or Accompany user!");
		return;
	}
	IServerUserItem *accompanyUser;
	while(pAccompanyWaitList->getAndPopFirstElement(accompanyUser) && idleTable->getTableUserCount() < maxUser){	//放入AI or 陪打
		if(!idleTable->insertUser(accompanyUser)){
			pAccompanyWaitList->addTail(accompanyUser);
			return;
		}
	}
	return;
}

BOOL CStartTableWorker::isUserDissconnect(CDistributeTable *idleTable){
	for(auto tableUser = idleTable->tableSeat.begin(); tableUser != idleTable->tableSeat.end(); ++tableUser){	//检查是否所有用户都存在
		IServerUserItem *user;
		user = *tableUser;
		if(user->IsAndroidUser()){
			continue;
		}
		switch(user->GetUserStatus()){	//陪打与玩家检查是否还存在
		case US_NULL:
		case US_OFFLINE:
			return TRUE;
		}
	}
	return FALSE;
}