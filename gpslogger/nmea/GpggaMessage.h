#pragma once

#include "../NmeaParser.h"

class GpggaMessage : public INmeaMessage
{
public:
	GpggaMessage()
		: m_UTCTime(0.0f), m_longitude(0.0f), m_latitude(0.0f), m_satsInView(0), m_fixQuality(-1),
		m_ns('N'), m_ew('E')
	{
		m_type = NmeaType::GGA;
	}

	~GpggaMessage() {}

	bool Parse(char* msg);

public:
	float Timestamp() const { return m_UTCTime; }
	float Longitude() const { return m_longitude; }
	char LongitudeDirection() const { return m_ew; }
	float Latitude() const { return m_latitude; }
	char LatitudeDirection() const { return m_ns; }
	float Altitude() const { return m_altitude; }
	unsigned int SatsInView() const { return m_satsInView; }


private:
	float m_UTCTime;
	float m_longitude;
	float m_latitude;
	float m_hdop;
	float m_altitude;
	unsigned int m_satsInView;
	char m_fixQuality;
	char m_ns;
	char m_ew;
};

