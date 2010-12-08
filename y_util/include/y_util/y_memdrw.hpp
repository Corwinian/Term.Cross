/*-----------------------------------------------------------------------------
    y_memdrw.hpp
-----------------------------------------------------------------------------*/
#ifndef _Y_MEMDRW_HPP_
#define _Y_MEMDRW_HPP_

#include <tc_config.h>
#include <c_lib/c_types.hpp>

void vector_mem_8( long x1, long y1, long x2, long y2, uint8_t pixel, uint8_t* array, long n_col );

void fill_mem_8( long seed_x, long seed_y, uint8_t fill_color, uint8_t border_color, uint8_t* array, long ncol, long nrow );



#endif
