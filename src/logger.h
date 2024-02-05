#ifndef _LOGGER_H_INCLUDED_
#define _LOGGER_H_INCLUDED_


#include <stdio.h>
#include <string.h>


#define FILE_NAME_MAX_LEN 256
#define DEFAULT_MAX_FILE_SIZE 1048576L // 1 mb


#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define log_debug(fmt, ...) logger_log(LogLevel_DEBUG, __FILENAME__, __LINE__, fmt, ##__VA_ARGS__)
#define log_info(fmt, ...)  logger_log(LogLevel_INFO , __FILENAME__, __LINE__, fmt, ##__VA_ARGS__)
#define log_error(fmt, ...) logger_log(LogLevel_ERROR, __FILENAME__, __LINE__, fmt, ##__VA_ARGS__)


typedef enum LogLevel {
    LogLevel_DEBUG,
    LogLevel_INFO,
    LogLevel_ERROR,
} LogLevel;


/**
 * Initializes the logger as a console logger.
 * @param output stdout or stderr.
 */
void logger_init_consolelog(FILE *output);


/**
 * Initializes the logger as a file logger.
 * @param filename the name of the output file. File name length must be < FILE_NAME_MAX_LEN.
 * @param max_filesize the maximum number of bytes to write to any one file.
 * @param max_backup_files the maximum number of files for backup.
 * @returns 0 on success, X_FAILED otherwise.
 */
int logger_init_filelog(const char *filename, long max_filesize, unsigned int max_fackup_files);


/**
 * Destroys file logger.
 */
void logger_destroy_filelog();


/**
 * Sets the log level.
 * Message levels lower than this value will be discarded.
 * The default log level is INFO.
 * @param level a log level.
 */
void logger_set_level(LogLevel level);


/**
 * Gets the log level that has been set.
 * The default log level is INFO.
 * @returns the log level.
 */
LogLevel logger_get_level(void);


/**
 * Flush automatically.
 * Auto flush is off in default.
 * @param interval A fulsh interval in milliseconds. Switch off if 0 or a negative integer.
 */
void logger_autoflush(long interval);


/**
 * Flush buffered log messages.
 */
void logger_flush(void);


/**
 * Log a message.
 * Make sure to call one of the following initialize functions before starting logging.
 * - logger_initConsoleLogger()
 * - logger_initFileLogger()
 *
 * @param level A log level.
 * @param file A file name string.
 * @param line A line number.
 * @param fmt A format string.
 */
void logger_log(LogLevel level, const char *file, int line, const char *fmt, ...);


#endif // _LOGGER_H_INCLUDED_
