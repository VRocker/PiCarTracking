#include "main.h"
#include "../shared/Logger.h"
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include "../shared/locationshm.h"
#include "SerialHandler.h"
#include "../shared/config.h"
#include "SocketHandler.h"

static bool g_isRunning = true;

static shmLocation* g_gpsLocationShm = nullptr;

static char g_deviceKey[64] = { 0 };

void exited()
{
	Logger::GetSingleton()->Write("Uploading coordinates on exit...", LogLevel::Information);
	uploadData();

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

	// Wait for a successful connection before proceeding
	while (!checkForPPPConnection())
		sleep(1);

	Logger::GetSingleton()->Write("Connected!", LogLevel::Information);

	SerialHandler::GetSingleton()->FlushPort();

	// How often should we report the coordinates? Default is 5 minutes
	unsigned int reportingInterval = 300;
	
	if (!Config::GetSingleton()->ReadItemInt("ReportingInterval", &reportingInterval))
	{
		Logger::GetSingleton()->Write("Failed to read reporting interval. Using default...", LogLevel::Warning);
	}

	Logger::GetSingleton()->Write("Reading Hologram device key...", LogLevel::Information);
	if (!Config::GetSingleton()->ReadItem("DeviceKey", g_deviceKey, sizeof(g_deviceKey)))
	{
		Logger::GetSingleton()->Write("Unable to read device key. Exiting...", LogLevel::Error);
		return 1;
	}

	// TODO: Upload initial coordinates from the Nova based on the 3g connection so we have some coordinates to work with

	// Wait until the GPS has a valid lock on the satellites
	Logger::GetSingleton()->Write("Waiting for initial GPS lock...", LogLevel::Information);
	waitForInitialFix();
	Logger::GetSingleton()->Write("GPS locked! Starting main loop...", LogLevel::Information);

	while (g_isRunning)
	{
		Logger::GetSingleton()->Write("Uploading data...", LogLevel::Information);

		uploadData();

		sleep(reportingInterval);
	}

	return 0;
}

void waitForInitialFix()
{
	while (g_isRunning)
	{
		// Check if the GPS says it has a satellite lock
		if (g_gpsLocationShm->valid)
		{
			// Exit the while loop, uploadData is the first thing done in the main loop
			break;
		}
		// Wait for 1 second
		sleep(1);
	}
}

void uploadData()
{
	// Create TCP socket to cloudsocket.hologram.io and upload data
	// echo '{"k":"<key>","d":"Test!","t":"test"}' | nc -i1 cloudsocket.hologram.io 9999
	// k = Device key, d = Data, t = Tags

	if (SocketHandler::GetSingleton()->Connect("cloudsocket.hologram.io", 9999))
	{
		char outBuffer[128] = { 0 };
		sprintf(outBuffer, "{\"k\":\"%s\",\"d\":\"{%f,%f,%f}\",\"t\":\"location\"", g_deviceKey, g_gpsLocationShm->longitude, g_gpsLocationShm->latitude, g_gpsLocationShm->speed);
		SocketHandler::GetSingleton()->Send(outBuffer, strlen(outBuffer));
	}
	else
		Logger::GetSingleton()->Write("Failed to connect  to Hologram Cloud.", LogLevel::Error);
}

bool checkForPPPConnection()
{
	char buffer[64] = { 0 };
	bool retVal = false;

	FILE* pNet = popen("ifconfig | grep 'ppp0'", "r");
	if (pNet)
	{
		if (fgets(buffer, sizeof(buffer), pNet))
		{
			if (*buffer)
				retVal = true;
		}

		pclose(pNet);
		pNet = 0;
	}

	return retVal;
}