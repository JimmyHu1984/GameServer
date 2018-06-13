#pragma once

#include <string>

class ServerParameterItem
{
public:
	ServerParameterItem(void);
	ServerParameterItem(const char *name, const char *value);
	~ServerParameterItem(void);

	void read(std::string parameterLine);
	void write(const char *filename);
	const char* getName() { return m_name.c_str(); }
	const char* getValue() { return m_value.c_str(); }
	void setName(const char* name) { m_name = name; }
	void setValue(const char* value) { m_value = value; }
private:
	std::string m_name;
	std::string m_value;
};

