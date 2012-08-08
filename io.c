#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <jpeglib.h>
#include <tiffio.h>

#include "logging.h"
#include "io.h"



extern LogLevel logLevel;



int sizeJpeg(char * path, unsigned int * width, unsigned int * height)
{
  // Caller supplies unallocated imageData.
  // Caller's responsibility to free it.
  //

  struct jpeg_decompress_struct cinfo;
  struct jpeg_error_mgr jerr;

  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_decompress(&cinfo);

  FILE * inFile;

  if ((inFile = fopen(path, "rb")) == NULL) {
    return 1;  // Fail.
  }
  
  jpeg_stdio_src(&cinfo, inFile);

  jpeg_read_header(&cinfo, TRUE);

  jpeg_start_decompress(&cinfo);

  jpeg_destroy_decompress(&cinfo);
  fclose(inFile);

  *width = cinfo.output_width;
  *height = cinfo.output_height;

  

  return 0;  // Win.
}



int loadJpeg(char * path, unsigned char ** imageData, unsigned int * width, unsigned int * height)
{
  // Caller supplies unallocated imageData.
  // Caller's responsibility to free it.
  //

  struct jpeg_decompress_struct cinfo;
  struct jpeg_error_mgr jerr;

  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_decompress(&cinfo);

  FILE * inFile;

  if ((inFile = fopen(path, "rb")) == NULL) {
    return 1;  // Fail.
  }
  
  jpeg_stdio_src(&cinfo, inFile);

  jpeg_read_header(&cinfo, TRUE);

  jpeg_start_decompress(&cinfo);

  assert(cinfo.output_components == N_COMPONENTS);

  size_t rowSize = cinfo.output_width * cinfo.output_components;

  int bufferHeight = 1;
  JSAMPARRAY buffer = (JSAMPARRAY) malloc(sizeof(JSAMPROW) * bufferHeight);
  buffer[0] = (JSAMPROW) malloc (sizeof(JSAMPLE) * rowSize);
  JDIMENSION rowsRead;

  unsigned int row = 0;
  while (cinfo.output_scanline < cinfo.output_height) {
    rowsRead = jpeg_read_scanlines(&cinfo, buffer, bufferHeight);
    memcpy(*imageData + rowSize * row, buffer[0], rowSize * rowsRead);
    row += rowsRead;
  }

  jpeg_destroy_decompress(&cinfo);
  fclose(inFile);

  *width = cinfo.output_width;
  *height = cinfo.output_height;

  return 0;  // Win.
}



int saveJpeg(char * path, unsigned char * imageData, unsigned int width, unsigned int height)
{
  FILE* outfile = fopen(path, "wb");

  struct jpeg_compress_struct cinfo;
  struct jpeg_error_mgr jerr;

  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_compress(&cinfo);
  jpeg_stdio_dest(&cinfo, outfile);  

  cinfo.image_width = width;
  cinfo.image_height = height;
  cinfo.input_components = N_COMPONENTS;
  cinfo.in_color_space = JCS_RGB;
  jpeg_set_defaults(&cinfo);
  jpeg_set_quality(&cinfo, 80, 1);
  jpeg_start_compress(&cinfo, TRUE);

  JSAMPROW row_pointer[1];        /* pointer to a single row */
  int row_stride;                 /* physical row width in buffer */
  row_stride = cinfo.image_width * cinfo.input_components;   /* JSAMPLEs per row in image_buffer */
  while (cinfo.next_scanline < cinfo.image_height)
    {
      row_pointer[0] = & imageData[cinfo.next_scanline * row_stride];
      jpeg_write_scanlines(&cinfo, row_pointer, 1);
    }

  jpeg_finish_compress(&cinfo);
  jpeg_destroy_compress(&cinfo);
  fclose(outfile);

  return 0;  // Win.
}



int loadTiff(char * path, unsigned char ** imageData, unsigned int * width, unsigned int * height)
{
  TIFF* image;

  if((image = TIFFOpen(path, "r")) == NULL){
    logFatal("%s: Could not open for reading.", path);
  }

  tsize_t scanline;
  tdata_t buf;
  uint32 row;

  TIFFGetField(image, TIFFTAG_IMAGEWIDTH, width);           
  TIFFGetField(image, TIFFTAG_IMAGELENGTH, height);        

  scanline = TIFFScanlineSize(image);
  assert(scanline == (*width) * N_COMPONENTS);
  buf = _TIFFmalloc(scanline);
  for (row = 0; row < *height; row++)
    {
      TIFFReadScanline(image, buf, row, 0);
      memcpy((*imageData) + (scanline * row), buf, scanline);
    }
  _TIFFfree(buf);
  TIFFClose(image);
  
  return 0;  // Win.
}



int saveTiff(char * path, unsigned char * imageData, unsigned int width, unsigned int height)
{
  TIFF *image;
  size_t s;

  if((image = TIFFOpen(path, "w")) == NULL){
    logFatal("%s: Could not open for writing.", path);
  }

  // We need to set some values for basic tags before we can add any data
  TIFFSetField(image, TIFFTAG_IMAGEWIDTH, width);
  TIFFSetField(image, TIFFTAG_IMAGELENGTH, height);
  TIFFSetField(image, TIFFTAG_BITSPERSAMPLE, 8);
  TIFFSetField(image, TIFFTAG_SAMPLESPERPIXEL, N_COMPONENTS);
  TIFFSetField(image, TIFFTAG_COMPRESSION, COMPRESSION_LZW);
  TIFFSetField(image, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
  TIFFSetField(image, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);

  // Write the information to the file
  for (s = 0; s < height; s++)
    {
      TIFFWriteScanline(image, imageData + (width * N_COMPONENTS) * s, s, 0);
    }

  // Close the file
  TIFFClose(image);
  return 0;
}


