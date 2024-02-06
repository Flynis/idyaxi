#ifndef _LOGGER_H_INCLUDED_
#define _LOGGER_H_INCLUDED_


#include <stdio.h>
#include <string.h>


#define FILE_NAME_MAX_LEN 256
#define DEFAULT_MAX_FILE_SIZE 1048576L // 1 mb


#define LOG_DEBUG 1
#define LOG_INFO 2
#define LOG_ERROR 3
#define LOG_LEVEL LOG_DEBUG


#if (LOG_LEVEL <= LOG_DEBUG)
#define log_debug(fmt, ...) logger_log('D', fmt, ##__VA_ARGS__)
#else 
#define log_debug(fmt, ...) do {} while(0)
#endif    


#if (LOG_LEVEL <= LOG_INFO)
#define log_info(fmt, ...) logger_log('I', fmt, ##__VA_ARGS__)
#else 
#define log_info(fmt, ...) do {} while(0)
#endif 


#if (LOG_LEVEL <= LOG_ERROR)
#define log_error(fmt, ...) logger_log('E', fmt, ##__VA_ARGS__)
#else 
#define log_error(fmt, ...) do {} while(0)
#endif 


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
 * @param fmt A format string.
 */
void logger_log(char levelch, const char *fmt, ...);


#endif // _LOGGER_H_INCLUDED_
