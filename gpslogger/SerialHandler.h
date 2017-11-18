#pragma once

#include "../shared/ISingleton.h"

class SerialHandler : public ISingleton<SerialHandler>
{
public:
	SerialHandler();
	virtual ~SerialHandler();

	bool OpenPort(const char* device);
	void ClosePort();

	bool OpenPPSPort(const char* device);
	void ClosePPSPort();

	bool ReadPort(char* buffer, unsigned int bytes);
	void FlushPort(void);

	bool WaitForPPS();

private:
	int m_fd;
	int m_ppsFd;
	int m_ppsHandle;
};

