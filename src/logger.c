#include "logger.h"


#include <assert.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/syscall.h>
#include <time.h>
#include <unistd.h>


#include "exceptions.h"


#define INDEX_STR_LEN 11


typedef struct ConsoleLogger {
    FILE *output;
    unsigned long long flushed_time;
} ConsoleLogger;


typedef struct FileLogger {
    FILE *output;
    char filename[FILE_NAME_MAX_LEN];
    long max_filesize;
    unsigned char max_backup_files;
    long current_filesize;
    unsigned long long flushed_time;
} FileLogger;


static ConsoleLogger clogger;
static bool is_consolelog_initialized = false;
static FileLogger flogger;
static bool is_filelog_initialized = false; 
static LogLevel log_level = LogLevel_INFO;
static long flush_interval = 0; // msec, 0 is auto flush off


void logger_init_consolelog(FILE *output) {
    assert(output == stdout || output == stderr);
    clogger.output = output;
    is_consolelog_initialized = true;
}


static long get_file_size(const char *filename) {
    long size = 0;

    FILE *f = fopen(filename, "rb");
    if(f == NULL) {
        return X_FAILED;
    }

    int ret = fseek(f, 0, SEEK_END);
    if(ret == -1) {
        goto fail;
    }
    size = ftell(f);
    if(size == -1) {
        goto fail;
    }

    fclose(f);
    return size;

fail:
    fclose(f);
    return X_FAILED;
}


int logger_init_filelog(const char *filename, long max_filesize, unsigned int max_backup_files) {
    assert(filename != NULL);
    assert(strlen(filename) < FILE_NAME_MAX_LEN);
    assert(max_filesize > 0);

    is_filelog_initialized = true;
    if(flogger.output != NULL) { // reinit
        fclose(flogger.output);
    }
    flogger.output = fopen(filename, "a");
    if(flogger.output == NULL) {
        fprintf(stderr, "logger: Failed to open log file: %s\n", filename);
        return X_FAILED;
    }

    flogger.current_filesize = get_file_size(filename);
    strncpy(flogger.filename, filename, strlen(filename));
    flogger.max_filesize = (max_filesize > 0) ? max_filesize : DEFAULT_MAX_FILE_SIZE;
    flogger.max_backup_files = max_backup_files;
    return 0;
}


void logger_set_level(LogLevel level) {
    log_level = level;
}


LogLevel logger_get_level(void) {
    return log_level;
}


static int logger_is_enabled(LogLevel level) {
    return log_level <= level;
}


void logger_autoflush(long interval) {
    flush_interval = interval > 0 ? interval : 0;
}


void logger_flush() {
    if(is_consolelog_initialized) {
        fflush(clogger.output);
    }
    if(is_filelog_initialized) {
        fflush(flogger.output);
    }
}


static char get_level_char(LogLevel level) {
    switch(level) {
        case LogLevel_DEBUG: return 'D';
        case LogLevel_INFO:  return 'I';
        case LogLevel_ERROR: return 'E';
        default: return ' ';
    }
}


static void get_timestamp(const struct timeval *time, char *timestamp, size_t size) {
    assert(size >= 25);

    struct tm calendar;
    localtime_r(&time->tv_sec, &calendar);
    strftime(timestamp, size, "%y-%m-%d %H:%M:%S", &calendar);
    sprintf(&timestamp[17], ".%06ld", (long) time->tv_usec);
}


static void get_backup_filename(const char* basename, unsigned int index,
        char* backupname, size_t size) {
    char index_str[INDEX_STR_LEN];
    assert(size >= strlen(basename) + sizeof(index_str));

    strncpy(backupname, basename, size);
    if(index > 0) {
        sprintf(index_str, ".%d", index);
        strncat(backupname, index_str, strlen(index_str));
    }
}


static bool is_file_exist(const char *filename) {
    FILE* f = fopen(filename, "r");
    if(f == NULL) {
        return false;
    } else {
        fclose(f);
        return true;
    }
}


static int rotateLogFiles(void) {
    // backup filename: <filename>.xxx (xxx: 1-255)
    char src[FILE_NAME_MAX_LEN + INDEX_STR_LEN];
    char dst[FILE_NAME_MAX_LEN + INDEX_STR_LEN];

    if(flogger.current_filesize < flogger.max_filesize) {
        return s_flog.output != NULL;
    }
    fclose(s_flog.output);
    for (i = (int) s_flog.max_backup_files; i > 0; i--) {
        getBackupFileName(s_flog.filename, i - 1, src, sizeof(src));
        getBackupFileName(s_flog.filename, i, dst, sizeof(dst));
        if (isFileExist(dst)) {
            if (remove(dst) != 0) {
                fprintf(stderr, "ERROR: logger: Failed to remove file: `%s`\n", dst);
            }
        }
        if (isFileExist(src)) {
            if (rename(src, dst) != 0) {
                fprintf(stderr, "ERROR: logger: Failed to rename file: `%s` -> `%s`\n", src, dst);
            }
        }
    }
    s_flog.output = fopen(s_flog.filename, "a");
    if (s_flog.output == NULL) {
        fprintf(stderr, "ERROR: logger: Failed to open file: `%s`\n", s_flog.filename);
        return 0;
    }
    s_flog.current_filesize = getFileSize(s_flog.filename);
    return 1;
}

static long vflog(FILE* fp, char levelch, const char* timestamp, long threadID,
        const char* file, int line, const char* fmt, va_list arg,
        unsigned long long currentTime, unsigned long long* flushed_time)
{
    int size;
    long totalsize = 0;

    if ((size = fprintf(fp, "%c %s %ld %s:%d: ", levelch, timestamp, threadID, file, line)) > 0) {
        totalsize += size;
    }
    if ((size = vfprintf(fp, fmt, arg)) > 0) {
        totalsize += size;
    }
    if ((size = fprintf(fp, "\n")) > 0) {
        totalsize += size;
    }
    if (s_flushInterval > 0) {
        if (currentTime - *flushed_time > s_flushInterval) {
            fflush(fp);
            *flushed_time = currentTime;
        }
    }
    return totalsize;
}


void logger_log(LogLevel level, const char* file, int line, const char* fmt, ...) {
    va_list carg, farg;

    if(!logger_isEnabled(level)) {
        return;
    }

    struct timeval now;
    gettimeofday(&now, NULL);
    unsigned long long currentTime = now.tv_sec * 1000 + now.tv_usec / 1000; // milliseconds
    char levelch = getLevelChar(level);

    char timestamp[32];
    get_timestamp(&now, timestamp, sizeof(timestamp));

    if(is_consolelog_initialized) {
        va_start(carg, fmt);
        vflog(s_clog.output, levelch, timestamp,
                file, line, fmt, carg, currentTime, &s_clog.flushed_time);
        va_end(carg);
    }
    if(is_filelog_initialized) {
        if (rotateLogFiles()) {
            va_start(farg, fmt);
            s_flog.current_filesize += vflog(s_flog.output, levelch, timestamp,
                    file, line, fmt, farg, currentTime, &s_flog.flushed_time);
            va_end(farg);
        }
    }
}


void logger_destroy_filelog() {
    if(flogger.output) {
        fclose(flogger.output);
    }
}
