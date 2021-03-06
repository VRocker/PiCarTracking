#include "main.h"
#include "../shared/Logger.h"
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include "../shared/locationshm.h"
#include "../shared/config.h"
#include "SocketHandler.h"
#include "ATLocHandler.h"

static bool g_isRunning = true;

static time_t g_lastLocationTime = 0;
static shmLocation* g_gpsLocationShm = nullptr;

static float g_longitude = 0.0f, g_latitude = 0.0f;

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

	// TODO: Upload initial coordinates from the Nova based on the 3g connection so we have some coordinates to work with
	{
		Logger::GetSingleton()->Write("Attempting to locate...", LogLevel::Information);
		if (!ATLocHandler::GetSingleton()->Setup("/dev/ttyACM0"))
		{
			Logger::GetSingleton()->Write("Failed to setup ATLoc stuff.", LogLevel::Error);
			return 1;
		}

		// Grab location and date info from the network.
		Logger::GetSingleton()->Write("Grabbing location data from network...", LogLevel::Information);
		float initialLong = 0.0f, initialLat = 0.0f;
		std::tie(initialLat, initialLong) = ATLocHandler::GetSingleton()->GrabNetworkLocation();
		g_longitude = initialLong;
		g_latitude = initialLat;
	}

	Logger::GetSingleton()->Write("Connecting to cellular network...", LogLevel::Information);
	// I know... using system() is bad practice but it does the job (and its not like this accepts user input...)
	system("/usr/sbin/pppd call hologram.provider");

	// Wait for a successful connection before proceeding
	while (!checkForPPPConnection())
		sleep(1);

	Logger::GetSingleton()->Write("Connected!", LogLevel::Information);

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
	Logger::GetSingleton()->Write("Hologram device key set to: %s", LogLevel::Information, g_deviceKey);

	Logger::GetSingleton()->Write("Sending initial location based on 3G modem.", LogLevel::Information);
	uploadData();

	// Wait until the GPS has a valid lock on the satellites
	Logger::GetSingleton()->Write("Waiting for initial GPS lock...", LogLevel::Information);
	waitForInitialFix();
	Logger::GetSingleton()->Write("GPS locked! Starting main loop...", LogLevel::Information);


	while (g_isRunning)
	{
		if ( (g_gpsLocationShm->lastUpdated != g_lastLocationTime) && (g_gpsLocationShm->valid))
		{
			// Make sure the data has changed since the last time we uploaded and that it's currently valid
			Logger::GetSingleton()->Write("Uploading data...", LogLevel::Information);
			g_longitude = g_gpsLocationShm->longitude;
			g_latitude = g_gpsLocationShm->latitude;

			uploadData();
			g_lastLocationTime = g_gpsLocationShm->lastUpdated;

			sleep(reportingInterval);
		}
		else
		{
			// The data hasn't changed, keep trying every second until it has
			// TODO: If the data is invalid for too long, send the 3g modem coordinates
			sleep(1);
		}
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

	if (!*g_deviceKey)
		return;

	if (SocketHandler::GetSingleton()->Connect("cloudsocket.hologram.io", 9999))
	{
		char outBuffer[128] = { 0 };
		sprintf(outBuffer, "{\"k\":\"%s\",\"d\":\"{%f,%f,%f}\",\"t\":\"location\"}", g_deviceKey, g_longitude, g_latitude, g_gpsLocationShm->speed);
		SocketHandler::GetSingleton()->Send(outBuffer, strlen(outBuffer));

		Logger::GetSingleton()->Write("Sent %s.", LogLevel::Information, outBuffer);
	}
	else
		Logger::GetSingleton()->Write("Failed to connect  to Hologram Cloud.", LogLevel::Error);

	SocketHandler::GetSingleton()->Disconnect();
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
