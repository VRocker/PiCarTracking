#pragma once

#include "../shared/ISingleton.h"

class SocketHandler : public ISingleton<SocketHandler>
{
public:
	SocketHandler();
	~SocketHandler();

	bool Connect(const char* host, unsigned int port);
	void Disconnect();

	void Send(const char* msg, unsigned int msgLen);

private:
	int m_sock;
};

