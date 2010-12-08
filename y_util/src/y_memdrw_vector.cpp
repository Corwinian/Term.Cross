/*-----------------------------------------------------------------------------
    y_memdrw_vector.cpp
-----------------------------------------------------------------------------*/
#include "y_util/y_memdrw.hpp"


void vector_mem_8( long x1, long y1, long x2, long y2, uint8_t pixel, uint8_t * array, long n_col ) {
	long dx = x2 - x1;
	long x_inc;
	long straight_count;

	if( dx >= 0 ) {
		x_inc = 1;
		straight_count = dx * 2;
	} else {
		x_inc = -1;
		straight_count = -dx * 2;
	}

	long dy = y2 - y1;
	long y_inc;
	long diag_count;

	if( dy >= 0 ) {
		y_inc = n_col;
		diag_count = dy * 2;
	} else {
		y_inc = -n_col;
		diag_count = -dy * 2;
	}

	uint8_t * p = array + y1 * n_col + x1;
	uint8_t * p_end = array + y2 * n_col + x2;
	if( straight_count >= diag_count ) {     // горизонтальные прямые сегменты отрезка
		long d = straight_count >> 1;
		while( p != p_end ) {
			*p = pixel;
			p += x_inc;
			if( (d -= diag_count) < 0 ) {
				p += y_inc;
				d += straight_count;
			}
		}
	} else {   // вертикальные прямые сегменты отрезка; здесь straight_count и diag_count поменялись ролями
		long d = diag_count >> 1;
		while( p != p_end ) {
			*p = pixel;
			p += y_inc;
			if( (d -= straight_count) < 0 ) {
				p += x_inc;
				d += diag_count;
			}
		}
	}
	*p = pixel;
}
