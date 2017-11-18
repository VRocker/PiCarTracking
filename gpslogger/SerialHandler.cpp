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
	: m_fd(-1), m_ppsFd(-1), m_ppsHandle(-1)
{
}


SerialHandler::~SerialHandler()
{
	ClosePPSPort();
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
	if ((tmpFd = open(name, O_RDONLY | O_NOCTTY)) < 0)
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

bool SerialHandler::OpenPPSPort(const char* device)
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
	if ((tmpFd = open(name, (O_RDWR | O_NOCTTY | O_NONBLOCK))) < 0)
	{
		Logger::GetSingleton()->Write("Unable to open pps port %s.", LogLevel::Error, name);
		return false;
	}

	m_ppsFd = tmpFd;

	if (time_pps_create(m_ppsFd, &m_ppsHandle) < 0)
	{
		Logger::GetSingleton()->Write("Failed to create PPS device.", LogLevel::Error);
		ClosePPSPort();
		return false;
	}

	return true;
}

void SerialHandler::ClosePPSPort()
{
	if (m_ppsHandle != -1)
	{
		time_pps_destroy(m_ppsHandle);
		m_ppsHandle = -1;
	}

	if (m_ppsFd != -1)
	{
		close(m_ppsFd);
		m_ppsFd = -1;
	}
}

bool SerialHandler::ReadPort(char* buffer, unsigned int bytes)
{
	if (m_fd == -1)
		return false;

	if (read(m_fd, buffer, bytes) == -1)
		return false;

	return true;
}

void SerialHandler::FlushPort(void)
{
	if (m_fd == -1)
		return;

	tcflush(m_fd, TCIFLUSH);
}

bool SerialHandler::WaitForPPS()
{
	pps_info_t ppsInfo;
	struct timespec timeout;

	timeout.tv_sec = 1;
	timeout.tv_nsec = 0;

	if (time_pps_fetch(m_ppsHandle, PPS_TSFMT_TSPEC, &ppsInfo, &timeout) < 0)
		return false;

	return true;
}