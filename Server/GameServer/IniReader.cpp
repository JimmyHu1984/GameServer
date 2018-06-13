#include "stdafx.h"
#include "IniReader.h"

/*
static CThreadLogManager instance;
	return &instance;
*/


CIniReader::CIniReader(){

}


CIniReader::~CIniReader(){

}

CIniReader* CIniReader::getInstance(){
	static CIniReader instance;
	return &instance;
}


int CIniReader::readIniFile(TCHAR* fileName){
	if(fileName == NULL){
		OutputDebugStringA("Can`t Get RoomParameter File Name.");
		return 0;
	}

	CString iniFilePath;	//ini挡案路径
	CString tempFoldName;	
	CFileFind cfile;
	char tempFold[MAX_PATH];
	////////////////////
	CString tempFileFolder;
	//获取目录
	BOOL exist = this->isIniFileExist(iniFilePath, fileName);
	int status = INI_STATUS_UNKNOWN;
	try{
		if(exist){
			CStdioFile fileReader;
			
			try{
				if(fileReader.Open(iniFilePath, CFile::modeRead&&CFile::modeWrite)){//这样可以防止游戏正在写入文件
					CString strPara;
					while(fileReader.ReadString(strPara)){
						int position = strPara.Find(L"=");
						CString parKey = strPara.Left(position);
						CString parValue = strPara.Mid(position + 1);
						std::wstring wParKey(parKey);
						std::wstring wParValue(parValue);

						this->localParaMeter[wParKey] = wParValue;
					}
					status = INI_STATUS_READ_SUCCESS;
				}else{
					
				}
			}catch(CFileException *e){
				return INI_STATUS_READ_FAIL;
			}
		}else{
			//read ini fail
			AfxMessageBox(L"Cannot Find Ini File, Please Check Release Plan.");
			OutputDebugStringA("Read INI File Fail.");
			return INI_STATUS_READ_FAIL;
		}
	}catch(CFileException  *e){
		CStringA debugStr;
		debugStr.Format("Read INI File Error. Exception Code:%d", e->m_cause);
		::OutputDebugStringA(debugStr);
		return INI_STATUS_OPEN_FILE_ERROR;
	}
	return status;
}
	
BOOL CIniReader::isIniFileExist(CString &szFileName, TCHAR* fileName){
	CFileFind cfile;
	TCHAR szPath[MAX_PATH]=TEXT("");
	int nIndex;

	CString iniFileName(fileName);
	iniFileName = iniFileName.Left(iniFileName.Find(L"."));
	iniFileName += TEXT("Config.ini");

	GetModuleFileName(AfxGetInstanceHandle(),szPath,sizeof(szPath));
	szFileName=szPath;
	nIndex = szFileName.ReverseFind(TEXT('\\'));
	szFileName = szFileName.Left(nIndex);
	szFileName += TEXT("\\");
	szFileName += iniFileName;

	return cfile.FindFile(szFileName);
}


BOOL CIniReader::clearLocalParaMeter(){
	return this->localParaMeter.empty();
}

void CIniReader::dumpLocalParaMeter(){
	auto iter = this->localParaMeter.begin();

	while(iter != this->localParaMeter.end()){
		CString strKey(iter->first.c_str());
		CString strValue(iter->second.c_str());
		OutputDebugStringW(strKey + " "+ strValue);
		++iter;
	}
}