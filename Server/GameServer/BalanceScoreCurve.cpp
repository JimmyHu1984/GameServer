#include "StdAfx.h"
#include "GameServiceHead.h"
#include "BalanceScoreCurve.h"

CBalanceScoreCurve::CBalanceScoreCurve()
{
}

CBalanceScoreCurve::~CBalanceScoreCurve()
{
	ResetBalanceScoreCurve();
}

void CBalanceScoreCurve::ResetBalanceScoreCurve()
{
	m_BalanceScoreCurveMap.clear();
}

bool CBalanceScoreCurve::AddBalanceScoreCurve(SCORE lScore, DWORD dwBalanceScore)
{
	m_BalanceScoreCurveMap[lScore] = dwBalanceScore;
	return true;
}

// ���ݵ�ǰ���������ƽ��ֵı仯ֵ
LONG CBalanceScoreCurve::CalculateBalanceScore(SCORE lOldScore, SCORE lNewScore)
{
	LONG lOldBalanceScore = GetBalanceScore(lOldScore);
	LONG lNewBalanceScore = GetBalanceScore(lNewScore);

	return (lNewBalanceScore - lOldBalanceScore);
}

// ���ݵ�ǰscore,�����BalanceScore
DWORD CBalanceScoreCurve::GetBalanceScore(SCORE lScore)
{
	LONG lBalanceScore = 0L;
	for (CBalanceScoreCurveMap::iterator iter = m_BalanceScoreCurveMap.begin(); iter != m_BalanceScoreCurveMap.end(); iter++)
	{
		SCORE lIter_Score = iter->first;
		LONG  lIter_BalanceScore = iter->second;	
		if (lIter_Score < 0)
		{
			if (lScore < lIter_Score)
			{
				lBalanceScore = lIter_BalanceScore;
				break;
			}
		}
		else
		{
			if (lScore > lIter_Score)
			{
				lBalanceScore = lIter_BalanceScore;
			}
		}
	}

	return lBalanceScore;
}

