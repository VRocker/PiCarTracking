#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include "../shared/Logger.h"
#include "SerialHandler.h"

void exited()
{
	Logger::GetSingleton()->Write("Terminating.", LogLevel::Information);
	Logger::CleanupSingleton();
}

void sig_handler(int signum)
{
	if ((signum == SIGTERM) || (signum == SIGINT))
		exit(1);
}

void daemonise(void)
{
	int pid = 0;
	if ((pid = fork()) < 0)
	{
		Logger::GetSingleton()->Write("Well that forking failed.", LogLevel::Error);
		exit(1);
	}

	if (pid > 0)
		exit(0);
	else
	{
		Logger::GetSingleton()->Write("Entering daemon mode.", LogLevel::Information);
		setsid();
	}
}

int main(int argc, char* argv[])
{
	Logger::SetSingleton(new Logger("/var/log/gpslogger"));
	Logger::GetSingleton()->SetLogLevel(LogLevel::All);
	Logger::GetSingleton()->EnableScreenOutput(true);


	Logger::GetSingleton()->Write("Initialising GPS Logger process...", LogLevel::Information);
	Logger::GetSingleton()->Write("Backgrounding...", LogLevel::Information);
	daemonise();

	// Add the atexit hook after daemonising else bad things will happen..
	atexit(exited);
	signal(SIGTERM, sig_handler);
	signal(SIGINT, sig_handler);

	Logger::GetSingleton()->Write("Opening serial port...", LogLevel::Information);
	// Serial port lives on ttyS0 on the pi zero
	if (!SerialHandler::GetSingleton()->OpenPort("/dev/ttyS0"))
	{
		Logger::GetSingleton()->Write("Failed to open serial port. Exiting...", LogLevel::Error);
		return 1;
	}

	// Flush the port so we don't end up with old crap
	SerialHandler::GetSingleton()->FlushPort();

	char msgbuf[128] = { 0 };
	unsigned int i = 0;

	// Read the string byte by byte until we find a \r\n, then parse
	do
	{
		SerialHandler::GetSingleton()->ReadPort(msgbuf + i, 1);
	} while ((msgbuf[i] != '\r') && (msgbuf[i++] != '\n') && (i < sizeof(msgbuf) - 1));
	msgbuf[i] = 0;
	return 0;
}
