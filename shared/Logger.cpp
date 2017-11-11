#include "Logger.h"
#include <stdarg.h>
#include <time.h>
#include <sys/time.h>

template<>
Logger* ISingleton<Logger>::m_singleton = nullptr;

Logger::Logger()
	: m_file(nullptr), m_logLevel(LogLevel::All), m_printToScreen(false)
{
}

Logger::Logger(const char* logName)
	: m_file(nullptr), m_logLevel(LogLevel::None), m_printToScreen(false)
{
	m_file = fopen(logName, "a+");
	if (!m_file)
	{
		fprintf(stderr, "Failed to open log file '%s'\n", logName);
	}
}

Logger::~Logger()
{
	if (m_file)
	{
		fflush(m_file);

		fclose(m_file);
		m_file = nullptr;
	}
}

void Logger::Write(const char* text, const LogLevel level, ...)
{
	if (!loglevel_flags(level & m_logLevel))
		return;

	char output[256] = { 0 };
	va_list marker;
	va_start(marker, level);
	vsnprintf(output, sizeof(output), text, marker);
	va_end(marker);

	timeval curTime;
	gettimeofday(&curTime, 0);
	unsigned long milli = (unsigned long)(curTime.tv_usec / 1000);

	char timeBuf[80] = { 0 };
	strftime(timeBuf, sizeof(timeBuf), "%d/%m/%Y %H:%M:%S", localtime(&curTime.tv_sec));

	char buffer[338] = { 0 };
	size_t len = sprintf(buffer, "<%s.%.3u> %.256s\n", timeBuf, milli, output);

	if (m_printToScreen)
	{
		if (loglevel_flags(level & LogLevel::Error))
			fprintf(stderr, "%s", buffer);
		else
			fprintf(stdout, "%s", buffer);
	}

	if (m_file)
	{
		fwrite(buffer, 1, len, m_file);
	}
}
