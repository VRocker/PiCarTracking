#pragma once

#include "../../shared/ISingleton.h"
#include "ublox_structures.h"

namespace ublox
{
	class Ublox : public ISingleton<Ublox>
	{
	public:
		Ublox();
		~Ublox();

		bool PollMessage(MessageClasses msgClass, uint8_t msgId);

		void ParseUbloxMessage(char* msgBuffer, size_t msgLength);

	private:
		void CalculateChecksum(uint8_t* in, size_t len, uint8_t* out);

	private:
		void ParseAidIniMessage(char* msgBuffer, size_t msgLength);
	};

};
