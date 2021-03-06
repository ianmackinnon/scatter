#ifndef __OPT_H__
#define __OPT_H__



void substitutePositiveInteger(char ** dst, char * src, unsigned int f);


int getOptionsScatterMean(LogLevel *logLevel,
                          char ***inPathList, unsigned int *nInPath, char **outPath,
                          unsigned int *fGiven, unsigned int *fStart, unsigned int *fEnd, unsigned int *fStep,
                          unsigned int *force, unsigned int *exposure,
                          int argc, char **argv
                          );
int getOptionsScatterSigma(LogLevel *logLevel,
                           char ***inPathList, unsigned int *nInPath, char **outPath, char **meanPath,
                           unsigned int *fGiven, unsigned int *fStart, unsigned int *fEnd, unsigned int *fStep,
                           unsigned int *force,
                           int argc, char **argv
                           );
int getOptionsScatterColorBar(LogLevel *logLevel,
                              char ***inPathList, unsigned int *nInPath,
                              unsigned int *fGiven, unsigned int *fStart, unsigned int *fEnd, unsigned int *fStep,
                              int argc, char **argv
                              );
int getOptionsScatterRolling(LogLevel *logLevel,
                             char ***inPathList, unsigned int *nInPath, char **outPath,
                             unsigned int *fGiven, unsigned int *fStart, unsigned int *fEnd, unsigned int *fStep,
                             unsigned int *force, double *exposure, double *rollOffset,
                             int argc, char **argv
                             );

int freePathList(char ***inPathList, unsigned int nInPath);

#endif // __OPT_H__

