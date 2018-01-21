#include "NmeaParser.h"
#include <string.h>
#include "../shared/strutils.h"
#include <stdio.h>

#include "nmea/GprmcMessage.h"
#include "nmea/GpggaMessage.h"

INmeaMessage* NmeaParser::ParseMessage(char* msg)
{
	char* context = nullptr, *checksumContext = nullptr;
	INmeaMessage* ret = nullptr;

	// Check the checksum
	char* unchecksummedData = (char*)safe_strtok(msg + 1, '*', &checksumContext);
	if (!unchecksummedData)
		return nullptr;

	char* dataChecksum = (char*)safe_strtok(0, '*', &checksumContext);
	if (!dataChecksum)
		return nullptr;

	char checksum = 0;

	while (*unchecksummedData)
	{
		checksum ^= *unchecksummedData;
		unchecksummedData++;
	}

	if ((dataChecksum) && (strlen(dataChecksum) == 2))
	{
		unsigned char parsedChecksum = (unsigned char)(hex_to_byte(dataChecksum[0]) * 16) + hex_to_byte(dataChecksum[1]);
		// Validate the checksum on the string
		if (checksum != parsedChecksum)
			return nullptr;
	}

	char* token = (char*)safe_strtok(msg, ',', &context);
	if (!token)
		return nullptr;

	if (str_equal(token, "$GPRMC"))
	{
		// GPRMC String
		GprmcMessage* tmp = new GprmcMessage();
		if (tmp->Parse(context, dataChecksum))
			ret = tmp;
		else
			delete tmp;
	}
	else if (str_equal(token, "$GPGLL"))
	{
		// GPGLL String
	}
	else if (str_equal(token, "$GPVTG"))
	{
		// GPVTG String
	}
	else if (str_equal(token, "$GPGGA"))
	{
		// GPGGA String
		GpggaMessage* tmp = new GpggaMessage();
		if (tmp->Parse(context, dataChecksum))
			ret = tmp;
		else
			delete tmp;
	}
	else if (str_equal(token, "$GPGSA"))
	{
		// GPGSA String
	}

	return ret;
}