#pragma once

#include "../shared/ISingleton.h"

class SerialHandler : public ISingleton<SerialHandler>
{
public:
	SerialHandler();
	virtual ~SerialHandler();

	bool OpenPort(const char* device);
	void ClosePort();

	bool ReadPort(char* buffer, unsigned int bytes);
	void FlushPort(void);

private:
	int m_fd;
	int m_ppsHandle;
};

