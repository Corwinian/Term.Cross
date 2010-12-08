/*---------------------------------------------------------------------
	Структура формата 0-блока исходных данных HRPT/CHRPT/GMS.
	Date: 2 march 2004
---------------------------------------------------------------------*/
#ifndef _F34_UNI_H_
#define _F34_UNI_H_

#include <c_lib/c_types.hpp>
#pragma pack(1)

struct TBlk0_uni {
/*---------------------------------------------------------------------
	Постоянная часть формата
---------------------------------------------------------------------*/
	uint8_t formatType;	/* тип формата (FFh)		0 */
	char satName[13];		/* ASCII имя ИСЗ		1 */
	uint32_t satId;		/* индекс ИСЗ			14 */
	uint32_t revNum;		/* номер витка (HRPT)
					для GMS не используется (0)	18 */

	/* время приема UTC ( из первого кадра файла ) */

	uint16_t s_year;		/* год ( полный )		22 */
	uint16_t s_day;		/* день года (1-based)		24 */
	uint32_t s_time;		/* время ( мсек от начала дня )	26 */

	/* время приема UTC ( из внешнего источника ) */
	/* для GMS пока не используется */

	uint16_t o_year;		/* год ( полный )		30 */
	uint16_t o_day;		/* день года (1-based)		32 */
	uint32_t o_time;		/* время ( мсек от начала дня )	34 */

   	uint8_t reserved1[23];	/* ==== резерв ====		37 */

	uint8_t sourceType;	/* источник данных, т.е. индекс
								приемной станции:
		0	старая приемная станция	( для совместимости );
		1	станция приема Time Step;
		2	станция приема UKW Technik ( полярно-орбитальная );
		*/
    uint8_t dataType1;	/* 62   Тип данных, часть 1 */
    uint8_t dataType2;	/* 63   Тип данных, часть 2 */
		/* Тип данных имеет следующие значения:
		dataType1:				dataType2:
		1	исходные данные		1	HRPT NOAA
		2	одноканальные данные	2	HRPT SeaStar
						3	CHRPT FENGYUN
						4...15	резерв
						16	GMS LRFAX
						17	GMS S-VISSR
						18...	резерв
		3	проекции		бит 0 - тип
							0 - меркаторская
							1 - равнопромежуточная
						бит 1 - калибровка
							0 - не калибрована
		4...	резерв
		*/

/*---------------------------------------------------------------------
	Переменная часть формата
---------------------------------------------------------------------*/
	uint16_t frame_c;		/* количество кадров без сбоя
					синхронизации, в том числе
					кадров, которые полностью
					удалось	восстановить		64 */
	uint16_t frame_lost;	/* количество потерянных
					кадров,	в том числе кадров со
					сбоем синхронизации, которые
					не удалось восстановить		66 */
	uint16_t frame_uni1;	/* для HRPT - количество кадров без
					сбоев по полю времени в кадре
					для GMS -  количество кадров, которые
					полностью удалось восстановить	68 */
	uint16_t frame_uni2;	/* для HRPT - количество кадров со
					сбоем по полю времени в кадре и
					откорректированных 
					для GMS - количество кадров со сбоем,
					в которых потеряна часть
					секторов			70 */
	uint16_t frame_gaps;	/* количество пропусков (gaps)	72 */

/* Примечания:
	Для HRPT
	1. Сумма frame_c ( 64 ) + frame_lost ( 66 ) дает общее количество
		кадров в файле hrpt нового формата;
	2. Сумма frame_uni1 ( 68 ) + frame_uni2 ( 70 ) равна
		количеству кадров без сбоя синхронизации frame_c ( 64 );
	3. Поле frame_gaps ( 72 ) дает количество дыр, по которым распределено
		количество кадров со сбоем синхронизации frame_lost ( 66 );
	4. Поля frame_uni1 ( 68 ), frame_uni2 ( 70 ) и
		frame_lost ( 66 ) составляют именно в таком порядке
		массив из 3 целых чисел, обозначаемый в формате 1b
		как качество данных ( DACS Quality );
	5. Для корректного преобразования форматов из старого в новый и
		обратно аналогичные поля добавлены в старый формат:
		Старый				Новый
		totalScans ( numberOfFrames )	frame_c + frame_lost
		frame_lost ( 476 )		frame_lost ( 66 )
		frame_corrected ( 478 )		frame_uni2 ( 70 )
		frame_gaps ( 480 )		frame_gaps ( 72 )
	6. Поля totalScans и numberOfFrames для архивных файлов старого
		формата, как правило, заполнены. Остальные поля содержат 0.
		Поле frame_corrected фиксируется программой CHECK_HT.EXE.
		Она же корректирует поля totalScans и numberOfFrames,
		если обнаружены сбои по времени в конце файла. Программа
		устанавливает поле new_flag и не работает, если оно равно 1.
		Программа H_CONC.EXE склеивает разорванные в результате потери
		синхронизации файлы в один и соответствующим образом
		корректирует поля totalScans, frame_lost и frame_gaps.
		Программа OLD2NEW.EXE переписывает в новый формат текущие
		значения полей frame_lost, frame_corrected и frame_gaps,
		а значения оставшихся двух полей вычисляет как
			frame_c = totalScans - frame_lost;
			frame_uni1 = frame_c - frame_uni2;
		При обратном преобразовании NEW2OLD.EXE переписывает в
		старый формат текущие значения полей frame_lost,
		frame_uni2 и frame_gaps, а оставшиеся поля
		вычисляет как
			totalScans = numberOfFrames = frame_c + frame_lost;
		Кроме того, она устанавливает поле new_flag для защиты
		от повторной корректировки времени.
	Для GMS
	1. Сумма frame_c ( 64 ) + frame_lost ( 66 ) + frame_uni2 ( 70 )
		дает общее количество кадров в файле gms данного формата;
	2. Сумма frame_lost ( 66 ) + frame_uni2 ( 70 ) равна общему
		количеству сбойных кадров;
	3. Поле frame_gaps ( 72 ) дает количество дыр, по которым распределено
		общее количество сбойных кадров frame_lost ( 66 ) +
		frame_uni2 ( 70 );
	4. Пока не понятно как интерпретировать эти величины в формате 1b,
		как качество данных ( DACS Quality );
*/

/*---------------------------------------------------------------------
	Описание кадра
---------------------------------------------------------------------*/
	uint16_t pack_type;	/* тип упаковки			74 */
		/*
			0	отсутствие упаковки	(GMS,HRPT)
			1	полуплотная		(HRPT)
			2	плотная			(HRPT)
		*/
	uint16_t frame_length;	/* для GMS - длина строки IR
					для HRPT - длина строки AVHRR	76 */
	uint32_t frame_mask;	/* маска сегментов кадра
					для HRPT по ней вычисляется реальный
					размер кадра HRPT
					для GMS имеет информационный смысл 78 */
	uint16_t pix_gap;		/* количество пропущенных пикселов
					от начала строки (IR,AVHRR)	82 */
	uint16_t pix_len;		/* количество принятых пикселов
					строки (IR,AVHRR)		84 */
	uint16_t uni3;		/* Для HRPT Тип витка:
						1 - восходящий,
						0 - нисходящий
					для GMS - длина кадра GMS	86 */

	/* следующие поля используются только для GMS */
	uint16_t vis_length;	/* длина строки VIS		88 */
	uint16_t vis_pix_gap;	/* количество пропущенных пикселов
					от начала строки VIS		90 */
	uint16_t vis_pix_len;	/* количество принятых пикселов
					строки VIS			92 */
	uint16_t first_frame_number;	/* номер первой строки VISSR в файле
									( из первой же строки ! )		94 */
	uint8_t unused2[32];	/* ==== резерв ====		96 */

/*---------------------------------------------------------------------
	Блок орбитальных данных NORAD
---------------------------------------------------------------------*/
	uint32_t ref_num;	/* номер витка				128 */
	uint16_t set_num;	/* номер набора элементов		132 */
	uint16_t ephem;	/* тип эфемерид				134 */
	uint16_t ep_year;	/* год ( полный )			136 */
	double ep_day;		/* день года ( 1-based )		138 */
	double n0;		/* среднее движение ( рад/мин )		146 */
	double bstar;		/* BSTAR Drag term			154 */
	double i0;		/* наклонение орбиты ( рад )		162 */
	double raan;		/* прямое восхождение восходящего узла ( рад ) 170 */
	double e0;		/* эксцентриситет орбиты		178 */
	double w0;		/* аргумент перигея ( рад )		186 */
	double m0;		/* средняя аномалия ( рад )		194 */
	uint8_t unused3[54];	/* ==== резерв ====		202 */

/*---------------------------------------------------------------------
	Блок коррекции
	Используется пока только для HRPT
---------------------------------------------------------------------*/
	uint16_t c_ver;	/* Номер версии коррекции		256 */
	int16_t s_delt;		/* Поправка бортовых часов (мсек) -
					- по TBUS			258 */
	int16_t delt;		/* Поправка времени ( мсек )		260 */
				/* Коррекция ориентации сканера (рад) */
	double roll;		/* крен					262 */
	double pitch;		/* тангаж				270 */
	double yaw;		/* рысканье				278 */

	uint8_t unused4[226];	/* ==== резерв ====		286 */
};

#pragma pack()

#endif