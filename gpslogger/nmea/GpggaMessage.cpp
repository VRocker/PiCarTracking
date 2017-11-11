#include "GpggaMessage.h"
#include <stdio.h>
#include <stdlib.h>
#include "../../shared/strutils.h"

// $GPGGA,,,,,,0,00,99.99,,,,,,*48


bool GpggaMessage::Parse(char* msg)
{
	printf("Parsing GPGGA string... [%s]\n", msg);

	size_t tokens = numtok(msg, ',');
	if (tokens != 13)
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
			m_fixQuality = atoi(token);
	}
	else
		context++;

	if (*context != ',')
	{
		token = (char*)safe_strtok(0, ',', &context);
		if (token)
			m_satsInView = atoi(token);
	}
	else
		context++;

	if (*context != ',')
	{
		token = (char*)safe_strtok(0, ',', &context);
		if (token)
			m_hdop = (float)atof(token);
	}
	else
		context++;

	if (*context != ',')
	{
		token = (char*)safe_strtok(0, ',', &context);
		if (token)
			m_altitude = (float)atof(token);
	}
	else
		context++;

	return true;
}