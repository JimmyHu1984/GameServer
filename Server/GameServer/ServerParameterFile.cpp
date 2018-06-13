#include "StdAfx.h"
#include "ServerParameterFile.h"


ServerParameterFile::ServerParameterFile(void)
{
}


ServerParameterFile::~ServerParameterFile(void)
{
}

void ServerParameterFile::read(const char *filename)
{
	char strbuf[512];
	int maxsize = sizeof(strbuf);
	FILE *file = fopen(filename, "r");
	if (file) {
		while (1) {
			if (fgets(strbuf, maxsize, file) != NULL){
				m_parameterFileLines.push_back(strbuf);
				continue;
			}
			break;
		}
		fclose(file);
	}
}

void ServerParameterFile::removeSection(const char *filename, 
	const char *section)
{
	read(filename);
	bool removed = false;
	do{
		removed = doRemove(section);
	}while(removed);
	rewriteFile(filename);
}

ParamterFileLines::iterator ServerParameterFile::findEnd(ParamterFileLines::iterator it)
{
	++it;
	for (; it != m_parameterFileLines.end(); ++it) {
		if (it->find("=") == std::string::npos) {
			break;
		}
	}
	return it;
}

bool ServerParameterFile::doRemove(const char *section)
{
	ParamterFileLines::iterator itStart;
	ParamterFileLines::iterator itEnd;
	std::string findSection = std::string("[") + std::string(section) + std::string("]");
	for (ParamterFileLines::iterator it = m_parameterFileLines.begin(); 
		it != m_parameterFileLines.end(); 
		++it) {
			if (it->find(findSection) != std::string::npos) {
				itStart = it;
				itEnd = findEnd(it);
				m_parameterFileLines.erase(itStart, itEnd);
				return true;
			}
	}
	return false;
}

void ServerParameterFile::rewriteFile(const char *filename)
{
	FILE *file = fopen(filename, "w");
	if (file) {
		for (ParamterFileLines::iterator it = m_parameterFileLines.begin(); 
			it != m_parameterFileLines.end();
			++it) {
			fprintf(file,"%s", it->c_str());
		}
		fclose(file);
	}
}