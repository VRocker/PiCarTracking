#include "GprmcMessage.h"
#include <stdio.h>
#include "../../shared/strutils.h"

bool GprmcMessage::Parse(char* msg)
{
	printf("Parsing GPRMC string... [%s]\n", msg);

	size_t tokens = numtok(msg, ',');
	printf("Tokens: %u\n", tokens);
	if (tokens != 11)
		return false;

	char* token = nullptr;
	char* context = nullptr;

	token = (char*)safe_strtok(msg, ',', &context);
	if (!token)
		return false;
		
	sscanf(token, "%f", &m_UTCTime);

	token = (char*)safe_strtok(0, ',', &context);
	if (!token)
		return false;
		
	m_fix = *token;

	token = (char*)safe_strtok(0, ',', &context);
	if (token)
		sscanf(token, "%f", &m_latitude);

	token = (char*)safe_strtok(0, ',', &context);
	if (token)
		m_ns = *token;

	token = (char*)safe_strtok(0, ',', &context);
	if (token)
		sscanf(token, "%f", &m_longitude);

	token = (char*)safe_strtok(0, ',', &context);
	if (token)
		m_ew = *token;

	token = (char*)safe_strtok(0, ',', &context);
	if (token)
		sscanf(token, "%f", &m_speed);

	token = (char*)safe_strtok(0, ',', &context);
	if (token)
		sscanf(token, "%f", &m_heading);

	token = (char*)safe_strtok(0, ',', &context);
	if (token)
		sscanf(token, "%lu", &m_date);

	return true;
}