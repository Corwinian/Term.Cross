/*--------------------------------------------------------------------------
    y_pcx.cpp
--------------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <y_util/y_pcx.hpp>

uint32_t pcx_pack_str( char * dst, char * src, uint32_t src_len ) {
	uint32_t srcpos=1, dstpos=0;
	uint8_t pix, rep=1;

	pix = src[0];
	while( srcpos < src_len ) {
		if( pix != src[srcpos] || rep==63 ) {
			if( rep == 1 ) {
				if( pix >= 0xc0 ) {
					dst[dstpos++] = 0xc1;
				}
			} else {
				dst[dstpos++] = 0xc0 + rep;
			}
			dst[dstpos++] = pix;
			pix = src[srcpos++];
			rep = 1;
		} else {
			rep++;
			srcpos++;
		}
	}

	if( rep == 1 ) {
		if( pix >= 0xc0 ) {
			dst[dstpos++] = 0xc1;
		}
	} else {
		dst[dstpos++] = 0xc0 + rep;
	}
	dst[dstpos++] = pix;

	if( src_len & 1 ) {
		dst[dstpos++] = 0;    // если входная строка нечётной длины, добавляем 0, чтобы выполнить стандарт PCX
	}

	return dstpos;
}


void pcx_ini_hdr_256( TPCXHdr *hdr, uint32_t width, uint32_t height ) {
	memset( hdr, 0, 128 );
	hdr->manuf = 10;
	hdr->hard = 5;
	hdr->encod = 1;
	hdr->bitpx = 8;
	hdr->x2 = width-1;
	hdr->y2 = height-1;
	hdr->nplanes = 1;
	hdr->bplin = (width & 1) ? width+1 : width;
	hdr->palinfo = 1;
}


int pcx_create_file_256( const char * file_name, const TPCXHdr * hdr, char * data_buf,
						 uint32_t width, uint32_t height, const PCXRGB pal[256], int orientation ) {
	uint32_t i;
	FILE * f;
	char * pcx_buf;
	char save = '\0';
	long l;
	char * src, * dst;
	char pal_prefix = 12;

	if( (f = fopen( file_name, "wb" )) == NULL )
		return 1;

	if( fwrite( hdr, 1, 128, f ) != 128 ) {
		fclose( f );
		return 1;
	}

	pcx_buf = new char [100000 + (width+1)*2];
	src = orientation == 1 ? data_buf : data_buf + (height - 1) * width;
	dst = pcx_buf;

	for( i = 0; i < height; i++ ) {
		if( width & 1 ) {
			save = src[width];
			src[width] = 0;
		}

		dst += pcx_pack_str( dst, src, hdr->bplin );     // hdr.bplin == width || hdr.bplin == width+1

		if( (l = dst - pcx_buf) >= 100000 ) {
			if( fwrite( pcx_buf, 1, l, f ) != (size_t)l ) {
				fclose( f );
				delete [] pcx_buf;
				return 1;
			}

			dst = pcx_buf;
		}

		if( width & 1 )
			src[width] = save;

		if( orientation == 1 )
			src += width;
		else
			src -= width;
	}

	// запись остатка в буфере
	if( (l = dst - pcx_buf) ) {
		if( fwrite( pcx_buf, 1, l, f ) != (size_t)l ) {
			fclose( f );
			delete [] pcx_buf;
			return 1;
		}
	}

	delete [] pcx_buf;

	if( fwrite( &pal_prefix, 1, 1, f ) != 1 || fwrite( pal, 1, 768, f ) != 768 ) {
		fclose( f );
		return 1;
	}

	fclose( f );
	return 0;
}
