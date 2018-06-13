#ifndef BALANCE_SCORE_CURVER_HEAD_FILE
#define BALANCE_SCORE_CURVER_HEAD_FILE
#pragma once

#define PRIME_BALANCE_SCORE_CURVE_AMOUNT	503L

#include <map>        
#include <algorithm>   
using namespace std;

// ���ݽṹ����
typedef map<LONGLONG, LONG> CBalanceScoreCurveMap;

class CBalanceScoreCurve
{
protected:
	//CBalanceScoreCurveMap	m_BalanceScoreCurveMap;
	//��������
public:
	//���캯��
	CBalanceScoreCurve();
	//��������
	virtual ~CBalanceScoreCurve();
public:
	//��ʼ��
	void ResetBalanceScoreCurve();
	//�����������
	bool AddBalanceScoreCurve(SCORE lScore, DWORD dwBalanceScore);
	// ���ݵ�ǰ���������ƽ��ֵı仯ֵ
	LONG CalculateBalanceScore(SCORE lOldScore, SCORE lNewScore);
protected:
	// ���ݵ�ǰscore,�����BalanceScore
	DWORD GetBalanceScore(SCORE lScore);

};

//////////////////////////////////////////////////////////////////////////////////

#endif