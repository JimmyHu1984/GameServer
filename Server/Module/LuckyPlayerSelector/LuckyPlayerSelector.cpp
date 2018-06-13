#include "StdAfx.h"
#include "LuckyPlayerSelector.h"

namespace{

	LuckyPlayerSelector::PlayerInfo GetWinner(const std::list<LuckyPlayerSelector::PlayerInfo>& list){
		// 加总 幸运值
		LONGLONG sumLuck = 0;
		for(auto it = list.cbegin(); it != list.cend(); ++it){
			sumLuck += it->luck;
		}

		// 取得随机产生的幸运值
		const LONGLONG luckNum = std::rand() % sumLuck;

		// 得出落点
		auto it = list.cbegin();
		LONGLONG accumulate = it->luck;
		while(accumulate < luckNum){
			++it;
			accumulate += it->luck;
		}
		return *it;
	}

	void RemoveWinner(std::list<LuckyPlayerSelector::PlayerInfo>& list, const LuckyPlayerSelector::PlayerInfo& winner){
		// 寻找赢家
		auto it = list.begin();
		while(it != list.end()){
			// 找到即移除
			if(it->index == winner.index){
				list.erase(it);
				break;
			}

			++it;
		}
		return;
	}

	// 判断是否落在幸运值内
	bool IsPlayerLucky(const LONGLONG luckParameter){
		// 总数为100
		static const LONGLONG total = 100;
		// 取得随机变数
		const LONGLONG luckPoint = std::rand() % total;
		bool result = false;
		if(luckPoint < luckParameter){
			result = true;
		}
		return result;
	}

	// 当有幸运值为0时，改成预设值
	void ModifyZeroLuckValueToDefault(std::vector<LONGLONG>& list) {
		for (auto it = list.begin(); it != list.end(); ++it) {
			if (*it == 0) {
				*it = 50;
			}
		}
		return;
	}

}

LuckyPlayerSelector::LuckyPlayerSelector(){
	// 初始化随机涵式库
	std::srand(static_cast<unsigned int>(std::time(nullptr)));
}

LuckyPlayerSelector::~LuckyPlayerSelector(){

}

bool LuckyPlayerSelector::decideLuckyPlayer(const std::list<PlayerInfo>& source, 
											std::list<PlayerInfo>& sortList, 
											const LONGLONG defaultLuckValue){
	// 确保幸运值不为0
	std::list<PlayerInfo> sourceBuf(source);
	for(auto it = sourceBuf.begin(); it != sourceBuf.end(); ++it){
		if(it->luck == 0){
			if(defaultLuckValue == 0){
				return false;
			}
			it->luck = defaultLuckValue;
		}
	}

	// 取得随机产生结果
	sortList.clear();
	const size_t dataNum = sourceBuf.size();
	for(size_t count = 0; count < dataNum; ++count){
		// 取得本次赢家
		const struct PlayerInfo winner = ::GetWinner(sourceBuf);
		// 写入结果
		sortList.push_back(winner);
		// 将赢家除列
		::RemoveWinner(sourceBuf, winner);
	}
	return true;
}

void LuckyPlayerSelector::decideLuckyOrNot(const LONGLONG playerLuck[], const size_t num, bool direct[]){
	// 打包资料
	const std::vector<LONGLONG> source(playerLuck, playerLuck + num);
	// 取得数值
	const std::vector<bool> result = this->decideLuckyOrNot(source);
	// 回写结果
	for(size_t count = 0; count < num; ++count){
		direct[count] = result[count];
	}
	return;
}

std::vector<bool> LuckyPlayerSelector::decideLuckyOrNot(const std::vector<LONGLONG>& playerLuck){
	// 取得资料笔数
	const size_t dataNum = playerLuck.size();

	// 当有幸运值为0时，改成预设值
	// 鹰可能修改数值所以先复制一份
	std::vector<LONGLONG> temp(playerLuck);
	::ModifyZeroLuckValueToDefault(temp);
	// 取得由信运直随机产生结果
	std::vector<bool> result(dataNum);
	for(size_t count = 0; count < dataNum; ++count){
		result[count] = ::IsPlayerLucky(temp[count]);
	}
	return std::move(result);
}
