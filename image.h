#ifndef __IMAGE_H__
#define __IMAGE_H__



#define N_COMPONENTS 3



void addCharToDoubleColor(double * dst, unsigned char * src, size_t pixels);
void addMultCharToDoubleColor(double * dst, unsigned char * src, size_t pixels, double gain);
void divideDouble(double * dst, double alpha, size_t pixels);
void divideSigmaDouble(double * sigma, unsigned int alpha, size_t pixels);
void doubleToChar(unsigned char * dst, double * src, size_t pixels);
double imageMean(unsigned char * src, size_t pixels);
void imageColorMean(double * r, double * g, double * b, unsigned char * src, size_t pixels);
void addCharToDoubleSigma(double * dst, unsigned char * src, unsigned char * color, size_t pixels);
float randomFloat(void);
float pixelRandom(double color, double sigma, double t, double gain);
void doubleSigmaToChar(unsigned char * dst, double * sigma, unsigned int alpha, size_t pixels);
void doubleSigmaColorToChar(unsigned char * dst, double * sigma, double * color, unsigned int alpha, size_t pixels, double gain);
void doubleSigmaColorToDoubleScale(double * dst, double * sigma, double * color, unsigned int alpha, 
                                   unsigned int width, unsigned int height, 
                                   double gain, unsigned int upscale);
void zeroDouble(double * dst, size_t pixels);
void levels(double * src, size_t pixels);



#endif // __IMAGE_H__
