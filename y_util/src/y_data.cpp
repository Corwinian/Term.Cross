/*-----------------------------------------------------------------------------
    y_data.cpp
-----------------------------------------------------------------------------*/
#include <c_lib.hpp>
#include <y_util/y_data.hpp>

void linear_lookup_8( uint8_t * lookup, uint32_t src_min, uint32_t src_max, uint8_t dst_min, uint8_t dst_max ) {
	long src_delta = src_max - src_min;
	long src_count = src_delta * 2;
	long dst_delta = dst_max - dst_min;
	long dst_count = dst_delta * 2;

	if( src_delta >= dst_delta ) {       // диапазон исходных значений шире диапазона, в который он преобразуется
		uint8_t dst_value = dst_min;
		long d = src_delta;
		for( long i = 0; i <= src_delta; i++ ) {
			lookup[i] = dst_value;
			if( (d -= dst_count) < 0 ) {
				d += src_count;
				dst_value++;
			}
		}
	} else {
		long i = 0;
		long d = 0;
		for( uint32_t dst_value = dst_min; dst_value <= uint32_t(dst_max); dst_value++ ) {     // uint32_t сделано для корректной работы в случае dst_min==0 && dst_max==255
			if( (d -= src_count) <= 0 ) {
				d += dst_count;
				lookup[i++] = uint8_t(dst_value);
			}
		}
	}
}



void linear_lookup_32( uint32_t * lookup, uint32_t src_min, uint32_t src_max, uint32_t dst_min, uint32_t dst_max, int algorithm ) {
	long src_delta;
	long src_count;
	long dst_delta;
	long dst_count;

	if( algorithm != 0 && algorithm != 1 )
		return;

	src_delta = src_max - src_min;
	src_count = src_delta * 2;

	if( dst_min <= dst_max ) {
		dst_delta = dst_max - dst_min;

		if( src_delta >= dst_delta ) {       // диапазон исходных значений шире диапазона, в который он преобразуется
			dst_delta += algorithm;
			dst_count = dst_delta * 2;

			uint32_t dst_value = dst_min;
			long d = (algorithm == 0) ? src_delta : src_count;

			for( long i = 0; i <= src_delta; i++ ) {
				lookup[i] = dst_value;
				if( (d -= dst_count) < 0 ) {
					d += src_count;
					dst_value++;
				}
			}
		} else {
			dst_count = dst_delta * 2;

			uint32_t i = 0;
			long d = 0;
			for( uint32_t dst_value = dst_min; dst_value <= dst_max; dst_value++ ) {
				if( (d -= src_count) <= 0 ) {
					d += dst_count;
					lookup[i++] = dst_value;
				}
			}
		}
	} else {
		dst_delta = dst_min - dst_max;

		if( src_delta >= dst_delta ) {       // диапазон исходных значений шире диапазона, в который он преобразуется
			dst_delta += algorithm;
			dst_count = dst_delta * 2;

			uint32_t dst_value = dst_min;
			long d = (algorithm == 0) ? src_delta : src_count;

			for( long i = 0; i <= src_delta; i++ ) {
				lookup[i] = dst_value;
				if( (d -= dst_count) < 0 ) {
					d += src_count;
					dst_value--;
				}
			}
		} else {
			dst_count = dst_delta * 2;

			uint32_t i = 0;
			long d = 0;
			for( long dst_value = dst_min; dst_value >= long(dst_max); dst_value-- ) {   // dst_value имеет тип long для корректной обработки ситуации dst_max==0
				if( (d -= src_count) <= 0 ) {
					d += dst_count;
					lookup[i++] = dst_value;
				}
			}
		}

	}
}



void hist_8( uint32_t * hist, uint8_t * buf, uint32_t bufsize ) {
	memset( hist, 0, sizeof( uint32_t ) * 256 );

	uint8_t * p = buf;
	uint8_t * p_end = p + bufsize;
	while( p != p_end )
		hist[*p++]++;
}


void hist_16( uint32_t * hist, short * buf, uint32_t bufsize, short min_value, short max_value ) {
	memset( hist, 0, sizeof( uint32_t ) * (max_value - min_value + 1) );

	short * p = buf;
	short * p_end = p + bufsize;
	while( p != p_end ) {
		short value = *p++;
		if( value >= min_value && value <= max_value )
			hist[value - min_value]++;
	}
}


uint32_t quant( uint32_t * hist, uint32_t hist_size, double p ) {
	uint32_t n = 0;
	for( uint32_t i = 0; i < hist_size; i++ )
		n += hist[i];
	uint32_t np = uint32_t( double(n) * p );

	n = 0;      // повторно используем для накопления суммы элементов гистограммы
	uint32_t value = 0;
	while( n < np )
		n += hist[value++];
	if( n > np && value > 0 )
		value--;

	return value;
}

