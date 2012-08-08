#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "logging.h"
#include "io.h"
#include "opt.h"
#include "image.h"



extern LogLevel logLevel;



#define MAX_UPSCALE 4



int getTarget(char *srcPath, unsigned int  *targetWidth, unsigned int *targetHeight)
{
  if (sizeJpeg(srcPath, targetWidth, targetHeight)) {
    logFatal("%s: Cannot open file.", srcPath);
  }

  return 0;  // Win.
}



int printColorLine(char* srcPath)
{
  unsigned int width, height;
  double r, g, b;
  unsigned char * frameData = NULL;

  char timestampString[100] = "";
  struct stat st;

  if (!stat(srcPath, &st)) {
    strftime(timestampString, 100, "%d/%m/%Y %H:%M:%S", localtime( &st.st_mtime));
  }

  if (sizeJpeg(srcPath, &width, &height)) {
    logFatal("%s: Cannot open file.", srcPath);
  }

  frameData = (unsigned char*) malloc(width * height * N_COMPONENTS * sizeof(unsigned char));

  if (loadJpeg(srcPath, &frameData, &width, &height))
    {
      logWarning("Loading failed.");
      free(frameData);
      return 1;  // Fail.
    }
  imageColorMean(&r, &g, &b, frameData, width * height);
  printf("<div style='background:rgb(%d, %d, %d);'>%s | %s</div>\n", 
         (unsigned char)round(r), (unsigned char)round(g), (unsigned char)round(b),
         srcPath, timestampString);
  free(frameData);

  return 0;  // Win.
}



int main (int argc, char **argv)
{
  char ** inPathList;
  unsigned int nInPath;
  unsigned int fGiven, fStart, fEnd, fStep;

  getOptionsScatterColorBar(&logLevel,
                        &inPathList, &nInPath,
                        &fGiven, &fStart, &fEnd, &fStep,
                        argc, argv);

  if (fGiven)
    {
      int f;
      for (f = fStart; f <= fEnd; f += fStep)
        {
          char * path;
          substitutePositiveInteger(&path, inPathList[0], f);
          logInfo(path);
          printColorLine(path);
          free(path);
        }
    }
  else
    {
      unsigned int i;
      for (i = 0; i < nInPath; i++)
        {
          char * path = inPathList[i];
          logInfo(path);
          printColorLine(path);
        }
    }

  freePathList(&inPathList, nInPath);

  return 0;
}
