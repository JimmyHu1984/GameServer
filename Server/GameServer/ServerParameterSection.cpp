#include "StdAfx.h"
#include "ServerParameterSection.h"


ServerParameterSection::ServerParameterSection(void)
{
}


ServerParameterSection::~ServerParameterSection(void)
{
}

void ServerParameterSection::read(const char *filename, const char *sectionName)
{
	FILE *fileread = fopen(filename, "r");
	
	if (fileread) {
		m_name = "";
		if (findSection(fileread, sectionName)) {
			findParamterItems(fileread);
		}
	}
}

bool ServerParameterSection::findSection(FILE *file, const char *name)
{
	char strbuf[512];
	int maxsize = sizeof(strbuf);
	while (1) {
		if (fgets(strbuf, maxsize, file) != NULL){
			if (!strstr(strbuf, name)) {
				continue;
			}
			char sectionName[256];
			sscanf(strbuf, "[%s]", sectionName);
			m_name = sectionName;
			return true;
		}
		break;
	}
	return false;
}

void ServerParameterSection::findParamterItems(FILE *file)
{
	char strbuf[512];
	int maxsize = sizeof(strbuf);
	while (1) {
		if (fgets(strbuf, maxsize, file) != NULL){
			if (strchr(strbuf, '=')) {
				ServerParameterItem item;
				item.read(strbuf);
				addParameterItem(item);
				continue;
			}
		}
		break;
	}
}

void ServerParameterSection::write(const char *filename)
{
	FILE *filewrite = fopen(filename, "a");
	if (filewrite) {
		char strbuf[256];
		sprintf(strbuf,"[%s]\n", m_name.c_str());
		fwrite(strbuf, sizeof(strbuf[0]), strlen(strbuf), filewrite);
		fclose(filewrite);
		for (ServerParameterItems::iterator it = m_serverParamterItems.begin(); 
			it != m_serverParamterItems.end(); ++it) {
				it->write(filename);
		}
	}
}