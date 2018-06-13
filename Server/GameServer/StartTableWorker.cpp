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
			BOOL isDismissTable = tableWoker.isUserDissconnect(idleTable);	//����Ƿ����û�/������
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
							_sntprintf(outStr, CountArray(outStr), L"�� %s(%s) ���AI POOL", user->GetUserInfo()->szAccount, user->GetUserInfo()->szNickName);
							::OutputDebugStringW(outStr);

							pAndroidWaitList->addTail(user);
						}else if(user->IsAccompanyUser()){
							TCHAR outStr[1024] = {0};
							_sntprintf(outStr, CountArray(outStr), L"�� %s(%s) ������ POOL", user->GetUserInfo()->szAccount, user->GetUserInfo()->szNickName);
							::OutputDebugStringW(outStr);

							pAccompanyWaitList->addTail(user);
						}else{
							TCHAR outStr[1024] = {0};
							_sntprintf(outStr, CountArray(outStr), L"�� %s(%s) ���USER POOL", user->GetUserInfo()->szAccount, user->GetUserInfo()->szNickName);
							::OutputDebugStringW(outStr);

							pUserWaitList->addTail(user);
						}
						break;
					}
				}
				::OutputDebugStringA("��ɢ����");
				delete idleTable;
				isRemoveTable = TRUE;	//�Ƴ�������
			}else if(idleTable->getTableUserCount() == maxUser){	//��������������
				tableWoker.readyToStartTable(pDistributeTableList, pStartTableList, idleTable);
				isRemoveTable = TRUE;	//�Ƴ�������
			}else if(idleTable->isReadyToAssign()){	//����δ��������AI�����
				::OutputDebugStringA("��������û�");
				tableWoker.insertAccompanyUser(idleTable, pAccompanyWaitList, maxUser);	//�������
				if(idleTable->getTableUserCount() < maxUser){	//������������δ��������AI
					::OutputDebugStringA("����AI�û�");
					tableWoker.insertAccompanyUser(idleTable, pAndroidWaitList, maxUser);	//����AI
				}
				if(idleTable->getTableUserCount() >= minUser){	//������С��������������
					//::OutputDebugStringA("������С����������");
					tableWoker.readyToStartTable(pDistributeTableList, pStartTableList, idleTable);
					isRemoveTable = TRUE;
				}
			}
			if(isRemoveTable){	//�Ƴ����Ӳ�ȡ����һ��
				::OutputDebugStringA("�Ƴ����Ӳ�ȡ����һ��");
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
	//����Ƿ�����ɱ��
	if(idleTable->isSatisfyKill()){		//����ɱ�����趨����ֵ
		idleTable->setUserLuckyValue();
	}else{									//������ɱ�����趨Ϊԭʼ����ֵ
		idleTable->setUserOriginalLuckyValue();
	}
	pStartTableList->addTail(idleTable);
	pDistributeTableList->removeElement(idleTable);
	return;
}

void CStartTableWorker::insertAccompanyUser(CDistributeTable *idleTable, CListManager<IServerUserItem*> *pAccompanyWaitList, int maxUser){
	if(pAccompanyWaitList->getCount() <= 0){	//û����� or AI����
		::OutputDebugStringA("No ai or Accompany user!");
		return;
	}
	IServerUserItem *accompanyUser;
	while(pAccompanyWaitList->getAndPopFirstElement(accompanyUser) && idleTable->getTableUserCount() < maxUser){	//����AI or ���
		if(!idleTable->insertUser(accompanyUser)){
			pAccompanyWaitList->addTail(accompanyUser);
			return;
		}
	}
	return;
}

BOOL CStartTableWorker::isUserDissconnect(CDistributeTable *idleTable){
	for(auto tableUser = idleTable->tableSeat.begin(); tableUser != idleTable->tableSeat.end(); ++tableUser){	//����Ƿ������û�������
		IServerUserItem *user;
		user = *tableUser;
		if(user->IsAndroidUser()){
			continue;
		}
		switch(user->GetUserStatus()){	//�������Ҽ���Ƿ񻹴���
		case US_NULL:
		case US_OFFLINE:
			return TRUE;
		}
	}
	return FALSE;
}