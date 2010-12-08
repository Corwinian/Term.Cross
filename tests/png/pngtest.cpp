/*
 * pngtest.cpp
 *
 *  Created on: 19.02.2010
 *      Author: vanger
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#define PNG_DEBUG 3
#include <png.h>

void abort_(const char * s, ...)
{
	va_list args;
	va_start(args, s);
	vfprintf(stderr, s, args);
	fprintf(stderr, "\n");
	va_end(args);
	abort();
}

int main(int argc, char** argv ){

	int width = 256, height = 256;
	png_byte color_type = PNG_COLOR_TYPE_GRAY;
	png_byte bit_depth = 8;

	png_bytep image = new png_byte[width * height];
	png_bytepp row_pointers = new png_bytep[height];
	for(int i=0; i < height; i++) row_pointers[i] = image + i * width;
	for(int i=0; i< width*height; i++) image[i] = i%255 ;

	/* create file */
	const char* file_name = "test.png";
	FILE *fp = fopen(file_name, "wb");
	if (!fp)
		abort_("[write_png_file] File %s could not be opened for writing", file_name);

	/* initialize stuff */
	png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

	if (!png_ptr)
		abort_("[write_png_file] png_create_write_struct failed");

	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
		abort_("[write_png_file] png_create_info_struct failed");

//	if (setjmp(png_jmpbuf(png_ptr)))
//		abort_("[write_png_file] Error during init_io");

	png_init_io(png_ptr, fp);

	/* write header */
//	if (setjmp(png_jmpbuf(png_ptr)))
//		abort_("[write_png_file] Error during writing header");

	png_set_IHDR(png_ptr, info_ptr, width, height,
			8, PNG_COLOR_TYPE_GRAY, PNG_INTERLACE_NONE,
		     PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

	png_text info[3];

	info[0].key = (char*)"Header";
	info[0].text = (char*)"Test png string";
	info[0].compression = PNG_TEXT_COMPRESSION_zTXt;
	info[1].key = (char*)"Header 2";
	info[1].text = (char*)"Test another png string";
	info[1].compression = PNG_TEXT_COMPRESSION_zTXt;
	info[2].key = (char*)"Header 3";
	info[2].text = (char*)"Test last png string";
	info[2].compression = PNG_TEXT_COMPRESSION_zTXt;

	png_set_text(png_ptr, info_ptr, info, 2);
	   /* Optionally write comments into the image */
//		   text_ptr[0].key = "Title";
//		   text_ptr[0].text = "Mona Lisa";
//		   text_ptr[0].compression = PNG_TEXT_COMPRESSION_NONE;
//		   text_ptr[1].key = "Author";
//		   text_ptr[1].text = "Leonardo DaVinci";
//		   text_ptr[1].compression = PNG_TEXT_COMPRESSION_NONE;
//		   text_ptr[2].key = "Description";
//		   text_ptr[2].text = "<long text>";
//		   text_ptr[2].compression = PNG_TEXT_COMPRESSION_zTXt;
//		   png_set_text(png_ptr, info_ptr, text_ptr, 2);

	png_write_info(png_ptr, info_ptr);

	/* write bytes */
//	if (setjmp(png_jmpbuf(png_ptr)))
//		abort_("[write_png_file] Error during writing bytes");

	png_write_image(png_ptr, row_pointers);


	/* end write */
//	if (setjmp(png_jmpbuf(png_ptr)))
//		abort_("[write_png_file] Error during end of write");

	png_write_end(png_ptr, NULL);

    fclose(fp);

    /* cleanup heap allocation */
	delete row_pointers;
	delete image;

	return 0;
}
