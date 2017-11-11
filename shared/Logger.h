#pragma once

#include "ISingleton.h"
#include <stdio.h>

enum class LogLevel
{
	None = 0,
	Critical = 1,
	Error = 2,
	Warning = 4,
	Information = 8,
	User = 16,
	Internal = 32,
	Debug = 64,
	All = 0xFF
};

inline LogLevel	operator	&	(LogLevel x, LogLevel y) { return static_cast<LogLevel>	(static_cast<int>(x) & static_cast<int>(y)); };
inline LogLevel	operator	|	(LogLevel x, LogLevel y) { return static_cast<LogLevel>	(static_cast<int>(x) | static_cast<int>(y)); };
inline LogLevel	operator	^	(LogLevel x, LogLevel y) { return static_cast<LogLevel>	(static_cast<int>(x) ^ static_cast<int>(y)); };
inline LogLevel	operator	~	(LogLevel x) { return static_cast<LogLevel>	(~static_cast<int>(x)); };
inline LogLevel&	operator	&=	(LogLevel& x, LogLevel y) { x = x & y;	return x; };
inline LogLevel&	operator	|=	(LogLevel& x, LogLevel y) { x = x | y;	return x; };
inline LogLevel&	operator	^=	(LogLevel& x, LogLevel y) { x = x ^ y;	return x; };

inline bool	loglevel_flags(LogLevel x) { return static_cast<int>(x) != 0; };

class Logger : public ISingleton<Logger>
{
public:
	Logger();
	Logger(const char* logName);
	virtual ~Logger();

	void Write(const char* text, const LogLevel level, ...);

	LogLevel GetLogLevel(void) { return m_logLevel; }
	void SetLogLevel(const LogLevel level) { m_logLevel = level; }

	void EnableScreenOutput(const bool b) { m_printToScreen = b; }

private:
	FILE* m_file;
	LogLevel m_logLevel;
	bool m_printToScreen;
};

