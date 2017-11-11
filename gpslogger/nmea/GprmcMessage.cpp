#include "GprmcMessage.h"
#include <stdio.h>
#include <stdlib.h>
#include "../../shared/strutils.h"

bool GprmcMessage::Parse(char* msg)
{
	printf("Parsing GPRMC string... [%s]\n", msg);

	size_t tokens = numtok(msg, ',');
	if (tokens != 12)
		return false;

	char* token = nullptr;
	char* context = msg;

	if (*context != ',')
	{
		token = (char*)safe_strtok(0, ',', &context);
		if (token)
			m_UTCTime = (float)atof(token);
	}
	else
		context++;

	token = (char*)safe_strtok(0, ',', &context);
	if (token)
		m_fix = *token;

	if (*context != ',')
	{
		token = (char*)safe_strtok(0, ',', &context);
		if (token)
			m_latitude = (float)atof(token);
	}
	else
		context++;

	if (*context != ',')
	{
		token = (char*)safe_strtok(0, ',', &context);
		if (token)
			m_ns = *token;
	}
	else
		context++;

	if (*context != ',')
	{
		token = (char*)safe_strtok(0, ',', &context);
		if (token)
			m_longitude = (float)atof(token);
	}
	else
		context++;

	if (*context != ',')
	{
		token = (char*)safe_strtok(0, ',', &context);
		if (token)
			m_ew = *token;
	}
	else
		context++;

	if (*context != ',')
	{
		token = (char*)safe_strtok(0, ',', &context);
		if (token)
			m_speed = (float)atof(token);
	}
	else
		context++;

	if (*context != ',')
	{
		token = (char*)safe_strtok(0, ',', &context);
		if (token)
			m_heading = (float)atof(token);
	}
	else
		context++;

	if (*context != ',')
	{
		token = (char*)safe_strtok(0, ',', &context);
		if (token)
			m_date = atol(token);
	}

	return true;
}