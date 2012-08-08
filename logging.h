#ifndef __LOGGING_H__
#define __LOGGING_H__



typedef enum
   {
     QUIET,
     FATAL,    
     ERROR,    
     WARNING,
     INFO,
     DEBUG,
   } LogLevel;



void logFatal(const char* , ... );
void logError(const char* , ... );
void logWarning(const char* , ... );
void logInfo(const char* , ... );
void logDebug(const char* , ... );

void logTest();



#endif // __LOGGING_H__
