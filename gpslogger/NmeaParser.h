#pragma once

enum class NmeaType
{
	RMC,
	VTG,
	GLL,
	GGA,
	GSA,
	None
};

class INmeaMessage
{
public:
	INmeaMessage()
		: m_type(NmeaType::None)
	{}

	NmeaType GetType() const { return m_type; }

protected:
	NmeaType m_type;
};

class NmeaParser
{
public:
	static INmeaMessage* ParseMessage(char* msg);
};

