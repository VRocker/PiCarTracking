#pragma once

#include "../NmeaParser.h"

class GprmcMessage : public INmeaMessage
{
public:
	GprmcMessage()
		: m_UTCTime(0.0f), m_longitude(0.0f), m_latitude(0.0f), m_speed(0.0f), m_heading(0.0f),
		m_date(0), m_fix('V'), m_ns('N'), m_ew('E')
	{
		m_type = NmeaType::RMC;
	}

	bool Parse(char* msg);

	bool HasFix() const { return m_fix == 'A'; }

private:
	float m_UTCTime;
	float m_longitude;
	float m_latitude;
	float m_speed;
	float m_heading;
	unsigned long m_date;
	char m_fix;
	char m_ns;
	float m_ew;
};

