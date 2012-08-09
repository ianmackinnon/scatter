#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <math.h>
#include <time.h>

#include "logging.h"
#include "image.h"



#define MAX_UPSCALE 4



void addCharToDoubleColor(double * dst, unsigned char * src, size_t pixels)
{
  size_t i;
  for (i = 0; i < pixels; i++)
    {
      dst[i * N_COMPONENTS] += (double)(src[i * N_COMPONENTS]);
      dst[i * N_COMPONENTS + 1] += (double)(src[i * N_COMPONENTS + 1]);
      dst[i * N_COMPONENTS + 2] += (double)(src[i * N_COMPONENTS + 2]);
    }
}



void addMultCharToDoubleColor(double * dst, unsigned char * src, size_t pixels, double gain)
{
  size_t i;
  for (i = 0; i < pixels * N_COMPONENTS; i++)
    {
      dst[i] += ((double)(src[i])) * gain;
    }
}



void addCharToDoubleColorRolling(double * dst, unsigned char * src, unsigned int width, unsigned int height, double t)
{
  unsigned int x, y;
  size_t i;
  double t2, mult;
  for (y = 0; y < height; y++)
    {
      for (x = 0; x < width; x++)
        {
          i = y * width  + x;
          t2 = (double)x / (double)width;
          mult = fmax(cos((t - t2) * 2 * M_PI), 0.0);
          dst[i * N_COMPONENTS] += (double)(src[i * N_COMPONENTS]) * mult;
          dst[i * N_COMPONENTS + 1] += (double)(src[i * N_COMPONENTS + 1]) * mult;
          dst[i * N_COMPONENTS + 2] += (double)(src[i * N_COMPONENTS + 2]) * mult;
        }
    }
}



void addMultCharToDoubleColorRolling(double * dst, unsigned char * src, unsigned int width, unsigned int height, double gain, double t)
{
  unsigned int x, y;
  size_t i;
  double t2, mult;
  for (y = 0; y < height; y++)
    {
      for (x = 0; x < width; x++)
        {
          i = y * width  + x;
          t2 = (double)x / (double)width;
          mult = fmax(cos((t - t2) * 2 * M_PI), 0.0) * gain;
          dst[i * N_COMPONENTS] += (double)(src[i * N_COMPONENTS]) * mult;
          dst[i * N_COMPONENTS + 1] += (double)(src[i * N_COMPONENTS + 1]) * mult;
          dst[i * N_COMPONENTS + 2] += (double)(src[i * N_COMPONENTS + 2]) * mult;
        }
    }
}



void addCharToDoubleRolling(double * dst, unsigned int width, unsigned int height, double t)
{
  unsigned int x, y;
  size_t i;
  double t2, mult;
  for (y = 0; y < height; y++)
    {
      for (x = 0; x < width; x++)
        {
          i = y * width  + x;
          t2 = (double)x / (double)width;
          mult = fmax(cos((t - t2) * 2 * M_PI), 0.0);
          dst[i] += mult;
        }
    }
}



void addMultCharToDoubleRolling(double * dst, unsigned int width, unsigned int height, double gain, double t)
{
  unsigned int x, y;
  size_t i;
  double t2, mult;
  for (y = 0; y < height; y++)
    {
      for (x = 0; x < width; x++)
        {
          i = y * width  + x;
          t2 = (double)x / (double)width;
          mult = fmax(cos((t - t2) * 2 * M_PI), 0.0) * gain;
          dst[i] += mult;
        }
    }
}



void divideDouble(double  * dst, double alpha, size_t pixels)
{
  size_t i;
  for (i = 0; i < pixels * N_COMPONENTS; i++)
    {
      dst[i] /= alpha;
    }
}



void divideDoubleColorDouble(double * dst, double * alpha, size_t pixels)
{
  size_t i;
  for (i = 0; i < pixels; i++)
    {
      dst[i * N_COMPONENTS] /= alpha[i];
      dst[i * N_COMPONENTS + 1] /= alpha[i];
      dst[i * N_COMPONENTS + 2] /= alpha[i];
    }
}



void divideSigmaDouble(double * sigma, unsigned int alpha, size_t pixels)
{
  size_t i;
  size_t r, g, b;
  double l;
  double alphaDouble = (double)alpha;

  for (i = 0; i < pixels; i++)
    {
      r = i * N_COMPONENTS;
      g = r + 1;
      b = g + 1;
      l = (sqrtf(sigma[r] / alphaDouble) + sqrtf(sigma[g] / alphaDouble) + sqrtf(sigma[b] / alphaDouble)) / 3;
      sigma[r] = sigma[g] = sigma[b] = (unsigned int)l;
    }

}



void doubleToChar(unsigned char * dst, double * src, size_t pixels)
{
  size_t i;
  for (i = 0; i < pixels * N_COMPONENTS; i++)
    {
      dst[i] = (unsigned int)(src[i]);
    }
}



double imageMean(unsigned char * src, size_t pixels)
{
  double mean;
  size_t alpha;
  size_t i;
  mean = 0.0;
  alpha = 0;
  for (i = 0; i < pixels * N_COMPONENTS; i++)
    {
      mean += (double)(src[i]);
      alpha ++;
    }
  return mean / (double)alpha;
}



void imageColorMean(double * r, double * g, double * b, unsigned char * src, size_t pixels)
{
  size_t alpha;
  size_t i;
  *r = *g = *b = 0.0;
  alpha = 0;
  for (i = 0; i < pixels; i++)
    {
      *r += (double)(src[i * N_COMPONENTS]);
      *g += (double)(src[i * N_COMPONENTS + 1]);
      *b += (double)(src[i * N_COMPONENTS + 2]);
      alpha ++;
    }
  *r /= (double)alpha;
  *g /= (double)alpha;
  *b /= (double)alpha;
}



void addCharToDoubleSigma(double * dst, unsigned char * src, unsigned char * color, size_t pixels)
{
  size_t i;
  for (i = 0; i < pixels * N_COMPONENTS; i++)
    {
      dst[i] += powf((double)(src[i]) - (double)(color[i]), 2.0);
    }
}



float randomFloat(void)
{
  return rand() / (double)(RAND_MAX);
}



float pixelRandom(double color, double sigma, double t, double gain)
{
  return color + gain * (-1 + 2 * t * sigma);
}



void doubleSigmaToChar(unsigned char * dst, double * sigma, unsigned int alpha, size_t pixels)
{
  size_t i;
  size_t r, g, b;
  double l;
  double alphaDouble = (double)alpha;

  for (i = 0; i < pixels; i++)
    {
      r = i * N_COMPONENTS;
      g = r + 1;
      b = g + 1;
      l = (sqrtf(sigma[r] / alphaDouble) + sqrtf(sigma[g] / alphaDouble) + sqrtf(sigma[b] / alphaDouble)) / 3;
      dst[r] = (unsigned int)l;
      dst[g] = (unsigned int)l;
      dst[b] = (unsigned int)l;
    }
}



void doubleSigmaColorToChar(unsigned char * dst, double * sigma, double * color, unsigned int alpha, size_t pixels, double gain)
{
  size_t i;
  size_t r, g, b;
  double t;
  double alphaDouble = (double)alpha;

  for (i = 0; i < pixels; i++)
    {
      r = i * N_COMPONENTS;
      g = r + 1;
      b = g + 1;
      t = randomFloat();
      dst[r] = (unsigned int)pixelRandom(color[r] / alphaDouble, sqrtf(sigma[r] / alphaDouble), t, gain);
      dst[g] = (unsigned int)pixelRandom(color[g] / alphaDouble, sqrtf(sigma[g] / alphaDouble), t, gain);
      dst[b] = (unsigned int)pixelRandom(color[b] / alphaDouble, sqrtf(sigma[b] / alphaDouble), t, gain);
    }
}



void doubleSigmaColorToDoubleScale(double * dst, double * sigma, double * color, unsigned int alpha, 
                                 unsigned int width, unsigned int height, 
                                 double gain, unsigned int upscale)
{
  size_t x, y, xx, yy;
  size_t y_offset, yy_offset;
  size_t r, g, b, ru, gu, bu;
  double t;
  
  double alphaDouble = (double)alpha;

  assert(upscale <= MAX_UPSCALE);

  for (y = 0; y < height; y++)
    {
      y_offset = y * width;
      for (x = 0; x < width; x++)
        {
          r = (y_offset + x) * N_COMPONENTS;
          g = r + 1;
          b = g + 1;
          
          for (yy = 0; yy < upscale; yy++)
            {
              yy_offset = (y * upscale + yy) * width * upscale;
              for (xx = 0; xx < upscale; xx++)
                {
                  ru = (yy_offset + x * upscale + xx) * N_COMPONENTS;
                  gu = ru + 1;
                  bu = gu + 1;
                  t = randomFloat();
                  dst[ru] = (unsigned int)pixelRandom(color[r] / alphaDouble, sqrtf(sigma[r] / alphaDouble), t, gain);
                  dst[gu] = (unsigned int)pixelRandom(color[g] / alphaDouble, sqrtf(sigma[g] / alphaDouble), t, gain);
                  dst[bu] = (unsigned int)pixelRandom(color[b] / alphaDouble, sqrtf(sigma[b] / alphaDouble), t, gain);
                }
            }
        }
    }
}



void zeroDouble(double * dst, size_t pixels)
{
  size_t i;
  for (i = 0; i < pixels * N_COMPONENTS; i++)
    {
      dst[i] = 0.0;
    }
}



void levels(double * src, size_t pixels)
{
  double mean = 0;
  double sigma = 0;
  double low, range;
  size_t alpha = pixels * N_COMPONENTS;
  size_t i;
  for (i = 0; i < alpha; i++)
    {
      mean += src[i];
    }
  logWarning("%f", mean);
  mean /= (double)alpha;
  logWarning("%f", mean);

  for (i = 0; i < alpha; i++)
    {
      sigma += powf(src[i] - mean, 2.0);
    }
  logWarning("%f", sigma);
  sigma = sqrtf(sigma / alpha);
  logWarning("%f", sigma);
  low = mean - 2 * sigma;
  range = 4 * sigma;

  for (i = 0; i < alpha; i++)
    {
      src[i] = 255.0 * fmin(fmax((src[i] - low) / range, 0.0), 1.0);
    }

  
}



