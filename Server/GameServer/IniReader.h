#pragma once
#ifndef _GAME_THREAD_LOG_H_
#define _GAME_THREAD_LOG_H_

#include "stdafx.h"
#include <string>
#include <map>

#define INI_STATUS_UNKNOWN			100
#define INI_STATUS_READ_SUCCESS		101
#define INI_STATUS_READ_FAIL		102
#define INI_STATUS_OPEN_FILE_ERROR	103

class CIniReader{
public:
	CIniReader();
	~CIniReader();
	static CIniReader* getInstance();

	std::map<std::wstring, std::wstring> localParaMeter;
	BOOL clearLocalParaMeter();

	int readIniFile(TCHAR* fileName);
	void getParameter();
	BOOL isIniFileExist(CString &szFileName, TCHAR* fileName);
	void dumpLocalParaMeter();
};

#endif