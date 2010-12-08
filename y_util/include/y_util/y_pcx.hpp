/*--------------------------------------------------------------------------
    y_pcx.hpp
--------------------------------------------------------------------------*/
#ifndef _Y_PCX_HPP_
#define _Y_PCX_HPP_

#include <tc_config.h>
#include <c_lib/c_types.hpp>

#pragma pack(1)
struct TPCXHdr {
	uint8_t manuf;
	uint8_t hard;
	uint8_t encod;
	uint8_t bitpx;
	uint16_t x1;
	uint16_t y1;
	uint16_t x2;
	uint16_t y2;
	uint16_t hres;
	uint16_t vres;
	uint8_t pal[48];
	uint8_t vmode;
	uint8_t nplanes;
	uint16_t bplin;
	uint16_t palinfo;
	uint16_t shres;
	uint16_t svres;
	uint8_t unused[54];
};
#pragma pack()


// Структура заголовка grayscale PCX.
#pragma pack(1)
struct TGrayscalePCXHdr {
	uint8_t manuf;                 // +0
	uint8_t hard;                  // +1
	uint8_t encod;                 // +2
	uint8_t bitpx;                 // +3
	uint16_t x1;                  // +4
	uint16_t y1;                  // +6
	uint16_t x2;                  // +8
	uint16_t y2;                  // +10
	uint16_t hres;                // +12
	uint16_t vres;                // +14

	uint8_t blk0_day;              // +16
	uint8_t blk0_month;            // +17
	uint8_t blk0_year;             // +18
	uint8_t blk0_satNumber;        // +19
	uint8_t blk0_hour;             // +20
	uint8_t blk0_minute;           // +21
	uint8_t blk0_second;           // +22
	uint8_t blk0_tic;              // +23
	uint16_t blk0_revNum;         // +24
	uint16_t blk0_numberOfScans;  // +26

	float fTime;                // +28
	float fRoll;                // +32
	float fPitch;               // +36
	float fYaw;                 // +40

	uint16_t fNoradYear;          // +44  полностью
	uint16_t fNoradDay;           // +46
	double fNoradDayFraction;   // +48
	double fNoradN0;            // +56

	uint8_t fCorrectionVersion;    // +64

	uint8_t nplanes;               // +65
	uint16_t bplin;               // +66
	uint16_t palinfo;             // +68
	uint16_t fInfoBarHeight;      // +70  Высота информационной панели.
	uint16_t svres;               // +72

	uint16_t fOptimalLowBound;    // +74
	uint16_t fOptimalHighBound;   // +76
	char fIdent[2];             // +78

	double fNoradBSTAR;         // +80
	double fNoradI0;            // +88
	double fNoradRAAN;          // +96
	double fNoradE0;            // +104
	double fNoradW0;            // +112
	double fNoradM0;            // +120
};
#pragma pack()


#pragma pack(1)
struct PCXRGB {
	uint8_t r;
	uint8_t g;
	uint8_t b;
};
#pragma pack()


/*-----------------------------------------------------------------------------
    Параметры:
    dst Указатель на буфер, в который нужно будет поместить PCX-упакованную
        область.
        Должен иметь размер 2*src_len, если src_len - чётное, и 2*src_len+1,
        если src_len - нечётное, так как в худшем случае именно столько байт
        будет туда записано.
    src Указатель на буфер со строкой-источником.
    src_len Объем данных в src, который нужно упаковать. Может быть нечётным,
            в этом случае функция поместит в конец dst нулевой байт.
    Возвращаемое значение: объем данных, помещенных в dst.
-----------------------------------------------------------------------------*/
uint32_t pcx_pack_str( char *dst, char *src, uint32_t src_len );


/*
    Функция для инициализации структуры заголовка PCX-файла.
*/
void pcx_ini_hdr_256( TPCXHdr * hdr, uint32_t width, uint32_t height );


/*
    Функция для создания 8-бит PCX-файла по массиву данных.
 
    Параметры:
    file_name       Имя создаваемого PCX-файла.
    hdr             Должным образом проинициалированная структура заголовка
                    PCX-файла. Функция просто записывает его в файл, ничего
                    не изменяя.
    width           Размер строки пикселов в массиве data_buf. Может быть
                    нечётным, в этом случае размер массива data_buf ДОЛЖЕН
                    быть как минимум на 1 байт больше, чем width*height.
    orientation     1   массив данных следует записывать в файл сверху вниз
                    2   снизу вверх
 
    Возвращаемое значение:
    0       успешно
    не 0    ошибка
*/
int pcx_create_file_256( const char * file_name, const TPCXHdr * hdr, char * data_buf,
						 uint32_t width, uint32_t height, const PCXRGB pal[256], int orientation );


#endif
