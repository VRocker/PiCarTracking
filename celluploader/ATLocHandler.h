#pragma once

#include "../shared/ISingleton.h"
#include <tuple>

struct shmLocation;

class ATLocHandler : public ISingleton<ATLocHandler>
{
public:
	ATLocHandler();
	virtual ~ATLocHandler();

	bool Setup(const char* portName);
	void Shutdown();

	std::tuple<float, float> GrabNetworkLocation();

private:
	bool m_portOpened;

	shmLocation* m_locationShm;
};

