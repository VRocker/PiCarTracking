#include "../shared/Logger.h"
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include "../shared/locationshm.h"
#include "SerialHandler.h"
#include "../shared/config.h"

static bool g_isRunning = true;

static shmLocation* g_gpsLocationShm = nullptr;

void exited()
{
	system("kill `pidof pppd`");
	Logger::GetSingleton()->Write("Terminating.", LogLevel::Information);
	Logger::CleanupSingleton();
}

void sig_handler(int signum)
{
	if ((signum == SIGTERM) || (signum == SIGINT))
		g_isRunning = false;
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
	Logger::SetSingleton(new Logger("/var/log/celluploader"));
	Logger::GetSingleton()->SetLogLevel(LogLevel::All);
	Logger::GetSingleton()->EnableScreenOutput(true);

	Logger::GetSingleton()->Write("Initialising Cellular uploader process...", LogLevel::Information);
	Logger::GetSingleton()->Write("Backgrounding...", LogLevel::Information);
	daemonise();

	atexit(exited);
	signal(SIGTERM, sig_handler);
	signal(SIGINT, sig_handler);

	Logger::GetSingleton()->Write("Opening configuration file...", LogLevel::Information);
	Config::SetSingleton(new Config("/picar/etc/picar.conf"));

	Logger::GetSingleton()->Write("Opening shared memory segment for GPS location data...", LogLevel::Information);
	g_gpsLocationShm = getShmLocation(LocationUnitIDs::GPS);
	if (!g_gpsLocationShm)
	{
		Logger::GetSingleton()->Write("Failed to open shared memory segment for GPS location data.", LogLevel::Error);
		return 1;
	}

	Logger::GetSingleton()->Write("Connecting to cellular network...", LogLevel::Information);
	// I know... using system() is bad practice but it does the job (and its not like this accepts user input...)
	system("/usr/sbin/pppd call hologram.provider");
	Logger::GetSingleton()->Write("Connected!", LogLevel::Information);

	Logger::GetSingleton()->Write("Opening serial port...", LogLevel::Information);
	// Serial port lives on ttyS0 on the pi zero
	if (!SerialHandler::GetSingleton()->OpenPort("/dev/ttyACM1"))
	{
		Logger::GetSingleton()->Write("Failed to open serial port. Exiting...", LogLevel::Error);
		return 1;
	}

	SerialHandler::GetSingleton()->FlushPort();

	// How often should we report the coordinates? Default is 5 minutes
	unsigned int reportingInterval = 300;
	
	if (!Config::GetSingleton()->ReadItemInt("ReportingInterval", &reportingInterval))
	{
		Logger::GetSingleton()->Write("Failed to read reporting interval. Using default...", LogLevel::Warning);
	}

	char msgbuf[64] = { 0 };

	while (g_isRunning)
	{
		Logger::GetSingleton()->Write("Tick.", LogLevel::Information);

		sleep(reportingInterval);
	}

	return 0;
}