#include "FileWriter.h"



FileWriter::FileWriter(const char* fileName)
{
	openFile(fileName);
}


FileWriter::~FileWriter()
{
	closeFile();
}

bool FileWriter::openFile(const char* fileName)
{
	FILE* tmp = fopen(fileName, "a+");
	if (tmp)
	{
		m_outFile = tmp;

		return true;
	}
	return false;
}

void FileWriter::closeFile()
{
	if (m_outFile)
	{
		fclose(m_outFile);
		m_outFile = nullptr;
	}
}

void FileWriter::WriteLine(const char* text, unsigned int len)
{
	if (!m_outFile)
		return;

	fwrite(text, len, 1, m_outFile);
}
