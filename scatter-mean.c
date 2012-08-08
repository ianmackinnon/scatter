#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

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



int accumulate(double *dstImage, unsigned char **srcBuffer, char* srcPath, double *alpha,
               unsigned int targetWidth, unsigned int targetHeight, unsigned int exposure)
{
  unsigned int width, height;
  double mean, gain;
  if (loadJpeg(srcPath, srcBuffer, &width, &height))
    {
      logWarning("Loading failed.");
      return 1;  // Fail.
    }
  if (targetWidth != width)
    {
      logError("%s: Width %u does not equal target width %u.", srcPath, width, targetWidth);
      return 1;  // Fail.
    }
  if (targetHeight != height)
    {
      logError("%s: Height %u does not equal target height %u.", srcPath, height, targetHeight);
      return 1;  // Fail.
    }
  if (exposure)
    {
      mean = imageMean(*srcBuffer, targetWidth * targetHeight);
      logDebug("%.2f", mean);
      gain = 255.0 / fmax(mean, 1.0);
      addMultCharToDoubleColor(dstImage, *srcBuffer, targetWidth * targetHeight, gain);
      *alpha += gain;
    }
  else
    {
      addCharToDoubleColor(dstImage, *srcBuffer, targetWidth * targetHeight);
      *alpha += 1.0;
    }

  return 0;  // Win.
}



int main (int argc, char **argv)
{
  char ** inPathList;
  unsigned int nInPath;
  char * outPath;
  unsigned int fGiven, fStart, fEnd, fStep;
  unsigned int force, exposure;

  unsigned int width, height;
  size_t pixels;

  double * meanData = NULL;
  unsigned char * frameData = NULL;
  double alpha = 0.0;
  
  getOptionsScatterMean(&logLevel,
                        &inPathList, &nInPath, &outPath,
                        &fGiven, &fStart, &fEnd, &fStep,
                        &force, &exposure,
                        argc, argv);
  
  if (fGiven)
    {
      int f;
      for (f = fStart; f <= fEnd; f += fStep)
        {
          char * path;
          substitutePositiveInteger(&path, inPathList[0], f);
          logInfo(path);
          if (!meanData)
            {
              getTarget(path, &width, &height);
              pixels = width * height;
              meanData = (double*) malloc(pixels * N_COMPONENTS * sizeof(double));
              frameData = (unsigned char*) malloc(pixels * N_COMPONENTS * sizeof(unsigned char));
              zeroDouble(meanData, pixels);
            }
          accumulate(meanData, &frameData, path, &alpha, width, height, exposure);
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
          if (!meanData)
            {
              getTarget(path, &width, &height);
              pixels = width * height;
              meanData = (double*) malloc(pixels * N_COMPONENTS * sizeof(double));
              frameData = (unsigned char*) malloc(pixels * N_COMPONENTS * sizeof(unsigned char));
              zeroDouble(meanData, pixels);
            }
          accumulate(meanData, &frameData, path, &alpha, width, height, exposure);
        }
    }

  divideDouble(meanData, alpha, pixels);
  doubleToChar(frameData, meanData, pixels);

  logDebug("Saving to: %s", outPath);
  if (saveTiff(outPath, frameData, width, height)) {
    logFatal("Saving failed.");
  }

  if (frameData) free(frameData);
  if (meanData) free(meanData);

  freePathList(&inPathList, nInPath);

  return 0;
}
