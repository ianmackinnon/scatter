#ifndef __IO_H__
#define __IO_H__



#define N_COMPONENTS 3



int sizeJpeg(char * path, unsigned int * width, unsigned int * height);
int loadJpeg(char * path, unsigned char ** imageData, unsigned int * width, unsigned int * height);
int saveJpeg(char * path, unsigned char * imageData, unsigned int width, unsigned int height);

int loadTiff(char * path, unsigned char ** imageData, unsigned int * width, unsigned int * height);
int saveTiff(char * path, unsigned char * imageData, unsigned int width, unsigned int height);



#endif // __IO_H__
