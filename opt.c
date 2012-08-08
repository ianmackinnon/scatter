#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <math.h>
#include <regex.h>
#include <unistd.h>

#include "logging.h"



void checkPositiveFrameRange(unsigned int frameStart, unsigned int frameEnd, unsigned int frameIncrement)
{
  if (frameStart > frameEnd)
    {
      logFatal("End frame is lower than start frame.");
    }

  if (frameIncrement <= 0)
    {
      logFatal("Frame step must be a positive integer.");
    }
}


int parsePositiveFrameRange(const char *optarg, unsigned int *frameStart, unsigned int *frameEnd, unsigned int *frameIncrement)
{
  // End frame is inclusive.
  //

  int status;
  regex_t timeRegex;
  int timeRegexStatus;

  status = regcomp(&timeRegex, "^\\(0\\|[1-9][0-9]*\\)[-]\\(0\\|[1-9][0-9]*\\)x[1-9][0-9]*$", 0);
  if (status) logFatal("Failed to compile regular expression.");
  timeRegexStatus = regexec(&timeRegex, optarg, (size_t)0, NULL, 0);
  regfree(&timeRegex);
  if (timeRegexStatus == 0)
    {
      if (sscanf(optarg, "%u-%ux%u", frameStart, frameEnd, frameIncrement) != 3)
	logFatal("Time option scanning error");
      checkPositiveFrameRange(*frameStart, *frameEnd, *frameIncrement);
      return 0; // Success
    }

  status = regcomp(&timeRegex, "^\\(0\\|[1-9][0-9]*\\)[-]\\(0\\|[1-9][0-9]*\\)$", 0);
  if (status) logFatal("Failed to compile regular expression.");
  timeRegexStatus = regexec(&timeRegex, optarg, (size_t)0, NULL, 0);
  regfree(&timeRegex);
  if (timeRegexStatus == 0)
    {
      if (sscanf(optarg, "%u-%u", frameStart, frameEnd) != 2)
	logFatal("Time option scanning error");
      *frameIncrement = 1;
      checkPositiveFrameRange(*frameStart, *frameEnd, *frameIncrement);
      return 0; // Success
    }

  status = regcomp(&timeRegex, "^\\(0\\|[1-9][0-9]*\\)$", 0);
  if (status) logFatal("Failed to compile regular expression.");
  timeRegexStatus = regexec(&timeRegex, optarg, (size_t)0, NULL, 0);
  regfree(&timeRegex);
  
  if (timeRegexStatus == 0)
    {
      if (sscanf(optarg, "%u", frameStart) != 1)
	logFatal("Time option scanning error");

      *frameEnd = *frameStart;
      *frameIncrement = 1;
      checkPositiveFrameRange(*frameStart, *frameEnd, *frameIncrement);
      return 0; // Success
    }

  return 1; // Fail
}



int canSubstitutePositiveIntever(char *s)
{
  regex_t regex;
  int status;
  status = regcomp(&regex, "%0*[1-9][0-9]*d", 0);
  if (status) logFatal("Failed to compile regular expression.");
  status = regexec(&regex, s, (size_t)0, NULL, 0);
  regfree(&regex);
  return !status;
}



unsigned int substitutePositiveIntegerLength(char *s)
{
  regex_t regex;
  int status;
  size_t nMatch = 5;
  regmatch_t match[nMatch];
  status = regcomp(&regex, "^.*%0*([1-9][0-9]*)d.*$", REG_EXTENDED);
  if (status) logFatal("Failed to compile regular expression.");
  status = regexec(&regex, s, nMatch, match, 0);
  if (status || match[1].rm_so == (size_t)-1) logFatal("Did not match.");
  regfree(&regex);
  return atoi(s + match[1].rm_so);
}



void substitutePositiveInteger(char ** dst, char * src, unsigned int f)
{
  // Allocates memory for dst. Caller's responsibility to delete.
  //
  size_t pathSize = 0;
  pathSize += strlen(src);
  pathSize += substitutePositiveIntegerLength(src);
  pathSize += (int)ceil(log10((double)f));
  *dst = (char *) malloc(pathSize);
  sprintf(*dst, src, f);
}




int getOptionsScatterMean(LogLevel *logLevel,
                          char ***inPathList, unsigned int *nInPath, char **outPath,
                          unsigned int *fGiven, unsigned int *fStart, unsigned int *fEnd, unsigned int *fStep,
                          unsigned int *force,
                          int argc, char **argv
                          )
{
  int c;

  *logLevel = WARNING;

  *force = 0;

  *fGiven = 0;
  *fStart = 0;
  *fEnd = 0;
  *fStep = 1;

  while (1)
    {
      static struct option long_options[] =
	{
	  {"verbose",  no_argument,        0, 'v'},
	  {"quiet",    no_argument,        0, 'q'},
	  {"force",    no_argument,        0, 'f'},
	  {"time",     required_argument,  0, 't'},
	  {0, 0, 0, 0}
	};
      
      int option_index = 0;
     
      c = getopt_long (argc, argv, "vqft:",
		       long_options, &option_index);  
      
      if (c == -1)
	break;

      switch (c)
	{
	case 0:
	  if (long_options[option_index].flag != 0)
	    break;

	  printf ("option %s", long_options[option_index].name);
	  if (optarg)
	    printf (" with arg %s", optarg);
	  printf ("\n");
	  break;
     
	case 'v' : // verbose output
	  *logLevel += 1;
	  if (*logLevel > DEBUG)
	    *logLevel = DEBUG;
	  break;
	  
	case 'q' : // quiet output
	  *logLevel -= 1;
	  if (*logLevel < QUIET)
	    *logLevel = QUIET;
	  break;
	  
	case 'f' : // quiet output
	  *force = 1;
	  break;
	  
	case 't' : // frame range (inclusive)
	  if (parsePositiveFrameRange(optarg, fStart, fEnd, fStep))
	    {
	      logFatal("Time option argument is invalid. Acceptable forms are 1, 1-10, 1-10x2");
	      break;
	    }
          *fGiven = 1;

	case '?':
	  /* getopt_long already printed an error message. */
	  break;
     
	default:
	  abort ();
	}
    }
     
  if (optind > argc - 2)
    {
      logFatal("At least one input and one output argument are required.");
    }

  *inPathList = NULL;
  *nInPath = 0;
  while (optind < argc - 1)
    {
      *inPathList = (char**)realloc(*inPathList, ((*nInPath) + 1) * sizeof(char*));
      (*inPathList)[*nInPath] = (char*)malloc((strlen(argv[optind]) + 1));
      strcpy((*inPathList)[*nInPath], argv[optind]);
      (*nInPath)++;
      optind++;
    }

  *outPath = (char*)malloc((strlen(argv[optind]) + 1));
  strcpy(*outPath, argv[optind]);
  optind++;





  if (*fGiven)
    {
      if (*nInPath > 1)
        {
          logFatal("Cannot supply time argument and multiple input paths.");
        }

      if (!canSubstitutePositiveIntever((*inPathList)[0]))
        {
          logFatal("Input path must contain a %%d parameter for frame substitution, eg. %%d, %%4d, %%04d.");
        }
    }

  if(access(*outPath, F_OK) != -1) {
    // file exists
    if (! (*force))
      {
        logFatal("Destination path exists (use -f to force overwrite).");
      }
    
  }


  return 0;  // Win.
}



int getOptionsScatterSigma(LogLevel *logLevel,
                          char ***inPathList, unsigned int *nInPath, char **outPath, char **meanPath,
                          unsigned int *fGiven, unsigned int *fStart, unsigned int *fEnd, unsigned int *fStep,
                          unsigned int *force,
                          int argc, char **argv
                          )
{
  int c;

  *logLevel = WARNING;

  *force = 0;

  *fGiven = 0;
  *fStart = 0;
  *fEnd = 0;
  *fStep = 1;

  while (1)
    {
      static struct option long_options[] =
	{
	  {"verbose",  no_argument,        0, 'v'},
	  {"quiet",    no_argument,        0, 'q'},
	  {"force",    no_argument,        0, 'f'},
	  {"time",     required_argument,  0, 't'},
	  {0, 0, 0, 0}
	};
      
      int option_index = 0;
     
      c = getopt_long (argc, argv, "vqft:",
		       long_options, &option_index);  
      
      if (c == -1)
	break;

      switch (c)
	{
	case 0:
	  if (long_options[option_index].flag != 0)
	    break;

	  printf ("option %s", long_options[option_index].name);
	  if (optarg)
	    printf (" with arg %s", optarg);
	  printf ("\n");
	  break;
     
	case 'v' : // verbose output
	  *logLevel += 1;
	  if (*logLevel > DEBUG)
	    *logLevel = DEBUG;
	  break;
	  
	case 'q' : // quiet output
	  *logLevel -= 1;
	  if (*logLevel < QUIET)
	    *logLevel = QUIET;
	  break;
	  
	case 'f' : // quiet output
	  *force = 1;
	  break;
	  
	case 't' : // frame range (inclusive)
	  if (parsePositiveFrameRange(optarg, fStart, fEnd, fStep))
	    {
	      logFatal("Time option argument is invalid. Acceptable forms are 1, 1-10, 1-10x2");
	      break;
	    }
          *fGiven = 1;

	case '?':
	  /* getopt_long already printed an error message. */
	  break;
     
	default:
	  abort ();
	}
    }
     
  if (optind > argc - 3)
    {
      logFatal("At least one input, one mean image and one output argument are required.");
    }

  *inPathList = NULL;
  *nInPath = 0;
  while (optind < argc - 2)
    {
      *inPathList = (char**)realloc(*inPathList, ((*nInPath) + 1) * sizeof(char*));
      (*inPathList)[*nInPath] = (char*)malloc((strlen(argv[optind]) + 1));
      strcpy((*inPathList)[*nInPath], argv[optind]);
      (*nInPath)++;
      optind++;
    }

  *meanPath = (char*)malloc((strlen(argv[optind]) + 1));
  strcpy(*meanPath, argv[optind]);
  optind++;

  *outPath = (char*)malloc((strlen(argv[optind]) + 1));
  strcpy(*outPath, argv[optind]);
  optind++;





  if (*fGiven)
    {
      if (*nInPath > 1)
        {
          logFatal("Cannot supply time argument and multiple input paths.");
        }

      if (!canSubstitutePositiveIntever((*inPathList)[0]))
        {
          logFatal("Input path must contain a %%d parameter for frame substitution, eg. %%d, %%4d, %%04d.");
        }
    }

  if(access(*outPath, F_OK) != -1) {
    // file exists
    if (! (*force))
      {
        logFatal("Destination path exists (use -f to force overwrite).");
      }
    
  }


  return 0;  // Win.
}



int freePathList(char ***inPathList, unsigned int nInPath)
{
  unsigned int i;
  for (i = 0; i < nInPath; i++)
    {
      free((*inPathList)[i]);
    }
  free(*inPathList);
  return 0;  // Win.
}
