#include "NtpUpdater.h"
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/time.h>
#include <math.h>

template<>
NtpUpdater* ISingleton< NtpUpdater >::m_singleton = nullptr;

NtpUpdater::NtpUpdater()
	: m_shmUnit(nullptr)
{
	SetupShmLink();
}


NtpUpdater::~NtpUpdater()
{
	if (m_shmUnit)
	{
		shmdt(m_shmUnit);
		m_shmUnit = nullptr;
	}
}

void NtpUpdater::SetNTPTime(unsigned long date, float time)
{
	struct tm unixtm;

	unixtm.tm_year = ((date % 100) < 80) ? (date % 100) + 100 : (date % 100);
	unixtm.tm_mon = ((date % 10000) / 100) - 1;
	unixtm.tm_mday = date / 10000;
	unixtm.tm_hour = (unsigned int)(time / 10000.0f);
	unixtm.tm_min = ((unsigned int)(time) % 10000) / 100;
	unixtm.tm_sec = ((unsigned int)(time) % 100);

	time_t dectime = mktime(&unixtm);
	if (dectime == -1)
		return;
	
	struct timeval gpsclocktv, localrecvtv;

	gettimeofday(&localrecvtv, 0);

	gpsclocktv.tv_sec = ++dectime;
	gpsclocktv.tv_usec = 0;

	// TODO: Wait for PPS before setting the time

	SetTime(&gpsclocktv, &localrecvtv, 0.000001f, 0);
}

void NtpUpdater::SetupShmLink()
{
	int shmid = shmget(0x4e545030, sizeof(struct shmTime), IPC_CREAT | 0777);
	if (shmid == -1)
		return;

	shmTime* p = (shmTime*)shmat(shmid, 0, 0);
	if ((int)(long)p != -1)
		m_shmUnit = p;
}

void NtpUpdater::SetTime(const struct timeval* gpsClock, const struct timeval* localClock, float err, unsigned int leap)
{
	if (!m_shmUnit)
		return;

	m_shmUnit->valid = 0;
	m_shmUnit->mode = 1;

	++m_shmUnit->count;
	m_shmUnit->clockTimeStampSec = gpsClock->tv_sec;
	m_shmUnit->clockTimeStampUSec = gpsClock->tv_usec;
	m_shmUnit->receiveTimeStampSec = localClock->tv_sec;
	m_shmUnit->receiveTimeStampUSec = localClock->tv_usec;
	m_shmUnit->leap = leap;
	m_shmUnit->precision = log(err) / log(2.0f);

	++m_shmUnit->count;

	m_shmUnit->valid = 1;
}
