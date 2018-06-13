#ifndef DISTRIBUTE_TABLE_H
#define DISTRIBUTE_TABLE_H
#include "Stdafx.h"
#include <list>
#include <string>

#define WAIT_DISTRIBUTE_TIME 3			//等待配桌时间，单位(秒)
#define WAIT_DISTRIBUTE_TIME_UNLIMIT 0	//等待配桌时间，无限大

class CDistributeTable{
public:
	//CDistributeTable();
	CDistributeTable(IServerUserItem *user, int minUser, int maxUser);
	std::vector<IServerUserItem*> tableSeat;

	//进入牌桌
	void setWaitTimeToUnlimit() {this->createTime = WAIT_DISTRIBUTE_TIME_UNLIMIT;}
	BOOL isSameMerchantName();					//桌子内的用户是否都同品牌?
	BOOL compareMerchantCode(TCHAR *tmpStr);
	BOOL findUser(IServerUserItem *tempUser);
	BOOL insertUser(IServerUserItem *tempUser);
	BOOL isUserCanEnterTable(IServerUserItem *tempUser);
	//开局
	UINT getTableUserCount() {return this->tableUserCount;}
	BOOL isReadyToAssign();						//是否可以安排陪打或AI入桌
	BOOL setMerchantName(TCHAR *tmpStr);		//设定桌子品牌名称
	//开局前设定
	BOOL setKillNum(int tempValue);				//设定桌子杀数
	BOOL isSatisfyKill();						//该桌是否满足杀数启动条件?
	BOOL setUserLuckyValue();
	void setUserOriginalLuckyValue();			//设定为起始的幸运值


private:
	UINT tableUserCount;		//桌子的人数
	std::wstring merchantName;	//用户品牌,userINFO channelName
	std::wstring roomKey;		//房卡KEY
	int killNum;				//品牌杀数
	LONGLONG createTime;
	BOOL isAllowMixMerchant;	//是否可以接受混桌
	int tableMinUser;			//桌子最小开局人数
	int tableMaxUser;			//桌子最大开局人数
	std::map<std::wstring, DWORD>	tableAllowMerchant;	//可入桌的品牌

};


#endif