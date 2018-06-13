#pragma once

#include <vector>
#include <string>

typedef std::vector<std::string> ParamterFileLines;

class ServerParameterFile
{
public:
	ServerParameterFile(void);
	~ServerParameterFile(void);

	void removeSection(const char *filename, const char *section);
private:
	void read(const char *filename);
	bool doRemove(const char *section);
	void rewriteFile(const char *filename);
	ParamterFileLines::iterator findEnd(ParamterFileLines::iterator it);
	ParamterFileLines m_parameterFileLines;
};

