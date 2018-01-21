#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include "../shared/Logger.h"
#include "SerialHandler.h"
#include "NmeaParser.h"
#include "nmea/GprmcMessage.h"
#include "nmea/GpggaMessage.h"
#include "NtpUpdater.h"
#include "../shared/locationshm.h"
#include "../shared/gpsutils.h"
#include <pthread.h>
#include "ublox/Ublox.h"
#include "FileWriter.h"

static bool g_isRunning = true;

static shmLocation* g_locationShm = nullptr;
static pthread_t g_ppsThread = 0;
static FileWriter* g_fileWriter = nullptr;

void exited()
{
	if (g_fileWriter)
	{
		Logger::GetSingleton()->Write("Closing file writer...", LogLevel::Information);
		delete g_fileWriter;
		g_fileWriter = nullptr;
	}

	if (g_ppsThread != 0)
	{
		Logger::GetSingleton()->Write("Waiting for PPS thread to exit...", LogLevel::Information);
		void* status = 0;
		pthread_join(g_ppsThread, &status);
	}

	Logger::GetSingleton()->Write("Shutting down NTP updater...", LogLevel::Information);
	NtpUpdater::CleanupSingleton();

	Logger::GetSingleton()->Write("Shutting down serial handler...", LogLevel::Information);
	SerialHandler::CleanupSingleton();

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

GprmcMessage* g_rmcMsg = nullptr;
volatile bool g_rmcMessageValid = false;

void* ppsThread(void* threadId)
{
	while (g_isRunning)
	{
		if (SerialHandler::GetSingleton()->WaitForPPS())
		{
			if ((g_rmcMessageValid) && (g_rmcMsg))
			{
				printf("[2] Setting time to %u - %f...\n", g_rmcMsg->Date(), g_rmcMsg->Timestamp());
				// Update the NTP shared memory segment
				NtpUpdater::GetSingleton()->SetNTPTime(g_rmcMsg->Date(), g_rmcMsg->Timestamp());

				g_rmcMessageValid = false;

				delete g_rmcMsg;
				g_rmcMsg = nullptr;
			}
		}
	}

	pthread_exit(0);
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

	Logger::GetSingleton()->Write("Opening PPS port...", LogLevel::Information);

	if (!SerialHandler::GetSingleton()->OpenPPSPort("/dev/pps0"))
	{
		Logger::GetSingleton()->Write("Failed to open PPS port. Exiting...", LogLevel::Error);
		return 1;
	}

	Logger::GetSingleton()->Write("Opening shared memory segment for location data...", LogLevel::Information);

	g_locationShm = getShmLocation(LocationUnitIDs::GPS);
	if (!g_locationShm)
	{
		Logger::GetSingleton()->Write("Failed to open shared memory segment for GPS location data.", LogLevel::Error);
		return 1;
	}

	{
		time_t startTime;
		time(&startTime);

		char fileName[128] = { 0 };
		sprintf(fileName, "/gpsdata/logs/%u.log", startTime);
		// Open the file to store the GPS data
		g_fileWriter = new FileWriter(fileName);
	}


	// Flush the port so we don't end up with old crap
	SerialHandler::GetSingleton()->FlushPort();

	//ublox::Ublox::GetSingleton()->PollMessage(ublox::MessageClasses::Aid, (uint8_t)ublox::MessageIDAid::Ini);

	// Setup PPS thread
	if (pthread_create(&g_ppsThread, NULL, ppsThread, NULL) > 0)
	{
		Logger::GetSingleton()->Write("Failed to setup PPS thread. Exiting...", LogLevel::Error);
		return 1;
	}

	char msgbuf[128] = { 0 };
	unsigned int i = 0;

	while (g_isRunning)
	{
		// Read the string byte by byte until we find a \r\n, then parse
		// Read the first byte to determine what message this is
		SerialHandler::GetSingleton()->ReadPort(msgbuf, 1);

		//printf("%s", msgbuf);

		if (*msgbuf == '$')
		{
			i = 1;
			do
			{
				SerialHandler::GetSingleton()->ReadPort(msgbuf + i, 1);
			} while ((msgbuf[i] != '\r') && (msgbuf[i++] != '\n') && (i < sizeof(msgbuf) - 1));
			msgbuf[i] = 0;

			// Parse the message
			INmeaMessage* msg = NmeaParser::ParseMessage(msgbuf);
			if (msg)
			{
				// Do stuff
				switch (msg->GetType())
				{
				case NmeaType::RMC:
				{
					// GPRMC String
					GprmcMessage* rmcMsg = (GprmcMessage*)msg;
					if (rmcMsg->HasFix())
					{
						{
							float lat = 0.0f, lon = 0.0f;
							float speedInMps = 0.0f;
							convertSpeedToMPS(rmcMsg->Speed(), &speedInMps);

							convertNMEAToLatLon(rmcMsg->Latitude(), rmcMsg->LatitudeDirection(), rmcMsg->Longitude(), rmcMsg->LongitudeDirection(), &lat, &lon);
							// Add the lat, long and speed (in m/s) to the shared memory block
							addLocation(g_locationShm, lat, lon, speedInMps);

							//printf("Setting location data to Lat: %f, Lon: %f\n", lat, lon);
						}

						if ((!g_rmcMsg) && (!g_rmcMessageValid))
						{
							printf("[1] Setting time to %u - %f...\n", rmcMsg->Date(), rmcMsg->Timestamp());
							g_rmcMsg = new GprmcMessage(*rmcMsg);
							g_rmcMessageValid = true;
						}

						// Write to the GPS log
						g_fileWriter->WriteLine(msgbuf, i);

						/*if (SerialHandler::GetSingleton()->WaitForPPS())
						{
							printf("Setting time...\n");
							// Update the NTP shared memory segment
							NtpUpdater::GetSingleton()->SetNTPTime(rmcMsg->Date(), rmcMsg->Timestamp());
						}*/
					}
					else
						printf("No fix :(\n");
				}
				break;

				case NmeaType::GGA:
				{
					// GPGGA String
					GpggaMessage* ggaMsg = (GpggaMessage*)msg;
					printf("Sats in view: %u\n", ggaMsg->SatsInView());

				}
				break;

				default:
				{

				}

				}

				delete msg;
				msg = nullptr;
			}
		}
		else if (*msgbuf == 0xB5)
		{
			// Received UBX message
			// Read 6 bytes to construct the header
			// Read payload_length bytes to get the payload
			// Read 2 bytes to get the checksum
			SerialHandler::GetSingleton()->ReadPort(msgbuf + 1, 1);
			if (msgbuf[1] == 0x62)
			{
				// Make sure its definately a UBX packet
				// Read the rest of the header
				SerialHandler::GetSingleton()->ReadPort(msgbuf + 2, 4);
				uint16_t payloadLen = *(uint16_t*)(msgbuf + 4);
				SerialHandler::GetSingleton()->ReadPort(msgbuf + 6, payloadLen + 2);

				// We add 8 bytes on for the header and checksum bytes
				ublox::Ublox::GetSingleton()->ParseUbloxMessage(msgbuf, payloadLen + 8);
			}
		}
	}

	return 0;
}
