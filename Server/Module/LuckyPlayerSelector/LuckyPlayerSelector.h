#ifndef LUCKYPLAYERSELECTOR_H
#define LUCKYPLAYERSELECTOR_H

#pragma once

#include <cstddef>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <list>
#include <Windows.h>

class LuckyPlayerSelector{
public:
	// ������Žṹ
	struct PlayerInfo{
		int index;
		LONGLONG luck;

		PlayerInfo()
			: index(0)
			, luck(0){

		}

		PlayerInfo(const int index, const LONGLONG luck)
			: index(index)
			, luck(luck){

		}
	};

public:
	explicit LuckyPlayerSelector();
	virtual ~LuckyPlayerSelector();
	LuckyPlayerSelector(const LuckyPlayerSelector& rv) {}
	LuckyPlayerSelector& operator=(const LuckyPlayerSelector& rv) {}

	// ���������Ѷ �ش�����������
	bool decideLuckyPlayer(const std::list<PlayerInfo>& source, 
						   std::list<PlayerInfo>& sortList, 
						   const LONGLONG defaultLuckValue = 50);

	// ��������ֵ ���Ƿ�����
	void decideLuckyOrNot(const LONGLONG playerLuck[], const size_t num, bool direct[]);
	std::vector<bool> decideLuckyOrNot(const std::vector<LONGLONG>& playerLuck);
};


#endif
