#pragma once

#include "../NmeaParser.h"

class GprmcMessage : public INmeaMessage
{
public:
	GprmcMessage()
		: m_UTCTime(0.0f), m_longitude(0.0f), m_latitude(0.0f), m_speed(0.0f), m_heading(0.0f), m_variation(0.0f),
		m_date(0), m_fix('V'), m_ns('N'), m_ew('E'), m_varDirection('W')
	{
		*m_checksum = 0;
		m_type = NmeaType::RMC;
	}

	GprmcMessage(const GprmcMessage &other);

	bool Parse(char* msg, char* checksum);

public:
	float Timestamp() const { return m_UTCTime; }
	float Longitude() const { return m_longitude; }
	char LongitudeDirection() const { return m_ew; }
	float Latitude() const { return m_latitude; }
	char LatitudeDirection() const { return m_ns; }
	float Speed() const { return m_speed; }
	float Heading() const { return m_heading; }
	float Variation() const { return m_variation; }
	char VariationDirection() const { return m_varDirection; }
	bool HasFix() const { return m_fix == 'A'; }
	unsigned int Date() const { return m_date; }
	const char* Checksum() const { return m_checksum; }

private:
	float m_UTCTime;
	float m_longitude;
	float m_latitude;
	float m_speed;
	float m_heading;
	float m_variation;
	unsigned int m_date;
	char m_fix;
	char m_ns;
	char m_ew;
	char m_varDirection;

	char m_checksum[4];
};

