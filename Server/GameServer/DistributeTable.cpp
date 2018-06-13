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
	this->isAllowMixMerchant = !user->GetUserInfo()->isEnableKill;	//以第一位进入的用户决定该桌是否可以混桌
	this->killNum = user->GetUserInfo()->killNum;
	this->setMerchantName(user->GetUserInfo()->channelName);		//品牌设定为第一位进入的用户品牌
	this->tableMinUser = minUser;
	this->tableMaxUser = maxUser;
	this->tableAllowMerchant.clear();
	/*
	for(auto itor = user->GetUserInfo()->userMixMerchant.begin(); itor != user->GetUserInfo()->userMixMerchant.end(); ++itor){	//可入桌的品牌
		::OutputDebugStringW(itor->first.c_str());
		this->tableAllowMerchant.insert(*itor);
	}
	*/
	this->tableAllowMerchant.insert(std::pair<std::wstring, DWORD>(user->GetUserInfo()->userMixMerchant, 0));
}
/**
	比较品牌名称
	Input:
		tmpStr	- 品牌名称
	Output:
		TRUE	- 品牌名称一致
		FALSE	- 品牌名称不一致
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
	寻找用户
	Input:
		tempUser	- 要找的用户
	Output:
		TRUE		- 存在
		FALSE		- 不存在
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
	插入使用者
	Input:
		tempUser	- 要插入的用户
	Output:
		TRUE		- 插入成功
		FALSE		- 插入失败
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
	用户是否可以入桌?
*/
BOOL CDistributeTable::isUserCanEnterTable(IServerUserItem *tempUser){
	if(this->tableUserCount >= this->tableMaxUser){		//满人
		return FALSE;
	}
	if(wcscmp(tempUser->GetUserInfo()->userMixMerchant, L"ALL") == 0){		//该用户可混桌

		auto tableMixMerchant = this->tableAllowMerchant.find(L"ALL");
		if(tableMixMerchant != this->tableAllowMerchant.end()){				//该桌可混桌，入桌
		
			this->insertUser(tempUser);
			return TRUE;
		}else{																//该桌仅接受特定品牌，看用户的品牌可不可以入内
		
			std::wstring userMerchant = tempUser->GetUserInfo()->channelName;
			tableMixMerchant = this->tableAllowMerchant.find(userMerchant);
			if(tableMixMerchant != this->tableAllowMerchant.end()){			//该桌可以接受用户的品牌
				
				this->insertUser(tempUser);
				return TRUE;
			}
		}
		return FALSE;
	}else{	//用户不可混桌，检查该桌可接受的品牌是否有该用户的品牌

		std::wstring userMerchant = tempUser->GetUserInfo()->channelName;
		auto merchantItor = this->tableAllowMerchant.find(userMerchant);
		if(merchantItor != this->tableAllowMerchant.end()){				//该桌可以接受用户的品牌，入桌
		
			this->insertUser(tempUser);
			merchantItor = tableAllowMerchant.begin();
			/*
			//开放多品牌配桌需更新桌子可以允许的品牌
			while(merchantItor != tableAllowMerchant.end()){			//更新桌子可允许的品牌
				auto userMerchant = tempUser->GetUserInfo()->userMixMerchant.find(merchantItor->first);
				if(userMerchant == tempUser->GetUserInfo()->userMixMerchant.end()){	//找不到该品牌，移除该品牌
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
	if(this->tableUserCount >= this->tableMaxUser){		//满人
		::OutputDebugStringA("不可入桌桌子满人");
		return FALSE;
	}
	if(tempUser->GetUserInfo()->isEnableKill){				//该用户启用杀数值，必须寻找同品牌的桌子
		::OutputDebugStringA("用户不可混桌，寻找可以进入的牌桌");
		if(!this->compareMerchantCode(tempUser->GetUserInfo()->channelName)){			//不同品牌，不可入桌
			return FALSE;
		}
		for(auto itor = this->tableSeat.begin(); itor != this->tableSeat.end(); ++itor){
			IServerUserItem *tableUser;
			tableUser = *itor;
			if(tableUser->IsAndroidUser() || tableUser->IsAccompanyUser()){
				continue;
			}
			if(wcscpy(tempUser->GetUserInfo()->channelName, tableUser->GetUserInfo()->channelName) != 0){	//该桌内有其他品牌玩家
				return FALSE;
			}
			if(tempUser->GetUserInfo()->killNum != tableUser->GetUserInfo()->killNum){	//同品牌但杀数不相同
				return FALSE;
			}
			this->insertUser(tempUser);
			::OutputDebugStringA("同品牌用户入桌");
			return TRUE;
		}
	}else{	//用户未启用杀数，可以混桌
		::OutputDebugStringA("用户可混桌，寻找可以进入的牌桌");
		if(this->isAllowMixMerchant){	//该桌子可混桌，直接入桌
			::OutputDebugStringA("可混桌，用户入桌");
			this->insertUser(tempUser);
			return TRUE;
		}else{
			::OutputDebugStringA("该桌不可以混桌，拒绝用户进入");
		}
	}
	return FALSE;
}
*/
/**
	是否可以安排陪打或AI入桌
	Output:
		TRUE	- 可以安排陪打或机器人入桌
		FALSE	- 不可安排陪打或机器人入桌
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
	是否满足杀数启动条件?
	1. 整桌玩家除了陪打 or AI都是同品牌
*/
BOOL CDistributeTable::isSatisfyKill(){
	if(this->isAllowMixMerchant){	//可混桌，不开启杀数
		return FALSE;
	}
	BOOL isTableHaveAccompanyUser = FALSE;
	BOOL isTableMerchantAllSame = TRUE;
	for(auto itor = this->tableSeat.begin(); itor != this->tableSeat.end(); ++itor){
		IServerUserItem *user = *itor;
		if(user->IsAndroidUser() || user->IsAccompanyUser()){	//陪打 或 AI
			isTableHaveAccompanyUser = TRUE;
			continue;
		}else{
			if(!this->compareMerchantCode(user->GetUserInfo()->channelName)){	//不同品牌
				isTableMerchantAllSame = FALSE;
			}
		}
	}
	if(isTableHaveAccompanyUser && isTableMerchantAllSame){	//整桌的用户都是同品牌且有陪打或AI
		return TRUE;
	}
	return FALSE;
}
/**
	设定品牌名称
	Input:
		tmpStr	- 品牌名称
	Output:
		TRUE	- 设定成功
		FALSE	- 设定失败
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
	根据品牌杀数设定幸运值
*/
BOOL CDistributeTable::setUserLuckyValue(){
	UINT userCount = 0;
	UINT aiCount = 0;
	for(auto itor = tableSeat.begin(); itor != tableSeat.end(); ++itor){	//统计用户跟陪打/AI的人数
		IServerUserItem *user = *itor;
		if(user->IsAndroidUser() || user->IsAccompanyUser()){
			++aiCount;
		}else{
			++userCount;
		}
	}
	short playerLucky = 50;
	short aiLucky = 50;
	short luckyConcession = 100 - this->killNum;		//让分
	if(luckyConcession % 2 == 0){		//偶数
		playerLucky -= luckyConcession / 2;
		aiLucky += luckyConcession / 2;
	}else{								//奇数
		playerLucky -= (luckyConcession / 2) + 1;
		aiLucky += luckyConcession / 2;
	}
	playerLucky /= userCount;
	aiLucky /= aiCount;
	for(auto itor = tableSeat.begin(); itor != tableSeat.end(); ++itor){	//统计用户跟陪打/AI的人数
		IServerUserItem *user = *itor;
		if(user->IsAndroidUser() || user->IsAccompanyUser()){	//陪打 / AI
			user->GetUserInfo()->fortuneProb = aiLucky;
		}else{													//品牌用户
			user->GetUserInfo()->fortuneProb = playerLucky;
		}
	}
	return TRUE;
}
/**
	不启用杀数，设定玩家为起始的幸运值
*/
void CDistributeTable::setUserOriginalLuckyValue(){
	for(auto itor = this->tableSeat.begin(); itor != this->tableSeat.end(); ++itor){
		IServerUserItem *user = *itor;
		user->GetUserInfo()->fortuneProb = user->GetUserInfo()->originalFortuneProb;
	}
	return;
}