/*---------------------------------------------------------------------
	A02LEV0.C
	Программа конвертации файла hrpt формата f34_uni ( a0 )
	в файл raw data типа level 0 ( l0b ).

	Версия 1.1
	1. Восстановлены синхросерии FSY, SPR, ASY и TIP, оставлена
	нулевой синхросерия HSY ( 1 слово ).
	2. Строки, являющиеся gap'ами, ОСТАВЛЕНЫ в файле. Они содержат
	в кадре все 0.
	Date: 7 february 2006
---------------------------------------------------------------------*/
#include <tc_config.h>
#include <c_lib/c_types.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "f34_uni.h"
#include "hrpt.h"

/* прототипы функций */
extern void close_all ( void );
extern uint16_t build_mask_hrpt ( uint32_t );
extern void unpack810 ( void*, void*, uint32_t );
extern int32_t splitpath(char*, char*, char*, char*, char* );
extern int32_t makepath( char*, const char*, const char*, const char*, const char* );
extern int32_t search_phase ( void );

/* файловые переменные */
FILE* inp_handle = NULL;
FILE* out_handle = NULL;
char out_file_name[MAX_DIR];
char inp_file_name[MAX_FNAME];
char drive[MAX_DRIVE];
char dir[MAX_DIR];
char ext[MAX_EXT];
char out_ext[MAX_EXT] = ".lev0";
char* inp_file;
uint32_t inp_file_len;

/* переменные файла hrpt */
struct TBlk0_uni b;
uint16_t frame_c;		/* общее количество кадров входного файла */
uint16_t lframe;		/* длина кадра входного файла в 10-битовых словах */
uint16_t nword;		/* длина упакованного кадра в 16-битовых словах */
uint16_t nword2;		/* длина упакованного кадра в байтах */
int hrpt_disp[15];			/* массив смещений полей в кадре hrpt
							( подробнее см. описание функции build_mask_hrpt ) */
uint16_t channel_c;	/* количество каналов во входном файле */
uint16_t pack_type;

/* буфера ввода/вывода */
uint8_t* io_buf = NULL;
uint8_t* p;
uint16_t* frame_buf = NULL;
uint16_t* s;
uint16_t* u;
uint16_t* data_buf = NULL;
uint16_t* d;

int leave_gap = 0;
uint16_t gap_flag;
uint16_t swap_word;

uint16_t fsy_data[6] = { 0x284, 0x16f, 0x35c, 0x19d, 0x20f, 0x095 };
uint16_t spr_data[127] = { 0x109, 0x1ee, 0x3aa, 0x0ee, 0x0b0, 0x2fb, 0x07d,
	0x124, 0x087, 0x08e, 0x198, 0x036, 0x1b1, 0x1c7, 0x25b, 0x304, 0x273,
	0x2d3, 0x1aa, 0x2fd, 0x0a9, 0x18d, 0x2e8, 0x277, 0x24b, 0x164, 0x148,
	0x007, 0x310, 0x08b, 0x126, 0x0cb, 0x0e9, 0x042, 0x261, 0x0ff, 0x2f6,
	0x1f3, 0x044, 0x2b5, 0x056, 0x099, 0x30a, 0x397, 0x3e7, 0x0af, 0x159,
	0x241, 0x018, 0x0b2, 0x2b7, 0x01a, 0x0fe, 0x2d0, 0x1c0, 0x2a9, 0x39e,
	0x2f1, 0x101, 0x0de, 0x237, 0x384, 0x1ed, 0x3c0, 0x0ba, 0x387, 0x187,
	0x394, 0x38d, 0x0fb, 0x26e, 0x13d, 0x3f1, 0x21b, 0x2cb, 0x2fa, 0x05b,
	0x117, 0x26a, 0x1a5, 0x33f, 0x1ae, 0x265, 0x067, 0x238, 0x246, 0x0ea,
	0x028, 0x235, 0x3c8, 0x18a, 0x21a, 0x2ed, 0x2c9, 0x2b6, 0x03c, 0x0cd,
	0x03d, 0x0eb, 0x00e, 0x206, 0x125, 0x0a1, 0x0bd, 0x375, 0x11d, 0x316,
	0x05f, 0x18f, 0x2a4, 0x210, 0x391, 0x333, 0x006, 0x336, 0x0b8, 0x3cb,
	0x1e0, 0x24e, 0x1da, 0x1b5, 0x15f, 0x295, 0x0b1, 0x2dd, 0x04e, 0x3c9 };

/* последнее слово в этой синхросерии заменено с 0x3cc на 0x3c0
по фактическому содержимому файлов данных */
uint16_t aux_data[100] = { 0x3e2, 0x3f3, 0x1b5, 0x2bd, 0x284,
	0x115, 0x1c4, 0x1d3, 0x341, 0x0e0, 0x2b6, 0x3de, 0x0dc, 0x199, 0x3f2,
	0x193, 0x28e, 0x069, 0x03e, 0x363, 0x04b, 0x095, 0x140, 0x2d5, 0x29c,
	0x245, 0x362, 0x06d, 0x0a6, 0x3ad, 0x3fe, 0x03b, 0x3dd, 0x0b6, 0x1cd,
	0x0c5, 0x2ef, 0x167, 0x2c0, 0x042, 0x183, 0x0ee, 0x352, 0x2ea, 0x1d9,
	0x23d, 0x11a, 0x006, 0x0d4, 0x0a9, 0x26f, 0x2f9, 0x3d3, 0x152, 0x0f9,
	0x1c0, 0x14b, 0x38f, 0x355, 0x218, 0x143, 0x2bf, 0x2c8, 0x172, 0x01e,
	0x384, 0x20f, 0x3d1, 0x11e, 0x09e, 0x01a, 0x31c, 0x2c1, 0x064, 0x1b0,
	0x203, 0x279, 0x04d, 0x041, 0x1e9, 0x0ba, 0x065, 0x196, 0x230, 0x094,
	0x166, 0x2e6, 0x071, 0x36e, 0x1c5, 0x1f5, 0x372, 0x20d, 0x39d, 0x179,
	0x144, 0x24d, 0x252, 0x1f0, 0x3c0 };

uint16_t phase;
uint16_t sca[6] = { 9, 5, 7, 3, 11, 13 };
uint32_t sidx[6] = { 21263, 23455, 25338, 26536, 27453, 28654 };
/* синхросерии TIP */
uint16_t sync_12[3] = { 0x3b4, 0x388, 0x25 };
uint16_t sync_14[3] = { 0x3b4, 0x388, 0x15 };
uint16_t sync_15[3] = { 0x3b4, 0x388, 0x23 };
uint16_t sync_16[3] = { 0x3b4, 0x388, 0x0d };
uint16_t sync_17[3] = { 0x3b4, 0x388, 0x2f };
uint16_t sync_18[3] = { 0x3b4, 0x388, 0x37 };
uint16_t tip_2[15] = { 0x1ad, 0x20f, 0x33, 0x386, 0x1a1, 0x3a7,
	0x0f4, 0x3ac, 0x03a, 0x207, 0x103, 0x092, 0x192, 0x14a, 0x04b };
uint16_t tip_3[3] = { 0x3cc, 0x1a9, 0x001 };

void printSyntax () {
//syntax:
	printf ( "Syntax: a02l0b.exe [-l] <hrpt_a0_input_file>\n" );
	printf ( "-l\tleave gap's.\n");
	printf ( "hrpt_a0_input_file\n\tinput HRPT file of f34 format;\n" );
	printf ( "\tThe name of output l0b file is generated automatically\n" );
	printf ( "\tfrom the name of input file.\n" );
}

void closeandexit(int code){
		close_all();
		exit(code);
}

int main ( int argc, char* argv[] ) {
	int i;
	int j;
	int k;

	if ( argc < 2 ) {
		printSyntax();
		return 1;
	}

	if ( !strcmp( argv[1],"-l" ) ){
		if ( argc < 3 ) {
			printSyntax();
			return 1;
		}
		leave_gap = 1;
		inp_file = argv[2];
	} else {
		inp_file = argv[1];
	}

	/* открываем файл */
	inp_handle = fopen ( inp_file, "rb" );
	if ( inp_handle == NULL ) {
		fprintf ( stderr, "Can't open input file: %s\n", inp_file );
		closeandexit(2);
	}

	/* длина файла */
	fseek ( inp_handle, 0, SEEK_END );
	inp_file_len = ftell ( inp_handle );
	fseek ( inp_handle, 0, SEEK_SET );

	/* если длина файла меньше 512 байт, это не файл данных, выход */
	if ( inp_file_len < sizeof( struct TBlk0_uni )) {
		fprintf ( stderr, "Input file is too small\n" );
		closeandexit(3);
	}

	/* читаем 0-блок */
	fread ( &b, 1, sizeof( struct TBlk0_uni ), inp_handle );

	/* проверяем формат файла ( f34 ) */
	if ( b.formatType != 0xff || b.dataType1 != 1 || b.dataType2 != 1 ) {
//iff:
		fprintf ( stderr, "Unsupported file format.\n" );
		closeandexit(3);
	}
	/* тип данных */
	//if ( b.dataType1 != 1 ) goto iff;
	//if ( b.dataType2 != 1 ) goto iff;

	/* проверяем первые 4 символа названия спутника;
	в данной версии программы распознаются только спутники NOAA */
#if defined STRICMP
	if ( strnicmp ( b.satName, "noaa", 4 ) != 0 ) {
#elif defined STRCASECMP
	if ( strncasecmp ( b.satName, "noaa", 4 ) != 0 ) {
#else
	if ( 0 ) {
#error "see tc_config.h"
#endif
		fprintf ( stderr, "Satellite is not supported\n" );
		closeandexit(3);
	}

	/* вычисляем переменные кадра входного файла */
	frame_c = b.frame_c + b.frame_lost;
	pack_type = b.pack_type;
	/* проверяем маску кадра */
	if ( b.frame_mask != 0xfd5f ) {
		fprintf ( stderr, "Frame mask ( %d ) does not fit to level 0\n", (unsigned)b.frame_mask );
		closeandexit(3);
	}
	/* получаем распакованную длину кадра входного файла, а также
	устанавливаем массив смещений для всех полей кадра согласно маске */
	lframe = build_mask_hrpt ( b.frame_mask );
	if ( pack_type == 0 ) nword = lframe;
	else if ( pack_type == 2 ) {
		nword = ( lframe * 5 ) / 8;
		if (( lframe * 5 ) % 8 ) nword++;
		nword++;
	}
	else {
		/* неправильный тип упаковки */
		fprintf ( stderr, "Unsupported pack type: %d\n", b.pack_type );
		closeandexit(3);
	}
	/* проверяем размер файла */
	nword2 = nword * 2;
	if ((uint32_t)nword2 * frame_c + sizeof( struct TBlk0_uni ) != inp_file_len ) closeandexit(3);
	/* проверяем количество каналов AVHRR */
	if ( channel_c == 0 ) closeandexit(3);

	/* теперь мы считаем, что текущий файл является файлом данных
	формата f34 */

	/* формируем имя выходного файла */
	splitpath ( inp_file, drive, dir, inp_file_name, ext );
	//memcpy ( out_file_name, inp_file_name, 6 );
	//strcpy ( out_file_name + 6, out_ext );
	makepath ( out_file_name, drive, dir, inp_file_name, out_ext );
	printf( "%s \n",out_file_name);

	/* выделяем буфера ввода/вывода */
	/* буфер для чтения одного упакованного кадра из входного файла */
	io_buf = (uint8_t*) malloc ( nword2 );
	/* буфер для формирования распакованного кадра */
	frame_buf = (uint16_t*) malloc ( lframe * 2 );
	/* выходной буфер полного малого кадра hrpt */
	data_buf = (uint16_t*) malloc ( LHRPT * 2 );

	/* выходной файл */
	out_handle = fopen ( out_file_name, "wb+" );
	if ( out_handle == NULL ) {
		printf ( "Error to open output file\n" );
		close_all();
		return 5;
	}

	/* ищем относительный номер кадра с фазой 1 */
	k = search_phase();
//	printf ( "search_phase: %d\n", k );
	/* вычисляем фазу для первого кадра */
	phase = 1;
	for ( i = 0; i < k; i++ ) {
		phase = ( phase == 1 )? 3: phase - 1;
	}
//	printf ( "first frame phase: %u\n", phase );

	/* получаем локальный индекс спутника */
	for ( k = 0; k < 6; k++ ) {
		if ( b.satId == sidx[k] ) break;
	}

	/* основной цикл чтения входного файла */
	for ( i = 0; i < frame_c; i++ ) {
		/* читаем очередной кадр */
		fread ( io_buf, 1, nword2, inp_handle );
		/* распаковываем очередной кадр */
		if ( pack_type == 0 ) memcpy ( frame_buf, io_buf, nword2 );
		else unpack810 ( io_buf, frame_buf, (uint32_t)lframe );

		/* проверка gap */
		if ( frame_buf[0] == 0 && frame_buf[1] == 0 && frame_buf[2] == 0 &&
		frame_buf[3] == 0 && frame_buf[4] == 0 && frame_buf[5] == 0 ) gap_flag = 1;
		else gap_flag = 0;

		/* чтобы вырезать gap'ы, надо откомментировать эту строку */
		if ( leave_gap && gap_flag ) {
			/* вычисляем новую фазу */
			phase = ( phase == 3 )? 1: phase + 1;
			continue;
		}

		/* формируем исходный кадр hrpt из распакованного */
		d = data_buf;
		s = frame_buf;
		memset ( d, 0, LHRPT * 2 );

//		/* распечатываем содержимое полей IDN */
//		printf ( "i: %d Sync: %1d Num: %1d ph: %1d Sc: %2d Stable: %1d\n", i,
//			(s[0] >> 9) & 1, (s[0] >> 7) & 3, phase, (s[0] >> 3) & 15, (s[0] >> 2) & 1 );

		/* исходная синхросерия FSY */
		memcpy ( d, fsy_data, LFSY * 2 );
		/* модифицируем поле IDN: вбиваем туда текущую фазу и spacecraft
		address */
		s[0] &= 0x207;	/* стираем текущие значения */
		s[0] |= ( sca[k] << 3 );		/* sca */
		s[0] |= ( phase << 7 );			/* phase */

		/* записываем IDN, TCD, TLM, BSC, SPC */
		memcpy ( d + IDN, s, ( HSY - IDN ) * 2 );
		s += HSY - IDN;
		/* синхросерию HSY оставляем 0 */
		s += 0;

		/* записываем TIP, отсутствующие синхросерии TIP оставляем нулями */
		for ( j = 0; j < NTIP; j++ ) {
			memcpy ( d + TIP+3 + (LTIP/NTIP) * j, s, LMTIP * 2 );
			s += LMTIP;
		}
		/* вписываем синхросерии TIP */
		if ( k < 2 ) {	/* noaa 12, noaa 14 */
			for ( j = 0; j < NTIP; j++ ) {
				if ( k == 0 ) memcpy ( d + TIP + (LTIP/NTIP) * j, sync_12, 6 );
				else if ( k == 1 ) memcpy ( d + TIP + (LTIP/NTIP) * j, sync_14, 6 );
			}
		}
		else {	/* noaa 15, noaa 16, noaa 17, noaa 18 */
			if ( phase == 1 ) {
				for ( j = 0; j < NTIP; j++ ) {
					if ( k == 2 ) memcpy ( d + TIP + (LTIP/NTIP) * j, sync_15, 6 );
					else if ( k == 3 ) memcpy ( d + TIP + (LTIP/NTIP) * j, sync_16, 6 );
					else if ( k == 4 ) memcpy ( d + TIP + (LTIP/NTIP) * j, sync_17, 6 );
					else if ( k == 5 ) memcpy ( d + TIP + (LTIP/NTIP) * j, sync_18, 6 );
				}
			}
			else if ( phase == 2 ) {
				u = tip_2;
				for ( j = 0; j < NTIP; j++ ) {
					memcpy ( d + TIP + (LTIP/NTIP) * j, u, 6 );
					u += 3;
				}
			}
			else if ( phase == 3 ) {
				for ( j = 0; j < NTIP; j++ )
					memcpy ( d + TIP + (LTIP/NTIP) * j, tip_3, 6 );
			}
		}

		/* записываем последовательность SPR */
		memcpy ( d + SPR, spr_data, LSPR * 2 );
		/* записываем AVHRR */
		memcpy ( d + AVH, s, LAVH * 2 );
		/* записываем последовательность ASY */
		memcpy ( d + ASY, aux_data, LASY * 2 );
		/* меняем порядок байт во всех словах */
		for ( j = 0; j < LHRPT; j++ ) {
			swap_word = ( data_buf[j] << 8 ) + ( data_buf[j] >> 8 );
			data_buf[j] = swap_word;
		}

		/* записываем выходные данные HRPT */
		fwrite ( data_buf, 2, LHRPT, out_handle );

		/* вычисляем новую фазу */
		phase = ( phase == 3 )? 1: phase + 1;
	}

	/* заключительные операции */
	close_all();
	return 0;
}

/*---------------------------------------------------------------------
	CLOSE_ALL.C
---------------------------------------------------------------------*/
void close_all ()
{
	if ( inp_handle ) {
		fclose ( inp_handle );
		inp_handle = 0;
	}
	if ( out_handle ) {
		fclose ( out_handle );
		out_handle = 0;
	}
	if ( io_buf ) {
		free ( io_buf );
		io_buf = NULL;
	}
	if ( frame_buf ) {
		free ( frame_buf );
		frame_buf = NULL;
	}
	if ( data_buf ) {
		free ( data_buf );
		data_buf = NULL;
	}
}

/*---------------------------------------------------------------------
	BUILD_MASK_HRPT.C
	Функция, аналогичная старой функции build_mask. Предназначена для
	вычисления длины кадра hrpt по маске hrpt.

	Кроме этого, функция вычисляет массив смещений для каждого поля hrpt
	в зависимости от заданной маски. Массив смещений задается как
	глобальная переменная hprt_disp[18] типа int. Каждый элемент
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
	Значение передается в функцию main через глобальную переменную channel_c.

	Используемые глобальные переменные: hrpt_disp[]
                                        channel_c
---------------------------------------------------------------------*/
uint16_t build_mask_hrpt ( uint32_t mask )
{
	int i = 0;
	int j = 0;
	static uint16_t m_len[15] = { LIDN, LTCD, LTLM, LBSC,
		LSPC, LHSY, LTIP-LTNO, LSPR, LAVHRR, LAVHRR, LAVHRR, LAVHRR,
		LAVHRR, LASY, LFSY };
	static uint32_t def_masks[15] = { MIDN, MTCD, MTLM, MBSC,
		MSPC, MHSY, MTIP, MSPR, M1, M2, M3, M4, M5, MASY, MFSY };
	uint16_t d;

	/* проходим по всем полям кадра, отмечаем отсутствующие поля,
	вычисляем смещения */
	d = 0;	/* начальное смещение для первого поля равно 0 */
	for ( i = 0; i < 15; i++ ) {
		/* если поле присутствует */
		if ( mask & def_masks[i] ) {
			/* записываем в поле накопленное значение смещения */
			hrpt_disp[i] = (int)d;
			/* и увеличиваем смещение для следующего поля */
			d += m_len[i];
		}
		else {	/* если текущее поле отсутствует, просто отмечаем поле
				 как отсутствующее; текущее накопленное смещение не меняется */
			hrpt_disp[i] = - 1;
		}
	}

	/* находим смещение первого присутствующего в файле канала AVHRR;
	отметим, что каналы AVHRR занимают в массиве hrpt_disp по порядку
	элементы с 8 по 12 включительно; это следует из определения
	статического массива def_masks */
	for ( i = 0; i < 5; i++ ) {
		if ( hrpt_disp[i+8] != - 1 ) {
			j = hrpt_disp[i+8];
			break;
		}
	}
	/* корректируем	смещения каналов AVHRR с учетом interleaving'а;
	заодно считаем количество каналов AVHRR */
	channel_c = 0;
	for ( i = 0; i < 5; i++ ) {
		if ( hrpt_disp[i+8] != - 1 ) {
			hrpt_disp[i+8] = j;
			j++;	/* смещения всех каналов отличаются на 1 слово */
			channel_c++;
		}
	}

	/* минимальную длину кадра в этом варианте программы вычислять не нужно;
	она уже вычислена в переменной d после окончания цикла */
	/* увеличиваем ее до числа, кратного 8 */
	while ( d & 7 ) d++;
	return d;
}

/*---------------------------------------------------------------------
	SEARCH_PHASE.C
	Функция аналогична известной функции search_base_line, входящей
	в состав многих конверторов и предназначенной для поиска базовой
	строки, начиная с которой наблюдается правильная последовательность
	временных меток кадра hrpt.
	Данная функция работает на уже конвертированных файлах, в которых
	восстановлена правильная временная последовательность, и предназначена
	для поиска правильной последовательности номеров кадра в поле
	10-битового идентификатора ( биты 7-8 ). Эти биты должны содержать
	повторяющуюся последовательность 1,2,3,1,2,3,1...
	Данная функция читает кадры исходного файла с текущего места,
	вынимает из кадра идентификатор ( это первое слово кадра ) и
	определяет, начиная с какого кадра ( от текущего ) имеет место появление
	правильной последовательности длиной 6: 1,2,3,1,2,3.
	Функция возвращает смещение ( в кадрах ) относительно текущего
	кадра для номера найденного кадра.
	Функция производит обратное позиционирование на исходную позицию
	в файле, так что вызывающая программа может спокойно продолжать
	чтение файла со своего места.
	Прототип: int search_phase ( void );
	Примечание: для чтения и распаковки файла функция использует
	глобальные массивы io_buf, frame_buf и сопутствующие глобальные
	переменные.
	Date: 7 february 2006
---------------------------------------------------------------------*/
int search_phase()
{
	int fc;
	int phase_c;
	int max_phase_c;
	uint16_t cur_phase;

	/* обнуляем счетчик прочитанных кадров */
	fc = 0;
	max_phase_c = 6;
	phase_c = 0;
	cur_phase = 1;

	/* ищем кадр с фазой cur_phase */
	while ( phase_c < max_phase_c ) {
		/* читаем очередной кадр */
		fread ( io_buf, 1, nword2, inp_handle );
		/* распаковываем очередной кадр */
		if ( pack_type == 0 ) memcpy ( frame_buf, io_buf, nword2 );
		else unpack810 ( io_buf, frame_buf, (uint32_t)lframe );
		/* счетчик прочитанных кадров */
		fc++;

		/* проверяем фазу кадра */
		if ((( frame_buf[0] >> 7 ) & 3 ) != cur_phase ) {
			/* сброс переменных */
			cur_phase = 1;
			phase_c = 0;
			continue;
		}

		/* увеличиваем переменные */
		phase_c++;
		cur_phase = ( cur_phase == 3 )? 1: cur_phase + 1;
	}

	/* позиционируемся назад */
	fseek ( inp_handle, (long)( - fc * nword2 ), SEEK_CUR );

	return ( fc - max_phase_c );
}
