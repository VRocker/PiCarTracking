#include "SocketHandler.h"
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <memory.h>
#include "../shared/Logger.h"

template<>
SocketHandler* ISingleton<SocketHandler>::m_singleton = nullptr;

SocketHandler::SocketHandler()
	: m_sock(-1)
{
}

SocketHandler::~SocketHandler()
{
	Disconnect();
}

bool SocketHandler::Connect(const char* host, unsigned int port)
{
	if (m_sock != -1)
	{
		// Already connceted
		return false;
	}

	Logger::GetSingleton()->Write("Setting up socket...", LogLevel::Information);
	m_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (m_sock < 0)
		return false;

	Logger::GetSingleton()->Write("Finding hostent", LogLevel::Information);
	struct hostent* hostEnt = gethostbyname(host);
	if (!hostEnt)
	{
		Disconnect();
		return false;
	}
	
	struct sockaddr_in serverAddr;
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = *((unsigned long *)(hostEnt->h_addr));
	serverAddr.sin_port = htons(port);

	Logger::GetSingleton()->Write("Connecting...", LogLevel::Information);

	if (connect(m_sock, (const struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0)
	{
		Disconnect();
		return false;
	}
	else
		return true;

	return false;
}

void SocketHandler::Disconnect()
{
	if (m_sock == -1)
		return;

	close(m_sock);
	m_sock = -1;
}

void SocketHandler::Send(const char* msg, unsigned int msgLen)
{
	if (m_sock == -1)
		return;

	send(m_sock, msg, msgLen, 0);
}
