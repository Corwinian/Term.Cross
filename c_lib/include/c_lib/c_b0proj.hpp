/*-------------------------------------------------------------------------
    c_b0proj.hpp
-------------------------------------------------------------------------*/
#ifndef _C_B0PROJ_HPP_
#define _C_B0PROJ_HPP_

#include <c_lib/c_types.hpp>

#pragma pack(1)

#define B0PRJ_MERC 1
#define B0PRJ_EQUI 2

/*-------------------------------------------------------------------------
    TBlk0_Proj
    Формат 0-блока файлов картографических проекций.
 
    Пикселы изображения, хранящегося в файле этого формата, имеют тип
    знаковое_двухбайтовое_целое и делятся на значимые и незначимые.
    Значимые пикселы сформированы сканером спутника или получены из них в
    результате обработки и имеют значения >=0. Незначимые пикселы - это
    пикселы сбойных строк или масок; имеют значения <0.
-------------------------------------------------------------------------*/
struct TBlk0_Proj {
	TBlk0_Common b0;

	uint32_t   processLevel;	/* 64 */

	uint16_t  channel;		/* 68 */
	int16_t   maxPixelValue;  /* 70   Максимальное значение пикселов проекции. */

	uint16_t  projType;		/* 72	Тип проекции (1 - меркаторская, 2 - равнопромежуточная). */

	uint16_t  scanNum;        /* 74   Количество строк. */
	uint16_t  pixNum;         /* 76   Количество пикселов в строке. */

	float   lat;            /* 78   Широта (градусы). */
	float   lon;            /* 82   Долгота (градусы). */
	float   latSize;        /* 86   Размер по широте (градусы).  */
	float   lonSize;        /* 90   Размер по долготе (градусы).  */
	float   latRes;         /* 94   Шаг по вертикали (секунды). */
	float   lonRes;         /* 98   Шаг по горизонтали (секунды). */

	double  ka;             /* 102 */
	double  kb;             /* 110 */

	uint8_t    reserved1[10];  /* 118 */

	/* Телеграмма NORAD. */
	uint32_t   revNum;         /* 128 */
	uint16_t  setNum;         /* 132 */
	uint16_t  ephemType;      /* 134 */
	uint16_t  year;           /* 136 */
	double  yearTime;       /* 138 */
	double  n0;             /* 146 */
	double  bstar;          /* 154 */
	double  i0;             /* 162 */
	double  raan;           /* 170 */
	double  e0;             /* 178 */
	double  w0;             /* 186 */
	double  m0;             /* 194 */

	//John
//	uchar   reserved2[54];  /* 202 */
	char	dataName[32];		/* 202	Наименование данных, напр. "chlor-a". */
	char	dataUnitsName[22];	/* 234	Наименование единиц измерения данных,
										например. "mg/m^3". */

	/* Параметры коррекции географической привязки. */
	uint16_t  corVersion;     /* 256 */
	int16_t   corTBUSTime;    /* 258 */
	int16_t   corTime;        /* 260 */
	double  corRoll;        /* 262 */
	double  corPitch;       /* 270 */
	double  corYaw;         /* 278 */

	uint8_t   spare[512-286]; /* 286 */
};


#pragma pack()


#endif
