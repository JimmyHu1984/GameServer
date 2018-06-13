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
	// 参数编号结构
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

	// 输入玩家资讯 回传髓机产生结果
	bool decideLuckyPlayer(const std::list<PlayerInfo>& source, 
						   std::list<PlayerInfo>& sortList, 
						   const LONGLONG defaultLuckValue = 50);

	// 输入信运值 看是否有中
	void decideLuckyOrNot(const LONGLONG playerLuck[], const size_t num, bool direct[]);
	std::vector<bool> decideLuckyOrNot(const std::vector<LONGLONG>& playerLuck);
};


#endif
