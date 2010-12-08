/*-------------------------------------------------------------------------
    c_b0gms.hpp
-------------------------------------------------------------------------*/
#ifndef _C_B0GMS_HPP_
#define _C_B0GMS_HPP_

#include <c_lib/c_types.hpp>

#pragma pack(1)


/*
    Формат 0-блока файла распакованных данных канала S-VISSR.
*/
struct TBlk0_VISSRChannel {
	TBlk0_Common b0;

	uint16_t  channel;    /* 1 - IR1, 2 - IR2, 3 - IR3, 4 - VIS */
	uint16_t  meaning;    /* физический смысл данных:
	                            0 - отсчёты сканера (необработанные данные)
	                            1 - температура
	                            2 - альбедо */
	uint32_t   process;  /* биты: 0 - проведена атм. коррекция */

	uint32_t   pixNum;
	uint32_t   scanNum;
	uint8_t   bytesPix;

	/*
	 * минимальное и максимальное значения пикселов в изображении;
	 * эти поля не используются, если meaining == 0
	 */
	int16_t   minValue;
	int16_t   maxValue;

	// value = double(pixel)*ka + kb
	double  ka;
	double  kb;

	//-------------------------------------------------------------
	//-------------------------------------------------------------
	uint32_t   firstVISSRFrame;
	uint32_t   lastVISSRFrame;
	uint32_t   firstPix;
	uint32_t   lastPix;

	uint8_t   spare[512-64-53];
};


/*
    Формат 0-блока файла проекции, построенной по данным S-VISSR.
*/
struct TBlk0_VISSRProj {
	TBlk0_Common b0;

	uint16_t  channel;  /* 1 - IR1, 2 - IR2, 3 - IR3, 4 - VIS */
	uint16_t  meaning;
	uint32_t   process;  /* биты: */

	uint32_t   pixNum;
	uint32_t   scanNum;
	uint8_t   bytesPix;

	int16_t   minValue;
	int16_t   maxValue;

	/* коэффициенты перехода к физическим величинам значений пикселов */
	double  ka;
	double  kb;

	/*
	 * Предыдущие поля совпадают со структурой TBlk0_VISSRChannel и занимают 35 байт.
	 */

	/* параметры проекции */
	uint8_t   projType; /* 1 - меркаторская, 2 - равнопромежуточная */
	float   lat; // гр
	float   lon; // гр
	float   latSize; // гр
	float   lonSize; // гр
	float   latRes; // сек
	float   lonRes; // сек

	uint8_t spare[512-64-62];
};


/*
    Структура заголовка DCS-файла.
*/
struct TBlk0_VISSRDCS {
	TBlk0_Common b0;

	/*
	 * смещения структур относительно начала файла
	 * если смещение равно 0, соответствующая структура или массив в файле отсутствует
	 */
	uint32_t o_orbit_attitude;     // TGMSOrbitAttitude
	uint32_t o_orbit;              // массив TGMSOrbit[8]
	uint32_t o_attitude;           // массив TGMSAttitude[10]

	uint32_t o_clb_vis;            // таблицы калибровки vis1, vis2, vis3, vis4 - четыре массива double [64] непосредственно друг за другом
	uint32_t o_clb_ir;             // таблицы калибровки ir1, ir2, ir3 - три массива double [256] непосредственно друг за другом

	uint8_t spare[512-64-20];
};


#pragma pack()


#endif
