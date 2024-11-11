#pragma once

#include <fstream>
#include <string.h>

#define _WINSOCKAPI_ // To avoid including winsock.h in Windows.h
#include <windows.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <shlobj.h>
#include <time.h>
#include <sys/stat.h>
#include <direct.h>
#include <filesystem>

/**
 * ATTENTION!!!
 * DO NOT CALL `Logger::getLogger().println()` OR ANY `Logger::getLogger()` METHODS DIRECTLY.
 * USE DEDICATED MACROS LIKE `logline()` OR `logger_setLogLevel()` ETC.
 * IT ENSURES THAT NO DEBUG CALLS END UP IN NON-DEBUG BUILDS.
 * 
 *  - sincerely Luz
 * (ten logger jest w pelni autoryzowany przez Alana)
 */

#ifndef NDEBUG

#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)

#define logger_spawnConsole() Logger::getLogger().spawnCMD()
#define logger_spawnLogFile() Logger::getLogger().spawnLogFile()
#define logger_setTimePrecision(precision) Logger::getLogger().setTimePrecision(precision)
#define logger_setLoglvl(lvl) Logger::getLogger().setLoglvl(lvl)
#define logline(lvl, format, ...) Logger::getLogger().println(lvl, __FILENAME__, __LINE__, __func__, format, __VA_ARGS__)
#define loginfo(format, ...) Logger::getLogger().println(Loglvl::INFO, __FILENAME__, __LINE__, __func__, format, __VA_ARGS__)
#define logwarn(format, ...) Logger::getLogger().println(Loglvl::WARN, __FILENAME__, __LINE__, __func__, format, __VA_ARGS__)
#define logerr(format, ...) Logger::getLogger().println(Loglvl::ERR, __FILENAME__, __LINE__, __func__, format, __VA_ARGS__)
/**
 * \enum Loglvl
 * \brief Defines the log levels for the Logger.
 *
 * This enumeration defines the levels of logging detail:
 * - \b INFO: Informational messages that highlight the progress of the application at a coarse-grained level.
 * - \b WARN: Potentially harmful situations which still allow the application to continue running.
 * - \b ERR: [DEFAULT] Error events that might prevent application from working correcly or cause crash.
 */
enum Loglvl {
	INFO	= 0,
	WARN	= 1,
	ERR		= 2
};

/**
 * \enum TimePrecision
 * \brief Defines the time precision levels for the Logger.
 *
 * This enumeration defines the levels of time detail to include in log messages:
 * - \b NONE: No time information is included.
 * - \b PARTIAL: [DEFAULT] Includes hour and minute.
 * - \b PRECISE: Includes hour, minute, and second.
 * - \b FULL: Includes hour, minute, second, and millisecond.
 */
enum TimePrecision {
	NONE,
	PARTIAL,
	PRECISE,
	FULL,
};

class Logger
{
	Loglvl loglvl;

	TimePrecision includeTime;

	HANDLE hConsole;

	bool logFileSpawned;

	bool failedToOpenFile;

	bool DbgIsInit;

	bool spawnedCMD;

	FILE* logfile;

	Logger();

	~Logger();


public:

	/**
	 * \brief Spawns a command line console for logging output.
	 *
	 * This method creates a new command line console for logging output if one does not already exist.
	 * If a console has already been spawned, a warning message is logged and the method returns without
	 * creating a new console.
	 *
	 * \note The console is set up to redirect standard output, standard error, and standard input streams.
	 *
	 * \note Usage Example:
	 * \code{.cpp}
	 * // Spawn a command line console for logging
	 * Logger::getLogger().spawnCMD();
	 * \endcode
	 */
	void spawnCMD();

	/**
	 * \brief Retrieves the singleton instance of the Logger class.
	 *
	 * This method returns a reference to the single instance of the Logger class, ensuring that there is only
	 * one instance of the logger throughout the application. The singleton instance is created on the first call
	 * to this method and reused on subsequent calls.
	 *
	 * \return A reference to the singleton Logger instance.
	 *
	 * \note Usage Example:
	 * \code{.cpp}
	 * // Retrieve the singleton logger instance and log an informational message
	 * Logger& logger = Logger::getLogger();
	 * logger.println(INFO, "This is an informational message");
	 * \endcode
	 */
	static Logger& getLogger();

	void spawnLogFile();

	/**
	 * \brief Logs a formatted message to the console and/or log file based on the specified log level.
	 *
	 * This method logs a message to the appropriate output streams if the specified log level meets or exceeds
	 * the logger's configured log level. The message is formatted using a printf-like format string and
	 * additional arguments.
	 *
	 * \param lvl The log level of the message. Must be one of the predefined log levels: INFO, WARN, or ERR.
	 *            Messages with a log level lower than the logger's current log level are ignored.
	 * \param format The format string, similar to printf, specifying how the additional arguments are formatted in the message.
	 * \param ... Additional arguments to be formatted according to the format string.
	 *
	 * \note Usage Guidelines:
	 * - Ensure that \p lvl is a valid log level.
	 * - The \p format string should follow printf-style formatting.
	 * - Provide a corresponding argument for each format specifier in the \p format string.
	 *
	 * \code{.cpp}
	 * // Example: Log an informational message
	 * Logger::getLogger().println(INFO, "This is an informational message with a number: %d", 42);
	 *
	 * // Example: Log a warning message
	 * Logger::getLogger().println(WARN, "This is a warning message with a string: %s", "warning");
	 *
	 * // Example: Log an error message
	 * Logger::getLogger().println(ERR, "This is an error message with two values: %d and %s", 100, "error");
	 * \endcode
	 */
	void println(Loglvl lvl, std::string filename, int line, std::string func, std::string format, ...);

	/**
	 * \brief Sets the log level for the Logger.
	 *
	 * This method sets the minimum log level for messages to be logged. Messages with a log level lower than
	 * the specified level will be ignored.
	 *
	 * \param lvl The log level to set. Must be one of the predefined log levels: INFO, WARN, or ERR.
	 *
	 * \note Log Levels:
	 * - \b INFO: Informational messages that highlight the progress of the application at a coarse-grained level.
	 * - \b WARN: Potentially harmful situations which still allow the application to continue running.
	 * - \b ERR: Error events that might prevent application from working correcly or cause crash.
	 *
	 * \note Usage Example:
	 * \code{.cpp}
	 * // Set the logger to only log warnings and errors
	 * Logger::getLogger().setLoglvl(WARN);
	 * \endcode
	 */
	void setLoglvl(Loglvl lvl);

	/**
	 * \brief Sets the time precision for the Logger.
	 *
	 * This method sets the level of time detail to include in each log message. The precision can range from
	 * no time information at all to full detail including milliseconds.
	 *
	 * \param precision The time precision to set. Must be one of the predefined precision levels: NONE, PARTIAL, PRECISE, or FULL.
	 *
	 * \note Time Precision Levels:
	 * - \b NONE: No time information is included.
	 * - \b PARTIAL: Includes hour and minute.
	 * - \b PRECISE: Includes hour, minute, and second.
	 * - \b FULL: Includes hour, minute, second, and millisecond.
	 *
	 * \note Usage Example:
	 * \code{.cpp}
	 * // Set the logger to include precise time (hour, minute, second) in each log message
	 * Logger::getLogger().setTimePrecision(PRECISE);
	 * \endcode
	 */
	void setTimePrecision(TimePrecision precision);

};


#else

#define logger_spawnConsole()
#define logger_spawnLogFile()
#define logger_setTimePrecision(precision)
#define logger_setLoglvl(lvl)
#define logline(lvl, format, ...)
#define loginfo(format, ...)
#define logwarn(format, ...)
#define logerr(format, ...)

#endif // NDEBUG