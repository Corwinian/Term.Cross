/*-------------------------------------------------------------------------
    hrpt.hpp
-------------------------------------------------------------------------*/
#ifndef _HRPT_HPP_
#define _HRPT_HPP_

#include <tc_config.h>
#include <stdio.h>
#include <c_lib.hpp>
/*  константы кадра HRPT */
#include <hrptconst.hpp>

class THRPTUnpacker {
public:
	/*
	 * Создание по файлу данных.
	 * memory_flag  = 0 При распаковке читать файл данных построчно.
	 *              = 1 Прочитать файл данных заранее, распаковку производить из буфера.
	 * Исключения:
	 * TAccessExc( 1, "THRPTUnpacker::THRPTUnpacker: ошибка доступа к файлу" );
	 */
	THRPTUnpacker( const char * file_name, int memory_flag ) throw ( TAccessExc );

	/* Создание по данным, уже загруженным в буфер. */
	THRPTUnpacker( char * file_data );

	~THRPTUnpacker();


	TBlk0* blk0();

	void channelsAvailability( int availabilityArray[5] );

	/*
	 * Содержит ли файл данные указанного канала AVHRR.
	 * channel  Номер канала (0-based).
	 */
	int channelAvailable( int channel );

	uint32_t totalFrames();

	/*
	 * Возвращает номер кадра (0-based),
	 * который будет получен следующим вызовом функции unpackNextFrame.
	 */
	uint32_t currentFrameNumber();

	/*
	 * frame_num    0..total_frames-1
	 *
	 * Исключения:
	 * TAccessExc( 1, "THRPTUnpacker::setCurrentFrameNumber: ошибка доступа к файлу" )
	 * TRequestExc( 2, "THRPTUnpacker::setCurrentFrameNumber: кадр с указанным номером в файле отсутствует" )
	 */
	void setCurrentFrameNumber( uint32_t frame_num ) throw( TAccessExc, TRequestExc );

	/*
	 * mask
	 *
	 * Функция возвращает маску малых кадров HRPT.
	 */
	uint32_t mask();

	/*
	 * frameWordLength
	 *
	 * Функция возвращает размер распакованного малого кадра HRPT, выраженный в
	 * 2-х байтовых словах.
	 */
	uint32_t frameWordLength();
	uint32_t packedFrameWordLength();

	/*
	 * unpackNextFrame
	 *
	 * Функция распаковывает очередной малый кадр HRPT и выделяет из него
	 * данные каналов AVHRR и телеметрии.
	 * В случае, если обработаны все малые кадры HRPT файла, функция ничего не
	 * делает.
	 * Если при приеме кадра произошел сбой синхронизации, во все указанные в
	 * массиве avhrrBuffers буфера записывается -1, а в остальные - 0.
	 *
	 * Прототип:
	 * void unpackNextFrame( USHORT* avhrrBuffers[5], USHORT* tlmBuffers[3], USHORT* tipBuffer ) throw ( TException )
	 * Параметры:
	 * avhrrDataBuffers     Массив указателей на буфера размером totalPix*2 байт под
	 *                      распакованные данные AVHRR. Любой из его элементов
	 *                      может быть равен NULL.
	 * tlmBuffers			Массив указателей на буфера размером 60 байт под
	 *                      данные телеметрии 3,4,5 каналов. Любой из его
	 *                      элементов может быть равен NULL.
	 * tipBuffer			Указатель на массив размером 1010 байт (505 слов, или 5 кадров по 101 слову) под данные TIP.
	 *						Модет быть равен NULL.
	 *
	 * Исключения:
	 * TAccessExc( 1, "THRPTUnpacker::unpackNextFrame: ошибка доступа к файлу" )
	 * TRequestExc( 2, "THRPTUnpacker::unpackNextFrame: кадр со сбоем синхронизации" )
	 */
	void unpackNextFrame( uint16_t * avhrrBuffers[5], uint16_t * tlmBuffers[3] = NULL, uint16_t * tipBuffer = NULL ) throw ( TAccessExc, TRequestExc );

	/*
	 * unpackNextFrame
	 *
	 * Функция распаковывает весь малый кадр AVHRR, не выделяя из него данные
	 * отдельных полей.
	 * Прототип:
	 * void unpackNextFrame( USHORT* buffer ) throw( TException )
	 * Параметры:
	 * buffer	Указатель на буфер под распаковываемые данные. Размер буфера
	 *			можно узнать при помощи функции frameWordLength().
	 *
	 * Исключения:
	 * TAccessExc( 1, "THRPTUnpacker::unpackNextFrame: ошибка доступа к файлу" )
	 */
	void unpackNextFrame( uint16_t * buffer ) throw( TAccessExc );

private:
	void init( char * buf );

	/*
	 * Прототип:
	 * unsigned buildMask ( uchar * mask, unsigned code, unsigned nPassPix, unsigned nPix )
	 * Параметры:
	 *      mask   Байтовый массив для маски. Если равен 0, построения маски
	 *             не происходит.
	 *      code    битовый код построения маски;
	 *      биты:   0 - IDN -идентификатор борта
	 *          1 - TCD -код времени
	 *          2 - TLM -телеметрия
	 *          3 - BSC -сканирование черного тела
	 *          4 - SPC -сканирование космоса (SPACE)
	 *          5 - HSY -синхросерия AVHRR
	 *          6 - TIP -информ. пакет TIROS (без синхросерий)
	 *          7 - SPR -резерв
	 *          8 - AVHRR
	 *          9 - ASY -синхросерия конца кадра
	 *             10 - FSY -синхросерия начала кадра
	 *             11 - канал 5 AVHRR
	 *             12 - канал 4 AVHRR
	 *             13 - канал 3 AVHRR
	 *             14 - канал 2 AVHRR
	 *             15 - канал 1 AVHRR
	 *      nPassPix    число пропускаемых пикселов от начала строки AVHRR
	 *      nPix        кол-во принимаемых пикселов строки AVHRR
	 *  Возвращает: кол-во записываемых слов из кадра HRPT
	 *
	 *  Примечание: значение 1 конкретного бита кода маски означает запись
	 *  полностью соответствующего сегмента из кадра HRPT, кроме TIP-а.
	 */
	unsigned buildMask( uint8_t *, unsigned, unsigned, unsigned );

	FILE *  fFile;
	
	/*
	 * Формат файла.
	 * 1 - файл с 0-блоком нового формата,
	 * 2 - файл с 0-блоком старого формата.
	 */
	int     fFileFormat;    

	/* Число малых кадров в файле данных. */
	uint32_t   fTotalFrames;

	/* Число каналов AVHRR, присутствующих в файле. */
	uint32_t   fNChannels;
	/* Флаги наличия каналов. */
	int     fChannels[5];
	/* Длина в словах распакованного кадра. */
	uint32_t   fUnpackedFrameLength;
	/* Длина в словах нераспакованного кадра. */
	uint32_t   fPackedFrameLength;
	/* Смещение данных AVHRR в кадре, выраженное в словах. */
	uint32_t   fAVHRROffset;
	/* Смещение данных телеметрии в кадре, выраженное в словах. */
	uint32_t   fTLMOffset;
	/* Смещение данных TIP в кадре, выраженное в словах. */
	uint32_t   fTIPOffset;
	uint32_t   fMask;
	uint32_t   fTotalPix;
	uint32_t   fPixPassed;
	uint32_t   fPix;
	uint32_t   fPackType;

	char *  fBuf;
	TBlk0 * fBlk0;
	uint16_t * fData;
	uint16_t * fUnpackedFrame;

	uint16_t * fCurPackedFrame;
	/* Номер кадра, который будет получен следующим вызовом функции unpackNextFrame. */
	unsigned fCurFrameNum;

	/*
	 * 0 - буфер под данные заведены самим объектом;
	 * 1 - буфер с данными файла получен извне
	 */
	int     fExtBufFlag;   
};

/*---------------------------------------------------------------------
	build_mask_hrpt
	Функция, аналогичная старой функции build_mask. Предназначена для
	вычисления длины кадра hrpt по маске hrpt.

	Кроме этого, функция вычисляет массив смещений для каждого поля hrpt
	в зависимости от заданной маски. Массив смещений задается как
	переменная hprt_disp[18] типа int. Каждый элемент
	этого массива задает смещение соответствующего маске поля в кадре
	входного файла ( смещение >= 0 ). Если поле отсутствует в кадре,
	элемент массива, соответствующий этому полю, имеет значение -1.
	Все поля в кадре обрабатываются по порядку. Порядок следования
	полей задан во второй части файла hrpt.h. Первым полем в кадре
	считается поле IDN, а последним - поле FSY ( это не совсем
	соответствует константам маски, описанным в первой части файла hrpt.h ).
	Для единоообразной обработки всех полей кадра hrpt введены локальные
	статические массивы, определенные через константы кадра hrpt.

	Также функция вычисляет количество каналов AVHRR во входном файле.
	Это значение необходимо для выделения отдельных каналов из битового
	потока, в котором все имеющиеся каналы чередуются ( т.н. interleaving ).
	Значение передается в функцию main через переменную channel_c.

---------------------------------------------------------------------*/
unsigned short build_mask_hrpt ( uint32_t mask, int* hrpt_disp, uint16_t* channel_c );

/*
 * Самый хитрый метод. Распаковывает 8 10 битовых значений упакованных в 10 байт.
 * Creation date: (20.11.00 16:32:37)
 *
 * @param src byte[]
 * @param dst short[]
 * @nWords  Сколько слов распаковывать. Должно быть кратно 8.
 */
void unpack810( void* in, void* out, uint32_t nWords );

#endif
