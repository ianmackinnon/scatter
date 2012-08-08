#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include "logging.h"



LogLevel logLevel = WARNING;



void logFatal(const char* format, ... )
{
  // Light blue

  va_list arglist;

  if (logLevel < FATAL) exit(1);

  fprintf(stderr, "\e[0;36mFatal: ");
  va_start( arglist, format );
  vfprintf(stderr, format, arglist );
  va_end( arglist );
  fprintf(stderr, "\nExiting...\n\e[0m");
  exit(1);
  
}



void logError(const char* format, ... )
{
  // Red

  va_list arglist;

  if (logLevel < ERROR) return;

  fprintf(stderr, "\e[0;31mError: ");
  va_start( arglist, format );
  vfprintf(stderr, format, arglist );
  va_end( arglist );
  fprintf(stderr, "\n\e[0m");
}



void logWarning(const char* format, ... )
{
  // Amber

  va_list arglist;

  if (logLevel < WARNING) return;

  fprintf(stderr, "\e[0;33mWarning: ");
  va_start( arglist, format );
  vfprintf(stderr, format, arglist );
  va_end( arglist );
  fprintf(stderr, "\n\e[0m");
}



void logInfo(const char* format, ... )
{
  // Green

  va_list arglist;

  if (logLevel < INFO) return;

  fprintf(stderr, "\e[0;32m");
  va_start( arglist, format );
  vfprintf(stderr, format, arglist );
  va_end( arglist );
  fprintf(stderr, "\n\e[0m");
}



void logDebug(const char* format, ... )
{
  // Purple

  va_list arglist;

  if (logLevel < DEBUG) return;

  fprintf(stderr, "\e[0;35m");
  va_start( arglist, format );
  vfprintf(stderr, format, arglist );
  va_end( arglist );
  fprintf(stderr, "\n\e[0m");
}



void logTest()
{
  logDebug("Debug");
  logInfo("Info");
  logWarning("Warning");
  logError("Error");
}

