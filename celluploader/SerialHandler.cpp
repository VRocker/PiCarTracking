#include "SerialHandler.h"
#include <string.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "../shared/strutils.h"
#include "../shared/Logger.h"
#include <sys/timepps.h>

template<>
SerialHandler* ISingleton< SerialHandler >::m_singleton = nullptr;

SerialHandler::SerialHandler()
	: m_fd(-1)
{
}


SerialHandler::~SerialHandler()
{
	ClosePort();
}

bool SerialHandler::OpenPort(const char* device)
{
	char name[32] = { 0 };

	if (*device != '/')
	{
		strcpy(name, "/dev/");
		str_cat(name, device, sizeof(name));
	}
	else
		str_cpy(name, device, sizeof(name));

	int tmpFd = -1;
	if ((tmpFd = open(name, O_RDWR | O_NOCTTY)) < 0)
	{
		Logger::GetSingleton()->Write("Unable to open port %s.", LogLevel::Error, name);
		return false;
	}

	m_fd = tmpFd;

	struct termios portset;
	tcgetattr(m_fd, &portset);
	cfmakeraw(&portset);
	cfsetispeed(&portset, B9600);

	tcsetattr(m_fd, TCSANOW | TCSAFLUSH, &portset);

	return true;
}

void SerialHandler::ClosePort()
{
	if (m_fd != -1)
	{
		close(m_fd);
		m_fd = -1;
	}
}

bool SerialHandler::ReadPort(char* buffer, unsigned int bytes)
{
	if (m_fd == -1)
		return false;

	//ioctl(m_fd, FIONREAD, &bytes);

	buffer[0] = 0;

	if (read(m_fd, buffer, bytes) == -1)
		return false;

	return true;
}

/*i = 1;
			do
			{
				SerialHandler::GetSingleton()->ReadPort(msgbuf + i, 1);
			} while ((msgbuf[i] != '\r') && (msgbuf[i++] != '\n') && (i < sizeof(msgbuf) - 1));
			msgbuf[i] = 0;
*/
bool SerialHandler::ReadLine(char* buffer, unsigned int bytes)
{
	if (m_fd == -1)
		return false;

	unsigned int i = 0;
	do
	{
		if (read(m_fd, buffer + i, 1) == -1)
			return false;
	} while ((buffer[i] != '\n') && (i++ < bytes - 1));

	buffer[i] = 0;

	return true;
}

bool SerialHandler::WritePort(const char* buffer, unsigned int bytes)
{
	if (m_fd == -1)
		return false;

	Logger::GetSingleton()->Write("Writing %u bytes: %s", LogLevel::Information, bytes, buffer);
	if (write(m_fd, buffer, bytes) == -1)
		return false;

	return true;
}

void SerialHandler::FlushPort(void)
{
	if (m_fd == -1)
		return;

	tcflush(m_fd, TCIFLUSH);
}
