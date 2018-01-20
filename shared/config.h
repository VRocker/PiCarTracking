#pragma once

#include "ISingleton.h"
#include <stdio.h>

class Config : public ISingleton<Config>
{
public:
	Config()
		: m_file(nullptr)
	{}
	Config(const char* fileName);
	~Config();

	bool ReadItem(const char* key, char* value, unsigned int valueSize);
	bool ReadItemInt(const char* key, unsigned int* value);

private:
	bool openFile(const char* fileName);
	void closeFile();

private:
	FILE * m_file;
};

