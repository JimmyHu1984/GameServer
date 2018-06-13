#pragma once
#include "ServerParameterItem.h"
#include <vector>
#include <string>

typedef std::vector<ServerParameterItem> ServerParameterItems;

class ServerParameterSection
{
public:
	ServerParameterSection(void);
	~ServerParameterSection(void);

	void read(const char *filename, const char *sectionName );
	void write(const char *filename);
	void setName(const char *name) { m_name = name; };
	void addParameterItem(ServerParameterItem item) { m_serverParamterItems.push_back(item); }
	ServerParameterItems* getParameterItems() { return &m_serverParamterItems; }
private:
	bool findSection(FILE *file, const char *name);
	void findParamterItems(FILE *file);
	std::string m_name;
	ServerParameterItems m_serverParamterItems;
};

