#include "../shared/config.h"
#include "../shared/strutils.h"
#include <string.h>
#include <stdlib.h>

template<>
Config* ISingleton<Config>::m_singleton = nullptr;

Config::Config(const char* fileName)
	: m_file(nullptr)
{
	openFile(fileName);
}


Config::~Config()
{
	closeFile();
}

bool Config::ReadItem(const char* key, char* value, unsigned int valueSize)
{
	if (!m_file)
		return false;

	rewind(m_file);

	static const char* tok = "\t = \r\n";

	char line[32] = { 0 };
	char* pch, *configKey;
	while (fgets(line, sizeof(line), m_file))
	{
		// Is the first character a #? Its probably a comment
		if (*line == '#')
			continue;

		// Is the line empty or a new line?
		if ((!*line) || (*line == '\r') || (*line == '\n'))
			continue;

		pch = (char*)strtok(line, tok);
		if (pch)
		{
			configKey = pch;
			if (((pch = (char*)strtok(0, tok)) == 0) || (*pch == '\n'))
				continue;

			if (str_equal_nocase(configKey, key))
			{
				str_cpy(value, pch, valueSize);
				return 0;
			}
		}
	}

	return false;
}

bool Config::ReadItemInt(const char* key, unsigned int* value)
{
	if (!m_file)
		return false;

	rewind(m_file);

	static const char* tok = "\t = \r\n";

	char line[32] = { 0 };
	char* pch, *configKey;
	while (fgets(line, sizeof(line), m_file))
	{
		// Is the first character a #? Its probably a comment
		if (*line == '#')
			continue;

		// Is the line empty or a new line?
		if ((!*line) || (*line == '\r') || (*line == '\n'))
			continue;

		pch = (char*)strtok(line, tok);
		if (pch)
		{
			configKey = pch;
			if (((pch = (char*)strtok(0, tok)) == 0) || (*pch == '\n'))
				continue;

			if (str_equal_nocase(configKey, key))
			{
				if (isnum(pch))
				{
					*value = atoi(pch);
					return 0;
				}
			}
		}
	}

	return false;
}

bool Config::openFile(const char* fileName)
{
	FILE* tmp = fopen(fileName, "a+");
	if (tmp)
	{
		m_file = tmp;

		return true;
	}
	return false;
}

void Config::closeFile()
{
	if (m_file)
	{
		fclose(m_file);
		m_file = nullptr;
	}
}
