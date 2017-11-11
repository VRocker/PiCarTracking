#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include "../shared/Logger.h"
#include "SerialHandler.h"
#include "NmeaParser.h"
#include "nmea/GprmcMessage.h"
#include "nmea/GpggaMessage.h"

static bool g_isRunning = true;

void exited()
{
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

	while (g_isRunning)
	{
		// Read the string byte by byte until we find a \r\n, then parse
		i = 0;
		do
		{
			SerialHandler::GetSingleton()->ReadPort(msgbuf + i, 1);
		} while ((msgbuf[i] != '\r') && (msgbuf[i++] != '\n') && (i < sizeof(msgbuf) - 1));
		msgbuf[i] = 0;

		printf("%s", msgbuf);

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
					printf("Has fix.\n");
				else
					printf("No fix :(\n");
			}
			break;

			case NmeaType::GGA:
			{
				// GPGGA String
				GpggaMessage* ggaMsg = (GpggaMessage*)msg;
				printf("Sats in view: %u", ggaMsg->SatsInView());
			}
			break;
			}

			delete msg;
			msg = nullptr;
		}
	}

	return 0;
}
