#include "Ublox.h"
#include <string.h>
#include "../SerialHandler.h"
#include <stdio.h>

namespace ublox
{
	template<>
	Ublox* ISingleton<Ublox>::m_singleton = nullptr;

	Ublox::Ublox()
	{
	}


	Ublox::~Ublox()
	{
	}

	bool Ublox::PollMessage(MessageClasses msgClass, uint8_t msgId)
	{
		char message[8];

		UBloxHeader header;
		header.messageClass = msgClass;
		header.messageId = msgId;
		header.payloadLength = 0;

		memcpy(message, &header, 6);

		uint8_t* msgPtr = (uint8_t*)&message;
		CalculateChecksum(msgPtr + 2, 4, msgPtr + 6);

		for (int i = 0; i < 8; i++)
		{
			printf("0x%.2x ", message[i]);
		}
		printf("\n");

		return SerialHandler::GetSingleton()->WritePort((char*)msgPtr, 8);
	}

	void Ublox::ParseUbloxMessage(char * msgBuffer, size_t msgLength)
	{
		if (msgLength > 6)
		{
			UBloxHeader header;
			memcpy(&header, msgBuffer, 6);

			if ((header.sync1 == UBX_SYNC_BYTE_1) && (header.sync2 == UBX_SYNC_BYTE_2))
			{
				switch (header.messageClass)
				{
				case MessageClasses::Aid:
				{
					ParseAidIniMessage(msgBuffer, header.payloadLength + 8);
				}
				break;
				}
			}
		}
	}

	void Ublox::CalculateChecksum(uint8_t * in, size_t len, uint8_t * out)
	{
		uint8_t a = 0;
		uint8_t b = 0;

		for (uint8_t i = 0; i < len; ++i)
		{
			a = a + in[i];
			b += a;
		}

		out[0] = (a & 0xFF);
		out[1] = (b & 0xFF);
	}

	void Ublox::ParseAidIniMessage(char * msgBuffer, size_t msgLength)
	{
		AidIni iniMessage;
		if (msgLength < sizeof(iniMessage))
		{
			printf("Something went wrong. %u - %u\n", msgLength, sizeof(iniMessage));
			return;
		}
		memcpy(&iniMessage, msgBuffer, sizeof(iniMessage));

		printf("AID INI: \n");
		printf("ecefXorLat: %u\n", iniMessage.ecefXorLat);
		printf("ecefYorLat: %u\n", iniMessage.ecefYorLon);
		printf("ecefZorLat: %u\n", iniMessage.ecefZorAlt);
		printf("position_accuracy: %u\n", iniMessage.position_accuracy);
		printf("time_configuration: %u\n", iniMessage.time_configuration);
		printf("week_number: %u\n", iniMessage.week_number);
		printf("time_of_week: %u\n", iniMessage.time_of_week);
		printf("time_of_week_ns: %u\n", iniMessage.time_of_week_ns);
		printf("time_accuracy_ms: %u\n", iniMessage.time_accuracy_ms);
		printf("time_accuracy_ns: %u\n", iniMessage.time_accuracy_ns);
		printf("clock_drift_or_freq: %u\n", iniMessage.clock_drift_or_freq);
		printf("clock_drift_or_freq_accuracy: %u\n", iniMessage.clock_drift_or_freq_accuracy);
		printf("flags: %u\n", iniMessage.flags);
	}

}
