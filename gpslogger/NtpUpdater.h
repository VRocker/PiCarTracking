#pragma once

#include "../shared/ISingleton.h"
#include <time.h>

class NtpUpdater : public ISingleton<NtpUpdater>
{
public:
	NtpUpdater();
	~NtpUpdater();

	void SetNTPTime(unsigned long date, float time);

private:
	void SetupShmLink();

	void SetTime(const struct timeval* gpsClock, const struct timeval* localClock, float err, unsigned int leap);

private:
	struct shmTime
	{
		int mode;
		volatile int count;
		time_t clockTimeStampSec;
		int clockTimeStampUSec;
		time_t receiveTimeStampSec;
		int receiveTimeStampUSec;
		int leap;
		int precision;
		int nsamples;
		volatile int valid;
		unsigned int clockTimeStampNSec;     /* Unsigned ns timestamps */
		unsigned int receiveTimeStampNSec;   /* Unsigned ns timestamps */
		int dummy[8];
	};

private:
	shmTime* m_shmUnit;
};

