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

public:
	float Timestamp() const { return m_UTCTime; }
	float Longitude() const { return m_longitude; }
	char LongitudeDirection() const { return m_ew; }
	float Latitude() const { return m_latitude; }
	char LatitudeDirection() const { return m_ns; }
	float Speed() const { return m_speed; }
	float Heading() const { return m_heading; }
	bool HasFix() const { return m_fix == 'A'; }
	unsigned int Date() const { return m_date; }

private:
	float m_UTCTime;
	float m_longitude;
	float m_latitude;
	float m_speed;
	float m_heading;
	unsigned int m_date;
	char m_fix;
	char m_ns;
	char m_ew;
};

