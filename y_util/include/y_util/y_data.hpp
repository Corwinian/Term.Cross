/*-----------------------------------------------------------------------------
    y_data.hpp
-----------------------------------------------------------------------------*/
#ifndef _Y_DATA_HPP_
#define _Y_DATA_HPP_

#include <tc_config.h>
#include <c_lib/c_types.hpp>

/*
    Построение 8-битной unsigned lookup-таблицы, линейно отображающей значения интервала [src_min, src_max] в интервал [dst_min, dst_max].
 
    lookup  Массив под таблицу размером (src_max - src_min + 1) элементов.
*/
void linear_lookup_8( uint8_t * lookup, uint32_t src_min, uint32_t src_max, uint8_t dst_min, uint8_t dst_max );

/*
    Построение 32-битной unsigned lookup-таблицы, линейно отображающей значения интервала [src_min, src_max] в интервал [dst_min, dst_max].
 
    lookup  Массив под таблицу размером (src_max - src_min + 1) элементов.
 
    Допускается dst_max < dst_min, т.е. построение таблицы инверсного преобразования.
 
    algorithm   0   по Брезенхему
                1   для масштабирования
В качестве примера работы функции в первом и во втором случае рассмотрим результаты вызовов:
    lookup0[40];
    lookup1[40];
    linear_lookup( lookup0, 0, 39, 0, 3, 0 );
    linear_lookup( lookup1, 0, 39, 0, 3, 1 );
 
lookup0:
 0000000111 1111111111 2222222222 2223333333
lookup1:
 0000000000 1111111111 2222222222 3333333333
*/
void linear_lookup_32( uint32_t * lookup, uint32_t src_min, uint32_t src_max, uint32_t dst_min, uint32_t dst_max, int algorithm );

/*
    Построение гистограммы массива данных типа unsigned char.
 
    hist    Массив под гистограмму размером 256 элементов.
*/
void hist_8( uint32_t * hist, uint8_t * buf, uint32_t bufsize );


/*
    Построение гистограммы массива данных типа short.
 
    min_value, max_value    Задают диапазон гистограммируемых значений. Значения вне этого диапазона в гистограмме не учитываются.
    hist                    Массив под гистограмму размером (max_value - min_value + 1) элементов.
*/
void hist_16( uint32_t * hist, short * buf, uint32_t bufsize, short min_value, short max_value );


/*
    p   [0, 1]
    Возвращаемое значение: номер элемента массива hist, квантиль которого равна p.
*/
uint32_t quant( uint32_t * hist, uint32_t hist_size, double p );


#endif
