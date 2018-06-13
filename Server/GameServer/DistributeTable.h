#ifndef DISTRIBUTE_TABLE_H
#define DISTRIBUTE_TABLE_H
#include "Stdafx.h"
#include <list>
#include <string>

#define WAIT_DISTRIBUTE_TIME 3			//�ȴ�����ʱ�䣬��λ(��)
#define WAIT_DISTRIBUTE_TIME_UNLIMIT 0	//�ȴ�����ʱ�䣬���޴�

class CDistributeTable{
public:
	//CDistributeTable();
	CDistributeTable(IServerUserItem *user, int minUser, int maxUser);
	std::vector<IServerUserItem*> tableSeat;

	//��������
	void setWaitTimeToUnlimit() {this->createTime = WAIT_DISTRIBUTE_TIME_UNLIMIT;}
	BOOL isSameMerchantName();					//�����ڵ��û��Ƿ�ͬƷ��?
	BOOL compareMerchantCode(TCHAR *tmpStr);
	BOOL findUser(IServerUserItem *tempUser);
	BOOL insertUser(IServerUserItem *tempUser);
	BOOL isUserCanEnterTable(IServerUserItem *tempUser);
	//����
	UINT getTableUserCount() {return this->tableUserCount;}
	BOOL isReadyToAssign();						//�Ƿ���԰�������AI����
	BOOL setMerchantName(TCHAR *tmpStr);		//�趨����Ʒ������
	//����ǰ�趨
	BOOL setKillNum(int tempValue);				//�趨����ɱ��
	BOOL isSatisfyKill();						//�����Ƿ�����ɱ����������?
	BOOL setUserLuckyValue();
	void setUserOriginalLuckyValue();			//�趨Ϊ��ʼ������ֵ


private:
	UINT tableUserCount;		//���ӵ�����
	std::wstring merchantName;	//�û�Ʒ��,userINFO channelName
	std::wstring roomKey;		//����KEY
	int killNum;				//Ʒ��ɱ��
	LONGLONG createTime;
	BOOL isAllowMixMerchant;	//�Ƿ���Խ��ܻ���
	int tableMinUser;			//������С��������
	int tableMaxUser;			//������󿪾�����
	std::map<std::wstring, DWORD>	tableAllowMerchant;	//��������Ʒ��

};


#endif