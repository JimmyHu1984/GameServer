#ifndef THREAD_WORKER_H
#define THREAD_WORKER_H
#pragma once

#include "Stdafx.h"
#include "DistributeHandler.h"
#include "ListManager.h"
#include "DistributeTable.h"
#include "GameServiceHead.h"


class CThreadWorker{
public:
	virtual CWinThread* start(CDistributeHandler* tempClass) = NULL;
};

#endif