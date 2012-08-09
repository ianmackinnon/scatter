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



int accumulate(double *dstImage, unsigned char **srcBuffer, double *alphaImage, char* srcPath, 
               unsigned int targetWidth, unsigned int targetHeight, unsigned int exposure,
               double t)
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

  mean = imageMean(*srcBuffer, targetWidth * targetHeight);
  logDebug("%.2f", mean);
  gain = 255.0 / fmax(mean, 1.0);
  gain = gain * exposure + (1.0 - exposure);
  addMultCharToDoubleColorRolling(dstImage, *srcBuffer, targetWidth, targetHeight, gain, t);
  addMultCharToDoubleRolling(alphaImage, targetWidth, targetHeight, gain, t);


  return 0;  // Win.
}



int main (int argc, char **argv)
{
  char ** inPathList;
  unsigned int nInPath;
  char * outPath;
  unsigned int fGiven, fStart, fEnd, fStep;
  unsigned int force;
  double exposure, rollOffset;

  unsigned int width, height;
  size_t pixels;

  double * meanData = NULL;
  unsigned char * frameData = NULL;
  double * alphaData = NULL;
  
  double t;

  getOptionsScatterRolling(&logLevel,
                           &inPathList, &nInPath, &outPath,
                           &fGiven, &fStart, &fEnd, &fStep,
                           &force, &exposure, &rollOffset,
                           argc, argv);
  

  if (fGiven)
    {
      int f;
      for (f = fStart; f <= fEnd; f += fStep)
        {
          char * path;
          t = (double)(f - fStart) / (double)(fEnd - fStart + 1);
          substitutePositiveInteger(&path, inPathList[0], f);
          logInfo(path);
          if (!meanData)
            {
              getTarget(path, &width, &height);
              pixels = width * height;
              meanData = (double*) malloc(pixels * N_COMPONENTS * sizeof(double));
              frameData = (unsigned char*) malloc(pixels * N_COMPONENTS * sizeof(unsigned char));
              alphaData = (double*) malloc(pixels * sizeof(double));
              zeroDouble(meanData, pixels);
              zeroDouble(alphaData, pixels / N_COMPONENTS);
            }
          accumulate(meanData, &frameData, alphaData, path, width, height, exposure, t + rollOffset);
          free(path);
        }
    }
  else
    {
      unsigned int i;
      for (i = 0; i < nInPath; i++)
        {
          char * path = inPathList[i];
          t = (double)i / (double)nInPath;
          logInfo(path);
          if (!meanData)
            {
              getTarget(path, &width, &height);
              pixels = width * height;
              meanData = (double*) malloc(pixels * N_COMPONENTS * sizeof(double));
              frameData = (unsigned char*) malloc(pixels * N_COMPONENTS * sizeof(unsigned char));
              alphaData = (double*) malloc(pixels * sizeof(double));
              zeroDouble(meanData, pixels);
              zeroDouble(alphaData, pixels / N_COMPONENTS);
            }
          accumulate(meanData, &frameData, alphaData, path, width, height, exposure, t + rollOffset);
        }
    }

  divideDoubleColorDouble(meanData, alphaData, pixels);
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
