#include "StdAfx.h"
#include "LuckyPlayerSelector.h"

namespace{

	LuckyPlayerSelector::PlayerInfo GetWinner(const std::list<LuckyPlayerSelector::PlayerInfo>& list){
		// ���� ����ֵ
		LONGLONG sumLuck = 0;
		for(auto it = list.cbegin(); it != list.cend(); ++it){
			sumLuck += it->luck;
		}

		// ȡ���������������ֵ
		const LONGLONG luckNum = std::rand() % sumLuck;

		// �ó����
		auto it = list.cbegin();
		LONGLONG accumulate = it->luck;
		while(accumulate < luckNum){
			++it;
			accumulate += it->luck;
		}
		return *it;
	}

	void RemoveWinner(std::list<LuckyPlayerSelector::PlayerInfo>& list, const LuckyPlayerSelector::PlayerInfo& winner){
		// Ѱ��Ӯ��
		auto it = list.begin();
		while(it != list.end()){
			// �ҵ����Ƴ�
			if(it->index == winner.index){
				list.erase(it);
				break;
			}

			++it;
		}
		return;
	}

	// �ж��Ƿ���������ֵ��
	bool IsPlayerLucky(const LONGLONG luckParameter){
		// ����Ϊ100
		static const LONGLONG total = 100;
		// ȡ���������
		const LONGLONG luckPoint = std::rand() % total;
		bool result = false;
		if(luckPoint < luckParameter){
			result = true;
		}
		return result;
	}

	// ��������ֵΪ0ʱ���ĳ�Ԥ��ֵ
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
	// ��ʼ�������ʽ��
	std::srand(static_cast<unsigned int>(std::time(nullptr)));
}

LuckyPlayerSelector::~LuckyPlayerSelector(){

}

bool LuckyPlayerSelector::decideLuckyPlayer(const std::list<PlayerInfo>& source, 
											std::list<PlayerInfo>& sortList, 
											const LONGLONG defaultLuckValue){
	// ȷ������ֵ��Ϊ0
	std::list<PlayerInfo> sourceBuf(source);
	for(auto it = sourceBuf.begin(); it != sourceBuf.end(); ++it){
		if(it->luck == 0){
			if(defaultLuckValue == 0){
				return false;
			}
			it->luck = defaultLuckValue;
		}
	}

	// ȡ������������
	sortList.clear();
	const size_t dataNum = sourceBuf.size();
	for(size_t count = 0; count < dataNum; ++count){
		// ȡ�ñ���Ӯ��
		const struct PlayerInfo winner = ::GetWinner(sourceBuf);
		// д����
		sortList.push_back(winner);
		// ��Ӯ�ҳ���
		::RemoveWinner(sourceBuf, winner);
	}
	return true;
}

void LuckyPlayerSelector::decideLuckyOrNot(const LONGLONG playerLuck[], const size_t num, bool direct[]){
	// �������
	const std::vector<LONGLONG> source(playerLuck, playerLuck + num);
	// ȡ����ֵ
	const std::vector<bool> result = this->decideLuckyOrNot(source);
	// ��д���
	for(size_t count = 0; count < num; ++count){
		direct[count] = result[count];
	}
	return;
}

std::vector<bool> LuckyPlayerSelector::decideLuckyOrNot(const std::vector<LONGLONG>& playerLuck){
	// ȡ�����ϱ���
	const size_t dataNum = playerLuck.size();

	// ��������ֵΪ0ʱ���ĳ�Ԥ��ֵ
	// ӥ�����޸���ֵ�����ȸ���һ��
	std::vector<LONGLONG> temp(playerLuck);
	::ModifyZeroLuckValueToDefault(temp);
	// ȡ��������ֱ����������
	std::vector<bool> result(dataNum);
	for(size_t count = 0; count < dataNum; ++count){
		result[count] = ::IsPlayerLucky(temp[count]);
	}
	return std::move(result);
}
