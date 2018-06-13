#ifndef BALANCE_SCORE_CURVER_HEAD_FILE
#define BALANCE_SCORE_CURVER_HEAD_FILE
#pragma once

#define PRIME_BALANCE_SCORE_CURVE_AMOUNT	503L

#include <map>        
#include <algorithm>   
using namespace std;

// 数据结构定义
typedef map<LONGLONG, LONG> CBalanceScoreCurveMap;

class CBalanceScoreCurve
{
protected:
	//CBalanceScoreCurveMap	m_BalanceScoreCurveMap;
	//函数定义
public:
	//构造函数
	CBalanceScoreCurve();
	//析构函数
	virtual ~CBalanceScoreCurve();
public:
	//初始化
	void ResetBalanceScoreCurve();
	//添加曲线配置
	bool AddBalanceScoreCurve(SCORE lScore, DWORD dwBalanceScore);
	// 根据当前积分推算出平衡分的变化值
	LONG CalculateBalanceScore(SCORE lOldScore, SCORE lNewScore);
protected:
	// 根据当前score,推算出BalanceScore
	DWORD GetBalanceScore(SCORE lScore);

};

//////////////////////////////////////////////////////////////////////////////////

#endif