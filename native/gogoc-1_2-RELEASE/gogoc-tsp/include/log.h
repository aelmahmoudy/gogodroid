/*
-----------------------------------------------------------------------------
 $Id: log.h,v 1.1 2009/11/20 16:53:15 jasminko Exp $
-----------------------------------------------------------------------------
  Copyright (c) 2007 gogo6 Inc. All rights reserved.

  For license information refer to CLIENT-LICENSE.TXT.
-----------------------------------------------------------------------------
*/

#ifndef LOG_H
#define LOG_H


#define LOG_LEVEL_DISABLED      0
#define LOG_LEVEL_1             1
#define LOG_LEVEL_2             2
#define LOG_LEVEL_3             3

#define LOG_LEVEL_MIN           LOG_LEVEL_DISABLED
#define LOG_LEVEL_MAX           LOG_LEVEL_3
#define LOG_IDENTITY_MAX_LENGTH 32
#define LOG_FILENAME_MAX_LENGTH 255
#define MAX_LOG_LINE_LENGTH     4096
#define LOG_IDENTITY            "gogoc"
#define DEFAULT_LOG_FILENAME    "gogoc.log"
#define DEFAULT_LOG_ROTATION_SIZE 32

enum tSeverityLevel
{
  ELError,
  ELWarning,
  ELInfo,
  ELDebug
};

typedef struct stLogConfiguration {
  char *    identity;
  char *    log_filename;
  sint32_t  log_level_stderr;
  sint32_t  log_level_console;
  sint32_t  log_level_syslog;
  sint32_t  log_level_file;
  sint32_t  syslog_facility;
  sint32_t  log_rotation_size;
  sint32_t  log_rotation;
  sint32_t  buffer;
  sint32_t  delete_rotated_log;       // 0 = FALSE
} tLogConfiguration;

sint32_t            DirectErrorMessage    (char *message, ...);
void                Display               (sint32_t, enum tSeverityLevel, const char *, char *, ...);
sint32_t            LogConfigure          (tLogConfiguration *);
void                LogClose              (void);
sint32_t            DumpBufferToFile      (char *filename);

#endif
