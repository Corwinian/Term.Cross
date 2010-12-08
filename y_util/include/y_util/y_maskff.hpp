/*-------------------------------------------------------------------------
    y_maskff.hpp
    Содержит определение формата файла маски.
-------------------------------------------------------------------------*/
#ifndef _Y_MASKFF_HPP_
#define _Y_MASKFF_HPP_

#include <c_lib/c_types.hpp>

#pragma pack(1)

struct TMaskFileHdr {
	char signature[5];  // ASCIIZ-строка "MASK"
	uint8_t dataFormat;    // формат данных маски:
	// 1 - массив char
	uint8_t projType;      // 1 - меркаторская, 2 - равнопромежуточная
	uint8_t unused;        // для выравнивания; должно быть равно 0
	/*  lon, lat, lonSize, latSize определяют географический  район, для которого построена маска;
	    Единицы измерения - градусы; отсчёт ведётся: по долготе - от Гринвича, по широте - от экватора. */
	double lon;
	double lat;
	double lonSize;
	double latSize;
	//long pixNum;        // число пикселов в строке
	//long scanNum;       // число строк
	uint32_t pixNum;        // число пикселов в строке
	uint32_t scanNum;       // число строк
};

#pragma pack()


#endif
