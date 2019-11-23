#pragma once

#include <sys/types.h>
#include <stdint.h>

namespace ublox
{

#define HDR_CHECKSUM_LEN	8
#define UBX_SYNC_BYTE_1		0xB5
#define UBX_SYNC_BYTE_2		0x62

#ifdef _MSC_VER // using MSVC
#define PACK( __Declaration__ ) __pragma( pack(push, 1) ) __Declaration__ __pragma( pack(pop) )
#else
#define PACK( __Declaration__ ) __Declaration__ __attribute__((__packed__))
#endif

	enum class MessageClasses : uint8_t
	{
		Nav = 0x01,
		Rxm = 0x02,
		Inf = 0x04,
		Ack = 0x05,
		Cfg = 0x06,
		Mon = 0x0A,
		Aid = 0x0B,
		Tim = 0x0D
	};

	enum class MessageIDAck : uint8_t
	{
		Nak = 0x00,
		Ack = 0x01,
	};

	enum class MessageIDAid : uint8_t
	{
		Req = 0x00,
		Ini = 0x01,
		Hui = 0x02,
		Data = 0x10,
		Alm = 0x30,
		Eph = 0x31,
		AlpSrv = 0x32,
		Aop = 0x33,
		Alp = 0x50,
	};

	enum class MessageFlagsAidIni
	{
		PositionValid = 0x01,
		TimeValid = 0x02,
		ClockDriftValid = 0x04,
		UseTimePulese = 0x08,
		ClockFreqValid = 0x10,
		UseLLA = 0x20,
		AltitudeInvalid = 0x40,
		UsePrevTimePulse = 0x80,
		TimeInUTC = 0x400
	};

#pragma pack(push, 1)
	struct UBloxHeader
	{
		UBloxHeader()
			: sync1(UBX_SYNC_BYTE_1), sync2(UBX_SYNC_BYTE_2)
		{}

		uint8_t sync1;
		uint8_t sync2;
		MessageClasses messageClass;
		uint8_t messageId;
		uint16_t payloadLength;
	};
#pragma pack(pop)

#pragma pack(push, 1)
	struct AidIni
	{
		AidIni()
			: ecefXorLat(0), ecefYorLon(0), ecefZorAlt(0), position_accuracy(0), time_configuration(0), week_number(0), 
			time_of_week(0), time_of_week_ns(0), time_accuracy_ms(0), time_accuracy_ns(0), clock_drift_or_freq(0), clock_drift_or_freq_accuracy(0),
			flags(0)
		{
			header.messageClass = MessageClasses::Aid;
			header.messageId = (uint8_t)MessageIDAid::Ini;
			header.payloadLength = 48;
		}

		UBloxHeader header;
		int32_t ecefXorLat;  //!< ECEF x position or latitude [cm or deg*1e-7]
		int32_t ecefYorLon;  //!< ECEF y position or longitude [cm or deg*1e-7]
		int32_t ecefZorAlt;  //!< ECEF z position or altitude [cm]
		uint32_t position_accuracy; //!< position accuracy - std dev [cm]
		uint16_t time_configuration; //!< time configuration bit misk
		uint16_t week_number; //!< actual week number
		uint32_t time_of_week; //!< actual time of week [ms]
		int32_t time_of_week_ns; //!< fractional part of time of week [ns]
		uint32_t time_accuracy_ms; //!< time accuracy [ms]
		uint32_t time_accuracy_ns; //!< time accuracy [ns]
		int32_t clock_drift_or_freq; //!< clock drift or frequency [ns/s or Hz*1e-2]
		uint32_t clock_drift_or_freq_accuracy; //!< clock drift or frequency accuracy [ns/s or ppb]
		uint32_t flags; //!< bit field that determines contents of other fields
		uint8_t checksum[2];
	};
#pragma pack(pop)
};
