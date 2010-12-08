/*-------------------------------------------------------------------------
    c_misc.hpp
-------------------------------------------------------------------------*/
#ifndef _C_MISC_HPP_
#define _C_MISC_HPP_

#include <stdio.h>
#include <c_lib/c_types.hpp>

/*
    Функция проверяет, является ли строка правильной записью плавающего
    числа с точки зрения языка C.
 
    Возвращаемое значение:
    0   bad
    1   ok
*/
int isFloatingPoint( const char * s );

/*
    Возвращает число, округленное вверх до 4.
*/
uint32_t padOn32BitBound( uint32_t value );

/*
    Проверить существование каталога path.
 
    Возвращаемое значение:
    0	каталог существует
    1	каталога не существует или path - пустая строка
*/
int check_dir( const char * path );

/*
    Переводит строку в нижний регистр. Вызывает стандартную tolower, сама переводит только русские символы.
 
    Возвращаемое значение:
    0	строка - нулевой длины
    1	строка обработана
int stringToLower( char * str );
*/

/*
    Функция печатает шестнадцатиричный и символьный дамп буфера. По той причине,
    что используются стандартные функции библиотеки C, вместо символов с кодами
    0x0, 0x7, 0x8, 0x9, 0xa, 0xd, 0x1b в символьном дампе печатается '.'.
*/
void memDump( FILE * f, const char * buffer, uint32_t length );

/*
	Замена функций DOS _splitpath и _makepath, т.к. не везде они есть.
	Под 'NIX платформами drive всегда нулевой длинны.
 */
int splitpath( char* path, char* drive, char* dir, char* name, char* ext);
int makepath( char* path, const char* drive, const char* dir, const char* name, const char* ext );

#endif
