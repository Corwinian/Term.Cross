/*-------------------------------------------------------------------------
    y_memdrw_fill.cpp
-------------------------------------------------------------------------*/

#include "y_util/y_memdrw.hpp"

struct TFillSegment {
	long x, y;
	long xl, xr;
	long pxl, pxr;
	long d;
	int s;
	TFillSegment * next;

	TFillSegment( long X, long Y, long XL, long XR, long PXL, long PXR, long D, int S, TFillSegment *n ) :
	x(X), y(Y), xl(XL), xr(XR), pxl(PXL), pxr(PXR), d(D), s(S), next(n) {}
}
;


void fill_pix_str( long, long, long, uint8_t, uint8_t *, long );
long scan_l( long, long, uint8_t, uint8_t, uint8_t *, long );
long scan_r( long, long, uint8_t, uint8_t, uint8_t *, long );

void fill_mem_8( long seed_x, long seed_y, uint8_t fill_color, uint8_t border_color, uint8_t * array, long ncol, long nrow ) {
	long xl = scan_l( seed_x, seed_y, fill_color, border_color, array, ncol );
	long xr = scan_r( seed_x, seed_y, fill_color, border_color, array, ncol );
	if( xl > xr )
		return;
	long x = xl;
	long y = seed_y;
	long prev_xl, prev_xr;
	prev_xl = prev_xr = xr+1;
	long d = 1;
	int straight_search_completed = 0;
	TFillSegment *seg_stack = 0;    // стек сегментов

	fill_pix_str( xl, xr, y, fill_color, array, ncol );

	for(;;) {
		// закраска вдоль основного направления
		if( straight_search_completed == 0 ) {
			try {
				if( (d == -1 && y > 0) || (d == 1 && y < nrow-1) ) {
					while( x <= xr ) {
						uint8_t pixel = array[(y+d)*ncol + x];
						if( pixel != border_color && pixel != fill_color )
							throw long(1);
						x++;
					}
				}
				// сюда попадаем, если вдоль основного направления незакрашенных сегментов больше нет
				straight_search_completed = 1;
				x = xl;
			} catch( long ) {
				long t = scan_r( x, y+d, fill_color, border_color, array, ncol );
				seg_stack = new TFillSegment( t, y, xl, xr, prev_xl, prev_xr, d, 0, seg_stack );
				prev_xl = xl;
				prev_xr = xr;
				xl = scan_l( x, y+d, fill_color, border_color, array, ncol );
				xr = t;
				x = xl;
				y += d;
				fill_pix_str( xl, xr, y, fill_color, array, ncol ); // отрисовка найденного сегмента
			}
		}

		// закраска против основного направления
		if( straight_search_completed == 1 ) {
			try {
				if( (d == -1 && y < nrow-1) || (d == 1 && y > 0) ) {
					if( x < prev_xl ) {
						while( x < prev_xl ) {
							uint8_t pixel = array[(y-d)*ncol + x];
							if( pixel != border_color && pixel != fill_color )
								throw long(1);
							x++;
						}
						x = prev_xr;
					}
					while( x <= xr ) {
						uint8_t pixel = array[(y-d)*ncol + x];
						if( pixel != border_color && pixel != fill_color )
							throw long(1);
						x++;
					}
				}
				// сюда попадаем, если против основного направления незакрашенных сегментов больше нет
				if( seg_stack ) {    // если стек сегментов не пуст, переходим к предыдущему сегменту
					TFillSegment * p = seg_stack;
					seg_stack = p->next;
					x = p->x;
					y = p->y;
					xl = p->xl;
					xr = p->xr;
					prev_xl = p->pxl;
					prev_xr = p->pxr;
					d = p->d;
					straight_search_completed = p->s;
					delete p;
				} else
					break; // конец закраски
			} catch( long ) {  // сюда попадаем, если нашли сегмент
				long t = scan_r( x, y-d, fill_color, border_color, array, ncol );
				seg_stack = new TFillSegment( t, y, xl, xr, prev_xl, prev_xr, d, 1, seg_stack );
				prev_xl = xl;
				prev_xr = xr;
				xl = scan_l( x, y-d, fill_color, border_color, array, ncol );
				xr = t;
				x = xl;
				y -= d;
				fill_pix_str( xl, xr, y, fill_color, array, ncol ); // отрисовка найденного сегмента
				d *= -1;
				straight_search_completed = 0;
			}
		}
	}
}


void fill_pix_str( long xl, long xr, long y, uint8_t color, uint8_t *array, long ncol ) {
	uint8_t *p = array + ncol*y + xl;
	uint8_t *p_end = p + xr - xl + 1;
	while( p != p_end )
		*p++ = color;
}


long scan_l( long x, long y, uint8_t fill_color, uint8_t border_color, uint8_t * array, long ncol ) {
	uint8_t *p = array + y*ncol + x;
	uint8_t *p_end = p - x;
	while( *p != fill_color && *p != border_color )
		if( p-- == p_end )
			return 0;
	return p - p_end + 1;
}


long scan_r( long x, long y, uint8_t fill_color, uint8_t border_color, uint8_t *array, long ncol ) {
	uint8_t *p = array + y*ncol + x;
	uint8_t *p_end = p - x + ncol - 1;
	while( *p != fill_color && *p != border_color )
		if( p++ == p_end )
			return ncol-1;
	return p - (p_end + 1 - ncol) - 1;
}
