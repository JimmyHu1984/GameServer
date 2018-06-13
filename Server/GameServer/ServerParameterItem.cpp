#include "StdAfx.h"
#include "ServerParameterItem.h"
#include <algorithm>


ServerParameterItem::ServerParameterItem(void)
{
}

ServerParameterItem::ServerParameterItem(const char *name, const char *value):
m_name(name), m_value(value)
{
}

ServerParameterItem::~ServerParameterItem(void)
{
}

void ServerParameterItem::read(std::string parameterLine)
{
	std::size_t pos = parameterLine.find("=");
	if (pos != std::string::npos) {
		m_name = parameterLine.substr(0, pos);
		m_value = parameterLine.substr(pos + 1);
	}
	m_name.erase(m_name.find_last_not_of(" \r\n\t")+1);

	m_value.erase(m_value.find_last_not_of(" \r\n\t")+1);
}

void ServerParameterItem::write(const char *filename)
{
	char strbuf[512];
	sprintf(strbuf, "%s=%s\n", m_name.c_str(), m_value.c_str());
	FILE *filewrite = fopen(filename, "a");
	if (filewrite) {
		fwrite(strbuf, sizeof(strbuf[0]), strlen(strbuf), filewrite);
		fclose(filewrite);
	}
}