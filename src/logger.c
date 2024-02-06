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
    unsigned int max_backup_files;
    long current_filesize;
    unsigned long long flushed_time;
} FileLogger;


static ConsoleLogger clogger;
static bool is_consolelog_initialized = false;
static FileLogger flogger;
static bool is_filelog_initialized = false; 
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
    assert(max_filesize >= 0);

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


static void get_timestamp(const struct timeval *time, char *timestamp, size_t size) {
    assert(size >= 25);

    struct tm calendar;
    localtime_r(&time->tv_sec, &calendar);
    strftime(timestamp, size, "%y-%m-%d %H:%M:%S", &calendar);
    sprintf(&timestamp[17], ".%06ld", (long) time->tv_usec);
}


static void get_backup_filename(const char *basename, unsigned int index,
        char *backupname, size_t size) {
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


static int rotate_log_files(void) {
    // backup filename: <filename>.xxxxxxxx
    char src[FILE_NAME_MAX_LEN + INDEX_STR_LEN];
    char dst[FILE_NAME_MAX_LEN + INDEX_STR_LEN];

    if(flogger.current_filesize < flogger.max_filesize) {
        return 0;
    }
    fclose(flogger.output);
    for(unsigned int i = flogger.max_backup_files; i > 0; i--) {
        get_backup_filename(flogger.filename, i - 1, src, sizeof(src));
        get_backup_filename(flogger.filename, i, dst, sizeof(dst));
        if(is_file_exist(dst)) {
            if(remove(dst) != 0) {
                fprintf(stderr, "logger: Failed to remove file: %s\n", dst);
            }
        }
        if(is_file_exist(src)) {
            if(rename(src, dst) != 0) {
                fprintf(stderr, "logger: Failed to rename file: %s -> %s\n", src, dst);
            }
        }
    }
    flogger.output = fopen(flogger.filename, "a");
    if (flogger.output == NULL) {
        fprintf(stderr, "logger: Failed to open file: %s\n", flogger.filename);
        return X_FAILED;
    }
    flogger.current_filesize = get_file_size(flogger.filename);
    return 0;
}


static long vflog(FILE *f, char levelch, const char *timestamp, 
        const char *fmt, va_list arg, unsigned long long currentTime, 
        unsigned long long *flushed_time) {
    int size;
    long totalsize = 0;

    if((size = fprintf(f, "[%c] %s:", levelch, timestamp)) > 0) {
        totalsize += size;
    }
    if((size = vfprintf(f, fmt, arg)) > 0) {
        totalsize += size;
    }
    if((size = fprintf(f, "\n")) > 0) {
        totalsize += size;
    }
    if(flush_interval > 0) {
        if(currentTime - *flushed_time > flush_interval) {
            fflush(f);
            *flushed_time = currentTime;
        }
    }
    return totalsize;
}


void logger_log(char levelch, const char *fmt, ...) {
    va_list carg, farg;

    struct timeval now;
    gettimeofday(&now, NULL);
    unsigned long long currentTime = now.tv_sec * 1000 + now.tv_usec / 1000; // milliseconds

    char timestamp[32];
    get_timestamp(&now, timestamp, sizeof(timestamp));

    if(is_consolelog_initialized) {
        va_start(carg, fmt);
        vflog(clogger.output, levelch, timestamp, fmt, carg, currentTime, 
                &clogger.flushed_time);
        va_end(carg);
    }
    if(is_filelog_initialized) {
        int ret = rotate_log_files();
        if(ret == X_FAILED) {
            return;
        }
        va_start(farg, fmt);
        flogger.current_filesize += vflog(flogger.output, levelch, timestamp,
                fmt, farg, currentTime, &flogger.flushed_time);
        va_end(farg);
    }
}


void logger_destroy_filelog() {
    if(flogger.output) {
        fclose(flogger.output);
    }
}
