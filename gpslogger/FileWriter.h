#pragma once

#include <stdio.h>

class FileWriter
{
public:
	FileWriter(const char* fileName);
	~FileWriter();

	void WriteLine(const char* text, unsigned int len);

private:
	bool openFile(const char* fileName);
	void closeFile();

private:
	FILE * m_outFile;
};

