#include "ATLocHandler.h"
#include "SerialHandler.h"
#include "../shared/strutils.h"
#include "../shared/Logger.h"
#include "../shared/locationshm.h"
#include <stdlib.h>

template<>
ATLocHandler* ISingleton< ATLocHandler >::m_singleton = nullptr;

ATLocHandler::ATLocHandler()
	: m_portOpened(false), m_locationShm(nullptr)
{
}


ATLocHandler::~ATLocHandler()
{
	Shutdown();
}

bool ATLocHandler::Setup(const char* device)
{
	if (!SerialHandler::GetSingleton()->OpenPort(device))
		return false;

	m_locationShm = getShmLocation(LocationUnitIDs::Modem);
	if (!m_locationShm)
	{
		Logger::GetSingleton()->Write("Failed to open shared memory segment for modem location data.", LogLevel::Error);
		Shutdown();
		return false;
	}

	m_portOpened = true;

	return true;
}

void ATLocHandler::Shutdown()
{
	SerialHandler::GetSingleton()->ClosePort();

	m_portOpened = false;
}

std::tuple<float, float> ATLocHandler::GrabNetworkLocation()
{
	// Set the port up for location stuff
	/*self.logger.info('Setting up PDP context')
        self.set('+UPSD', '0,1,\"hologram\"')
        self.set('+UPSD', '0,7,\"0.0.0.0\"')
ok, _ = self.set('+UPSDA', '0,3', timeout=30)
*/
	char outBuffer[128] = { 0 };

	std::tuple<float, float> retVal = std::make_tuple(0.0f, 0.0f);

	char updsCmd1[] = "AT+UPSD=0,1,\"hologram\"\r";
	if (!SerialHandler::GetSingleton()->WritePort(updsCmd1, sizeof(updsCmd1)))
		return retVal;

	// Read what we wrote
	*outBuffer = 0;
	SerialHandler::GetSingleton()->ReadLine(outBuffer, sizeof(outBuffer));

	Logger::GetSingleton()->Write("Waiting for OK after first command.", LogLevel::Information);
	if (!SerialHandler::GetSingleton()->ReadLine(outBuffer, sizeof(outBuffer)))
	{
		// Failed to read port
		Logger::GetSingleton()->Write("Failed to read port.", LogLevel::Information);
		return retVal;
	}

	char updsCmd2[] = "AT+UPSD=0,7,\"0.0.0.0\"\r";
	if (!SerialHandler::GetSingleton()->WritePort(updsCmd2, sizeof(updsCmd2)))
		return retVal;

	// Read what we wrote
	*outBuffer = 0;
	SerialHandler::GetSingleton()->ReadLine(outBuffer, sizeof(outBuffer));

	Logger::GetSingleton()->Write("Waiting for OK after second command.", LogLevel::Information);
	if (!SerialHandler::GetSingleton()->ReadLine(outBuffer, sizeof(outBuffer)))
	{
		// Failed to read port
		Logger::GetSingleton()->Write("Failed to read port.", LogLevel::Information);
		return retVal;
	}

	char updsCmd3[] = "AT+UPSDA=0,3\r";
	if (!SerialHandler::GetSingleton()->WritePort(updsCmd3, sizeof(updsCmd2)))
		return retVal;

	// Read what we wrote
	*outBuffer = 0;
	SerialHandler::GetSingleton()->ReadLine(outBuffer, sizeof(outBuffer));

	Logger::GetSingleton()->Write("Waiting for OK after third command.", LogLevel::Information);
	if (!SerialHandler::GetSingleton()->ReadLine(outBuffer, sizeof(outBuffer)))
	{
		// Failed to read port
		Logger::GetSingleton()->Write("Failed to read port.", LogLevel::Information);
		return retVal;
	}

	// Timeout of 30 seconds, accuracy of 10m
	char _locCmd[] = "AT+ULOC=2,3,0,30,10\r";
	if (!SerialHandler::GetSingleton()->WritePort(_locCmd, sizeof(_locCmd)))
		return retVal;

	// Read what we wrote
	*outBuffer = 0;
	SerialHandler::GetSingleton()->ReadLine(outBuffer, sizeof(outBuffer));

	Logger::GetSingleton()->Write("Done setting up cell for location.", LogLevel::Information);

	// Modem should reply with 'OK'
	if (!SerialHandler::GetSingleton()->ReadLine(outBuffer, sizeof(outBuffer)))
	{
		// Failed to read port
		Logger::GetSingleton()->Write("Failed to read port.", LogLevel::Information);
		return retVal;
	}

	Logger::GetSingleton()->Write("Read %s from port.", LogLevel::Information, outBuffer);

	if (str_equal(outBuffer, "OK\r"))
	{
		Logger::GetSingleton()->Write("Waiting for location...", LogLevel::Information);
		// Now we need to wait for the actual location data
		*outBuffer = 0;
		if (!SerialHandler::GetSingleton()->ReadLine(outBuffer, sizeof(outBuffer)))
		{
			// Failed to read
			// Shouldn't happen
			Logger::GetSingleton()->Write("Failed to read port.", LogLevel::Information);
			return retVal;
		}

		Logger::GetSingleton()->Write("Read %s from port.", LogLevel::Information, outBuffer);

		if (!SerialHandler::GetSingleton()->ReadLine(outBuffer, sizeof(outBuffer)))
		{
			// Failed to read
			// Shouldn't happen
			Logger::GetSingleton()->Write("Failed to read port.", LogLevel::Information);
			return retVal;
		}

		Logger::GetSingleton()->Write("Read %s from port.", LogLevel::Information, outBuffer);

		// Make sure the reply starts with +
		// The reply string should be +UULOC: <date>,<time>,<Lat>,<Lon>,<Altitude>,<Uncertainty>
		if (*outBuffer == '+')
		{
			Logger::GetSingleton()->Write("Received data: %s", LogLevel::Information, outBuffer);

			char* cmdContext = nullptr, *context = nullptr;
			// Split the string

			char* cmdData = (char*)safe_strtok(outBuffer + 1, ':', &cmdContext);
			if (str_equal(cmdData, "UULOC"))
			{
				// Make sure it was a UULOC response
				char* actualData = (char*)safe_strtok(0, ' ', &cmdContext);
				if (actualData)
				{
					float latitude = 0.0f, longitude = 0.0f;

					// Now we have the data after the +UULOC: bit, we need to split it based on commas
					char* token = (char*)safe_strtok(actualData, ',', &context);
					if (*context != ',')
					{
						// We have a date
						Logger::GetSingleton()->Write("Date: %s", LogLevel::Information, token);
					}
					else 
						context++;

					token = (char*)safe_strtok(0, ',', &context);
					if (*context != ',')
					{
						// We have the time
						Logger::GetSingleton()->Write("Time: %s", LogLevel::Information, token);
					}
					else
						context++;

					token = (char*)safe_strtok(0, ',', &context);
					if (*context != ',')
					{
						// We have the latitude
						Logger::GetSingleton()->Write("Lat: %s", LogLevel::Information, token);
						latitude = atof(token);
					}
					else
						context++;

					token = (char*)safe_strtok(0, ',', &context);
					if (*context != ',')
					{
						// We have the longitude
						Logger::GetSingleton()->Write("Lon: %s", LogLevel::Information, token);
						longitude = atof(token);
					}
					else
						context++;

					token = (char*)safe_strtok(0, ',', &context);
					if (*token)
					{
						// We have the altitude
						Logger::GetSingleton()->Write("Altitude: %s", LogLevel::Information, token);
					}

					addLocation(m_locationShm, latitude, longitude, 0.0f);

					retVal = std::make_tuple(latitude, longitude);
				}
			}
		}

		return retVal;
	}
}

