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



void divideDouble(double * dst, unsigned int alpha, size_t pixels)
{
  size_t i;
  double alphaDouble = (double)alpha;
  for (i = 0; i < pixels * N_COMPONENTS; i++)
    {
      dst[i] /= alphaDouble;
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



