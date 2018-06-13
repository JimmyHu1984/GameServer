#include "Stdafx.h"
#include "DistributeTable.h"
/**
	
*/
/*
CDistributeTable::CDistributeTable(){
	this->createTime = ::GetTickCount64() / 1000L;
	this->killNum = 100;
	this->tableUserCount = 0;
	this->merchantName = L"";
	this->tableMinUser = 0;
	this->tableMaxUser = 0;
}
*/
CDistributeTable::CDistributeTable(IServerUserItem *user, int minUser, int maxUser){
	this->createTime = ::GetTickCount64() / 1000L;
	this->killNum = 100;
	this->tableUserCount = 0;
	this->insertUser(user);
	this->isAllowMixMerchant = !user->GetUserInfo()->isEnableKill;	//�Ե�һλ������û����������Ƿ���Ի���
	this->killNum = user->GetUserInfo()->killNum;
	this->setMerchantName(user->GetUserInfo()->channelName);		//Ʒ���趨Ϊ��һλ������û�Ʒ��
	this->tableMinUser = minUser;
	this->tableMaxUser = maxUser;
	this->tableAllowMerchant.clear();
	/*
	for(auto itor = user->GetUserInfo()->userMixMerchant.begin(); itor != user->GetUserInfo()->userMixMerchant.end(); ++itor){	//��������Ʒ��
		::OutputDebugStringW(itor->first.c_str());
		this->tableAllowMerchant.insert(*itor);
	}
	*/
	this->tableAllowMerchant.insert(std::pair<std::wstring, DWORD>(user->GetUserInfo()->userMixMerchant, 0));
}
/**
	�Ƚ�Ʒ������
	Input:
		tmpStr	- Ʒ������
	Output:
		TRUE	- Ʒ������һ��
		FALSE	- Ʒ�����Ʋ�һ��
*/
BOOL CDistributeTable::compareMerchantCode(TCHAR *tmpStr){
	try{
		if(!_tcscmp(merchantName.c_str(), tmpStr)){
			return TRUE;
		}
		return FALSE;
	}catch(...){
		::OutputDebugStringA("compareMerchantCode fail");
		return FALSE;
	}
}
/**
	Ѱ���û�
	Input:
		tempUser	- Ҫ�ҵ��û�
	Output:
		TRUE		- ����
		FALSE		- ������
*/
BOOL CDistributeTable::findUser(IServerUserItem *tempUser){
	try{
		for(auto iter = tableSeat.begin(); iter != tableSeat.end(); ++iter){
			if(*iter == tempUser){
				return TRUE;
			}
		}
		return FALSE;
	}catch(...){
		::OutputDebugStringA("findUser fail");
		return FALSE;
	}
}
/**
	����ʹ����
	Input:
		tempUser	- Ҫ������û�
	Output:
		TRUE		- ����ɹ�
		FALSE		- ����ʧ��
*/
BOOL CDistributeTable::insertUser(IServerUserItem *tempUser){
	TCHAR strOut[1024];
	try{
		for(auto iter = tableSeat.begin(); iter != tableSeat.end(); ++iter){
			if(*iter == tempUser){
				_sntprintf(strOut, CountArray(strOut), L"User Already exist. User: %d. LINE: %d.", tempUser->GetUserID(), __LINE__);
				OutputDebugString(strOut);
				//::OutputDebugStringA("User Already exist. User:" << tempUser->GetUserID());
				return FALSE;
			}
		}
		tableSeat.push_back(tempUser);
		++this->tableUserCount;
		return TRUE;
	}catch(...){
		::OutputDebugStringA("insertUser fail");
		return FALSE;
	}
}
/**
	�û��Ƿ��������?
*/
BOOL CDistributeTable::isUserCanEnterTable(IServerUserItem *tempUser){
	if(this->tableUserCount >= this->tableMaxUser){		//����
		return FALSE;
	}
	if(wcscmp(tempUser->GetUserInfo()->userMixMerchant, L"ALL") == 0){		//���û��ɻ���

		auto tableMixMerchant = this->tableAllowMerchant.find(L"ALL");
		if(tableMixMerchant != this->tableAllowMerchant.end()){				//�����ɻ���������
		
			this->insertUser(tempUser);
			return TRUE;
		}else{																//�����������ض�Ʒ�ƣ����û���Ʒ�ƿɲ���������
		
			std::wstring userMerchant = tempUser->GetUserInfo()->channelName;
			tableMixMerchant = this->tableAllowMerchant.find(userMerchant);
			if(tableMixMerchant != this->tableAllowMerchant.end()){			//�������Խ����û���Ʒ��
				
				this->insertUser(tempUser);
				return TRUE;
			}
		}
		return FALSE;
	}else{	//�û����ɻ������������ɽ��ܵ�Ʒ���Ƿ��и��û���Ʒ��

		std::wstring userMerchant = tempUser->GetUserInfo()->channelName;
		auto merchantItor = this->tableAllowMerchant.find(userMerchant);
		if(merchantItor != this->tableAllowMerchant.end()){				//�������Խ����û���Ʒ�ƣ�����
		
			this->insertUser(tempUser);
			merchantItor = tableAllowMerchant.begin();
			/*
			//���Ŷ�Ʒ��������������ӿ��������Ʒ��
			while(merchantItor != tableAllowMerchant.end()){			//�������ӿ������Ʒ��
				auto userMerchant = tempUser->GetUserInfo()->userMixMerchant.find(merchantItor->first);
				if(userMerchant == tempUser->GetUserInfo()->userMixMerchant.end()){	//�Ҳ�����Ʒ�ƣ��Ƴ���Ʒ��
					auto nextItor = merchantItor;
					++nextItor;
					this->tableAllowMerchant.erase(merchantItor);
					merchantItor = nextItor;
				}else{
					++merchantItor;
				}
			}
			*/
			return TRUE;
		}
	}
	return FALSE;
}
/*
BOOL CDistributeTable::isUserCanEnterTable(IServerUserItem *tempUser){
	if(this->tableUserCount >= this->tableMaxUser){		//����
		::OutputDebugStringA("����������������");
		return FALSE;
	}
	if(tempUser->GetUserInfo()->isEnableKill){				//���û�����ɱ��ֵ������Ѱ��ͬƷ�Ƶ�����
		::OutputDebugStringA("�û����ɻ�����Ѱ�ҿ��Խ��������");
		if(!this->compareMerchantCode(tempUser->GetUserInfo()->channelName)){			//��ͬƷ�ƣ���������
			return FALSE;
		}
		for(auto itor = this->tableSeat.begin(); itor != this->tableSeat.end(); ++itor){
			IServerUserItem *tableUser;
			tableUser = *itor;
			if(tableUser->IsAndroidUser() || tableUser->IsAccompanyUser()){
				continue;
			}
			if(wcscpy(tempUser->GetUserInfo()->channelName, tableUser->GetUserInfo()->channelName) != 0){	//������������Ʒ�����
				return FALSE;
			}
			if(tempUser->GetUserInfo()->killNum != tableUser->GetUserInfo()->killNum){	//ͬƷ�Ƶ�ɱ������ͬ
				return FALSE;
			}
			this->insertUser(tempUser);
			::OutputDebugStringA("ͬƷ���û�����");
			return TRUE;
		}
	}else{	//�û�δ����ɱ�������Ի���
		::OutputDebugStringA("�û��ɻ�����Ѱ�ҿ��Խ��������");
		if(this->isAllowMixMerchant){	//�����ӿɻ�����ֱ������
			::OutputDebugStringA("�ɻ������û�����");
			this->insertUser(tempUser);
			return TRUE;
		}else{
			::OutputDebugStringA("���������Ի������ܾ��û�����");
		}
	}
	return FALSE;
}
*/
/**
	�Ƿ���԰�������AI����
	Output:
		TRUE	- ���԰����������������
		FALSE	- ���ɰ����������������
*/
BOOL CDistributeTable::isReadyToAssign(){
	if(createTime == WAIT_DISTRIBUTE_TIME_UNLIMIT){
		return FALSE;
	}
	if((GetTickCount64() / 1000L) - createTime > WAIT_DISTRIBUTE_TIME){
		return TRUE;
	}
	return FALSE;
}
/**
	�Ƿ�����ɱ����������?
	1. ������ҳ������ or AI����ͬƷ��
*/
BOOL CDistributeTable::isSatisfyKill(){
	if(this->isAllowMixMerchant){	//�ɻ�����������ɱ��
		return FALSE;
	}
	BOOL isTableHaveAccompanyUser = FALSE;
	BOOL isTableMerchantAllSame = TRUE;
	for(auto itor = this->tableSeat.begin(); itor != this->tableSeat.end(); ++itor){
		IServerUserItem *user = *itor;
		if(user->IsAndroidUser() || user->IsAccompanyUser()){	//��� �� AI
			isTableHaveAccompanyUser = TRUE;
			continue;
		}else{
			if(!this->compareMerchantCode(user->GetUserInfo()->channelName)){	//��ͬƷ��
				isTableMerchantAllSame = FALSE;
			}
		}
	}
	if(isTableHaveAccompanyUser && isTableMerchantAllSame){	//�������û�����ͬƷ����������AI
		return TRUE;
	}
	return FALSE;
}
/**
	�趨Ʒ������
	Input:
		tmpStr	- Ʒ������
	Output:
		TRUE	- �趨�ɹ�
		FALSE	- �趨ʧ��
*/
BOOL CDistributeTable::setMerchantName(TCHAR* tmpStr){
	try{
		std::wstring newStr (tmpStr);
		this->merchantName = newStr;
		return TRUE;
	}catch(...){
		::OutputDebugStringA("setMerchantCode fail");
		return FALSE;
	}
}
/**
	����Ʒ��ɱ���趨����ֵ
*/
BOOL CDistributeTable::setUserLuckyValue(){
	UINT userCount = 0;
	UINT aiCount = 0;
	for(auto itor = tableSeat.begin(); itor != tableSeat.end(); ++itor){	//ͳ���û������/AI������
		IServerUserItem *user = *itor;
		if(user->IsAndroidUser() || user->IsAccompanyUser()){
			++aiCount;
		}else{
			++userCount;
		}
	}
	short playerLucky = 50;
	short aiLucky = 50;
	short luckyConcession = 100 - this->killNum;		//�÷�
	if(luckyConcession % 2 == 0){		//ż��
		playerLucky -= luckyConcession / 2;
		aiLucky += luckyConcession / 2;
	}else{								//����
		playerLucky -= (luckyConcession / 2) + 1;
		aiLucky += luckyConcession / 2;
	}
	playerLucky /= userCount;
	aiLucky /= aiCount;
	for(auto itor = tableSeat.begin(); itor != tableSeat.end(); ++itor){	//ͳ���û������/AI������
		IServerUserItem *user = *itor;
		if(user->IsAndroidUser() || user->IsAccompanyUser()){	//��� / AI
			user->GetUserInfo()->fortuneProb = aiLucky;
		}else{													//Ʒ���û�
			user->GetUserInfo()->fortuneProb = playerLucky;
		}
	}
	return TRUE;
}
/**
	������ɱ�����趨���Ϊ��ʼ������ֵ
*/
void CDistributeTable::setUserOriginalLuckyValue(){
	for(auto itor = this->tableSeat.begin(); itor != this->tableSeat.end(); ++itor){
		IServerUserItem *user = *itor;
		user->GetUserInfo()->fortuneProb = user->GetUserInfo()->originalFortuneProb;
	}
	return;
}