#include "GprmcMessage.h"
#include <stdio.h>
#include <stdlib.h>
#include "../../shared/strutils.h"

GprmcMessage::GprmcMessage(const GprmcMessage &other)
	: m_UTCTime(other.m_UTCTime), m_longitude(other.m_longitude), m_latitude(other.m_latitude), m_speed(other.m_speed), m_heading(other.m_heading),
	m_variation(other.m_variation), m_date(other.m_date), m_fix(other.m_fix), m_ns(other.m_ns), m_ew(other.m_ew), m_varDirection(other.m_varDirection)
{
	str_cpy(m_checksum, other.m_checksum, sizeof(m_checksum));
	m_type = other.m_type;
}

bool GprmcMessage::Parse(char* msg, char* checksum)
{
	//printf("Parsing GPRMC string... [%s]\n", msg);

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
			m_date = (unsigned int)atol(token);
	}
	else
		context++;

	if (*context != ',')
	{
		token = (char*)safe_strtok(0, ',', &context);
		if (token)
			m_variation = (float)atof(token);
	}
	else
		context++;

	if (*context != ',')
	{
		token = (char*)safe_strtok(0, ',', &context);
		if (token)
			m_varDirection = *token;
	}
	
	str_cpy(m_checksum, checksum, sizeof(m_checksum));

	return true;
}